/*
 * Ram Verma, ramv
 *
 * cache.h: header file for cache.c
 *
 * The cache will be a linked list
 * with nodes for each id (page).
 *
 */ 
 
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* Make the node and list as structs */
typedef struct cache_node
{
  void *data;
  unsigned int data_len;
  char *id;
  struct cache_node *next;
} cache_node;

typedef struct cache_list
{
  unsigned int read_counter; /* Helps check for exclusion */
  unsigned available_len;
  cache_node *front;
  cache_node *back;
  /* Semaphores, to make sure the cache access doesn't disrupt proxy*/
  sem_t w, r;   /* r is a mutex */  
} cache_list;


/* Function prototypes */

/* Dealing with cache_list */
cache_list *init_cache_list();
cache_node *init_cache_node(char *id, int len);
cache_node *search_cache_list(cache_list *list, char *id);
void terminate_cache_node(cache_node *node);

/* Dealing with individual nodes */
void add_cache_node(cache_list *list, cache_node *node);
cache_node *delete_cache_node(cache_list *list);

/* Dealing with available size in cache */
void add_cache_node_wrapper(cache_list *list, cache_node *node);

/* Dealing with adding and removing nodes based on id */
cache_node *remove_cache_node(cache_list *list, char *id);
int proxy_read_from_cache(cache_list *list, char *id, void *data,
                          unsigned int *len);
int proxy_write_to_cache(cache_list *list, char *id,
                              void *data, unsigned int len);
 
 

 
 

