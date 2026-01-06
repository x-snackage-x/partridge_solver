#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum { OK } ERROR_CODES;

// DYNAMIC ARRAY
typedef struct {
    size_t elem_size;
    size_t dynarr_size;
    size_t dynarr_capacity;
    float growth_fac;
    char* ptr_first_elem;
} dynarr_head;

void dynarr_init(dynarr_head* const ptr_head);
char* dynarr_push(dynarr_head* const ptr_head, void const* element);
void dynarr_free(dynarr_head* const ptr_head);
// internals
void dynarr_expand(dynarr_head* const ptr_head);

// LINKED LIST
typedef enum {
    // Standard primitive types
    NODE_BOOL = 1,
    NODE_CHAR,
    NODE_UCHAR,
    NODE_SHORT,
    NODE_USHORT,
    NODE_INT,
    NODE_UINT,
    NODE_LONG,
    NODE_ULONG,
    NODE_LONGLONG,
    NODE_ULONGLONG,
    NODE_FLOAT,
    NODE_DOUBLE,
    NODE_LONGDOUBLE,

    // Fixed-width integer types
    NODE_INT8,
    NODE_UINT8,
    NODE_INT16,
    NODE_UINT16,
    NODE_INT32,
    NODE_UINT32,
    NODE_INT64,
    NODE_UINT64,

    // Reserved range for user-defined types
    NODE_USER_START = 1000
} node_type;

typedef enum { OPEN = 1, CIRCULAR } list_type;
typedef struct list_node list_node;
typedef struct {
    list_type list_type;
    size_t list_len;
    list_node* ptr_first_node;
    list_node* ptr_last_node;
} linked_list_head;

struct list_node {
    char* data;
    node_type dtype;
    size_t data_size;
    list_node* next_node;
    list_node* previous_node;
};

typedef struct {
    bool node_found;
    list_node* found_node_ptr;
} list_node_return;

void linlst_init(linked_list_head* const ptr_head);
void linlst_append_node(linked_list_head* const ptr_head,
                        node_type dtype,
                        size_t data_size,
                        void const* data);
void linlst_prepend_node(linked_list_head* const ptr_head,
                         node_type dtype,
                         size_t data_size,
                         void const* data);
void linlst_insert_node(linked_list_head* const ptr_head,
                        list_node* const pre_node,
                        node_type dtype,
                        size_t data_size,
                        void const* data);
void linlst_get_node(linked_list_head* const ptr_head,
                     list_node_return* found_node_struct,
                     uint8_t index);
void linlst_delete_node(linked_list_head* const ptr_head,
                        list_node* const node);
void linlst_delete_list(linked_list_head* const ptr_head);
// internals
list_node* linlst_prepare_data_node(node_type dtype,
                                    size_t data_size,
                                    void const* data);

// TREE
