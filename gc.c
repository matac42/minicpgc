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
/*  mini_cpgc_malloc */
/* ========================================================================== */

typedef struct object_header {
  size_t flags;
  size_t size;
  size_t *forwarding;
} Object_Header;

typedef struct heap_header {
  size_t size;
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

  p2 = malloc(req_size + PTRSIZE + HEADER_SIZE);

  to_start = (Heap_Header *)ALIGN((size_t)p2, PTRSIZE);
  to_start->size = req_size;
}

// void *mini_cpgc_malloc(size_t req_size) {
//   Object_Header *p;
// }

/* ========================================================================== */
/*  mini_cpgc */
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
/* ==========================================================================
 */
/*  test */
/* ==========================================================================
 */

static void test(void) {
  heap_init(TINY_HEAP_SIZE);
  printf("from_start : %p, to_start : %p\n", from_start, to_start);
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "test") == 0)
    test();
  return 0;
}