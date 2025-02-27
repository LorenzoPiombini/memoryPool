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

    printf("base address is %p\n", (void*)pool.base_address);
    printf("top address is %p\n", (void*)pool.top_address);

    /* Allocate memory for an array of 5 integers */
    int *ptr = NULL;
    if (pool_alloc(&pool, (void**)&ptr, 5 * sizeof(int), i32) == -1) {
        fprintf(stderr, "can't allocate memory.\n");
        pool_destroy(&pool);
        return -1;
    }

    /* Assign values to the allocated array */
    for (int i = 0; i < 5; i++)
        ptr[i] = i;

    /* Print the data */
    for (int i = 0; i < 5; i++)
        printf("int a index %d in the array is: %d, address: %p.\n", i, ptr[i], (void*)&ptr[i]);

    /* Assign a string of 5 chars */
    char *str = NULL;
    if (pool_alloc(&pool, (void**)&str, 5 * sizeof(char), s) == -1) {
        fprintf(stderr, "memory pool init failed.\n");
        pool_destroy(&pool);
        return -1;
    }

    strncpy(str, "ciao", 5);
    printf("we allocated a string of 5 bytes: %s\nfirst address of string is %p.\n", str, (void*)&str[0]); 

    pool_free((void**)&str, 5 * sizeof(char), &pool);
    pool_free((void**)&ptr, 5 * sizeof(int), &pool);

    pool_destroy(&pool);
    return 0;
}
```

In this example, the memory pool is initialized once, and its base and top addresses are printed to show the range of the allocated block. 

Then, it’s used to allocate space first for an array of 5 integers and later for a 5-byte string. 

For the integer array, we use 20 bytes (on my machine, where `sizeof(int)` is 4 bytes). 

After assigning and printing the values along with their addresses, we allocate a string "ciao" (occupying 5 bytes). 

This demonstrates how the same memory block can be reused for different data types with a single `malloc` call during `pool_init()`. 

After each allocation is used (e.g., printing the integers or the string with its starting address), `pool_free()` returns the memory to the pool, making it available for reuse. 

This showcases the efficiency of a memory pool: rather than relying on the system's heap management for every allocation, you manage a single block and reuse it flexibly. 

Finally, `pool_destroy()` ensures all resources are properly released when the program ends.

### Memory Alignment in the Pool

A critical aspect of memory management, especially in a memory pool, is ensuring proper alignment of allocated memory. 

Alignment refers to positioning data in memory at addresses that are multiples of a specific value (e.g., 4, 8, or 16 bytes), which is often required by hardware or optimizes access speed. 

Misaligned memory can lead to performance penalties or even crashes on some architectures. 

To handle this, the memory pool implementation includes two helper functions, defined in `mem_pool.c`:

```c
static int is_align(void *ptr, size_t alignment)
{
    return ((uintptr_t)ptr % alignment) == 0;
}

static uintptr_t align(uintptr_t address, size_t alignment)
{
    return (uintptr_t)((address + (alignment - 1)) & ~(alignment - 1));
}
```


