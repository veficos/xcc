
#define A a
#define B A
#define C D
#define D C
#define E E
   A;
      B;
          C;
              D;
                   E;


#define f(a,b,c) \
    do {    \
        int A;\
        int b;\
        int c;\
    } while (0)


#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__ 

#define Z() 123


EXPAND(DEFER(Z)())


int main(  )
{
    f(fuck, you, father);
    return  0;
}