/* ========================================================================== */
/*  mini_cpgc_malloc */
/* ========================================================================== */

typedef struct header {
  size_t flags;
  size_t size;
  size_t *forwarding;
} Header;

void *mini_cpgc_malloc(size_t req_size) {}

/* ========================================================================== */
/*  mini_cpgc */
/* ========================================================================== */

void copying(void) {
  copy();
  swap();
}

Header copy(Header) {}

void swap(size_t from_start, size_t to_start) {}
/* ==========================================================================
 */
/*  test */
/* ==========================================================================
 */

static void test_mini_cpgc_malloc_free(void) {
  void *p1;

  p1 = (void *)mini_cpgc_malloc(10);
}

static void test(void) { test_mini_cpgc_malloc_free(); }

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "test") == 0)
    test();
  return 0;
}