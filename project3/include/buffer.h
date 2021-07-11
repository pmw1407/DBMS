#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "file.h"

int BUF_SIZE;

typedef struct buffer{
    page_t* frame;
    int table_id;
    pagenum_t page_num;
    int is_dirty;
    int is_pinned;
    struct buffer* next_buffer;
    struct buffer* prev_buffer;
}buffer;

void init_buffer(pagenum_t page_num,  int table_id);

buffer* buffer_read_page(pagenum_t page_num,  int table_id);

void buffer_write_page(buffer* buf, int table_id);

void buffer_write_header_page(int table_id);

int determine_leaf_or_internal(pagenum_t pagenum, int table_id);

buffer* buffer_alloc_page(int table_id) ;

void print_buffer_list();

void print_buffer_info(buffer* buf);

#endif /* __BUFFER_H__*/