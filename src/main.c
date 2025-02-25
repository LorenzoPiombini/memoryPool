#include <stdio.h>
#include "mem_pool.h"


int main(void)
{
	struct m_pool pool = {0};

	if(pool_init(&pool) == -1){
		fprintf(stderr,"memory pool init failed.\n");
		return -1;
	}

	int *ptr = NULL;
	if(pool_alloc(&pool,(void**)&ptr, 5*sizeof(int), i32) == -1){
		fprintf(stderr,"memory pool init failed.\n");
		pool_destroy(&pool);
		return -1;
	}

	for(int i = 0; i < 5; i++)
		ptr[i] = i;

	for(int i = 0; i < 5; i++)
		printf("int a index %d in the array is: %d\n",i,ptr[i]);

	pool_destroy(&pool);
	return 0;
}
