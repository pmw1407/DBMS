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

int table_id = -1;
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
int leaf_order = 5;
int internal_order = 5;

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

extern FILE* fp;
extern pagenum_t current_pagenum;   //temporarily save page number

// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

#define MAX 100000

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
    printf("8. Delete in range    TYpe \'8\'\n");
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


int path_to_root(pagenum_t root, pagenum_t child){
    int length = 0;
    pagenum_t c = child;

    while(c != root){

        page_t* current_page = alloc_page_t();
        
        if(determine_leaf_or_internal(c)){
            current_page->page_type = 3;
            file_read_page(c, current_page);
            c = current_page->lpage.parent_page_number;
        }
        else{    
            current_page->page_type = 4;
            file_read_page(c, current_page);
            c = current_page->ipage.parent_page_number;
        }
        length++;

        free(current_page); 
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

void print_tree(){

    page_t* hp = alloc_page_t();
    hp->page_type = 1;
    file_read_page(0, hp);
    pagenum_t root = hp->hpage.root_page_number;

    pagenum_t temp;
    queue* n;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    page_t* current_page = alloc_page_t();

    if(root == 0){
        printf("Empty tree.\n");
        return;
    }

    start = NULL;
    enqueue(root);
    while(start != NULL){
        n = dequeue();
        temp = n->pagenum;
        free(n);
        
        new_rank = path_to_root(root, temp);
        if(new_rank != rank){
            rank = new_rank;
            printf("\n");
        }

        if(determine_leaf_or_internal(temp)){
            current_page->page_type = 3;
            file_read_page(temp, current_page);
            for(i = 0; i < current_page->lpage.number_of_keys; i++){
                printf("%ld ", current_page->lpage.records[i].key);
            }
        }
        

        if(!determine_leaf_or_internal(temp)){
            current_page->page_type = 4;
            file_read_page(temp, current_page);

            for(i = 0; i < current_page->ipage.number_of_keys; i++){
               printf("%ld ", current_page->ipage.entries[i].key);
            }

            enqueue(current_page->ipage.extra_page_number);
            for(i = 0; i < current_page->ipage.number_of_keys; i++){
                enqueue(current_page->ipage.entries[i].page_number);
            }
        }
        printf("| ");
    }
    printf("\n");

    free(current_page);
    free(hp);
}


/* Finds and returns the record to which
 * a key refers.
*/

pagenum_t find_leaf(pagenum_t root, int64_t key, bool verbose){
    int i = 0;
    internal_page* ipage;
    leaf_page* lpage;
    pagenum_t next;


    if(root == 0){
        if(verbose)
            printf("Empty tree\n");
        return root;
    }

    if(determine_leaf_or_internal(root))
        return root;

    page_t* current_page = alloc_page_t();
    
    current_page->page_type = 4;
    file_read_page(root, current_page);
    ipage = &current_page->ipage;

    page_t* next_page = alloc_page_t();

    while(!ipage->is_leaf){
        if(verbose){
            printf("[");
            for(i = 0; i < ipage->number_of_keys - 1; i++)
                printf(" %ld", ipage->entries[i].key);
            printf("%ld]\n", ipage->entries[i].key);
        }
        i = 0;
        while(i < ipage->number_of_keys){
            if(key >= ipage->entries[i].key) i++;
            else break;
        }
        if(verbose)
            printf("%d ->\n", i);
        if(i > 0)
            next = ipage->entries[i - 1].page_number;
        else 
            next = ipage->extra_page_number;

        int is_leaf = determine_leaf_or_internal(next);

        

        if(!is_leaf){
            next_page->page_type = 4;
            file_read_page(next, next_page);
            ipage = &next_page->ipage;
        }
        else{
            next_page->page_type = 3;
            file_read_page(next, next_page);
            lpage = &next_page->lpage;
            break;
        }
    }

    if(verbose){
        printf("Leaf [");
        for(i = 0; i < current_page->lpage.number_of_keys - 1; i++){
            printf("%ld ", current_page->lpage.records[i].key);
        }
        printf("%ld]\n", lpage->records[i].key);
    }

    //printf("key found in %ld page\n", next);
    free(current_page);
    free(next_page);
    return next;
}

pagenum_t find(pagenum_t root, int64_t key, bool verbose){
    if(root == 0)
        return root;    //empty tree

    int i = 0;

    page_t* current_page = alloc_page_t();

    pagenum_t pnum = find_leaf(root, key, verbose);
    current_page->page_type = 3;
    file_read_page(pnum, current_page);

    pagenum_t ret = current_page->lpage.right_sibling_page_number;
    int num = current_page->lpage.number_of_keys;
    
    for(i = 0; i < num; i++){
        if(current_page->lpage.records[i].key == key){
            ret = 1;
            free(current_page);
            
            return ret;
        }
    }
    
    if(i == num)
        ret = pnum;    

    else
        ret = current_page->lpage.right_sibling_page_number;

    free(current_page);

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

pagenum_t make_node(void){

    pagenum_t new_node = file_alloc_page();

    page_t* current_page = alloc_page_t();

    internal_page* new_root_page = (internal_page*)malloc(sizeof(internal_page));

    if(new_root_page == NULL){
        perror("Node Creation Failed\n");
        exit(EXIT_FAILURE);
    }

    new_root_page->parent_page_number = 0;
    new_root_page->is_leaf = 0;
    new_root_page->number_of_keys = 0;
    new_root_page->extra_page_number = 0;
    
    current_page->page_type = 4;
    current_page->ipage = *new_root_page;
    file_write_page(new_node, current_page);

    free(current_page);

    return new_node;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
*/

pagenum_t make_leaf(void){
    pagenum_t leaf_pagenum = file_alloc_page();
    //printf("%ld page allocated\n\n", leaf_pagenum);

    page_t* current_page = alloc_page_t();

    leaf_page* leaf_node = (leaf_page*)malloc(sizeof(leaf_page));
    leaf_node->parent_page_number = 0;
    leaf_node->is_leaf = 1;
    leaf_node->number_of_keys = 0;
    leaf_node->right_sibling_page_number = 0;

    current_page->lpage = *leaf_node;
    current_page->page_type = 3;
    //printf("Have to write leaf page\n");
    file_write_page(leaf_pagenum, current_page);
    //printf("Did it write leaf page?\n");

    free(current_page);

    return leaf_pagenum;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
*/

int get_left_index(pagenum_t parent, pagenum_t left){

    page_t* current_page = alloc_page_t();

    int left_index = 0;
    internal_page* parent_page;
    current_page->page_type = 4;
    file_read_page(parent, current_page);
    parent_page = &current_page->ipage;

    if(parent_page->extra_page_number == left){
        free(current_page);
        return -1;
    }
        

    while(left_index <= parent_page->number_of_keys &&
            parent_page->entries[left_index].page_number != left)
        left_index++;

    free(current_page);
    
    return left_index;
}


/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
*/

pagenum_t insert_into_leaf(pagenum_t pagenum, int64_t key, char* value){
    int i, insertion_point;

    page_t* current_page = alloc_page_t();

    current_page->page_type = 3;
    file_read_page(pagenum, current_page);
    leaf_page* leaf = &current_page->lpage;
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
    file_write_page(pagenum, current_page);

   // printf("Key written in leaf page %ld\n", pagenum);

    free(current_page);

    //printf("%ld inserted\n", key);

    return pagenum;
}

/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
*/

pagenum_t insert_into_leaf_after_splitting(pagenum_t root, pagenum_t leaf, int64_t key, char* value){
    pagenum_t new_leaf;
    int64_t* temp_keys = (int64_t*)malloc(sizeof(int64_t) * leaf_order);
    char temp_values[leaf_order][120];
    int insertion_index, split, i, j;
    int64_t new_key;

    new_leaf = make_leaf();
    //printf("New leaf made. New leaf page number : %ld\n\n", new_leaf);

    page_t* current_page = alloc_page_t();
    current_page->page_type = 3;
    file_read_page(leaf, current_page);

    insertion_index = 0;
    
    leaf_page *lpage = &current_page->lpage;
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

    page_t* new_page = alloc_page_t();
    leaf_page* new_lpage;
    new_page->page_type = 3;
    file_read_page(new_leaf, new_page);
    new_lpage = &new_page->lpage;

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

    file_write_page(new_leaf, new_page);
    file_write_page(leaf, current_page);

    //printf("Successfully splitted leaves.\n\n");

    free(new_page);
    free(current_page);

    return insert_into_parent(root, leaf, new_key, new_leaf);
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
*/

pagenum_t insert_into_node(pagenum_t root, pagenum_t n, int left_index, int64_t key, pagenum_t right){
    int i;

    page_t* current_page = alloc_page_t();

    current_page->page_type = 4;
    internal_page* npage;
    file_read_page(n, current_page);
    npage = &current_page->ipage;

    for(i = npage->number_of_keys; i > left_index; i--){
        npage->entries[i + 1] = npage->entries[i];
    }

    npage->entries[left_index + 1].key = key;
    npage->entries[left_index + 1].page_number = right;
    npage->number_of_keys++;

    file_write_page(n, current_page);
    free(current_page);

    return root;
}



/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
*/

pagenum_t insert_into_node_after_splitting(pagenum_t root, pagenum_t old_node, int left_index, int64_t key, pagenum_t right){
    //printf("insert_into_node_after_splittimg\n");
    int i, j, split;
    int64_t k_prime;
    pagenum_t new_node, child;
    internal_entry temp[internal_order];

    page_t* current_page = alloc_page_t();

    internal_page* old_node_page;
    current_page->page_type = 4;
    file_read_page(old_node, current_page);
    old_node_page = &current_page->ipage;

    for(i = 0, j = 0; i < internal_order; i++, j++){
        if(j == left_index + 1) j++;
        temp[j] = old_node_page->entries[i];
    }

    temp[left_index + 1].key = key;
    temp[left_index + 1].page_number = right;

    split = cut(internal_order);
    new_node = make_node();
    old_node_page->number_of_keys = 0;

    page_t* new_page = alloc_page_t();

    internal_page* new_node_page;
    new_page->page_type = 4;
    file_read_page(new_node, new_page);
    new_node_page = &new_page->ipage;

   

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

    page_t* child_page = alloc_page_t();
    page_t* child_not_leaf = alloc_page_t();

    if(determine_leaf_or_internal(child)){
        child_page->page_type = 3;
        file_read_page(child, child_page);
        child_page->lpage.parent_page_number = new_node;
        file_write_page(child, child_page);
    }
    else{
        child_not_leaf->page_type = 4;
        file_read_page(child, child_not_leaf);
        child_not_leaf->ipage.parent_page_number = new_node;
        file_write_page(child, child_not_leaf);
    }
    
    for(i = 0; i < new_node_page->number_of_keys; i++){
        child = new_node_page->entries[i].page_number;
        
        if(determine_leaf_or_internal(child)){
            file_read_page(child, child_page);
            child_page->lpage.parent_page_number = new_node;
            file_write_page(child, child_page);
        }

        else{
            file_read_page(child, child_not_leaf);
            child_not_leaf->ipage.parent_page_number = new_node;
            file_write_page(child, child_not_leaf);
        }
    }

    file_write_page(new_node, new_page);
    file_write_page(old_node, current_page);

    free(current_page);
    free(new_page);
    free(child_page);
    free(child_not_leaf);

    return insert_into_parent(root, old_node, k_prime, new_node);
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
*/

pagenum_t insert_into_parent(pagenum_t root, pagenum_t left, int64_t key, pagenum_t right){
    
   // printf("Insert into parent : %ld\n", root);

    int left_index;
    pagenum_t parent;

    leaf_page* left_child_page;

    internal_page* parent_page;

    page_t* current_page = alloc_page_t();
    if(determine_leaf_or_internal(left)){
        current_page->page_type = 3;
        file_read_page(left, current_page);
        parent = current_page->lpage.parent_page_number;
    }
    else{
        current_page->page_type = 4;
        //printf("pp")
        file_read_page(left, current_page);
        parent = current_page->ipage.parent_page_number;
    }
    
    //printf("parent pagenum : %ld\n", parent);

    free(current_page);

    if(parent == 0){
        return insert_into_new_root(left, key, right);
    }

    page_t* p_page = alloc_page_t();

    p_page->page_type = 4;
    file_read_page(parent, p_page);

    parent_page = &p_page->ipage;

    left_index = get_left_index(parent, left);

    int temp = parent_page->number_of_keys;

    free(p_page); 

    if(temp < internal_order - 1)
        return insert_into_node(root, parent, left_index, key, right);

    return insert_into_node_after_splitting(root, parent, left_index, key, right);
}

/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
*/

pagenum_t insert_into_new_root(pagenum_t left, int64_t key, pagenum_t right){

    pagenum_t root = make_node();

    //printf("new root made : %ld\n", root);

    page_t* current_page = alloc_page_t();

    internal_page* rpage;
    current_page->page_type = 4;
    file_read_page(root, current_page);

    rpage = &current_page->ipage;

    rpage->extra_page_number = left;
    rpage->entries[0].key = key;
    rpage->entries[0].page_number = right;
    rpage->number_of_keys++;
    rpage->parent_page_number = 0;

    file_write_page(root, current_page);

    current_page->page_type = 3;
    file_read_page(left, current_page);
    current_page->lpage.parent_page_number = root;
   //("left child's new parent : %ld\n", current_page->lpage->parent_page_number);
    file_write_page(left, current_page);

    free(current_page);

    page_t* page = alloc_page_t();

    page->page_type = 3;
    file_read_page(right, page);
    page->lpage.parent_page_number = root;
    file_write_page(right, page);

    page->page_type = 1;
    file_read_page(0, page);
    page->hpage.root_page_number = root;
    file_write_page(0, page);

    free(page);

    return root;
}



/* First insertion:
 * start a new tree.
*/

pagenum_t start_new_tree(int64_t key, char* value){ 
    //printf("Start new tree func called\n");
    
    pagenum_t root = make_leaf();
   // printf("New Tree made. Root Page number : %ld\n\n", root);

    page_t* current_page = alloc_page_t();
    //printf("page_t created\n");

    leaf_page* rpage;
    current_page->page_type = 3;
    file_read_page(root, current_page);
    rpage = &current_page->lpage;

    rpage->records[0].key = key;
    strcpy(rpage->records[0].value, value);
    rpage->number_of_keys++;

    //printf("before write func check if the key is right : %ld\n", rpage->records[0].key);
    file_write_page(root, current_page);

    current_page->page_type = 1;
    file_read_page(0, current_page);
    current_page->hpage.root_page_number = root;
    //printf("trying to write root page : %ld\n", root);
    file_write_page(0, current_page);

    free(current_page);

    return root;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
*/

pagenum_t insert(pagenum_t root, int64_t key, char* value){

    //printf("Master insert %ld func called\n", key);
    //printf("Root page number : %ld\n", root);
    //printf("root page num : %ld\n\n", root);
    
    pagenum_t leaf;


    if(find(root, key, false) == 1){
        //printf("found key so don't need to insert key\n\n");
        printf("\nKey already exists\n");
        return root;
    }
        

    if(root == 0)
        return start_new_tree(key, value);

    leaf = find_leaf(root, key, false);
  //  printf("Leaf page number found : %ld\n\n", leaf);

    page_t* current_page = alloc_page_t();

    leaf_page* lpage;
    current_page->page_type = 3;
    file_read_page(leaf, current_page);
    lpage = &current_page->lpage;

    int num = lpage->number_of_keys;

    free(current_page);

    if(num < leaf_order - 1){
        leaf = insert_into_leaf(leaf, key, value);
        return root;
    }

    return insert_into_leaf_after_splitting(root, leaf, key, value);
}

// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
*/

int get_neighbor_index(pagenum_t n){

    int i;

    page_t* page = alloc_page_t();
    page->page_type = 3;
    file_read_page(n, page);

    page_t* parent = alloc_page_t();
    parent->page_type = 4;
    file_read_page(page->lpage.parent_page_number, parent);
    
    free(page);

    if(n == parent->ipage.extra_page_number){
        free(parent);
        return -2;
    }

    for(i = 0; i < parent->ipage.number_of_keys; i++){
        if(parent->ipage.entries[i].page_number == n){
            free(parent);
            return i - 1;
        }
    }
}


pagenum_t remove_entry_from_node(pagenum_t n, int64_t key){
    int i, num_pointers;

    i = 0;
    page_t* current_page = alloc_page_t();
    if(determine_leaf_or_internal(n)){
        current_page->page_type = 3;
        file_read_page(n, current_page);
        leaf_page* lpage = &current_page->lpage;

        while(lpage->records[i].key != key){
            i++;
        }
        for(++i; i < lpage->number_of_keys; i++){
            lpage->records[i - 1] = lpage->records[i];
        }

        lpage->number_of_keys--;

        file_write_page(n, current_page);
    }

    else{
        current_page->page_type = 4;
        file_read_page(n, current_page);
        internal_page* ipage = &current_page->ipage;

        while(ipage->entries[i].key != key){
            i++;
        }

        for(++i; i < ipage->number_of_keys; i++){
            ipage->entries[i - 1] = ipage->entries[i];
        }

        ipage->number_of_keys--;

        file_write_page(n, current_page);
    }

    free(current_page);

    return n;
}


pagenum_t adjust_root(pagenum_t root){
    pagenum_t new_root;

    page_t* current_page = alloc_page_t();
    current_page->page_type = 4;
    file_read_page(root, current_page);
    internal_page* root_page = &current_page->ipage;

    //Case : nonempty root
    if(root_page->number_of_keys > 0){
        free(current_page);
        return root;
    }

    page_t* temp = alloc_page_t();
    //Case : empty root
    //If it has a child, promote the first child as the new root
    if(!root_page->is_leaf){
        new_root = root_page->extra_page_number;
        
        temp->page_type = 4;
        file_read_page(new_root, temp);
        temp->ipage.parent_page_number = 0;

        file_write_page(new_root, temp);

        temp->page_type = 1;
        file_read_page(0, temp);
        temp->hpage.root_page_number = new_root;
        file_write_page(0, temp);
    }

    //If it is a leaf then the whole tree is empty.
    else{
        new_root = 0;

        temp->page_type = 1;
        file_read_page(0, temp);
        temp->hpage.root_page_number = 0;
        file_write_page(0, temp);
    }

    free(temp);
    free(current_page);

    return new_root;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
*/

pagenum_t coalesce_nodes(pagenum_t root, pagenum_t n, pagenum_t neighbor, int neighbor_index, int64_t k_prime){

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
    page_t* neighbor_page = alloc_page_t();
    if(determine_leaf_or_internal(neighbor)){
        neighbor_page->page_type = 3;
        file_read_page(neighbor, neighbor_page);
        leaf_page* neighbor_leaf = &neighbor_page->lpage;

        neighbor_insertion_index = neighbor_leaf->number_of_keys;

        parent = neighbor_leaf->parent_page_number;

        page_t* n_page = alloc_page_t();
        n_page->page_type = 3;
        file_read_page(n, n_page);
        leaf_page* n_leaf = &n_page->lpage;

        for(i = neighbor_insertion_index, j = 0; j < n_leaf->number_of_keys; i++, j++){
            neighbor_leaf->records[i] = n_leaf->records[j];
            neighbor_leaf->number_of_keys++;
        }

        neighbor_leaf->right_sibling_page_number = n_leaf->right_sibling_page_number;

        file_write_page(neighbor, neighbor_page);
        file_free_page(n);

        free(n_page);
    }

    else{
        neighbor_page->page_type = 4;
        file_read_page(neighbor, neighbor_page);
        internal_page* neighbor_internal = &neighbor_page->ipage;

        neighbor_insertion_index = neighbor_internal->number_of_keys;

        neighbor_internal->entries[neighbor_insertion_index].key = k_prime;
        neighbor_internal->number_of_keys++;

        parent = neighbor_internal->parent_page_number;

        page_t* n_page = alloc_page_t();
        n_page->page_type = 4;
        file_read_page(n, n_page);
        internal_page* n_internal = &n_page->ipage;

        n_end = n_internal->number_of_keys;

        neighbor_internal->entries[neighbor_insertion_index].page_number = n_internal->extra_page_number;

        for(i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++){
            neighbor_internal->entries[i] = n_internal->entries[j];
            neighbor_internal->number_of_keys++;
            n_internal->number_of_keys--;
        }

        page_t* temp_child = alloc_page_t();
        if(determine_leaf_or_internal(neighbor_internal->extra_page_number))
            temp_child->page_type = 3;
        else
            temp_child->page_type = 4;
        
        for(i = neighbor_insertion_index; i < neighbor_internal->number_of_keys; i++){
            if(temp_child->page_type = 4){
                file_read_page(neighbor_internal->entries[i].page_number, temp_child);
                temp_child->ipage.parent_page_number = neighbor;
                file_write_page(neighbor_internal->entries[i].page_number, temp_child);
            }
            else if(temp_child->page_type = 3){
                file_read_page(neighbor_internal->entries[i].page_number, temp_child);
                temp_child->lpage.parent_page_number = neighbor;
                file_write_page(neighbor_internal->entries[i].page_number, temp_child);
            }
        }

        file_write_page(neighbor, neighbor_page);
        file_write_page(n, n_page);

        file_free_page(n);

        free(temp_child);
        free(n_page);
    }

    free(neighbor_page);
    root = delete_entry(root, parent, k_prime);
    
    return root;
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
*/

pagenum_t redistribute_nodes(pagenum_t root, pagenum_t n, pagenum_t neighbor, int neighbor_index, int k_prime_index, int64_t k_prime){
    
    int i;
    pagenum_t temp;

    page_t* neighbor_internal = alloc_page_t();
    neighbor_internal->page_type = 4;
    file_read_page(neighbor, neighbor_internal);
    internal_page* neigh = &neighbor_internal->ipage;

    page_t* n_internal = alloc_page_t();
    n_internal->page_type = 4;
    file_read_page(n, n_internal);
    internal_page* npage = &n_internal->ipage;

    //Case : n has a neighbor to the left.
    //Pull the neighbor's last key_pointer pair over from the neighbor's right end to n's left end
    if(neighbor_index != -2){
        npage->entries[0].page_number = npage->extra_page_number;
        npage->extra_page_number = neigh->entries[neigh->number_of_keys - 1].page_number;
        npage->entries[0].key = k_prime;

        temp = npage->extra_page_number;
        page_t* tmp = alloc_page_t();
        tmp->page_type = 3;
        file_read_page(temp, tmp);

        tmp->lpage.parent_page_number = n;
        file_write_page(temp, tmp);

        tmp->page_type = 4;
        file_read_page(npage->parent_page_number, tmp);
        tmp->ipage.entries[k_prime_index] = neigh->entries[neigh->number_of_keys - 1];
        file_write_page(npage->parent_page_number, tmp);

        free(tmp);

        npage->number_of_keys++;
        file_write_page(n, n_internal);

        neigh->number_of_keys--;
        file_write_page(neighbor, neighbor_internal);
    }

    else{
        npage->entries[0].key = k_prime;
        npage->entries[0].page_number = neigh->extra_page_number;
        
        temp = npage->entries[0].page_number;
        page_t* tmp = alloc_page_t();
        tmp->page_type = 3;
        file_read_page(temp, tmp);

        tmp->lpage.parent_page_number = n;
        file_write_page(temp, tmp);

        temp = npage->parent_page_number;
        tmp->page_type = 4;
        file_read_page(temp, tmp);
        tmp->ipage.entries[k_prime_index].key = neigh->entries[0].key;
        file_write_page(temp, tmp);

        free(tmp);

        npage->number_of_keys++;
        file_write_page(n, n_internal);

        neigh->extra_page_number = neigh->entries[0].page_number;
        for(i = 0; i < neigh->number_of_keys - 1; i++){
            neigh->entries[i] = neigh->entries[i + 1];
        }

        neigh->number_of_keys--;
        file_write_page(neighbor, neighbor_internal);
    }

    return root;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
*/

pagenum_t delete_entry(pagenum_t root, pagenum_t n, int64_t key){
    int64_t min_keys;
    pagenum_t neighbor;
    int neighbor_index;
    int k_prime_index;
    int64_t k_prime;
    int capacity;

    //Remove key and pointer from node
    n = remove_entry_from_node(n, key);

    //Case : deletion from the root
    if(n == root)
        return adjust_root(root);

    //Case : node stays at or above minimum
    page_t* current_page = alloc_page_t();
    current_page->page_type = 4;
    file_read_page(n, current_page);
    internal_page* ipage = &current_page->ipage;
    int lpage_num = ipage->number_of_keys;
    free(current_page);

    if(lpage_num > 0)
        return root;


    page_t* parent = alloc_page_t();
    parent->page_type = 4;
    file_read_page(ipage->parent_page_number, parent);
    internal_page* parent_page = &parent->ipage;

    //Case : node falls below miminum.
    //Either coalescence or redistribution is needed
    neighbor_index = get_neighbor_index(n);
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

    page_t* temp = alloc_page_t();
    temp->page_type = 3;
    file_read_page(neighbor, temp);
    pagenum_t neighbor_keynum = temp->lpage.number_of_keys;
    free(temp);

    //Coalescence
    if(determine_leaf_or_internal(n) | neighbor_keynum != internal_order - 1){
        free(parent);
        return coalesce_nodes(root, n, neighbor, neighbor_index, k_prime);
    } 

    else{
        free(parent);
        return redistribute_nodes(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
    }
}


/* Master deletion function.
*/

pagenum_t delete(pagenum_t root, int64_t key){
    bool verbose = false;
    pagenum_t key_leaf;

    key_leaf = find(root, key, verbose);
    if(key_leaf != 1){
        printf("Key doesn't exist so can't delete key\n\n");
        return root;
    }
    else{
        key_leaf = find_leaf(root, key, verbose);
        root = delete_entry(root, key_leaf, key);

        page_t* headerpage = alloc_page_t();
        headerpage->page_type = 1;
        file_read_page(0, headerpage);
        headerpage->hpage.root_page_number = root;
        file_write_page(0, headerpage);

        free(headerpage);
    }

    return root;
}

/*
*/
int open_table(char* pathname){
    fp = fopen(pathname, "r+");

    if(fp == NULL){
        fp = fopen(pathname, "w+");
    }

    fseeko(fp, 0, SEEK_END);
    int file_size = ftell(fp);

    if(file_size == 0){
        init_page();
        table_id++;
        return table_id;
    }

    else{
        page_t* hpage = alloc_page_t();
        hpage->page_type = 1;
        file_read_page(0, hpage);

        printf("Free page number : %ld\n", hpage->hpage.free_page_number);
        printf("Root page number : %ld\n", hpage->hpage.root_page_number);
        printf("Number of pages : %ld\n", hpage->hpage.number_of_pages); 

        free(hpage);   
    }

    
}

int db_insert(int64_t key, char* value){
    //printf("Successfully called db_insert func\n");
    pagenum_t root;

    page_t* current_page = alloc_page_t();
    
    current_page->page_type = 1;
    //printf("Before file_read_page func, page type : %d\n", current_page->page_type);
    file_read_page(0, current_page);
    //printf("Successfully read root page info\n");
    root = current_page->hpage.root_page_number;
    //printf("root page num : %ld\n", root);

    pagenum_t res = insert(root, key, value);

    if(res == 1){
        free(current_page);
        return 1;
    }

    free(current_page);

    return 0;
}


int db_delete(int64_t key){
    page_t* headerpage = alloc_page_t();
    headerpage->page_type = 1;
    file_read_page(0, headerpage);

    pagenum_t root = headerpage->hpage.root_page_number;

    root = delete(root, key);
    free(headerpage);

    return 0;
}

int db_find(int64_t key, char* ret_val){
    page_t* headerpage = alloc_page_t();
    headerpage->page_type = 1;
    file_read_page(0, headerpage);

    pagenum_t root = headerpage->hpage.root_page_number;

    pagenum_t res = find(root, key, false);

    if(res == 1){
        headerpage->page_type = 3;
        file_read_page(find_leaf(root, key, false), headerpage);

        int num = headerpage->lpage.number_of_keys;
        for(int i = 0; i < num; i++){
            if(key == headerpage->lpage.records[i].key){
                ret_val = headerpage->lpage.records[i].value;
                return 0;
            }
        }
    }

    return 1;
}