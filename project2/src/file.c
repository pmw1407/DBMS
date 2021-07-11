#include "file.h"
#include "bpt.h"

FILE* fp;
pagenum_t current_pagenum;      //temporarily save page number

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(){
    page_t* hpage = alloc_page_t();
    hpage->page_type = 1;
    file_read_page(0, hpage);
    pagenum_t alloc = hpage->hpage.free_page_number;
    //printf("Free page num : %ld\n", alloc);

    if(alloc == 0){
        alloc = alloc_free_page(current_pagenum);

        hpage->page_type = 2;
        file_read_page(alloc, hpage);

        hpage->hpage.free_page_number = hpage->fpage.next_free_page_number;

        hpage->page_type = 1;
        file_write_page(0, hpage);
        //printf("4new free page number : %ld\n", hpage->hpage->free_page_number);

        free(hpage);
        //printf("%ld page allocated\n", alloc);
        return alloc;
    }

    else{
        page_t* current_page = alloc_page_t();
        
        current_page->page_type = 2;
        file_read_page(alloc, current_page);

        pagenum_t next = current_page->fpage.next_free_page_number;
        hpage->hpage.free_page_number = next;
        
        file_write_page(0, hpage);
        //printf("3new free page number : %ld\n", next);

        free(current_page);
        free(hpage);

        //printf("%ld page allocated\n", alloc);
        return alloc;
    }
}

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum){
    page_t* headerpage = alloc_page_t();
    headerpage->page_type = 1;
    file_read_page(0, headerpage);

    free_page* fpage = (free_page*)malloc(sizeof(free_page));
    fpage->next_free_page_number = headerpage->hpage.free_page_number;
    
    headerpage->hpage.free_page_number = pagenum;

    page_t* current_page = alloc_page_t();

    current_page->fpage = *fpage;
    current_page->page_type = 2;
    file_write_page(pagenum, current_page);
    
    file_write_page(0, headerpage);

    free(headerpage);
    free(current_page);
}   

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest){
    int page_type = dest->page_type;
    //printf("In file_read_page func, page_type : %d\n", page_type);
    fseeko(fp, pagenum, SEEK_SET);
    //printf("Successfully changed file pointer\n");
    switch(page_type){
        case 1:
            fread(&dest->hpage, 4096, 1, fp);
            //printf("Header page read\n");
            //printf("free page number : %ld\n", dest->hpage.free_page_number);
            //printf("root page number : %ld\n", dest->hpage.root_page_number);
            //printf("number of page : %ld\n", dest->hpage.number_of_pages);
            break;
        
        case 2:
            fread(&dest->fpage, 4096, 1, fp);
            break;
        
        case 3:
            fread(&dest->lpage, 4096, 1, fp);
            break;

        case 4:
            fread(&dest->ipage, 4096, 1, fp);
            break;
        
        default:
            printf("page type unrecognized\n");
            break;
    }
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src){
    int page_type = src->page_type;
    //printf("%dtyped page writed\n", page_type);
    fseeko(fp, pagenum, SEEK_SET);
    switch(page_type){
        case 1:
            fwrite(&src->hpage, 4096, 1, fp);
            //printf("free page written : %ld\n", src->hpage.free_page_number);
            break;
        
        case 2:
            fwrite(&src->fpage, 4096, 1, fp);
            break;
        
        case 3:
            //printf("key inserted %ld\n", src->lpage->records[0]->key);
            fwrite(&src->lpage, 4096, 1, fp);
            break;

        case 4:
            fwrite(&src->ipage, 4096, 1, fp);
            break;
    }
    sync();
}

void init_page(){
    
    page_t* current_page = alloc_page_t();

    current_page->hpage.free_page_number = 0;
    current_page->hpage.root_page_number = 0;
    current_page->hpage.number_of_pages = 1;
    current_page->page_type = 1;

    file_write_page(current_pagenum, current_page);
    //printf("1st free page number : %ld\n", current_page->hpage->free_page_number);
    current_pagenum += 4096;

    free(current_page);
}

pagenum_t alloc_free_page(pagenum_t pagenum){
    page_t* current_page = alloc_page_t();
    current_page->page_type = 1;
    file_read_page(0, current_page);
    //printf("\navailable free page number : %ld\n", current_page->hpage.free_page_number);

    free_page* fpage = (free_page*)malloc(sizeof(free_page));
    fpage->next_free_page_number = current_page->hpage.free_page_number;
    current_page->hpage.free_page_number = pagenum;
    current_page->hpage.number_of_pages++;
    //printf("\nnew free page list : %ld\n", pagenum);

    file_write_page(0, current_page);

    current_pagenum += 4096;
    
    current_page->fpage = *fpage;
    current_page->page_type = 2;
    file_write_page(pagenum, current_page);

    free(current_page);

    return pagenum;
}

int determine_leaf_or_internal(pagenum_t pagenum){
    int ret;

    fseeko(fp, pagenum + 8, SEEK_SET);
    fread(&ret, 4, 1, fp);

    return ret;
}

page_t* alloc_page_t(){
    page_t* current_page = (page_t*)malloc(sizeof(page_t));

    return current_page;
}