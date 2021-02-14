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
        int advancekey = (hashcode + iter) % BDD_HASH_SIZE;
        key = *(bdd_hash_map + advancekey);
        // compare if this node and the input node are the same
        int nodelevel = (int)((key->level) - '0');
        int nodeleftval = key -> left;
        int noderightval = key -> right;

        // compare with input arguments
        if(nodelevel == level && nodeleftval == left && noderightval == right) {
            for(int i = BDD_NUM_LEAVES; i < BDD_NUM_LEAVES + bdd_node_size; i++) {
                BDD_NODE *thisnode = (bdd_nodes + i);
                if((int)((thisnode -> level) - '0') == level && thisnode -> left == left && thisnode -> right == right) {
                    return i;
                }
            }
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
    // level is zero, negative was handled before
}

int get_square_d(int w, int h) {
    int pow = 2;
    int d = 1;
    int maxwh = w > h ? w : h;
    if(maxwh == 1) return 0;
    if(maxwh == 2) return 1;
    while(pow * pow <= maxwh) {
        pow *= 2;
        d++;
    }
    return (pow == maxwh) ? d : ++d;
}

int count = BDD_NUM_LEAVES;

BDD_NODE *from_raster_helper(int level, int loww, int lowh, int highw, int highh, unsigned char *raster, int k, int count) {
    if(level == 0) {
        // handle base case
        unsigned char c = *(raster + (level * highh) + loww);
        int val = (int)c;

        if(val == 0) {
            return NULL;
        }
        return bdd_nodes + val;
    }

    int newnodeleftindex = 2 * count + 1;
    int newnoderightindex = 2 * count + 2;
    int newnodeindex = bdd_lookup(level, newnodeleftindex, newnoderightindex);

    if(k % 2 == 0) {
        from_raster_helper(level - 1, loww, lowh, highw, highh / 2, raster, k + 1, 2 * count + 1);
        from_raster_helper(level - 1, loww, highh / 2, highw, highh, raster, k + 1, 2 * count + 2);
    }

    else {
        from_raster_helper(level - 1, loww, lowh, highw / 2, highh, raster, k + 1, 2 * count + 1);
        from_raster_helper(level - 1, highw / 2, lowh, highw, highh, raster, k + 1, 2 * count + 2);
    }

    return bdd_nodes + newnodeindex;

}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    // find highest d such that 2^d <= max(w,h)
    int d = get_square_d(w, h);
    if(d > 8192) return NULL;
    int pow = 2;
    for(int i = 0; i < d; i++) {
        pow *= 2;
    }
    return from_raster_helper(2 * d, 0, 0, pow, pow, raster, 0, 0);
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
}

int nodecount = 1;
int serializehelper(BDD_NODE *node, int index, FILE *out) {
    // post-order traversal
    // serialize left, right
    int nodelevel = (int)((node -> level) - '0');
    if(nodelevel != 0) serializehelper(bdd_nodes + node->left, node->left, out);
    if(nodelevel != 0) serializehelper(bdd_nodes + node->right, node->right, out);

    // process root
    // level is zero
    if(nodelevel == 0) {
        fputc('@', out);
        // convert int to char because i have to print 1 byte
        char c = (char)(index);
        fputc(c, out);
        *(bdd_index_map + index) = nodecount++;
        return 0;
    }
    // level is greater than zero
    else if (nodelevel > 0) {
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
