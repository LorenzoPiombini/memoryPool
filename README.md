# Memory Pool

This is a small memory pool study aimed at understanding the memory pool technique and applying it to a larger project.

## What is a Memory Pool?

The concept is straightforward: the `pool_init()` function uses `malloc` to allocate a fixed block of memory—1024 bytes in this case (as a `void*`). Instead of repeatedly calling `malloc` and `free` for small, individual allocations, you use `pool_alloc()` to carve out chunks of memory from this pre-allocated block as needed. This reduces fragmentation and improves performance in systems with frequent allocations. Once you're done with a chunk, `pool_free()` releases it back to the pool for reuse, and `pool_destroy()` cleans up the entire pool when you're finished. The example below, from `main.c`, demonstrates this process with integers and strings:

```c
#include <stdio.h>
#include <string.h>
#include "mem_pool.h"

int main(void)
{
    struct m_pool pool = {0};
    /* Initialize the memory pool */
    if (pool_init(&pool) == -1) {
        fprintf(stderr, "memory pool init failed.\n");
        return -1;
    }

    /* Allocate memory for an array of 5 integers */
    int *ptr = NULL;
    if (pool_alloc(&pool, (void**)&ptr, 5 * sizeof(int), i32) == -1) {
        fprintf(stderr, "memory pool init failed.\n");
        pool_destroy(&pool);
        return -1;
    }

    /* Assign values to the allocated array */
    for (int i = 0; i < 5; i++)
        ptr[i] = i;

    /* Print the data */
    for (int i = 0; i < 5; i++)
        printf("int at index %d in the array is: %d\n", i, ptr[i]);

    /* Free the allocated integer array back to the pool */
    pool_free((void**)&ptr, 5 * sizeof(int), &pool);

    /* Now we can reuse the memory with other data */
    char *str = NULL;
    if (pool_alloc(&pool, (void**)&str, 5 * sizeof(char), i32) == -1) {
        fprintf(stderr, "memory pool init failed.\n");
        pool_destroy(&pool);
        return -1;
    }

    /* Assign a string to the allocated memory */
    strncpy(str, "ciao", 5);
    printf("we allocated a string of 5 bytes: %s\n", str);

    /* Free the string memory back to the pool */
    pool_free((void**)&str, 5 * sizeof(char), &pool);

    /* Clean up the memory pool */
    pool_destroy(&pool);
    return 0;
}
```

In this example, the memory pool is initialized once, and then used to allocate space first for an array of 5 integers and later for a 5-byte string. 
It’s important to understand that, in the first allocation, we use 20 bytes (on my machine, where sizeof(int) is 4 bytes) for the array of integers. 
With pool_free(), we effectively zero out those 20 bytes, making that memory available again. 
We then reuse it for our string "ciao" (which occupies the first 5 bytes of those 40). This demonstrates how we can use the same memory multiple times for different data types, relying on a single malloc call during pool_init(). 
After each allocation is used (e.g., printing the integers or the string), pool_free() returns the memory to the pool, making it available for reuse. 
This showcases the efficiency of a memory pool: rather than relying on the system's heap management for every allocation, you manage a single block and reuse it flexibly. Finally, pool_destroy() ensures all resources are properly released when the program ends.
