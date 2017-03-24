

#include "config.h"
#include "array.h"
#include "cstring.h"
#include "pmalloc.h"
#include "preprocessor.h"


static inline void __preprocessor_expand__();


preprocessor_t preprocessor_create(lexer_t lexer)
{
    preprocessor_t pp;
    array_t std_include_paths;

    std_include_paths = array_create_n(sizeof(cstring_t), 8);
    if (std_include_paths == NULL) {
        goto done;
    }

    pp = (preprocessor_t) pmalloc(sizeof(struct preprocessor_s));
    if (pp == NULL) {
        goto clean_paths;
    }

    pp->std_include_paths = std_include_paths;
    pp->lexer = lexer;
    return pp;

clean_paths:
    array_destroy(std_include_paths);

done:
    return NULL;
}


void preprocessor_destroy(preprocessor_t pp)
{
    cstring_t *std_include_paths;
    size_t i;

    array_foreach(pp->std_include_paths, std_include_paths, i) {
        cstring_destroy(std_include_paths[i]);
    }

    array_destroy(pp->std_include_paths);

    pfree(pp);
}


bool preprocessor_add_include_path(preprocessor_t pp, const char *path)
{
    cstring_t *item;
    cstring_t cs;
    
    cs = cstring_create(path);
    if (cs == NULL) {
        return false;
    }

    item = (cstring_t *) array_push(pp->std_include_paths);
    if (item == NULL) {
        cstring_destroy(cs);
        return false;
    }

    *item = cs;

    return true;
}


token_t preprocessor_get(preprocessor_t pp)
{

}
