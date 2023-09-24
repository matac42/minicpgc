/**
 * @file gc.c
 * @brief This file contains the implementation for a Copying Garbage Collector.
 * @author matac
 * @date 2023/09/23
 */

#include "gc.h"
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ========================================================================== */
/*  mini_cpgc_malloc                                                          */
/* ========================================================================== */

/**
 * @struct Object_Header
 * @brief Object metadata for garbage-collected heap objects.
 *
 * This struct stores metadata for each object allocated in the heap.
 * It includes information like the size of the object.
 *
 * @var Object_Header::size
 * The size of the object, in bytes.
 */
typedef struct object_header {
  size_t size;
} Object_Header;

/**
 * @struct Heap_Header
 * @brief Metadata for managing the heap used in garbage collection.
 *
 * This struct contains metadata for heap management, including the size of the
 * heap and a pointer to the current position within the heap for subsequent
 * allocations.
 *
 * @var Heap_Header::size
 * The total size of the heap, in bytes.
 *
 * @var Heap_Header::current
 * The current position within the heap for new allocations. This is updated
 * each time a new object is allocated.
 */
typedef struct heap_header {
  size_t size;
  size_t current;
} Heap_Header;

Heap_Header *from_start;
Heap_Header *to_start;

#define TINY_HEAP_SIZE 0x4000
#define PTRSIZE ((size_t)sizeof(void *))
#define HEADER_SIZE ((size_t)sizeof(Heap_Header))
#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))

/**
 * @fn void heap_init(size_t req_size)
 * @brief Initializes the heap areas for From-space and To-space.
 *
 * Initializes two heap areas: From-space and To-space, both with the same size,
 * specified by the req_size parameter. If the given req_size is smaller than
 * TINY_HEAP_SIZE, then TINY_HEAP_SIZE is used as the size.
 *
 * @param req_size The requested size of the heap areas in bytes.
 * @return None
 */
void heap_init(size_t req_size) {
  void *p1, *p2;

  if (req_size < TINY_HEAP_SIZE)
    req_size = TINY_HEAP_SIZE;

  p1 = malloc(req_size + PTRSIZE + HEADER_SIZE);

  from_start = (Heap_Header *)ALIGN((size_t)p1, PTRSIZE);
  from_start->size = req_size;
  from_start->current = (size_t)from_start;

  p2 = malloc(req_size + PTRSIZE + HEADER_SIZE);

  to_start = (Heap_Header *)ALIGN((size_t)p2, PTRSIZE);
  to_start->size = req_size;
  to_start->current = (size_t)to_start;
}

/**
 * @fn void *mini_cpgc_malloc(size_t req_size)
 * @brief Allocates memory in the From-space heap area.
 *
 * Allocates a memory block of size req_size in the From-space heap.
 * The function aligns the requested size to the nearest PTRSIZE boundary.
 * If the size is zero or negative, the function returns NULL.
 *
 * @param req_size The requested size of the memory block in bytes.
 * @return A pointer to the allocated memory block, or NULL if the allocation
 * failed.
 * @warning The function does not check for heap overflow or perform garbage
 * collection.
 */
void *mini_cpgc_malloc(size_t req_size) {
  Object_Header *p;

  req_size = ALIGN(req_size, PTRSIZE);
  if (req_size <= 0) {
    return NULL;
  }

  p->size = req_size;
  from_start->current = from_start->current + req_size;

  return (void *)(p + 1);
}

// void mini_cpgc_free(void *ptr) {}

/* ========================================================================== */
/*  mini_cpgc                                                                 */
/* ========================================================================== */

// void copying(void) {
//   size_t free = to_start;
//   for () {
//   }
//   copy();
//   swap();
// }

// Header copy(Header) {}

// void swap(size_t from_start, size_t to_start) {}

/* ========================================================================== */
/*  test                                                                      */
/* ========================================================================== */

static void test(void) {
  heap_init(TINY_HEAP_SIZE);
  printf("from_start : %zx, to_start : %zx, current : %zx\n",
         (size_t)from_start, (size_t)to_start, from_start->current);

  /* malloc check */
  mini_cpgc_malloc(1);
  printf("from_start : %zx, to_start : %zx, current : %zx\n",
         (size_t)from_start, (size_t)to_start, from_start->current);
  assert((size_t)from_start == (size_t)(from_start->current - 8));
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "test") == 0)
    test();
  return 0;
}