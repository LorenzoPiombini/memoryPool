#include <stdio.h>
#include <string.h>
#include "mem_pool.h"


int main(void)
{
	struct m_pool pool = {0};
	/* Initialize the memory pool*/
	if(pool_init(&pool) == -1){
		fprintf(stderr,"memory pool init failed.\n");
		return -1;
	}

	printf("base address is %p\n", (void*)pool.base_address);
	printf("top address is %p\n", (void*)pool.top_address);

	/* Allocate memory for an array of 5 integers */
	int *ptr = NULL;
	if(pool_alloc(&pool,(void**)&ptr, 5*sizeof(int), i32) == -1){
		fprintf(stderr,"can't allocate memory.\n");
		pool_destroy(&pool);
		return -1;
	}

	/*Assign values to the allocated array*/
	for(int i = 0; i < 5; i++)
		ptr[i] = i;

	/* print the data  */
	for(int i = 0; i < 5; i++)
		printf("int a index %d in the array is: %d\n",i,ptr[i]);



	pool_free((void**)&ptr,5*sizeof(int),&pool);

	/*now we can reuse the memory with other data*/
	char *str = NULL;
	if(pool_alloc(&pool,(void**)&str, 5 * sizeof(char), i32) == -1){
		fprintf(stderr,"memory pool init failed.\n");
		pool_destroy(&pool);
		return -1;
	}

	strncpy(str,"ciao",5);
	printf("we allocated a string of 5 bytes: %s\n",str);
	
	pool_free((void**)&ptr,5 * sizeof(char),&pool);

	pool_destroy(&pool);
	return 0;
}
