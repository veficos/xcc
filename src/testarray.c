

#include "unittest.h"
#include "array.h"


void test_array(void)
{
    array_t *array;
    int *p;
    int i;
    
    array = array_create(sizeof(int));

    for (i = 0; i < 10; i++) {
        p = array_push(array);
        *p = i;
    }
    
    array_foreach(array, p, i) {
        TEST_COND("array_foreach()", p == array->elts);
        TEST_COND("array_push()", p[i] == i);
    }

    array_destroy(array);

    TEST_REPORT();
}

int main(void)
{
    test_array();
    return 0;
}
