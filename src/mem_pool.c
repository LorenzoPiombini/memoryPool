#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"



void pool_free(void **ptr, size_t size, struct m_pool *pool)
{
	if((*pool).allocated == 0) return;
	
	if(!(*ptr)) return;

	if(!pool) return;
	
	if(size == 0) return;

	memset(*ptr,0,size);
	(*pool).allocated -= size;
	(*pool).m_free += size;
	*ptr = NULL;		
}

int pool_init(struct m_pool *pool)
{
	(*pool).chunk = (void*) malloc(CHUNK_SIZE);
	if(!(*pool).chunk){
		fprintf(stderr,"%s() failed.\n",__func__);
		return -1;
	}

	memset(((*pool).chunk),0,CHUNK_SIZE);
	(*pool).allocated = 0;
	(*pool).m_free = CHUNK_SIZE;
	
	return 0;
}

/*size mean sizeof(type) * the type you want*/
int pool_alloc(struct m_pool *pool, void **ptr, size_t size, enum type t)
{
	if(!pool) return -1;

	if(size > CHUNK_SIZE) return -1;

	if(((*pool).m_free) < size) return -1;

	
	switch(t){
	case i32:
	{
		/*move along the chunk */
		*ptr =(int*)((char*)(*pool).chunk + (*pool).allocated);
		(*pool).allocated += size;
		(*pool).m_free -= size;
		return 0;
	}
	case i64:
	{
		/*move along the chunk */
		
		*ptr =(long*)((char*)(*pool).chunk + (*pool).allocated);
		(*pool).allocated += size;
		(*pool).m_free -= size;
		return 0;
	}
	case s:
	{
		/*move along the chunk */
		*ptr = (char*)(*pool).chunk + (*pool).allocated;
		(*pool).allocated += size;
		(*pool).m_free -= size;
		return 0;
	}
	default:
		fprintf(stderr,"type %d not supported.\n",t);
		return -1;
	}		
	
	return 1;	
}

void pool_destroy(struct m_pool *pool)
{	
	free((*pool).chunk);
	(*pool).m_free = 0;
	(*pool).allocated = 0;
}
