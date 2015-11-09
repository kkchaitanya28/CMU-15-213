//Ram Verma, ramv

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

//make a struct for a line, but no need for block byte
//need a counter for which line we are at
typedef struct
{
  int valid;
  int tag;
  int counter;
} line;

//a lot off lines make a set
typedef struct
{
  line *lines;
} set;

//a cache consists of all its sets and lines per set
typedef struct
{
  int setNum; //number of sets in cache
  int lineNum; //lines per set
  set *sets; 
} fakeCache;

//start with all zeros
//since global values, can alter them in getTraceInfo
int hits = 0;
int misses = 0;
int evictions = 0;
int state = 0; //for verbose to debug
/*
state 1 = hit
state 2 = miss
state 3 = miss & hit
state 4 = miss & eviction
state 5 = miss & eviction & hit
*/

#define BUFLEN 100//for char array sizes

//in order to link stuff from main function
//to other functions
//basically to remove implicit declaration errors
void getCommandInfo(int argc, char **argv, int *ss, int *EE, int *bb,
                   char *traceName, int *v);
void makeCache(fakeCache *cache, int s, int E);
int getTraceInfo(fakeCache *cache, char *lineInTrace, int s,
                  int b);
void updateCounter(fakeCache *cache, int setbit, int linei);



int main(int argc, char **argv)
{
  FILE *f;
  int s, E, b, v;
  char traceName[BUFLEN];
  char lineInTrace[BUFLEN];
  getCommandInfo(argc, argv, &s, &E, &b, traceName, &v);
  //gets the values and filename from command line
  fakeCache cache;
  makeCache(&cache, s, E);
  //make a cache and allocate all valid to zeros
  f = fopen(traceName, "r");  
  if (f == NULL)
  {
    printf("No file found\n");
  }
  while (NULL != fgets(lineInTrace, BUFLEN, f))//read the trace file
  {
    if ((' ' == lineInTrace[0]) && (' ' != lineInTrace[1]))
    {
	  getTraceInfo(&cache, lineInTrace, s, b);
	  if (v == 1)//verbose state
	  {
	    switch (state)
	    {
	      case 1:
	        printf("%s hit\n", lineInTrace+1);
	        break;
	      case 2:
	        printf("%s miss\n", lineInTrace+1);
	        break;
	      case 3:
	        printf("%s miss hit\n", lineInTrace+1);
	        break;
	      case 4:
	        printf("%s miss eviction\n", lineInTrace+1);
	        break;
	      case 5:
	        printf("%s miss eviction hit\n", lineInTrace+1);
	        break;        
	      default:
	        break;  
	    }
	  }
    }  
  }
  
  fclose(f);   
  int i;   
  //free all malloc memory
  for (i = 0; i < cache.setNum; i++)
  {
    free(cache.sets[i].lines);
  }
  free(cache.sets);   
  printSummary(hits, misses, evictions);
  return 0;
}



void getCommandInfo(int argc, char **argv, int *ss, int *EE, int *bb,
                   char *traceName, int *v)
{
  int opt;
  while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1)
  {
    switch(opt)
    {
      case 'v':
        *v = 1;
        break;
      case 's':
        *ss = atoi(optarg);
        break;
      case 'E':
        *EE = atoi(optarg);
        break;
      case 'b':
        *bb = atoi(optarg);
        break;
      case 't':
        strcpy(traceName, optarg); 
        break;
      default:
        printf("nothing in argument\n");
        break;        
    }
  }
}


void makeCache(fakeCache *cache, int s, int E)
{
  cache->setNum = (2 << s);
  cache->lineNum = E;
  cache->sets = (set *)malloc(cache->setNum * sizeof(set));
  if (cache->sets == NULL)
    printf("Malloc returned NULL\n");
  int i, j;
  for (i = 0; i < cache->setNum; i++)
  {
    cache->sets[i].lines = (line *)malloc(cache->lineNum * sizeof(line));
    if (cache->sets[i].lines == NULL)
      printf("Malloc returned NULL\n");
    for (j = 0; j < cache->lineNum; j++)
    {
      cache->sets[i].lines[j].valid = 0;//set valid to all zeros
      cache->sets[i].lines[j].counter = 0;
    }
  }
}


int getTraceInfo(fakeCache *cache, char *lineInTrace, int s,
                 int b)
//also does miss, hits, evictions
{
  int i;
  char operationField;
  int address, tagbit, setbit, numberOfBytes;
  int mask = 0x7fffffff;
  sscanf(lineInTrace, " %c %x,%d", &operationField, &address, &numberOfBytes);
  //get setbits and tagbits from address
  setbit = (mask >> (31 - s)) & (address >> b);
  tagbit = (mask >> (31 - s - b)) & (address >> (s + b));
  
  //now we start with checking cache for hits,misses and evictions
  for (i=0; i < cache->lineNum; i++)
  {
    if (cache->sets[setbit].lines[i].valid == 1 &&
        cache->sets[setbit].lines[i].tag == tagbit)
        //its a hit!
      {
        hits++;//normal L or S
        state = 1;
        if (operationField == 'M')//data load & data store
        { 
          hits++;//another hit needed
        }  
        updateCounter(cache, setbit, i);
        return i;  //to help in debugging we return i and not 0 
      }       
  }
  
  //not a hit
  misses++;
  state = 2;
  for (i=0; i < cache->lineNum; i++)
  {
    if (cache->sets[setbit].lines[i].valid == 0)
    //empty line, no eviction
      {
        //so set all the things in cache cos empty
        cache->sets[setbit].lines[i].valid = 1;
        cache->sets[setbit].lines[i].tag = tagbit;
        updateCounter(cache, setbit, i); 
        if (operationField == 'M')
        {
          hits++;//its miss and hit
          state = 3;
        }  
        else 
        {
          state = 2;//just miss
        } 
        return i;  
      }      
  }
  
  //need to evict now after getting miss only and no hits
  evictions++;
  state = 4;//miss and eviction
  for (i=0; i < cache->lineNum; i++)
  {
    if (cache->sets[setbit].lines[i].counter == 1)
    //with the LRU line
    //i replace it
      {
        cache->sets[setbit].lines[i].valid = 1;
        cache->sets[setbit].lines[i].tag = tagbit;
        updateCounter(cache, setbit, i); 
        if (operationField == 'M')
        {
          hits++;//its miss,eviction,hit
          state = 5;
        }  
        else
        {
          state = 4;
        }
        return i;
      }      
  }
  return 0;
}


void updateCounter(fakeCache *cache, int setbit, int linei)
//for locality
//LRU
{
  int j;
  for (j=0; j < cache->lineNum; j++)
  {
    if (cache->sets[setbit].lines[j].valid == 1 &&
        (cache->sets[setbit].lines[j].counter > 
         cache->sets[setbit].lines[linei].counter))
         {
           cache->sets[setbit].lines[j].counter--;
           //used by one less everytime 
         }
  }
  cache->sets[setbit].lines[linei].counter = cache->lineNum;
  //whichever line most recently used gets value of lineNum
}
