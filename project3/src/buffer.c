#include "buffer.h"
#include "file.h"

buffer* head = NULL;

int buf_size = 0;
extern int BUF_SIZE;
extern page_t headerpage[10];

void init_buffer(pagenum_t page_num, int table_id){
    head = (buffer*)malloc(sizeof(buffer));

    page_t* page = alloc_page_t();
    file_read_page(page_num, table_id, page);

    head->frame = page;
    head->table_id = table_id;
    head->page_num = page_num;
    head->is_dirty = 0;
    head->is_pinned = 1;
    head->next_buffer = NULL;
    head->prev_buffer = NULL;

    buf_size++;
}

buffer* buffer_read_page(pagenum_t page_num, int table_id){
    //print_buffer_list();
    
    if(head == NULL){
        init_buffer(page_num, table_id);
        return head;
    }

    else{
        page_t* page = alloc_page_t();

        buffer* temp = head;

        while(temp->next_buffer != NULL){   //check if the page already exists in memory
            if(temp->table_id == table_id && temp->page_num == page_num){
                temp->is_pinned++;
                free(page);
                return temp;
            }
            else{
                temp = temp->next_buffer;
            }
        }

        //printf("while loop executed\n");

        if(temp->table_id == table_id && temp->page_num == page_num){
            temp->is_pinned = 1;
            free(page);
            return temp;
        }

        //printf("current buf size : %d\n", buf_size);
        //printf("max buf size : %d\n", BUF_SIZE);
        if(buf_size < BUF_SIZE){            //create new buffer
            file_read_page(page_num, table_id, page);

            buffer* new = (buffer*)malloc(sizeof(buffer));
            new->frame = page;
            new->table_id = table_id;
            new->page_num = page_num;
            new->is_dirty = 0;
            new->is_pinned = 1;
            new->next_buffer = NULL;
            new->prev_buffer = temp;

            temp->next_buffer = new;

            buf_size++;

            return new;
        }

        else{                               //read new page from previous buffer. buffer changed as well.
            temp = head;
            while(temp->next_buffer != NULL){
                //print_buffer_info(temp);
                if(temp->is_pinned == 0){
                    //printf("found victim buf\n");
                    if(temp->is_dirty){
                        buffer_write_page(temp, table_id);

                        file_read_page(page_num, table_id, page);

                        temp->frame = page;
                        temp->is_dirty = 0;
                        temp->is_pinned = 1;
                        temp->table_id = table_id;
                        temp->page_num = page_num;

                        return temp;
                    }

                    else{
                        file_read_page(page_num, table_id, page);

                        temp->frame = page;
                        temp->is_dirty = 0;
                        temp->is_pinned = 1;
                        temp->table_id = table_id;
                        temp->page_num = page_num;

                        return temp;
                    }
                }

                temp = temp->next_buffer;
            }
        }
    }
}

void buffer_write_page(buffer* buf, int table_id){
    file_write_page(buf->page_num, table_id, buf->frame);
    
    buf->is_pinned = 0;
    buf->is_dirty = 0;
}

void buffer_write_header_page(int table_id){
    file_write_page(0, table_id, &headerpage[table_id - 1]);
}

int determine_leaf_or_internal(pagenum_t pagenum, int table_id){
    int ret;

    buffer* temp = buffer_read_page(pagenum, table_id);

    ret = temp->frame->ipage.is_leaf;   

    return ret;
}

buffer* buffer_alloc_page(int table_id){
    pagenum_t new_page = file_alloc_page(table_id);
    //printf("new free pageum : %ld\n", new_page);

    buffer* new_buf;
    new_buf = buffer_read_page(new_page, table_id);
    //printf("new page allocated : %ld\n", new_page);

    return new_buf;
}

//Buffer check func
void print_buffer_list(){
    buffer* temp = head;

    int buffer_idx = 1;
    while(temp != NULL){
        printf("buffer%d\n", buffer_idx);
        printf("table_id : %d\n", temp->table_id);
        printf("page num : %ld\n", temp->page_num);
        printf("is_dirty : %d\n", temp->is_dirty);
        printf("is_pinned : %d\n\n", temp->is_pinned);

        temp = temp->next_buffer;
        buffer_idx++;
    }
}

void print_buffer_info(buffer* buf){
    printf("Buffer info\n");
    printf("table_id : %d\n", buf->table_id);
    printf("page num : %ld\n", buf->page_num);
    printf("is pinned : %d\n", buf->is_pinned);
    printf("is_dirty : %d\n", buf->is_dirty);
}