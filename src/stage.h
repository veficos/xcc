

#ifndef __STAGE__H__
#define __STAGE__H__


#include "config.h"


typedef struct diag_s*          diag_t;
typedef struct reader_s*        reader_t;
typedef struct lexer_s*         lexer_t;
typedef struct preprocessor_s*  preprocessor_t;


typedef enum stage_type_e {
    STAGE_READER,
    STAGE_LEXER,
    STAGE_PREPROCESSOR,
} stage_type_t;


typedef struct stage_s {
    diag_t diag;
    reader_t reader;
    lexer_t lexer;
    preprocessor_t pp;
} *stage_t;


#define STAGE_READER(stage)\
    ((stage)->reader)


#endif