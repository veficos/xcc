

#include "config.h"
#include "pmalloc.h"
#include "unittest.h"


static inline
void __handler__(const char *fn, int line, void *ud)
{
    printf("custom handler:\n"
           "%s:%d:%p\n", fn, line, ud);
}


static
void test_pmalloc(void)
{
    void *ptr;
    set_alloc_oom_handler(__handler__, (void*) 1);
    pmalloc(INT_MAX);
}


int main(void)
{
    test_pmalloc();
    return 0;
}