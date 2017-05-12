

#include "config.h"
#include "option.h"
#include "cstring.h"


static void parse_opts(option_t option, int argc, char *argv[]);


int main(int argc, char **argv)
{
    option_t option;
    
    option = option_create();

    parse_opts(option, argc, argv);

    option_destroy(option);

    return 0;
}


static 
void parse_opts(option_t option, int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp(arg, "-o") == 0) {
            if (++i >= argc) {
                printf("missing file name after '-o'");
            }
            option->outfile = cstring_create(argv[i]);
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0 ||
            strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            exit(EXIT_FAILURE);
        } else if (!strcmp(arg, "-c")) {
            option->cflag = true;
        } else if (!strcmp(arg, "-S")) {
            option->Sflag = true;
        } else if (!strcmp(arg, "-E")) {
            option->Eflag = true;
        } else if (!strcmp(arg, "-dump-ast")) {
            option->dump_ast = true;
        }
    }
}