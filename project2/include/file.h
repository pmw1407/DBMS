//file.h
#ifndef __FILE_H__
#define __FILE_H__

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>


typedef uint64_t pagenum_t;

//Header Page
typedef struct header_page{
    pagenum_t free_page_number;
    pagenum_t root_page_number;
    pagenum_t number_of_pages;
    pagenum_t reserved[509];
}header_page;

//Free Page
typedef struct free_page{
    pagenum_t next_free_page_number;
    pagenum_t notused[511];
}free_page;

//Record in Leaf Page
typedef struct record{
    int64_t key;
    char value[120];
}record;

//Leaf Page
typedef struct leaf_page{
    pagenum_t parent_page_number;   // 0 if root page
    int is_leaf;                    // 0 internal page, 1 leaf page
    int number_of_keys;
    int reserved[26];
    pagenum_t right_sibling_page_number;
    struct record records[31];
}leaf_page;

//Entries in Internal Page
typedef struct internal_entry{
    int64_t key;
    pagenum_t page_number;
}internal_entry;

//Internal Page
typedef struct internal_page{
    pagenum_t parent_page_number;
    int is_leaf;
    int number_of_keys;
    int reserved[26];
    pagenum_t extra_page_number;
    internal_entry entries[248];
}internal_page;

typedef struct page_t{
    header_page hpage;
    free_page fpage;
    leaf_page lpage;
    internal_page ipage;
    int page_type;          //header page(1)    free page(2)    leaf page(3)    internal page(4)
}page_t;

//global

//Allocate an on-disk page from the free page list
pagenum_t file_alloc_page();

//Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum);

//Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest);

//Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src);

void init_page();

pagenum_t alloc_free_page(pagenum_t pagenum);

int determine_leaf_or_internal(pagenum_t pagenum);

page_t* alloc_page_t();

#endif /* __FILE_H__*/