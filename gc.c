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
 * @struct Block_Header
 * @brief Object metadata for garbage-collected heap objects.
 *
 * This struct stores metadata for each object allocated in the heap.
 * It includes information like the size of the object.
 *
 * @var Block_Header::flags
 * Flags indicating the state of the block. Use FL_ALLOC for allocated blocks
 * and FL_FREE for blocks that are in the free list.
 *
 * @var Block_Header::size
 * The size of the object, in bytes.
 *
 * @var Block_Header::next_free
 * A pointer to the next free object in the free list.
 */
typedef struct block_header {
  size_t flags;
  size_t size;
  struct block_header *next_free;
} Block_Header;

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
 *
 * @var Heap_Header::end
 * The end position of the heap. This marks the last byte that can be allocated
 * within the heap.
 */
typedef struct heap_header {
  size_t size;
  size_t current;
  size_t end;
} Heap_Header;

Block_Header *free_list;
Heap_Header *from_start;
Heap_Header *to_start;

#define TINY_HEAP_SIZE 0x4000
#define PTRSIZE ((size_t)sizeof(void *))
#define HEAP_HEADER_SIZE ((size_t)sizeof(Heap_Header))
#define BLOCK_HEADER_SIZE ((size_t)sizeof(Block_Header))
#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))
#define NEXT_HEADER(x) ((Block_Header *)((size_t)(x + 1) + x->size))

#define FL_ALLOC 0x1
#define FL_FREE 0x0

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

  p1 = malloc(req_size + PTRSIZE + HEAP_HEADER_SIZE);

  from_start = (Heap_Header *)ALIGN((size_t)p1, PTRSIZE);
  from_start->size = req_size;
  from_start->current = (size_t)(from_start + 1);
  from_start->end = (size_t)(from_start + 1 + req_size);

  p2 = malloc(req_size + PTRSIZE + HEAP_HEADER_SIZE);

  to_start = (Heap_Header *)ALIGN((size_t)p2, PTRSIZE);
  to_start->size = req_size;
  to_start->current = (size_t)(to_start + 1);
  to_start->end = (size_t)(to_start + 1 + req_size);
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
  Block_Header *p, *prev;

  req_size = ALIGN(req_size, PTRSIZE);
  if (req_size <= 0) {
    return NULL;
  }

  p = (Block_Header *)from_start->current;
  p->size = req_size;
  p->flags = FL_ALLOC;
  from_start->current = from_start->current + req_size;

  return (void *)(p + 1);
}

/**
 * @fn void mini_cpgc_free(void *ptr)
 * @brief Frees a memory block allocated by mini_cpgc_malloc.
 *
 * This function takes a pointer to a memory block previously allocated with
 * mini_cpgc_malloc and adds it back to the free list for potential future
 * reuse.
 *
 * @param ptr A pointer to the memory block to be freed.
 */
void mini_cpgc_free(void *ptr) {
  Block_Header *target, *hit;

  target = (Block_Header *)ptr - 1;

  if (free_list == NULL) {
    free_list = target;
    target->next_free = target;
    target->flags = FL_FREE;

    return;
  }

  /* search join point of target to free_list */
  for (hit = free_list; !(target > hit && target < hit->next_free);
       hit = hit->next_free)
    /* heap end? And hit(search)? */
    if (hit >= hit->next_free && (target > hit || target < hit->next_free))
      break;

  if (NEXT_HEADER(target) == hit->next_free) {
    /* merge */
    target->size += (hit->next_free->size + BLOCK_HEADER_SIZE);
    target->next_free = hit->next_free->next_free;
  } else {
    /* join next free block */
    target->next_free = hit->next_free;
  }
  if (NEXT_HEADER(hit) == target) {
    /* merge */
    hit->size += (target->size + BLOCK_HEADER_SIZE);
    hit->next_free = target->next_free;
  } else {
    /* join before free block */
    hit->next_free = target;
  }
  free_list = hit;
}

/* ========================================================================== */
/*  mini_cpgc                                                                 */
/* ========================================================================== */

/**
 * @brief Copy a block from the "from" heap to the "to" heap.
 *
 * This function copies a block, including its header, from the source heap
 * to the destination heap. The destination heap's free pointer is then
 * updated.
 *
 * @param from_block Pointer to the block in the "from" heap to be copied.
 * @param pfree Pointer to the current free space in the "to" heap.
 * @return Returns a pointer to the new block in the "to" heap.
 */
Block_Header *copy(Block_Header *from_block, void *pfree) {
  Block_Header *to_block;

  to_block = memcpy(pfree, from_block, BLOCK_HEADER_SIZE + from_block->size);
  pfree = (void *)(pfree + BLOCK_HEADER_SIZE + from_block->size);

  return to_block;
}

/**
 * @brief Swap the "from" and "to" heaps.
 *
 * This function swaps the pointers for the "from" and "to" heaps,
 * effectively making the "to" heap the new "from" heap for the next
 * garbage collection cycle.
 */
void swap() {
  Heap_Header *tmp;

  tmp = from_start;
  from_start = to_start;
  to_start = tmp;

  free_list = NULL;
}

/**
 * @brief Perform the copying garbage collection.
 *
 * This function iterates over all the blocks in the "from" heap, copies
 * the allocated blocks to the "to" heap, and then swaps the heaps.
 */
void copying(void) {
  Block_Header *p;
  void *pfree = (void *)to_start + 1;

  for (p = (Block_Header *)from_start + 1; (size_t)p < (size_t)from_start->end;
       p = NEXT_HEADER(p)) {
    if (p->flags == FL_ALLOC) {
      copy(p, pfree);
    }
  }

  swap();
}

/* ========================================================================== */
/*  test                                                                      */
/* ========================================================================== */

static void test_mini_cpgc_malloc_free(void) {
  void *p;

  /* malloc check */
  unsigned int alloc_size = 9;
  p = mini_cpgc_malloc(alloc_size);
  assert((size_t)(from_start + 1) ==
         (size_t)(from_start->current - ALIGN(alloc_size, PTRSIZE)));

  /* free check */
  mini_cpgc_free(p);
  assert((Block_Header *)p - 1 == free_list);
}

static void test(void) {
  heap_init(TINY_HEAP_SIZE);
  test_mini_cpgc_malloc_free();
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "test") == 0)
    test();
  return 0;
}