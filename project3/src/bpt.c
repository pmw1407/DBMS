    /*
 *  bpt.c  
 */
#define Version "1.14"
/*
 *
 *  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
 
 *  3. Neither the name of the copyright holder nor the names of its 
 *  contributors may be used to endorse or promote products derived from this 
 *  software without specific prior written permission.
 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 
 *  Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *  Original Date:  26 June 2010
 *  Last modified: 17 June 2016
 *
 *  This implementation demonstrates the B+ tree data structure
 *  for educational purposes, includin insertion, deletion, search, and display
 *  of the search path, the leaves, or the whole tree.
 *  
 *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 *  Usage:  bpt [order]
 *  where order is an optional argument
 *  (integer MIN_ORDER <= order <= MAX_ORDER)
 *  defined as the maximal number of pointers in any node.
 *
 */

#include "bpt.h"
#include "file.h"
#include "buffer.h"

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
int leaf_order = 32;
int internal_order = 248;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
//pagenum_t queue = 0;


/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
bool verbose_output = false;

int BUF_SIZE;

extern FILE* fp;
extern pagenum_t current_pagenum;   //temporarily save page number
extern buffer* head;
extern page_t headerpage[10];

table tables[10];
FILE* fp_table[10];

int table_index = 0;
// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

queue* start;


/* First message to the user.
 */

void show_menu(){
    printf("\n");
    printf("Welcome to disk-based b+ tree\n");
    printf("1. Open data file     Type \'1\'\n");
    printf("2. Insert key/vale    Type \'2\'\n");
    printf("3. Find key           Type \'3\'\n");
    printf("4. Delete key         Type \'4\'\n");
    printf("5. Quit               Type \'5\'\n");
    printf("6. Print tree         Type \'6\'\n");
    printf("7. Insert in range    Type \'7\'\n");
    printf("8. Delete in range    Type \'8\'\n");
    printf("9. Close Table        Type \'9\'\n");
    printf("\n");
    printf("Input Command : ");
}

void init_queue(pagenum_t new_node){
    start = (queue*)malloc(sizeof(queue));
    start->next = NULL;
    start->pagenum = new_node;
}


void enqueue(pagenum_t new_node){
    queue* temp;
    queue* c;

    if(start == NULL){
        
        init_queue(new_node);
    }

    else{
        temp = start;
        while(temp->next != NULL){
            temp = temp->next;
        }
        c = (queue*)malloc(sizeof(queue));
        c->next = NULL;
        c->pagenum = new_node;
        
        temp->next = c;
    }
}


queue* dequeue(){
    queue* n = start;
    start = start->next;
    n->next = NULL;
    return n;
}



/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */


int path_to_root(pagenum_t root, int table_id, pagenum_t child){
    int length = 0;
    pagenum_t c = child;
    //printf("root page num : %ld\n", root);
    //printf("child page num : %ld\n", child);
    while(c != root){
        int is_leaf = determine_leaf_or_internal(c, table_id);
        //printf("is leaf : %d\n", is_leaf);
        buffer* temp;
        if(is_leaf){
            temp = buffer_read_page(c, table_id);
            c = temp->frame->lpage.parent_page_number;
            temp->is_pinned = 0;
        }        

        else{
            temp = buffer_read_page(c, table_id);
            c = temp->frame->ipage.parent_page_number;
            temp->is_pinned = 0;
        }
        //printf("c : %ld\n", c);
        length++;
    }

    return length;
}

/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
*/

void print_tree(int table_id){

    //print_buffer_info(root_buf);

    pagenum_t root = headerpage[table_id - 1].hpage.root_page_number;
    //printf("In print tree func, root page num : %ld\n", root);

    pagenum_t temp;
    queue* n;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    buffer* current_buf;

    if(root == 0){
        printf("Empty tree.\n");
        return;
    }

    start = NULL;
    enqueue(root);
    while(start != NULL){
        n = dequeue();
        temp = n->pagenum;
        //printf("In print tree func, page %ld dequeued\n", temp);
        free(n);
        
        new_rank = path_to_root(root, table_id, temp);
        //printf("new rank : %d ", new_rank);
        if(new_rank != rank){
            rank = new_rank;
            printf("\n");
        }

        //printf("leaf : %d\n", determine_leaf_or_internal(temp, table_id));

        if(determine_leaf_or_internal(temp, table_id)){
            current_buf = buffer_read_page(temp, table_id);
            //printf("buffer page num_keys : %d\n", current_buf->frame->lpage.number_of_keys);
            for(i = 0; i < current_buf->frame->lpage.number_of_keys; i++){
                printf("%ld ", current_buf->frame->lpage.records[i].key);
            }

            current_buf->is_pinned = 0;
        }
        

        else{
            current_buf = buffer_read_page(temp, table_id);

            for(i = 0; i < current_buf->frame->ipage.number_of_keys; i++){
               printf("%ld ", current_buf->frame->ipage.entries[i].key);
            }

            enqueue(current_buf->frame->ipage.extra_page_number);
            //printf("child page %ld enqueued  ", current_buf->frame->ipage.extra_page_number);
            for(i = 0; i < current_buf->frame->ipage.number_of_keys; i++){
                enqueue(current_buf->frame->ipage.entries[i].page_number);
                //printf("child page %ld enqueued  ", current_buf->frame->ipage.entries[i].page_number);
            }

            current_buf->is_pinned = 0;
        }
        printf("| ");
    }
    printf("\n");
}


/* Finds and returns the record to which
 * a key refers.
*/

pagenum_t find_leaf(pagenum_t root, int table_id, int64_t key){
    int i = 0;
    internal_page* ipage;
    leaf_page* lpage;
    pagenum_t next;


    if(root == 0){
        return root;
    }

    if(determine_leaf_or_internal(root, table_id))
        return root;

    buffer* current_buf = buffer_read_page(root, table_id);
    ipage = &current_buf->frame->ipage;

    buffer* next_page;

    while(!ipage->is_leaf){
        
        i = 0;
        while(i < ipage->number_of_keys){
            if(key >= ipage->entries[i].key) i++;
            else break;
        }
        
        if(i > 0)
            next = ipage->entries[i - 1].page_number;
        else 
            next = ipage->extra_page_number;

        int is_leaf = determine_leaf_or_internal(next, table_id);

        if(!is_leaf){
            next_page = buffer_read_page(next, table_id);
            ipage = &next_page->frame->ipage;
            next_page->is_pinned = 0;
        }
        else{
            next_page = buffer_read_page(next, table_id);
            lpage = &next_page->frame->lpage;
            next_page->is_pinned = 0;
            break;
        }
    }

    //printf("key found in %ld page\n", next);

    return next;
}

pagenum_t find(pagenum_t root, int table_id, int64_t key){
    if(root == 0)
        return root;    //empty tree

    int i = 0;

    pagenum_t pnum = find_leaf(root, table_id, key);
    
    buffer* current_buf = buffer_read_page(pnum, table_id);
    current_buf->is_pinned = 0;

    pagenum_t ret = current_buf->frame->lpage.right_sibling_page_number;
    int num = current_buf->frame->lpage.number_of_keys;
    
    for(i = 0; i < num; i++){
        if(current_buf->frame->lpage.records[i].key == key){
            ret = 1;            
            return ret;
        }
    }
    
    if(i == num)
        ret = pnum;    

    else
        ret = current_buf->frame->lpage.right_sibling_page_number;

    return ret;    
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}


// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
*/

/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */

pagenum_t make_node(int table_id){

    internal_page* new_root_page = (internal_page*)malloc(sizeof(internal_page));

    buffer* new_node = buffer_alloc_page(table_id);

    if(new_root_page == NULL){
        perror("Node Creation Failed\n");
        exit(EXIT_FAILURE);
    }

    new_root_page->parent_page_number = 0;
    new_root_page->is_leaf = 0;
    new_root_page->number_of_keys = 0;
    new_root_page->extra_page_number = 0;
    
    new_node->frame->ipage = *new_root_page;
    new_node->is_dirty = 1;

    return new_node->page_num;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
*/

pagenum_t make_leaf(int table_id){
    buffer* new_leaf = buffer_alloc_page(table_id);

    leaf_page* leaf_node = (leaf_page*)malloc(sizeof(leaf_page));
    leaf_node->parent_page_number = 0;
    leaf_node->is_leaf = 1;
    leaf_node->number_of_keys = 0;
    leaf_node->right_sibling_page_number = 0;

    new_leaf->frame->lpage = *leaf_node;
    new_leaf->is_dirty = 1;
    
    return new_leaf->page_num;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
*/

int get_left_index(pagenum_t parent, pagenum_t left, int table_id){

    int left_index = 0;
    internal_page* parent_page;
    
    buffer* parent_buf;
    parent_buf = buffer_read_page(parent, table_id);
    parent_buf->is_pinned = 0;

    parent_page = &parent_buf->frame->ipage;

    if(parent_page->extra_page_number == left){
        return -1;
    }
        
    while(left_index <= parent_page->number_of_keys &&
            parent_page->entries[left_index].page_number != left)
        left_index++;
    

    return left_index;
}


/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
*/

pagenum_t insert_into_leaf(pagenum_t pagenum, int table_id, int64_t key, char* value){
    int i, insertion_point;

    buffer* leaf_buf;
    leaf_buf = buffer_read_page(pagenum, table_id);

    leaf_page* leaf = &leaf_buf->frame->lpage;
    int num = leaf->number_of_keys;

    insertion_point = 0;
    while(insertion_point < num && leaf->records[insertion_point].key < key)
        insertion_point++;
    
    for(i = num; i > insertion_point; i--){
        leaf->records[i].key = leaf->records[i - 1].key;
        strcpy(leaf->records[i].value, leaf->records[i - 1].value);
    }
    
    leaf->records[insertion_point].key = key;
    strcpy(leaf->records[insertion_point].value, value);
    leaf->number_of_keys++;

    //printf("Have to write leaf page\n");
    //printf("Key written in leaf page %ld\n", pagenum);
    //printf("%ld inserted\n", key);

    leaf_buf->is_dirty = 1;
    leaf_buf->is_pinned = 0;

    return pagenum;
}

/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
*/

pagenum_t insert_into_leaf_after_splitting(pagenum_t root, int table_id, pagenum_t leaf, int64_t key, char* value){
    pagenum_t new_leaf;
    int64_t* temp_keys = (int64_t*)malloc(sizeof(int64_t) * leaf_order);
    char temp_values[leaf_order][120];
    int insertion_index, split, i, j;
    int64_t new_key;

    //printf("Try to make new leaf\n");
    new_leaf = make_leaf(table_id);
    //printf("New leaf made. New leaf page number : %ld\n\n", new_leaf);

    buffer* old_leaf = buffer_read_page(leaf, table_id);

    insertion_index = 0;
    
    leaf_page *lpage = &old_leaf->frame->lpage;
    while(insertion_index < leaf_order - 1 && lpage->records[insertion_index].key < key)
        insertion_index++;

    for(i = 0, j = 0; i < lpage->number_of_keys; i++, j++){
        if(j == insertion_index) j++;
        temp_keys[j] = lpage->records[i].key;
        strcpy(temp_values[j], lpage->records[i].value);
    }

    temp_keys[insertion_index] = key;
    strcpy(temp_values[insertion_index], value);

    lpage->number_of_keys = 0;

    split = cut(leaf_order - 1);
    //printf("split : %d\n\n", split);

    for(i = 0; i < split; i++){
        lpage->records[i].key = temp_keys[i];
        strcpy(lpage->records[i].value, temp_values[i]);
        lpage->number_of_keys++;
    }

    buffer* new_page = buffer_read_page(new_leaf, table_id);
    leaf_page* new_lpage;
    new_lpage = &new_page->frame->lpage;

    for(i = split, j = 0; i < leaf_order; i++, j++){
        new_lpage->records[j].key = temp_keys[i];
        strcpy(new_lpage->records[j].value, temp_values[i]);
        new_lpage->number_of_keys++;
    }

    free(temp_keys);

    new_lpage->right_sibling_page_number = lpage->right_sibling_page_number;
    lpage->right_sibling_page_number = new_leaf;
    new_lpage->parent_page_number = lpage->parent_page_number;

    //printf("after split, new root page : %ld\n", new_lpage->parent_page_number);

    new_key = new_lpage->records[0].key;

    //printf("Old leaf's right sibling : %ld ", lpage->right_sibling_page_number);
    //printf("New leaf's right sibling : %ld\n\n", new_lpage->right_sibling_page_number);
    //printf("Successfully splitted leaves.\n\n");

    new_page->is_dirty = 1;
    old_leaf->is_dirty = 1;
    new_page->is_pinned = 0;
    old_leaf->is_pinned = 0;

    return insert_into_parent(root, table_id, leaf, new_key, new_leaf);
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
*/

pagenum_t insert_into_node(pagenum_t root, int table_id, pagenum_t n, int left_index, int64_t key, pagenum_t right){
    
    int i;
    buffer* current_page = buffer_read_page(n, table_id);
    internal_page* npage;
    npage = &current_page->frame->ipage;

    for(i = npage->number_of_keys; i > left_index; i--){
        npage->entries[i + 1] = npage->entries[i];
    }

    npage->entries[left_index + 1].key = key;
    npage->entries[left_index + 1].page_number = right;
    npage->number_of_keys++;

    current_page->is_dirty = 1;
    current_page->is_pinned = 0;

    return root;
}



/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
*/

pagenum_t insert_into_node_after_splitting(pagenum_t root, int table_id, pagenum_t old_node, int left_index, int64_t key, pagenum_t right){
    //printf("insert_into_node_after_splitting\n");
    int i, j, split;
    int64_t k_prime;
    pagenum_t new_node, child;
    internal_entry temp[internal_order];

    internal_page* old_node_page;
    buffer* current_page = buffer_read_page(old_node, table_id);
    old_node_page = &current_page->frame->ipage;

    for(i = 0, j = 0; i < old_node_page->number_of_keys; i++, j++){
        if(j == left_index + 1) j++;
        temp[j] = old_node_page->entries[i];
    }

    temp[left_index + 1].key = key;
    temp[left_index + 1].page_number = right;

    split = cut(internal_order - 1);
    new_node = make_node(table_id);
    old_node_page->number_of_keys = 0;

    internal_page* new_node_page;
    buffer* new_page = buffer_read_page(new_node, table_id);
    new_node_page = &new_page->frame->ipage;

    for(i = 0, j = 0; i < split; i++, j++){
        old_node_page->entries[i] = temp[i];
        old_node_page->number_of_keys++;
    }
    
    k_prime = temp[split].key;

    new_node_page->extra_page_number = temp[split].page_number;

    for(i = split + 1, j = 0; i < internal_order; i++, j++){
        new_node_page->entries[j] = temp[i];
        new_node_page->number_of_keys++;
    }

    new_node_page->parent_page_number = old_node_page->parent_page_number;
    child = new_node_page->extra_page_number;

    current_page->is_dirty = 1;
    new_page->is_dirty = 1;

    buffer* child_page;
    buffer* child_not_leaf;

    if(determine_leaf_or_internal(child, table_id)){
        child_page = buffer_read_page(child, table_id);
        child_page->frame->lpage.parent_page_number = new_node;
        child_page->is_dirty = 1;
        child_page->is_pinned = 0;
    }
    else{
        child_not_leaf = buffer_read_page(child, table_id);
        child_not_leaf->frame->ipage.parent_page_number = new_node;
        child_not_leaf->is_dirty = 1;
        child_not_leaf->is_pinned = 0;
    }
    
    for(i = 0; i < new_node_page->number_of_keys; i++){
        child = new_node_page->entries[i].page_number;
        
        if(determine_leaf_or_internal(child, table_id)){
            child_page = buffer_read_page(child, table_id);
            child_page->frame->lpage.parent_page_number = new_node;
            child_page->is_dirty = 1;
            child_page->is_pinned = 0;
        }

        else{
            child_not_leaf = buffer_read_page(child, table_id);
            child_not_leaf->frame->ipage.parent_page_number = new_node;
            child_not_leaf->is_dirty = 1;
            child_not_leaf->is_pinned = 0;
        }
    }

    current_page->is_pinned = 0;
    new_page->is_pinned = 0;

    return insert_into_parent(root, table_id, old_node, k_prime, new_node);
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
*/

pagenum_t insert_into_parent(pagenum_t root, int table_id, pagenum_t left, int64_t key, pagenum_t right){
    
    //printf("Insert into parent : %ld\n", root);

    int left_index;
    pagenum_t parent;

    internal_page* parent_page;

    buffer* left_child = buffer_read_page(left, table_id);
    parent = left_child->frame->lpage.parent_page_number;

    left_child->is_pinned = 0;
    
    //printf("parent pagenum : %ld\n", parent);

    if(parent == 0){
        return insert_into_new_root(left, table_id, key, right);
    }

    buffer* parent_buf = buffer_read_page(parent, table_id);
    parent_page = &parent_buf->frame->ipage;
    left_index = get_left_index(parent, left, table_id);

    int temp = parent_page->number_of_keys;

    if(temp < internal_order - 1)
        return insert_into_node(root, table_id, parent, left_index, key, right);

    return insert_into_node_after_splitting(root, table_id, parent, left_index, key, right);
}

/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
*/

pagenum_t insert_into_new_root(pagenum_t left, int table_id, int64_t key, pagenum_t right){

    pagenum_t root = make_node(table_id);

    //printf("new root made : %ld\n", root);

    buffer* current_page = buffer_read_page(root, table_id);

    internal_page* rpage;
    rpage = &current_page->frame->ipage;

    rpage->extra_page_number = left;
    rpage->entries[0].key = key;
    rpage->entries[0].page_number = right;
    rpage->number_of_keys++;
    rpage->parent_page_number = 0;

    current_page->is_dirty = 1;

    current_page = buffer_read_page(left, table_id);
    current_page->frame->lpage.parent_page_number = root;
    current_page->is_dirty = 1;
    current_page->is_pinned = 0;

    buffer* page = buffer_read_page(right, table_id);
    page->frame->lpage.parent_page_number = root;
    page->is_dirty = 1;
    page->is_pinned = 0;

    headerpage[table_id - 1].hpage.root_page_number = root;
    
    return root;
}



/* First insertion:
 * start a new tree.
*/

pagenum_t start_new_tree(int64_t key, int table_id, char* value){ 
    //printf("Start new tree func called\n");
    
    pagenum_t root = make_leaf(table_id);
    //printf("New Tree made. Root Page number : %ld\n\n", root);

    buffer* current_page = buffer_read_page(root, table_id);
    //printf("page_t created\n");

    leaf_page* rpage = &current_page->frame->lpage;

    rpage->records[0].key = key;
    strcpy(rpage->records[0].value, value);
    rpage->number_of_keys++;

    //printf("New tree info\n");
    //printf("root : %ld\n", current_page->frame->lpage.parent_page_number);
    //printf("key : %ld\n\n", current_page->frame->lpage.records[0].key);

    //printf("before write func check if the key is right : %ld\n", rpage->records[0].key);
    current_page->is_dirty = 1;

    headerpage[table_id - 1].hpage.root_page_number = root;
    
    //printf("header page info changed\n");
    //printf("root page num : %ld\n", current_page->frame->hpage.root_page_number);

    return root;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
*/

pagenum_t insert(pagenum_t root, int table_id, int64_t key, char* value){

    //printf("\nMaster insert %ld func called\n", key);
    //printf("Root page number : %ld\n", root);
    
    pagenum_t leaf;


    if(find(root, table_id, key) == 1){
        //printf("found key so don't need to insert key\n\n");
        printf("\nKey already exists\n");
        return 1;
    }
        

    if(root == 0)
        return start_new_tree(key, table_id, value);

    leaf = find_leaf(root, table_id, key);
    //printf("Leaf page number found : %ld\n\n", leaf);

    buffer* current_page = buffer_read_page(leaf, table_id);
    current_page->is_pinned = 0;
    //printf("read buffer successfully\n");

    leaf_page* lpage;
    lpage = &current_page->frame->lpage;

    int num = lpage->number_of_keys;

    if(num < leaf_order - 1){
        leaf = insert_into_leaf(leaf, table_id, key, value);
        return root;
    }

    return insert_into_leaf_after_splitting(root, table_id, leaf, key, value);
}

// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
*/

int get_neighbor_index(pagenum_t n, int table_id){

    int i;

    buffer* page = buffer_read_page(n, table_id);

    buffer* parent = buffer_read_page(page->frame->lpage.parent_page_number, table_id);

    page->is_pinned = 0;
    parent->is_pinned = 0;

    if(n == parent->frame->ipage.extra_page_number){
        
        return -2;
    }

    for(i = 0; i < parent->frame->ipage.number_of_keys; i++){
        if(parent->frame->ipage.entries[i].page_number == n){

            return i - 1;
        }
    }
}


pagenum_t remove_entry_from_node(pagenum_t n, int table_id, int64_t key){
    int i, num_pointers;

    i = 0;

    buffer* current_page;

    if(determine_leaf_or_internal(n, table_id)){
        current_page = buffer_read_page(n, table_id);
        leaf_page* lpage = &current_page->frame->lpage;

        while(lpage->records[i].key != key){
            i++;
        }
        for(++i; i < lpage->number_of_keys; i++){
            lpage->records[i - 1] = lpage->records[i];
        }

        lpage->number_of_keys--;

        current_page->is_dirty = 1;
        current_page->is_pinned = 0;
    }

    else{
        current_page = buffer_read_page(n, table_id);
        internal_page* ipage = &current_page->frame->ipage;

        while(ipage->entries[i].key != key){
            i++;
        }

        for(++i; i < ipage->number_of_keys; i++){
            ipage->entries[i - 1] = ipage->entries[i];
        }

        ipage->number_of_keys--;

        current_page->is_dirty = 1;
        current_page->is_pinned = 0;

    }

    return n;
}


pagenum_t adjust_root(pagenum_t root, int table_id){
    pagenum_t new_root;

    buffer* current_page = buffer_read_page(root, table_id);
    internal_page* root_page = &current_page->frame->ipage;

    //Case : nonempty root
    if(root_page->number_of_keys > 0){

        return root;
    }

    buffer* temp;
    //Case : empty root
    //If it has a child, promote the first child as the new root
    if(!root_page->is_leaf){
        new_root = root_page->extra_page_number;
        temp = buffer_read_page(new_root, table_id);
        temp->frame->ipage.parent_page_number = 0;

        headerpage[table_id - 1].hpage.root_page_number = new_root;

        temp->is_pinned = 0;
        temp->is_dirty = 1;
        
    }

    //If it is a leaf then the whole tree is empty.
    else{
        new_root = 0;
        headerpage[table_id - 1].hpage.root_page_number = 0;
    }

    current_page->is_pinned = 0;
    current_page->is_dirty = 1;

    return new_root;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
*/

pagenum_t coalesce_nodes(pagenum_t root, int table_id, pagenum_t n, pagenum_t neighbor, int neighbor_index, int64_t k_prime){

    int i, j, neighbor_insertion_index, n_end;
    pagenum_t temp;
    pagenum_t parent;

    //Swap neighbor with node if node is on the extreme left and neighbor is to its right
    if(neighbor_index == -2){
        temp = n;
        n = neighbor;
        neighbor = temp;
    }

    //Starting point in the neighbor for copying keys and pointers from n
    buffer* neighbor_page;
    if(determine_leaf_or_internal(neighbor, table_id)){
        neighbor_page = buffer_read_page(neighbor, table_id);
        leaf_page* neighbor_leaf = &neighbor_page->frame->lpage;

        neighbor_insertion_index = neighbor_leaf->number_of_keys;

        parent = neighbor_leaf->parent_page_number;

        buffer* n_page = buffer_read_page(n, table_id);
        leaf_page* n_leaf = &n_page->frame->lpage;

        for(i = neighbor_insertion_index, j = 0; j < n_leaf->number_of_keys; i++, j++){
            neighbor_leaf->records[i] = n_leaf->records[j];
            neighbor_leaf->number_of_keys++;
        }

        neighbor_leaf->right_sibling_page_number = n_leaf->right_sibling_page_number;

        neighbor_page->is_dirty = 1;
        neighbor_page->is_pinned = 0;

        n_page->is_pinned = 0;
        
    }

    else{
        neighbor_page = buffer_read_page(neighbor, table_id);
        internal_page* neighbor_internal = &neighbor_page->frame->ipage;

        neighbor_insertion_index = neighbor_internal->number_of_keys;

        neighbor_internal->entries[neighbor_insertion_index].key = k_prime;
        neighbor_internal->number_of_keys++;

        parent = neighbor_internal->parent_page_number;

        buffer* n_page = buffer_read_page(n, table_id);
        internal_page* n_internal = &n_page->frame->ipage;

        n_end = n_internal->number_of_keys;

        neighbor_internal->entries[neighbor_insertion_index].page_number = n_internal->extra_page_number;

        for(i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++){
            neighbor_internal->entries[i] = n_internal->entries[j];
            neighbor_internal->number_of_keys++;
            n_internal->number_of_keys--;
        }

        buffer* temp_child;
        
        for(i = neighbor_insertion_index; i < neighbor_internal->number_of_keys; i++){
            temp_child = buffer_read_page(neighbor_internal->entries[i].page_number, table_id);
            temp_child->frame->ipage.parent_page_number = neighbor;
            temp_child->is_pinned = 0;
            temp_child->is_dirty = 1;
            
        }


        neighbor_page->is_dirty = 1;
        neighbor_page->is_pinned = 0;
        n_page->is_dirty = 1;
        n_page->is_pinned = 0;
    }

    root = delete_entry(root, table_id, parent, k_prime);
    
    return root;
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
*/

pagenum_t redistribute_nodes(pagenum_t root, int table_id, pagenum_t n, pagenum_t neighbor, int neighbor_index, int k_prime_index, int64_t k_prime){
    
    int i;
    pagenum_t temp;

    buffer* neighbor_internal = buffer_read_page(neighbor, table_id);
    internal_page* neigh = &neighbor_internal->frame->ipage;

    buffer* n_internal = buffer_read_page(n, table_id);
    internal_page* npage = &n_internal->frame->ipage;

    //Case : n has a neighbor to the left.
    //Pull the neighbor's last key_pointer pair over from the neighbor's right end to n's left end
    if(neighbor_index != -2){
        npage->entries[0].page_number = npage->extra_page_number;
        npage->extra_page_number = neigh->entries[neigh->number_of_keys - 1].page_number;
        npage->entries[0].key = k_prime;

        temp = npage->extra_page_number;
        buffer* tmp = buffer_read_page(temp, table_id);

        tmp->frame->lpage.parent_page_number = n;
        tmp->is_pinned = 0;
        tmp->is_dirty = 1;

        tmp = buffer_read_page(npage->parent_page_number, table_id);
        tmp->frame->ipage.entries[k_prime_index] = neigh->entries[neigh->number_of_keys - 1];
        tmp->is_dirty = 1;
        tmp->is_pinned = 0;

        npage->number_of_keys++;
        n_internal->is_pinned = 0;
        n_internal->is_dirty = 1;

        neigh->number_of_keys--;

        neighbor_internal->is_pinned = 0;
        neighbor_internal->is_dirty = 1;
    }

    else{
        npage->entries[0].key = k_prime;
        npage->entries[0].page_number = neigh->extra_page_number;
        
        temp = npage->entries[0].page_number;
        buffer* tmp = buffer_read_page(temp, table_id);

        tmp->frame->lpage.parent_page_number = n;
        tmp->is_dirty = 1;
        tmp->is_pinned = 0;

        temp = npage->parent_page_number;
        tmp = buffer_read_page(temp, table_id);
        tmp->frame->ipage.entries[k_prime_index].key = neigh->entries[0].key;
        tmp->is_dirty = 1;
        tmp->is_pinned = 0;

        npage->number_of_keys++;
        
        n_internal->is_pinned = 0;
        n_internal->is_dirty = 1;

        neigh->extra_page_number = neigh->entries[0].page_number;
        for(i = 0; i < neigh->number_of_keys - 1; i++){
            neigh->entries[i] = neigh->entries[i + 1];
        }

        neigh->number_of_keys--;
        
        neighbor_internal->is_pinned = 0;
        neighbor_internal->is_dirty = 1;
    }

    return root;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
*/

pagenum_t delete_entry(pagenum_t root, int table_id, pagenum_t n, int64_t key){
    int64_t min_keys;
    pagenum_t neighbor;
    int neighbor_index;
    int k_prime_index;
    int64_t k_prime;
    int capacity;

    //Remove key and pointer from node
    n = remove_entry_from_node(n, table_id, key);

    //Case : deletion from the root
    if(n == root)
        return adjust_root(root, table_id);

    //Case : node stays at or above minimum
    buffer* current_page = buffer_read_page(n, table_id);
    internal_page* ipage = &current_page->frame->ipage;
    int lpage_num = ipage->number_of_keys;

    if(lpage_num > 0)
        return root;

    buffer* parent = buffer_read_page(ipage->parent_page_number, table_id);
    internal_page* parent_page = &parent->frame->ipage;

    //Case : node falls below miminum.
    //Either coalescence or redistribution is needed
    neighbor_index = get_neighbor_index(n, table_id);
    //printf("neighbor index : %d\n", neighbor_index);

    if(neighbor_index == -2){       //n = parent->extra_page_number
        k_prime_index = 0;
        k_prime = parent_page->entries[0].key;
        neighbor = parent_page->entries[0].page_number;
    }

    else if(neighbor_index == -1){  //n = parent->entries[0].page_num
        k_prime_index = 0;
        k_prime = parent_page->entries[0].key;
        neighbor = parent_page->extra_page_number;
    }

    else{                           
        k_prime_index = neighbor_index + 1;
        k_prime = parent_page->entries[k_prime_index].key;
        neighbor = parent_page->entries[neighbor_index].page_number;
    }

    buffer* temp = buffer_read_page(neighbor, table_id);
    pagenum_t neighbor_keynum = temp->frame->lpage.number_of_keys;

    //Coalescence
    if(determine_leaf_or_internal(n, table_id) | neighbor_keynum != internal_order - 1){
        
        return coalesce_nodes(root, table_id, n, neighbor, neighbor_index, k_prime);
    } 

    else{
    
        return redistribute_nodes(root, table_id, n, neighbor, neighbor_index, k_prime_index, k_prime);
    }
}


/* Master deletion function.
*/

pagenum_t delete(pagenum_t root, int table_id, int64_t key){
    bool verbose = false;
    pagenum_t key_leaf;

    key_leaf = find(root, table_id, key);
    if(key_leaf != 1){
        printf("Key doesn't exist so can't delete key\n\n");
        return root;
    }
    else{
        key_leaf = find_leaf(root, table_id, key);
        root = delete_entry(root, table_id, key_leaf, key);
        
        headerpage[table_id - 1].hpage.root_page_number = root;
    }

    return root;
}


/*
Used in Main func
*/
int open_table(char* pathname){
    
    int table_id;
    int new = 1;

    for(int i = 0; i < table_index; i++){
        if(strcmp(tables[i].pathname, pathname) == 0){      //same
            table_id = tables[i].table_id;
            new = 0;
            break;
        }
    }

    if(new){
        strcpy(tables[table_index].pathname, pathname);
        tables[table_index].table_id = table_index + 1;
        table_id = table_index + 1;
        table_index++;
    }
    
    fp_table[table_id - 1] = fopen(pathname, "r+");
    
    if(fp_table[table_id - 1] == NULL){
        fp_table[table_id - 1] = fopen(pathname, "w+");
    }

    fseeko(fp_table[table_id - 1], 0, SEEK_END);
    int file_size = ftell(fp_table[table_id - 1]);

    if(file_size == 0){
        init_page(table_id);
        
        return table_id;
    }

    else{
        file_read_page(0, table_id, &headerpage[table_id - 1]);

        printf("Free page number : %ld\n", headerpage[table_id - 1].hpage.free_page_number);
        printf("Root page number : %ld\n", headerpage[table_id - 1].hpage.root_page_number);
        printf("Number of pages : %ld\n", headerpage[table_id - 1].hpage.number_of_pages); 

        return table_id;   
    }

    return -1;
}

int db_insert(int table_id, int64_t key, char* value){
    //printf("Successfully called db_insert func\n");
    pagenum_t root;

    //printf("Successfully read root page info\n");
    root = headerpage[table_id - 1].hpage.root_page_number;
    //printf("root page num : %ld\n", root);

    pagenum_t res = insert(root, table_id, key, value);
    //printf("Master insert func called successfully\n");

    if(res == 1){
        return 1;
    }

    return 0;
}


int db_delete(int table_id, int64_t key){
    pagenum_t root = headerpage[table_id - 1].hpage.root_page_number;

    root = delete(root, table_id, key);

    return 0;
}

int db_find(int table_id, int64_t key, char* ret_val){
    buffer* current_page;

    pagenum_t root = headerpage[table_id - 1].hpage.root_page_number;

    pagenum_t res = find(root, table_id, key);

    if(res == 1){
        current_page = buffer_read_page(find_leaf(root, table_id, key), table_id);

        int num = current_page->frame->lpage.number_of_keys;
        for(int i = 0; i < num; i++){
            if(key == current_page->frame->lpage.records[i].key){
                strcpy(ret_val, current_page->frame->lpage.records[i].value);

                current_page->is_pinned = 0;
                return 0;
            }
        }
    }

    return 1;
}

int init_db(int num_buf){
    if(num_buf <= 0 || num_buf < 5){
        return -1;
    }
    else{
        BUF_SIZE = num_buf;
        return 0;
    }
}

int close_table(int table_id){
    buffer* temp = head;

    buffer_write_header_page(table_id);

    while(temp->next_buffer != NULL){
        if(temp->table_id == table_id){
            buffer_write_page(temp, table_id);
        }

        temp = temp->next_buffer;
    }

    if(temp->table_id == table_id){
        buffer_write_page(temp, table_id);
    }

    fclose(fp_table[table_id - 1]);

    return 0;
}

int shutdown_db(){

    for(int i = 0; i < table_index; i++){
        buffer_write_header_page(i + 1);
    }

    while(head != NULL){
        buffer* temp = head;
        buffer_write_page(head, head->table_id);
        head = head->next_buffer;
        free(temp->frame);
        free(temp);
    }

    for(int i = 0; i < table_index; i++){
        fclose(fp_table[i]);
    }

    return 0;
}

