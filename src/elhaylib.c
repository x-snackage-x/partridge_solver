#include "elhaylib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DYNAMIC ARRAY
void dynarr_init(dynarr_head* const ptr_head) {
    assert(ptr_head->elem_size > 0 && "Element size must be greater zero.");
    if(ptr_head->dynarr_capacity <= 0) {
        ptr_head->dynarr_capacity = 10;
    }

    if(ptr_head->growth_fac <= 1.0f) {
        ptr_head->growth_fac = 2.0f;
    }

    ptr_head->dynarr_size = 0;
    ptr_head->ptr_first_elem =
        calloc(ptr_head->dynarr_capacity, ptr_head->elem_size);
}

char* dynarr_push(dynarr_head* const ptr_head, const void* element) {
    if(ptr_head->dynarr_size == ptr_head->dynarr_capacity) {
        dynarr_expand(ptr_head);
    }

    float test_input = *(float*)element;
    float test_first_element = *(float*)ptr_head->ptr_first_elem;
    test_first_element = test_first_element + test_input;

    void* dest = ptr_head->ptr_first_elem +
                 (ptr_head->dynarr_size * ptr_head->elem_size);
    memcpy(dest, element, ptr_head->elem_size);

    ++ptr_head->dynarr_size;

    return ptr_head->ptr_first_elem;
}

void dynarr_free(dynarr_head* const ptr_head) {
    free(ptr_head->ptr_first_elem);
    ptr_head->dynarr_size = 0;
    ptr_head->ptr_first_elem = NULL;
}

// internals
void dynarr_expand(dynarr_head* const ptr_head) {
    ptr_head->dynarr_capacity =
        (size_t)(ptr_head->dynarr_capacity * ptr_head->growth_fac);
    void* new_ptr = realloc(ptr_head->ptr_first_elem,
                            ptr_head->dynarr_capacity * ptr_head->elem_size);
    if(!new_ptr) {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    }
    ptr_head->ptr_first_elem = new_ptr;
}

// LINKED LIST
void linlst_init(linked_list_head* const ptr_head) {
    assert(ptr_head->list_type > 0 && "List-type must be defined");
    ptr_head->list_len = 0;
    ptr_head->ptr_first_node = NULL;
    ptr_head->ptr_last_node = NULL;
}

void linlst_append_node(linked_list_head* const ptr_head,
                        node_type dtype,
                        size_t data_size,
                        void const* data) {
    list_node* new_node_ptr = linlst_prepare_data_node(dtype, data_size, data);

    list_node* old_last_node_ptr = ptr_head->ptr_last_node;
    ptr_head->ptr_last_node = new_node_ptr;
    if(ptr_head->list_len == 0) {
        ptr_head->list_len++;
        ptr_head->ptr_first_node = new_node_ptr;

        switch(ptr_head->list_type) {
            case CIRCULAR:
                new_node_ptr->next_node = new_node_ptr;
                new_node_ptr->previous_node = new_node_ptr;
                break;
            case OPEN:
                break;
        }
        return;
    }

    old_last_node_ptr->next_node = new_node_ptr;
    new_node_ptr->previous_node = old_last_node_ptr;

    switch(ptr_head->list_type) {
        case CIRCULAR:
            new_node_ptr->next_node = ptr_head->ptr_first_node;
            ptr_head->ptr_last_node = new_node_ptr;
            ptr_head->ptr_first_node->previous_node = new_node_ptr;
            break;
        case OPEN:
            break;
    }

    ptr_head->list_len++;
}

void linlst_prepend_node(linked_list_head* const ptr_head,
                         node_type dtype,
                         size_t data_size,
                         void const* data) {
    list_node* new_node_ptr = linlst_prepare_data_node(dtype, data_size, data);

    list_node* old_first_node_ptr = ptr_head->ptr_first_node;

    ptr_head->ptr_first_node = new_node_ptr;
    if(ptr_head->list_len == 0) {
        ptr_head->list_len++;
        ptr_head->ptr_last_node = new_node_ptr;

        switch(ptr_head->list_type) {
            case CIRCULAR:
                new_node_ptr->next_node = new_node_ptr;
                new_node_ptr->previous_node = new_node_ptr;
                break;
            case OPEN:
                break;
        }

        return;
    }

    old_first_node_ptr->previous_node = new_node_ptr;
    new_node_ptr->next_node = old_first_node_ptr;

    switch(ptr_head->list_type) {
        case CIRCULAR:
            new_node_ptr->previous_node = ptr_head->ptr_last_node;
            ptr_head->ptr_last_node->next_node = new_node_ptr;
            break;
        case OPEN:
            break;
    }

    ptr_head->list_len++;
}

void linlst_insert_node(linked_list_head* const ptr_head,
                        list_node* const pre_node,
                        node_type dtype,
                        size_t data_size,
                        void const* data) {
    if(pre_node->next_node == NULL ||
       pre_node->next_node == ptr_head->ptr_first_node) {
        linlst_append_node(ptr_head, dtype, data_size, data);
        return;
    }

    if(pre_node == NULL) {
        linlst_prepend_node(ptr_head, dtype, data_size, data);
        return;
    }

    list_node* new_node_ptr = linlst_prepare_data_node(dtype, data_size, data);

    new_node_ptr->previous_node = pre_node;
    new_node_ptr->next_node = pre_node->next_node;
    pre_node->next_node = new_node_ptr;
    (new_node_ptr->next_node)->previous_node = new_node_ptr;

    ptr_head->list_len++;
}

void linlst_get_node(linked_list_head* const ptr_head,
                     list_node_return* found_node_struct,
                     uint8_t index) {
    if(ptr_head->list_len == 0 || ptr_head->list_len - 1 < index) {
        found_node_struct->found_node_ptr = NULL;
        found_node_struct->node_found = false;
        return;
    }

    found_node_struct->node_found = true;
    if(index == 0 && ptr_head->list_len != 0) {
        found_node_struct->found_node_ptr = ptr_head->ptr_first_node;
        return;
    }
    if(index == ptr_head->list_len - 1) {
        found_node_struct->found_node_ptr = ptr_head->ptr_last_node;
        return;
    }

    found_node_struct->found_node_ptr = ptr_head->ptr_first_node;
    for(uint8_t i = 1; i <= index; ++i) {
        found_node_struct->found_node_ptr =
            (found_node_struct->found_node_ptr)->next_node;
    }
}

void linlst_delete_node(linked_list_head* const ptr_head,
                        list_node* const node) {
    free(node->data);
    list_node* pre_node = node->previous_node;
    list_node* post_node = node->next_node;

    if(pre_node != NULL && post_node != NULL) {
        post_node->previous_node = pre_node;
        pre_node->next_node = post_node;
    } else if(pre_node != NULL && post_node == NULL) {
        pre_node->next_node = NULL;
    } else if(pre_node == NULL && post_node != NULL) {
        post_node->previous_node = NULL;
    }

    switch(ptr_head->list_type) {
        case OPEN:
            if(ptr_head->ptr_first_node == node) {
                if(pre_node == NULL) {
                    ptr_head->ptr_first_node = post_node;
                } else {
                    ptr_head->ptr_first_node = pre_node;
                }
            }

            if(ptr_head->ptr_last_node == node) {
                if(post_node == NULL) {
                    ptr_head->ptr_last_node = pre_node;
                } else {
                    ptr_head->ptr_last_node = post_node;
                }
            }
            break;

        case CIRCULAR:
            if(ptr_head->ptr_first_node == node) {
                ptr_head->ptr_first_node = post_node;
            }

            if(ptr_head->ptr_last_node == node) {
                ptr_head->ptr_last_node = pre_node;
            }
            break;
    }

    free(node);

    ptr_head->list_len--;
}

void linlst_delete_list(linked_list_head* const ptr_head) {
    list_node* cur_last_node = ptr_head->ptr_last_node;
    while(ptr_head->list_len > 0) {
        linlst_delete_node(ptr_head, cur_last_node);
        cur_last_node = ptr_head->ptr_last_node;
    }

    linlst_init(ptr_head);
}

// internals
list_node* linlst_prepare_data_node(node_type dtype,
                                    size_t data_size,
                                    void const* data) {
    list_node* new_node_ptr = calloc(1, sizeof(list_node));
    new_node_ptr->data = calloc(1, data_size);
    new_node_ptr->data_size = data_size;
    new_node_ptr->dtype = dtype;
    memcpy(new_node_ptr->data, data, data_size);

    return new_node_ptr;
}