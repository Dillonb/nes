#include "set.h"
#include <stdlib.h>

address_tree* new_address_tree() {
    address_tree* tree =  malloc(sizeof(address_tree));
    tree->root = NULL;
    return tree;
}

void delete_address_tree_node(address_tree_node* root) {
    if (root->lt != NULL) {
        delete_address_tree_node(root->lt);
    }
    if (root->gt != NULL) {
        delete_address_tree_node(root->gt);
    }
    free(root);
}

void delete_address_tree(address_tree* root) {
    if (root->root != NULL) {
        delete_address_tree_node(root->root);
    }
    free(root);
}

address_tree_node* new_address_node(uint16_t address) {
    address_tree_node* node = malloc(sizeof(address_tree_node));
    node->address = address;
    node->gt = NULL;
    node->lt = NULL;
    return node;
}

void insert_address_child(address_tree_node* node, uint16_t address) {
    if (address > node->address) {
        if (node->gt == NULL) {
            node->gt = new_address_node(address);
        }
        else {
            insert_address_child(node->gt, address);
        }
    }
    else {
        if (node->lt == NULL) {
            node->lt = new_address_node(address);
        }
        else {
            insert_address_child(node->lt, address);
        }
    }

}

void insert_to_address_tree(address_tree* root, uint16_t address) {
    if (root->root == NULL) {
        root->root = new_address_node(address);
    }
    else {
        insert_address_child(root->root, address);
    }
}


bool address_tree_node_contains(address_tree_node* node, uint16_t address) {
    if (node == NULL) {
        return false;
    }
    if (address == node->address) {
        return true;
    }
    else if (address > node->address) {
        return address_tree_node_contains(node->gt, address);
    }
    else {
        return address_tree_node_contains(node->lt, address);
    }
}
bool address_tree_contains(address_tree* root, uint16_t address) {
    if (root == NULL || root->root == NULL) {
        return false;
    }
    return address_tree_node_contains(root->root, address);
}
