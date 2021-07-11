#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

// TYPES.

/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */


/* Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */

typedef uint64_t pagenum_t;

typedef struct queue{
    pagenum_t pagenum;
    struct queue* next;
       
}queue;

typedef struct table{
    char pathname[20];
    int table_id;
}table;


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
extern int order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */


/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */


// FUNCTION PROTOTYPES.

// Output and utility.

void show_menu();

void init_queue(pagenum_t new_node);
void enqueue(pagenum_t new_node);
queue* dequeue();

int path_to_root(pagenum_t root, int table_id, pagenum_t child);
void print_leaves( pagenum_t root );
void print_tree();
pagenum_t find_leaf(pagenum_t root, int table_id, int64_t key);
pagenum_t find(pagenum_t root, int table_id, int64_t key);
int cut( int length );


// Insertion.
pagenum_t make_node(int table_id);
pagenum_t make_leaf(int table_id);
int get_left_index(pagenum_t parent, pagenum_t left, int table_id);
pagenum_t insert_into_leaf(pagenum_t pagenum, int table_id, int64_t key, char* value );
pagenum_t insert_into_leaf_after_splitting(pagenum_t root, int table_id, pagenum_t leaf, int64_t key,
                                        char* value);
pagenum_t insert_into_node(pagenum_t root, int table_id, pagenum_t parent, 
        int left_index, int64_t key, pagenum_t right);
pagenum_t insert_into_node_after_splitting(pagenum_t root, int table_id, pagenum_t parent,
                                        int left_index, int64_t key, pagenum_t right);
pagenum_t insert_into_parent(pagenum_t root, int table_id, pagenum_t left, int64_t key, pagenum_t right);
pagenum_t insert_into_new_root(pagenum_t left, int table_id, int64_t key, pagenum_t right);
pagenum_t start_new_tree(int64_t key, int table_id, char * value);
pagenum_t insert( pagenum_t root, int table_id, int64_t key, char* value );


// Deletion.
int get_neighbor_index( pagenum_t n, int table_id );
pagenum_t adjust_root(pagenum_t root, int table_id);
pagenum_t remove_entry_from_node(pagenum_t n, int table_id, int64_t key);
pagenum_t coalesce_nodes(pagenum_t root, int table_id, pagenum_t n, pagenum_t neighbor,
                      int neighbor_index, int64_t k_prime);
pagenum_t redistribute_nodes(pagenum_t root, int table_id, pagenum_t n, pagenum_t neighbor, int neighbor_index, int k_prime_index, int64_t k_prime);
pagenum_t delete_entry(pagenum_t root, int table_id, pagenum_t n, int64_t key);
pagenum_t delete(pagenum_t root, int table_id, int64_t key);


// Index Layer
int open_table(char* pathname);
int db_insert(int table_id, int64_t key, char* value);
int db_delete(int table_id, int64_t key);
int db_find(int table_id, int64_t key, char* ret_val);
int init_db(int num_buf);
int close_table(int table_id);
int shutdown_db();
#endif /* __BPT_H__*/


