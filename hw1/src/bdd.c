#include <stdlib.h>
#include <stdio.h>

#include "bdd.h"
#include "debug.h"

/*
 * Macros that take a pointer to a BDD node and obtain pointers to its left
 * and right child nodes, taking into account the fact that a node N at level l
 * also implicitly represents nodes at levels l' > l whose left and right children
 * are equal (to N).
 *
 * You might find it useful to define macros to do other commonly occurring things;
 * such as converting between BDD node pointers and indices in the BDD node table.
 */
#define LEFT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->left)
#define RIGHT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->right)

int bdd_node_size = 0;

int hash(int level, int left, int right) {
    return (level + left + right) % BDD_HASH_SIZE;
}

/**
 * Look up, in the node table, a BDD node having the specified level and children,
 * inserting a new node if a matching node does not already exist.
 * The returned value is the index of the existing node or of the newly inserted node.
 *
 * The function aborts if the arguments passed are out-of-bounds.
 */
int bdd_lookup(int level, int left, int right) {
    // TO BE IMPLEMENTED

    if(left < 0 || right < 0 || left >= BDD_NODES_MAX || right >= BDD_NODES_MAX || level < 0 || level >= BDD_LEVELS_MAX) return -1;

    if(left == right) return left;

    int hashcode = hash(level, left, right);

    int iter = 0;

    // if hashmap contains the node already
    BDD_NODE *key = *(bdd_hash_map + hashcode + iter);

    while(key != NULL) {
        int advanceKey = (hashcode + iter) % BDD_HASH_SIZE;
        key = *(bdd_hash_map + advanceKey);
        // compare if this node and the input node are the same
        int nodelevel = (int)((key->level) - '0');
        int nodeleftval = key -> left;
        int noderightval = key -> right;

        // compare with input arguments
        if(nodelevel == level && nodeleftval == left && noderightval == right) {
            return advanceKey;
        }

        else {
            iter++;
        }
    }

    // node about to be inserted is not a leaf
    BDD_NODE *newnode = bdd_nodes + (256 + bdd_node_size);
    if(level > 0) {
        newnode -> level = (char)(level + '0');
        newnode -> left = left;
        newnode -> right = right;
        bdd_node_size++;
    }

    // if hashmap doesn't contain the node, insert into hashmap
    iter = 0;
    int advanceKey = (hashcode + iter) % BDD_HASH_SIZE;
    key = *(bdd_hash_map + advanceKey);

    while(key != NULL) {
        iter++;
        advanceKey = (hashcode + iter) % BDD_HASH_SIZE;
        key = *(bdd_hash_map + advanceKey);
    }

    // insert node pointer into hashmap
    *(bdd_hash_map + advanceKey) = newnode;

    return bdd_node_size + BDD_NUM_LEAVES - 1;
    // level is zero, negative was handled before
}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    return NULL;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

BDD_NODE *bdd_deserialize(FILE *in) {
    // TO BE IMPLEMENTED
    return NULL;
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    return 0;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    // TO BE IMPLEMENTED
    return NULL;
}
