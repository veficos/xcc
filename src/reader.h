

#ifndef __READER__H__
#define __READER__H__


typedef struct file_reader_s * file_reader_t;


file_reader_t file_reader_create(const char *filename);
void file_reader_destroy(file_reader_t fr);
int file_reader_get(file_reader_t fr);
int file_reader_peek(file_reader_t fr);
void file_reader_unget(file_reader_t fr, char ch);
const char *file_reader_name(file_reader_t fr);


#endif

