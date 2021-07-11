#include "file.h"
#include "bpt.h"

extern FILE* fp_table[10];
pagenum_t current_pagenum;      //temporarily save page number
page_t headerpage[10];

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int table_id){

    page_t *hpage = alloc_page_t();

    //printf("number_of_pages : %ld\n", headerpage.hpage.number_of_pages);
    pagenum_t alloc = headerpage[table_id - 1].hpage.free_page_number;
    //printf("Free page num : %ld\n", alloc);

    if(alloc == 0){
        alloc = alloc_free_page(table_id);
        current_pagenum += 4096;    

        file_read_page(alloc, table_id, hpage);

        headerpage[table_id - 1].hpage.free_page_number = hpage->fpage.next_free_page_number;

        //printf("4new free page number : %ld\n", hpage->hpage->free_page_number);

        free(hpage);
        //printf("%ld page allocated\n", alloc);
        return alloc;
    }

    else{
        page_t* current_page = alloc_page_t();
        
        file_read_page(alloc, table_id, current_page);

        pagenum_t next = current_page->fpage.next_free_page_number;
        headerpage[table_id - 1].hpage.free_page_number = next;
        
        //printf("3new free page number : %ld\n", next);

        free(current_page);
        free(hpage);

        //printf("%ld page allocated\n", alloc);
        return alloc;
    }
}

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum, int table_id){

    free_page* fpage = (free_page*)malloc(sizeof(free_page));
    fpage->next_free_page_number = headerpage[table_id - 1].hpage.free_page_number;
    
    headerpage[table_id - 1].hpage.free_page_number = pagenum;

    page_t* current_page = alloc_page_t();

    current_page->fpage = *fpage;
    file_write_page(pagenum, table_id, current_page);
    
    free(current_page);
}   

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, int table_id, page_t* dest){
    //printf("In file_read_page func, page_type : %d\n", page_type);
    fseeko(fp_table[table_id - 1], pagenum, SEEK_SET);
    //printf("Successfully changed file pointer\n");
    fread(&dest->hpage, 4096, 1, fp_table[table_id - 1]);
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, int table_id, const page_t* src){
    //printf("%dtyped page writed\n", page_type);
    fseeko(fp_table[table_id - 1], pagenum, SEEK_SET);
    fwrite(&src->hpage, 4096, 1, fp_table[table_id - 1]);
    sync();
}

void init_page(int table_id){
    headerpage[table_id - 1].hpage.free_page_number = 0;
    headerpage[table_id - 1].hpage.number_of_pages = 1;
    headerpage[table_id - 1].hpage.root_page_number = 0;
}

pagenum_t alloc_free_page(int table_id){
    page_t* current_page = alloc_page_t();
    //printf("\navailable free page number : %ld\n", current_page->hpage.free_page_number);

    //printf("number of pages : %ld\n", headerpage.hpage.number_of_pages);
    current_pagenum = headerpage[table_id - 1].hpage.number_of_pages * 4096;

    free_page* fpage = (free_page*)malloc(sizeof(free_page));
    fpage->next_free_page_number = headerpage[table_id - 1].hpage.free_page_number;
    headerpage[table_id - 1].hpage.free_page_number = current_pagenum;
    headerpage[table_id - 1].hpage.number_of_pages++;
    //printf("\nnew free page list : %ld\n", pagenum);

    current_page->fpage = *fpage;
    file_write_page(current_pagenum, table_id, current_page);

    free(current_page);

    return current_pagenum;
}

page_t* alloc_page_t(){
    page_t* current_page = (page_t*)malloc(sizeof(page_t));

    return current_page;
}