/*
 * runyunz - Runyun Zhang
 * Cache Lab Part A
 */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "cachelab.h"

typedef struct
{
	int valid;
	long long tag; /* tag = (64 - S - B) bits -> 8 bytes at most*/
} Line;

int parseOption(int argc, char* argv[], int* verbal, int* s, int* E, int* b,
		char** t)
{
	/*
	 * parseOption - retrieve command parameters
	 */

	int opt;

	while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch(opt)
		{
			case 'h':
				printf("help info");
				break;
			case 'v':
				*verbal = 1;
				break;
			case 's':
				*s = atoi(optarg);
				break;
			case 'E':
				*E = atoi(optarg);
				break;
			case 'b':
				*b = atoi(optarg);
				break;
			case 't':
				*t = optarg;
				break;
			default:
				printf("wrong parameter");
				return -1;

		}
	}
	return 0;
}

int initCache(int S, int E, Line*** pCache, int*** pQueue)
{
	/*
	 * initCache - allocate cache
	 */

	int i, j;

	*pCache = (Line**)malloc(sizeof(Line*) * S);
	if(*pCache == NULL)
	{
		return -1;
	}

	(*pCache)[0] = (Line*)malloc(sizeof(Line) * S * E);
	if((*pCache)[0] == NULL)
	{
		return -1;
	}

	for(i = 1; i < S; i++)
	{
		(*pCache)[i] = (*pCache)[i-1] + E;
	}
	memset((*pCache)[0], 0, S * E);

	/* initialize queue for LRU*/
	*pQueue = (int**)malloc(sizeof(int*) * S);
	if(*pQueue == NULL)
	{
		return -1;
	}

	(*pQueue)[0] = (int*)malloc(sizeof(int) * S * E);
	if((*pQueue)[0] == NULL)
	{
		return -1;
	}

	for(i = 1; i < S; i++)
	{
		(*pQueue)[i] = (*pQueue)[i-1] + E;

	}
	for(i = 0; i < S; i++)
	{
		for(j = 0; j < E; j++)
		{
			(*pQueue)[i][j] = j;
		}
	}

	return 0;
}

int updateQueue(int E, int*** pQueue, long long set, int pos)
{
	/*
	 * updateQueue - update queue to follow LRU
	 */

	int i = 0;

	for(i = 0; i < E; i++)
	{
		if((*pQueue)[set][i] > pos)
		{
			(*pQueue)[set][i]--;
		}
	}

	return 0;
}

int scanCache(int s, int E, Line*** pCache, int*** pQueue, long long set,
		long long tag)
{
	/*
	 * scanCache - search data in the cache
	 */

	int i = 0;

	for(i = 0; i < E; i++)
	{
		if((*pCache)[set][i].tag == tag && (*pCache)[set][i].valid == 1)
		{
			updateQueue(E, pQueue, set, (*pQueue)[set][i]);
			(*pQueue)[set][i] = (E - 1);
			return 1;
		}
	}
	return 0;
}

int loadCache(int E, Line*** pCache, int*** pQueue, long long set,
		long long tag)
{
	/*
	 * loadCache - load and replace data
	 */

	int i = 0;
	int eviction = 0;

	for(i = 0; i < E; i++)
	{
		if((*pQueue)[set][i] == 0)
		{
			eviction = (*pCache)[set][i].valid;

			(*pCache)[set][i].valid = 1;
			(*pCache)[set][i].tag = tag;

			updateQueue(E, pQueue, set, (*pQueue)[set][i]);
			(*pQueue)[set][i] = (E - 1);

			return eviction;
		}
	}
	return -1;
}

int simulateCache(int verbal, int s, int E, int b, char* trace, Line*** pCache,
		int*** pQueue, int* cHit, int* cMiss, int* cEviction)
{
	/*
	 * simulateCache - simulate cache behavior
	 */

	FILE* pFile = NULL;
	char ins = 0;
	long long addr = 0;
	int size = 0;
	long long set = 0, tag = 0;
	int hit, eviction = 0;

	pFile = fopen(trace,"r");
	if(pFile == NULL)
	{
		return -1;
	}

	while(fscanf(pFile, " %c %llx,%d", &ins, &addr, &size) != -1)
	{
		if(ins == 'I')
		{
			continue;
		}

		/* addr = [tag][set][block] */
		set = (addr >> b) & ~(-1 << s);
		tag = addr >> (s + b);

		/* scan cache to retrieve data */
		if((hit = scanCache(s, E, pCache, pQueue, set, tag)) == -1)
		{
			return -1;
		}

		/* load data if miss */
		if(hit == 0)
		{
			if((eviction = loadCache(E, pCache, pQueue, set, tag)) == -1)
			{
				return -1;
			}
		}

		/* sum up*/
		if(hit == 1)
		{
			(*cHit)++;
		}
		else
		{
			(*cMiss)++;

			if(eviction == 1)
			{
				(*cEviction)++;
			}
		}

		if(ins == 'M')
		{
			(*cHit)++;
		}

		/* print the record if -v is specified*/
		if(verbal == 1)
		{
			printf("%c %llx,%d %s %s %s\n", ins, addr, size,
					(hit ? "hit" : "miss"),
					(hit && eviction ? "eviction" : ""),
					(ins == 'M' ? "hit" : ""));
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	int verbal;
	int s = 0, E = 0, b = 0;
	char* trace = NULL;
	Line** cache = NULL;
	int** queue = NULL;
	int cHit = 0, cMiss = 0, cEviction = 0;

	if(parseOption(argc, argv, &verbal, &s, &E, &b, &trace) == -1)
	{
		return -1;
	}

	if(initCache(1 << s, E, &cache, &queue) == -1)
	{
		return -1;
	}

	if(simulateCache(verbal, s, E, b, trace, &cache, &queue, &cHit, &cMiss,
			&cEviction) == -1)
	{
		return -1;
	}

	printSummary(cHit, cMiss, cEviction);
    return 0;
}
