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
        // compare if this node and the input node are the same
        int nodelevel = (int)((key->level) - '0');
        int nodeleftval = key -> left;
        int noderightval = key -> right;

        if(nodelevel == level && nodeleftval == left && noderightval == right) {
            return key - bdd_nodes;
        }

        iter++;

        int advancekey = (hashcode + iter) % BDD_HASH_SIZE;
        key = *(bdd_hash_map + advancekey);
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
    int advancekey = (hashcode + iter) % BDD_HASH_SIZE;
    key = *(bdd_hash_map + advancekey);

    while(key != NULL) {
        iter++;
        advancekey = (hashcode + iter) % BDD_HASH_SIZE;
        key = *(bdd_hash_map + advancekey);
    }

    // insert node pointer into hashmap
    *(bdd_hash_map + advancekey) = newnode;

    return bdd_node_size + BDD_NUM_LEAVES - 1;
}

int get_square_d(int w, int h) {
    int d = 1;
    int maxwh = w > h ? w : h;
    int pow = 2;
    if(maxwh < 4) return 1;
    while(pow * 2 <= maxwh) {
        d++;
        pow *= 2;
    }
    return d;
}

int left = 0, right = 0;
int from_raster_helper(int level, int loww, int lowh, int highw, int highh, unsigned char *raster, int k, int pow, int w, int h) {
    if(level == 0) {
        // handle base case
        unsigned char c = *(raster + (pow * lowh) + loww);
        int val = (int)c;

        // printf("%d %d %d %d %d\n", loww, highw, lowh, highh, val);

        if(highh >= h || highw >= w) {
            return 0;
        }
        return val;
    }

    if(k % 2 == 0) {
        left = from_raster_helper(level - 1, loww, lowh, highw, (lowh + highh) / 2, raster, k + 1, pow, w, h);
        right = from_raster_helper(level - 1, loww, ((lowh + highh) / 2) + 1, highw, highh, raster, k + 1, pow, w, h);
    }

    else {
        left = from_raster_helper(level - 1, loww, lowh, (loww + highw) / 2, highh, raster, k + 1, pow, w, h);
        right = from_raster_helper(level - 1, ((loww + highw) / 2) + 1, lowh, highw, highh, raster, k + 1, pow, w, h);
    }

    return bdd_lookup(level, left, right);

}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    // find highest d such that 2^d <= max(w,h)
    int d = get_square_d(w, h);
    printf("%d\n", d);
    int pow = 1;
    for(int i = 0; i < d; i++) {
        pow *= 2;
    }
    printf("pow: %d, w: %d, h: %d\n", pow, w, h);
    int index = from_raster_helper(2 * d, 0, 0, pow - 1,  pow - 1, raster, 0, pow, w, h);
    return bdd_nodes + BDD_NUM_LEAVES + index;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
}

int nodecount = 1;
int serializehelper(BDD_NODE *node, int index, FILE *out) {
    // post-order traversal
    // serialize left, right

    int nodelevel = (int)((node -> level) - '0');
    if(nodelevel == 0) {
        fputc('@', out);
        // convert int to char because i have to print 1 byte
        char c = (char)(index);
        fputc(c, out);
        *(bdd_index_map + index) = nodecount++;
        return 0;
    }

    // return 0;
    if(nodelevel == 1) {
        serializehelper(bdd_nodes + node->left, node->left, out);
        serializehelper(bdd_nodes + node->right, node->right, out);
    }
    else if(nodelevel > 1) {
        serializehelper(bdd_nodes + BDD_NUM_LEAVES + node->left, BDD_NUM_LEAVES + node->left, out);
        serializehelper(bdd_nodes + BDD_NUM_LEAVES + node->right, BDD_NUM_LEAVES + node->right, out);
    }

    if (nodelevel > 0) {
        // convert level int to char level
        char clevel = (char)('@' + nodelevel);
        fputc(clevel, out);
        *(bdd_index_map + node->left) = nodecount++;
        *(bdd_index_map + node->right) = nodecount++;
        int leftindex = nodecount - 2;
        int rightindex = nodecount - 1;
        fprintf(out, "%d", leftindex);
        fprintf(out, "%d", rightindex);
        return 0;
    }

    return 0;
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    int nodeindex = bdd_lookup((int)((node -> level) - '0'), node->left, node->right);
    return serializehelper(node, nodeindex, out);
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
