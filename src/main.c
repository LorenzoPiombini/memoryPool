#include <stdio.h>
#include <string.h>
#include "mem_pool.h"

struct My_struct{
	int num;
	char s[2];
};


int main(void)
{
	struct m_pool pool = {0};
	/* Initialize the memory pool*/
	if(pool_init(&pool) == -1){
		fprintf(stderr,"memory pool init failed.\n");
		return -1;
	}

	/*only for dev porpuses*/
	printf("base address is %p\n", (void*)pool.base_address);
	printf("top address is %p\n", (void*)pool.top_address);

	/* Allocate memory for an array of 5 integers */
	int *ptr = NULL;
	if(pool_alloc(&pool,(void**)&ptr, sizeof(int),5, i32) == -1){
		fprintf(stderr,"can't allocate memory.\n");
		pool_destroy(&pool);
		return -1;
	}
	
	/*Assign values to the allocated array*/
	for(int i = 0; i < 5; i++)
		ptr[i] = i;

	

	/*assign a string of 5 chars*/
	char *str = NULL;
	if(pool_alloc(&pool,(void**)&str,sizeof(char),7, s) == -1){
		fprintf(stderr,"can't allocate memory.\n");
		pool_destroy(&pool);
		return -1;
	}

	strncpy(str,"ciao!!",7);
	printf("\nwe allocated a string of 7 bytes: %s\nfirst addres of string is %p.\n",str,(void*)&str[0]); 

	/*expand the array to 7 integers*/
	if(pool_realloc(&pool,(void**)&ptr,sizeof(int),2,i32) == -1){
		fprintf(stderr,"can't realloc memory.\n");
		return 0;
	}

	for(int i = 5 ; i < 7;i++)
		ptr[i] = i;

	/* print the data  */
	for(int i = 0; i < 7; i++)
		printf("int a index %d in the array is: %d, address: %p.\n",i,ptr[i],(void*)&ptr[i]);

	/*allocate memory for the struct My_struct*/
	struct My_struct *data = NULL;
	if(pool_alloc(&pool, (void**)&data,sizeof(struct My_struct),1,ud) == -1 ){
		fprintf(stderr,"can't allocate memory.\n");
		pool_destroy(&pool);
		return -1;
	}
	
	data->num = 15;
	data->s[0] = 'h';
	data->s[1] = '\0';

	printf("\n\n\nfirst address of struct %p.\n"\
			"first int address %p\n"\
			"value of integer %d\n"\
			"first address of char array %p.\n"\
			"value of char is %c\n"
			,data,(void*)&data->num,data->num,(void*)&data->s[0],data->s[0]);

	pool_free((void**)&str,5 * sizeof(char),&pool);
	pool_free((void**)&ptr,5 * sizeof(int),&pool);
	pool_free((void**)&data,1 * sizeof(struct My_struct),&pool);

	pool_destroy(&pool);
	return 0;
}
