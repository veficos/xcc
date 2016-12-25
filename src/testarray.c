

#include "unittest.h"
#include "array.h"


void test_array(void)
{
    array_t *array;
    int *a;
    int i;
    
    array = array_create(sizeof(int));

    TEST_COND("array_empty()", array_empty(array));
    TEST_COND("array_size()", array_size(array) == 0);
    TEST_COND("array_capacity()", array_capacity(array) == 0);

    for (i = 0; i < 10; i++) {
        a = array_push(array);
        *a = i;
    }
    
    TEST_COND("array_empty()", !array_empty(array));
    TEST_COND("array_size()", array_size(array) == 10);
    TEST_COND("array_capacity()", array_capacity(array) == 6);

    array_foreach(array, a, i) {
        TEST_COND("array_foreach()", a == array->elts);
        TEST_COND("array_push()", a[i] == i);
    }

    array_pop(array);
    TEST_COND("array_pop()", array_size(array) == 9);

    array_pop_n(array, 2);
    TEST_COND("array_pop_n()", array_size(array) == 7);

    array_clear(array);
    TEST_COND("array_clear()", array_empty(array));
    TEST_COND("array_capacity()", array_capacity(array) == 16);

    array_destroy(array);

    TEST_REPORT();
}


int main(void)
{
    test_array();
    return 0;
}
