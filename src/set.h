#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef struct address_tree_node_t {
    uint16_t address;
    struct address_tree_node_t* lt;
    struct address_tree_node_t* gt;
} address_tree_node;

typedef struct address_tree_t {
    address_tree_node* root;
} address_tree;

address_tree* new_address_tree();
void delete_address_tree(address_tree* root);
void insert_to_address_tree(address_tree* root, uint16_t address);
bool address_tree_contains(address_tree* root, uint16_t address);
