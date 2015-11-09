/*
 * Ram Verma, ramv
 *
 * Multi-thread proxy with cache implemented as a linked list
 * handles GET method
 *
 * Modification in csapp.c:
 * - Exit functions do not exit the process, 
 *   proxy will just exit the thread
 * - Rio_read & Rio_write dont raise error on EPIPE
 *   and ECONNRESET signals
 * 
 * Used source code from CSAPP:
 * http://csapp.cs.cmu.edu/2e/code.html
 *
 * This one is the latest version of proxy with edits
 * to modularize code
 *
 * I have an extension of 2 days due to illness.
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "cache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";

/* Some other strings used as well based on handout */
static const char *http_version_str = "HTTP/1.0\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";

static const char *default_port_str = "80";
static const char *space_str = " ";
static const char *end_str = "\r\n";

/* Global Variables */
cache_list *cache = NULL; /* This is the cache list */


/* Function prototypes */
void thread(void *vargp);
int echo(int client_fd, int *server_fd, char *cache_id, 
          unsigned int *cache_len, char *cache_data);
int parse_uri(char *uri, char *method, char *url, char *http_version,
                       char *protocol,char *host_name, char *suffix,
                       char *request_host, char *request_port);
int write_to_cache(int client_fd, int server_fd, 
                    void *cache_data, char *cache_id); 
int add_data(char *cache_data, unsigned int *cache_len, 
         unsigned int len, char *server_fd_line, int valid);




/* Taken from code from page 953 of textbook */
int main(int argc, char **argv) 
{
  int listenfd, *connfdp;
  char *port;
  int clientlen = 0;
  struct sockaddr_in clientaddr;
  pthread_t tid;
  
  /* ignore SIGPIPE (from hints) */
  Signal(SIGPIPE, SIG_IGN);
    
  /* Wrong number of inputs */
  if (argc != 2) 
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  
  /* This is the port specified on cmd line */
  port = argv[1];
  
  /* Initialize cache */
  cache = init_cache_list();
  
  /* Carry out processes of socket,bind,listen with error handling */
  listenfd = Open_listenfd(port);
  if (listenfd < 0)
  {
    fprintf(stderr ,"Listenfd = %d less than zero\n",listenfd);
    exit(1);
  } 
  while (1) 
  {
    clientlen = sizeof(clientaddr);
    connfdp = (int *)malloc(sizeof(int));
    *connfdp = Accept(listenfd, (SA *) &clientaddr, (socklen_t *)&clientlen);
    Pthread_create(&tid, NULL, (void*)thread, (void*)connfdp);  
  }
  return 0;
}



/* Thread routine: to process http requests & react  
 * to the request by
 * 1) checking cache for hit
 * 2) sending response from cache if hit or from server if no hit to client
 * 3) closing file descriptors and exiting threads
 */
void thread(void *vargp)
{
  int client_fd = *((int *)vargp);
  Pthread_detach(pthread_self());
  
  Free(vargp);
  
  /* Variables used */
  char cache_id[MAXLINE];
  unsigned int cache_len;
  char cache_data[MAX_OBJECT_SIZE];
  int variable = 0;    /* Variable for return value of echo */
  int server_fd= 0;   /* Set file descriptor for server */
  
  variable = echo(client_fd, &server_fd, cache_id, &cache_len, cache_data);

  /* Consider the different cases based on variable value */
  if (variable == -1)      /* Error encountered */
  {
    Close(client_fd);
    Pthread_exit(NULL);
  }
  
  else if (variable == 1)  /* Read from cache   */
  {
    if (Rio_writen(client_fd, (void *)cache_data, (size_t)cache_len) == -1)
    {
      Close(client_fd);
      Pthread_exit(NULL);
    }  
  }
  
  else                     /* Write to cache    */
  {
    if (write_to_cache(client_fd, server_fd, cache_data, cache_id) == -1)
    {
      Close(client_fd);
      Close(server_fd);
      Pthread_exit(NULL);
    }  
  }
  return;
}



/* Echo: send the request to the server,
 * returns: 
 * (-1) on error
 * ( 0) when cache miss (forward to server)
 * ( 1) when cache hit
 */
int echo(int client_fd, int *server_fd, char *cache_id, 
          unsigned int *cache_len, char *cache_data) 
{
  /* Variables used */
  rio_t rio_client_fd;
  /* To send request                */
  char request_host[MAXLINE];   
  char request_port[MAXLINE];  
  /* From file descriptor, to parse */ 
  char uri[MAXBUF];       
  /* To send request concatenate    */     
  char request_line[MAXBUF];   
  /* Outputs form uri to be parsed  */
  char method[MAXLINE];        
  char protocol[MAXLINE];      
  char host_name[MAXLINE];
  char suffix[MAXLINE];
  char http_version[MAXLINE];
  char url[MAXLINE];
  /* If no cache hit, need these originals */
  char uri_first_line[MAXLINE];
  char host_header[MAXLINE];
  /* Valid bits set if header does have any of the 6 components */
  int host_bit = 0;
  /* To fix host port */
  char temp1[MAXLINE];        
  char temp2[MAXLINE];
  char temp3[MAXLINE];
  char *tmp = NULL;
  
  /* Setting strings */
  memset(cache_data, 0, MAX_OBJECT_SIZE);
  
  /* Initialise rio buffer for client file descriptor */
  Rio_readinitb(&rio_client_fd, client_fd);
  Rio_readlineb(&rio_client_fd, (void*)uri, (size_t)MAXBUF);
  
  /* Keep copy of first line from client_fd */
  strcpy(uri_first_line, uri);
  
  /* Parse the buffer from the client file descriptor */
  if (parse_uri(uri, method, url, http_version, protocol, 
                host_name, suffix, request_host, request_port) == -1) 
  {
    return -1;
  }
  
  /* Check if GET method */
  if (strstr(method,"GET") == NULL)
  {
    return -1;      /* non GET method */
  }
  else              /* GET method     */
  {
    /* Need to concatenate the request line */
    strcpy(request_line, method);
    strcat(request_line, space_str);
    strcat(request_line, suffix);
    strcat(request_line, space_str);
    strcat(request_line, http_version_str);
    

    /* Request Headers, iterate through the client_fd */
    while (Rio_readlineb(&rio_client_fd, (void*)uri, (size_t)MAXBUF) != 0)
    {
      /* Check end of file */
      if (strcmp(uri, "\r\n") == 0)
      {
        break;   /* reached end of request by client */
      }
      /* Host header */
      else if (strstr(uri, "Host:") != NULL)
      {
        strcpy(host_header, uri);
        sscanf(uri, "Host: %s", host_header);
        
        /* Same as parse_uri function */
        tmp = index(host_header, ':');
        if (tmp != NULL) 
        {
          *tmp = '\0';       
          strcpy(temp2, tmp + 1);
        } 
        else 
        {
          strcpy(temp2, default_port_str);
        }
        strcpy(temp1, host_header);
 
        sprintf(temp3, "Host: %s:%s\r\n", temp1, temp2);
        strcat(request_line, temp3);
        
        
        host_bit = 1;
        strcpy(host_header, uri);
      }
      /* User-Agent header */
      else if (strstr(uri, "User-Agent:") != NULL)
      {
        continue;
      }
      /* Accept header */
      else if (strstr(uri, "Accept:") != NULL)
      {
        continue;
      }
      /* Accept-Encoding header */
      else if (strstr(uri, "Accept-Encoding:") != NULL)
      {
        continue;
      }
      /* Connection header */
      else if (strstr(uri, "Connection:") != NULL)
      {
        continue;
      }
      /* Proxy-Connection header */
      else if (strstr(uri, "Proxy-Connection:") != NULL)
      {
        continue;
      }
      /* Default for additional requests*/
      else 
      {
        strcat(request_line, uri);
      }
    }
    
    /* Adding the 6 (at least 5) components not in uri */
    if (host_bit == 0)
    {
      sprintf(uri, "Host: %s:%s\r\n", request_host, request_port);
      strcat(request_line, uri);
    }

    strcat(request_line, user_agent_hdr);
    strcat(request_line, accept_hdr);
    strcat(request_line, accept_encoding_hdr);
    strcat(request_line, connection_hdr);
    strcat(request_line, proxy_connection_hdr);
    /* End the request_line */
    strcat(request_line, end_str);
    
    /* Make a cache id for this request & check cache for hit*/
    strcpy(cache_id, uri_first_line);
    if (proxy_read_from_cache(cache, cache_id, cache_data, cache_len)==0)
    {
      return 1;      
    }
    
    /* Cache miss, open client_fd to connect to server */
    *server_fd = Open_clientfd(request_host, request_port);
    if (*server_fd < 0)
    {
      return -1;
    }
    
    
    if (Rio_writen(*server_fd, (void*)request_line, 
        (size_t)strlen(request_line)) == -1)
    {
      return -1;
    }
    return 0;
  }
}

/* parse_uri: takes the request in form of a buffer
 * and parses into 3 ways:
 * 1) The uri       into  method, url and http_version
 * 2) The url       into host_port and suffix
 * 3) The host_name into host and port
 */
int parse_uri(char *uri, char *method, char *url, char *http_version,
                       char *protocol,char *host_name, char *suffix,
                       char *request_host, char *request_port) 
{
  /* Variables Used */
  char *tmp = NULL;

  /* Check for errors */
  if (strlen(uri) < 1)
  {
    return -1;
  }
  
  /* In case of just host name, set suffix to '/' */
  strcpy(suffix, "/");
  
  /* Set method, url & http_version */
  sscanf(uri, "%s %s %s", method, url, http_version);
  
  /* Depending on whether it has a protocol (eg:http://....), parse url */
  if (strstr(url, "://") != NULL)  /*  has protocol   */
  {
    sscanf(url, "%[^:]://%[^/]%s",protocol, host_name, suffix);
  }
  else                             /* has no protocol */
  {
    sscanf(url, "%[^/]%s", host_name, suffix);
  }
  
  /* Now we have host_name, we can parse it (example page 913) */
  tmp = index(host_name, ':');
  if (tmp != NULL) 
  {
    *tmp = '\0';
    strcpy(request_port, tmp + 1);
  } 
  else 
  {
    strcpy(request_port, default_port_str);
  }
  strcpy(request_host, host_name);

  return 0;
}



/*
 * write_to_cache: write to cache, and Rio_written for client
 *
 * also adds to cache if size is appropriate
 *
 * returns -1 on error and 0 on normal success
 *
 */
int write_to_cache(int client_fd, int server_fd, 
                    void *cache_data, char *cache_id)
{
  /* Variables used */
  rio_t rio_server_fd;
  /* For caching purposes */
  unsigned int cache_len = 0;
  /* For server_fd */
  char server_fd_line[MAXLINE];
  /* To check size acceptability */
  ssize_t size_valid_bit = 1;
  ssize_t size=0;

   
  /*Initialize server_fd */
  Rio_readinitb(&rio_server_fd, server_fd);
  /* Reading from server_fd */
  while ((size = Rio_readnb(&rio_server_fd, 
         (void*)server_fd_line, (size_t)MAXBUF)) >0)
  {
    /* Preparing to cache it */
    if (size_valid_bit)
    {
      size_valid_bit=add_data(cache_data, &cache_len, 
      (unsigned int)size, server_fd_line, size_valid_bit);
    }
    /* Writing to server */
    if (Rio_writen(client_fd, (void*)server_fd_line, (size_t)size) == -1)
    {
      return -1;
    }
  }
  if (size == -1)
  {
    return -1;
  }
  /* Can be added to cache */
  if (size_valid_bit)
   {
     if (proxy_write_to_cache(cache, cache_id, cache_data, cache_len) == -1)
     {
       return -1;
     }
   }   
   return 0; 
}




/*
 * add_data: updates the cache by copying server_fd_line 
 * to cache_data 
 * 
 *It also updates the total cache_len and sets valid bit to add to buffer
 *
 */
int add_data(char *cache_data, unsigned int *cache_len, 
         unsigned int len, char *server_fd_line, int valid)
{
  /* enough space */
  if (((*cache_len + len) <= MAX_OBJECT_SIZE)&&(valid==1))
  {
    /* Only if valid has never been 0, since otherwise never cached */
    valid = 1;
  }
  else 
  {
    valid = 0;
  }
  /* Done in terms of pointer addressing since local variable in 
     write_to_cache has to be updated as well */
  void *data_pointer = (void *)((char *)cache_data + *cache_len);
  memcpy(data_pointer, server_fd_line, len);
  *cache_len = *cache_len + len;
  return valid;
}
