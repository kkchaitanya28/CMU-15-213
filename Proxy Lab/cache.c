/*
 * Ram Verma, ramv
 *
 * cache.h: A cache implementation 
 * specifically for proxy.c
 *
 * Cache is a double linked list
 * 
 * The nodes are added in a LRU policy,
 * with the front of the list being least used.
 *
 */

#include "cache.h"



/*
 * init_cache_list: initialize a cache list
 * and return the pointer to that list
 *
 */
cache_list *init_cache_list() 
{
  //Wrapper Malloc takes care of error case
  cache_list *list = (cache_list *)Malloc(sizeof(cache_list));

  //Initialize data for the list
  list->front = NULL;
  list->back = NULL;
  list->available_len = MAX_CACHE_SIZE;
  Sem_init(&list->w, 0, 1); /* as given in book, initialize sem to 1 */
  Sem_init(&list->r, 0, 1);
  list->read_counter = 0;
  
  return list;
}



/*
 * init_cache_node: init a cache node
 * and return a pointer to that node
 */
cache_node *init_cache_node(char *id, int len) 
{
  //Allocate memonry for the node and id string
  cache_node *node = (cache_node *)Malloc(sizeof(cache_node));
  node->id = (char *)Malloc(sizeof(char) * (strlen(id) + 1));
  
  //Initialize the data for the node 
  strcpy(node->id, id);
  node->data_len = 0;
  node->data = Malloc(len);
  node->next = NULL;

  return node;
}



/*
 * search_cache_list: search cache list with id as token.
 *
 * Return node if node->id == id, 
 * else return NULL.  
 */
cache_node *search_cache_list(cache_list *list, char *id) 
{
  cache_node *node = list->front;
  while (node != NULL) 
  {
    if (strcmp(node->id, id)==0) 
    {
	  return node;
    }
    node = node->next;
  }
  return NULL;
}



/*
 * terminate_cache_node: Free the cache node 
 * and its contents
 */
void terminate_cache_node(cache_node *node) 
{
  //Use wrapper free to avoid errors
  //Actually I dont think free can have errors
  Free(node->id);
  Free(node->data);
  Free(node);
}



/*
 * add_cache_node: Add a node to cache list 
 */
void add_cache_node(cache_list *list, cache_node *node) 
{
  if (list->front == NULL)  /* Case where nothing in list */
  {
	list->front = list->back = node;
	list->available_len -= node->data_len;
  } 
  else   /* Case where list is non-empty */
  {
	list->back->next = node;
	list->back = node;
	list->available_len -= node->data_len;
  }
}



/*
 * delete_cache_node: Delete a cache node from the
 *                    front of the cache list.
 *
 *                  : Return NULL if list empty,
 *                    else return the node pointer 
 *   
 */
cache_node *delete_cache_node(cache_list *list) 
{
  /* Node at front of list */
  cache_node *node = list->front; 
  if (node == NULL) 
  {
	return NULL;
  }
  
  /* Node at middle of list */
  list->front = node->next;
  list->available_len += node->data_len;
  
  /* Node at back of list */                    
  if (node == list->back) 
  {
	list->back = NULL;
  }
  
  return node;
}



/*
 * add_cache_node_wrapper: Remove nodes from start of list
 *                         till there is enough space for new node.
 *                       : Add new node at back of list
 */
void add_cache_node_wrapper(cache_list *list, cache_node *node) 
{
    P(&(list->w));    /* Surround with P&V function to protect cache */
    while (list->available_len < node->data_len) 
    {
        cache_node *node = delete_cache_node(list);
        terminate_cache_node(node);
    }
    add_cache_node(list, node);
    V(&(list->w));   /* Surround with P&V function to protect cache */
}



/*
 * remove_cache_node: Remove a cache node based on specified
 * id bu checking against the list
 * 
 *                  : Return NULL if cant find it, else return 
 * the pointer to that node   
 */
cache_node *remove_cache_node(cache_list *list, char *id) 
{
  /* Variables for the current and previous nodes */
  cache_node *node_one = NULL;
  cache_node *node_two = list->front;
  
  while (node_two != NULL) 
  {
    /* If id has been found in list, consider case */
    if (strcmp(node_two->id,id)==0) 
	{
	
	  /* Case 1, at front of list */
	  if (list->front == node_two) 
	  {
		list->front = node_two->next;
	  }
	  
      /* Case 2, at back of list */
	  if (list->back == node_two) 
	  {
		list->back = node_one;
	  }
	  
      /* Case 3, in middle of list */ 
	  if (node_one != NULL) 
	  {
	    node_one->next = node_two->next;
	  }
      
	  node_two->next = NULL;
	  list->available_len += node_two->data_len;
	  return node_two;
	}
	
	/* Else case, id not found */
	node_one = node_two;
	node_two = node_two->next;
  }
  
  return NULL;
}



/*
 * proxy_read_from_cache: get content from cache for the
 * proxy, using LRU policy
 *
 * Error signaled on return of -1, 0 on normal return
 * Error in this case means not found in cache
 */
int proxy_read_from_cache(cache_list *list, char *id, void *data,
                          unsigned int *len) 
{
  if (list == NULL) 
  {
	return -1;   /* Error */
  }
  /* Using semaphores for mutual exclusion */
  P(&(list->r));
  list->read_counter++;  /* Raise the counter to check nodes */
  if (list->read_counter == 1) 
  {
	P(&(list->w));
  }
  V(&(list->r));

  cache_node *node = search_cache_list(list, id);

  if (node == NULL) 
  {
	P(&(list->r));
	list->read_counter--;
	if (list->read_counter == 0) 
	{
	  V(&(list->w));
	}
	V(&(list->r));
	return -1;  /* Error */
  }
  /* Else, found node with id */
  *len = node->data_len;
  memcpy(data, node->data, *len);
  P(&(list->r));
  list->read_counter--;
  if (list->read_counter == 0) 
  {
	V(&(list->w));
  }
  V(&(list->r));
  //Using LRU
  P(&(list->w));
  /* Re enter into list since just used */
  node = remove_cache_node(list, id);
  add_cache_node(list, node);
  V(&(list->w));
  return 0;  /* Normal return */
}



/*
 * proxy_write_to_cache: create a cache node, 
 * write to it using the data & id
 *
 * Similar to read, a return of -1 signals an erro
 */
int proxy_write_to_cache(cache_list *list, char *id,
                              void *data, unsigned int len) 
{
  if (list == NULL) 
  {
	return -1; /* Error */
  }

  /* Initialize node */
  cache_node *node = init_cache_node(id, len);

  if (node == NULL) 
  {
	return -1; /* Error */
  }
  
  memcpy(node->data, data, len);
  node->data_len = len;
  add_cache_node_wrapper(list, node);
  return 0;
}