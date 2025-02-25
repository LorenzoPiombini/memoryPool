# Memory Pool

This is a small memory pool study aimed at understanding the memory pool technique and applying it to a larger project.

## What is a Memory Pool?

The concept is straightforward: the `pool_init()` function uses `malloc` to allocate 1024 bytes of memory (as a `void*`). Then, you use `pool_alloc()` to carve out the specific number of bytes you need from this pre-allocated memory block. This is demonstrated in the `main.c` file below:

```c
#include <stdio.h>
#include "mem_pool.h"

int main(void)
{
    struct m_pool pool = {0};

    // Initialize the memory pool
    if (pool_init(&pool) == -1) {
        fprintf(stderr, "memory pool init failed.\n");
        return -1;
    }

    // Allocate memory for an array of 5 integers
    int *ptr = NULL;
    if (pool_alloc(&pool, (void**)&ptr, 5 * sizeof(int), i32) == -1) {
        fprintf(stderr, "memory pool allocation failed.\n");
        pool_destroy(&pool);
        return -1;
    }

    // Assign values to the allocated array
    for (int i = 0; i < 5; i++)
        ptr[i] = i;

    // Print the values
    for (int i = 0; i < 5; i++)
        printf("int at index %d in the array is: %d\n", i, ptr[i]);

    // Clean up the memory pool
    pool_destroy(&pool);
    return 0;
}
```

