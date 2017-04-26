#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__ 

#define A() 123

A()

DEFER(A)()
OBSTRUCT(DEFER(A)())
EXPAND(DEFER(A)())
