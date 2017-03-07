

#ifndef __CONFIG__H__
#define __CONFIG__H__


#define     DEBUG

#define     USE_MALLOC

#ifndef bool 
typedef enum {
    false,
    true,
}bool;
#endif 

#ifndef inline
#define inline
#endif

#ifndef va_copy 
# ifdef __va_copy 
# define va_copy(DEST,SRC) __va_copy((DEST),(SRC)) 
# else 
# define va_copy(DEST, SRC) memcpy((&DEST), (&SRC), sizeof(va_list)) 
# endif 
#endif 

#endif
