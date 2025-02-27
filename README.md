# Memory Pool

This is a small memory pool study aimed at understanding the memory pool technique and applying it to a larger project.

## What is a Memory Pool?

The concept is straightforward: the `pool_init()` function uses `malloc` to allocate a fixed block of memory—1024 bytes in this case (as a `void*`). Instead of repeatedly calling `malloc` and `free` for small, individual allocations, you use `pool_alloc()` to carve out chunks of memory from this pre-allocated block as needed. This reduces fragmentation and improves performance in systems with frequent allocations. Once you're done with a chunk, `pool_free()` releases it back to the pool for reuse, and `pool_destroy()` cleans up the entire pool when you're finished. The example below, from `main.c`, demonstrates this process with integers, strings, and a custom struct:

```c
#include <stdio.h>
#include <string.h>
#include "mem_pool.h"

struct My_struct {
    int num;
    char s[2];
};

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
    if (pool_alloc(&pool, (void**)&ptr, sizeof(int), 5, i32) == -1) {
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

    /* Assign a string of 7 chars */
    char *str = NULL;
    if (pool_alloc(&pool, (void**)&str, sizeof(char), 7, s) == -1) {
        fprintf(stderr, "can't allocate memory.\n");
        pool_destroy(&pool);
        return -1;
    }

    strncpy(str, "ciao!!", 7);
    printf("we allocated a string of 7 bytes: %s\nfirst addres of string is %p.\n", str, (void*)&str[0]); 

    /* Allocate memory for the struct My_struct */
    struct My_struct *data = NULL;
    if (pool_alloc(&pool, (void**)&data, sizeof(struct My_struct), 1, ud) == -1) {
        fprintf(stderr, "can't allocate memory.\n");
        pool_destroy(&pool);
        return -1;
    }

    data->num = 15;
    data->s[0] = 'h';
    data->s[1] = '\0';

    printf("first address of struct %p.\n"
           "first int address %p\n"
           "value of integer %d\n"
           "first address of char array %p.\n"
           "value of char %c",
           data, (void*)&data->num, data->num, (void*)&data->s[0], data->s[0]);

    pool_free((void**)&str, 5 * sizeof(char), &pool);
    pool_free((void**)&ptr, 5 * sizeof(int), &pool);
    pool_free((void**)&data, 1 * sizeof(struct My_struct), &pool);

    pool_destroy(&pool);
    return 0;
}
```

In this example, the memory pool is initialized once, and its base and top addresses are printed to show the range of the allocated block. 
Then, it’s used to allocate space first for an array of 5 integers (20 bytes on a system where sizeof(int) is 4), then for a 7-byte string "ciao!!", and finally for a custom struct My_struct containing an integer and a 2-char array. 
The program assigns values to each allocation and prints them along with their memory addresses, showing how the memory is laid out within the pool. 
This demonstrates how the same 1024-byte block can be reused for different data types—integers, strings, and structs—with a single malloc call during pool_init(). 
After each allocation is used, pool_free() returns the memory to the pool, making it available for reuse, though in this example, we free everything at the end before calling pool_destroy() to release all resources.
This showcases the efficiency of a memory pool: rather than relying on the system’s heap management for every allocation, you manage a single block and reuse it flexibly.
Memory Alignment in the Pool
A critical aspect of memory management, especially in a memory pool, is ensuring proper alignment of allocated memory. 
Alignment refers to positioning data in memory at addresses that are multiples of a specific value (e.g., 4, 8, or 16 bytes), which is often required by hardware or optimizes access speed. 
Misaligned memory can lead to performance penalties or even crashes on some architectures. 
To handle this, the memory pool implementation includes two helper functions, defined in mem_pool.c:
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

- **`is_align()`**: 

  This function checks if a given pointer (`ptr`) is aligned to a specified boundary (`alignment`). 

  It casts the pointer to an integer type (`uintptr_t`) and uses the modulo operator (`%`) to see if the address is evenly divisible by the alignment value. 

  If the result is 0, the pointer is aligned.

- **`align()`**: 

  This function adjusts an address to the next aligned boundary. 

  It takes an unaligned address and an alignment value (e.g., 8 for 8-byte alignment), adds enough bytes to reach the next boundary, and uses a bitwise operation to ensure the result is a multiple of the alignment. 


These functions are used within 'pool_alloc()' to ensure that each chunk of memory returned from the pool is properly aligned for the requested data type (e.g., int, char, or struct). 
For instance, integers typically require 4-byte alignment on many systems, and structs may require alignment based on their largest member, so 'pool_alloc()' uses 'align()' to adjust the starting address of each allocated chunk if needed. 
This ensures the memory pool is both efficient and safe for diverse data types, preventing alignment-related issues while reusing the same block.
The types that you can use (e.g., i32, s, ud) are defined in 'mem_pool.h', along with the struct 'm_pool' that implements the memory pool itself.

#Example Output
Here’s the output from running the main.c program on a sample system (addresses will vary by system and run):

```plain text
base address is 0xfce5a93cf010
top address is 0xfce5a94cf00f
int a index 0 in the array is: 0, address: 0xfce5a93cf010.
int a index 1 in the array is: 1, address: 0xfce5a93cf014.
int a index 2 in the array is: 2, address: 0xfce5a93cf018.
int a index 3 in the array is: 3, address: 0xfce5a93cf01c.
int a index 4 in the array is: 4, address: 0xfce5a93cf020.
we allocated a string of 7 bytes: ciao!!
first addres of string is 0xfce5a93cf024.
first address of struct 0xfce5a93cf030.
first int address 0xfce5a93cf030
value of integer 15
first address of char array 0xfce5a93cf034.
value of char h
```

This output shows the memory layout: the integer array starts at the base address, followed by the string, and then the struct, with proper alignment maintained throughout.



