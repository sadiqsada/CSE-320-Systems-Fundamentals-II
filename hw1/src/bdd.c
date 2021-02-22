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

// int left = 0, right = 0;

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
        // printf("haha: %d %d %d\n", nodelevel, nodeleftval, noderightval);
        if(nodelevel == level && nodeleftval == left && noderightval == right) {
            return key - bdd_nodes;
        }

        iter++;

        int advancekey = (hashcode + iter) % BDD_HASH_SIZE;
        key = *(bdd_hash_map + advancekey);
    }

    // node about to be inserted is not a leaf
    BDD_NODE *newnode = bdd_nodes + (256 + bdd_node_size);

    newnode -> level = (char)(level + '0');
    newnode -> left = left;
    newnode -> right = right;
    bdd_node_size++;


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
    return (pow == maxwh)? d : ++d;
}

int from_raster_helper(int level, int loww, int lowh, int highw, int highh, unsigned char *raster, int pow, int w, int h, int left, int right) {
    if(level == 0) {
        if(lowh >= h || highh >= h || highw >= w || loww >= w) {
            return 0;
        }
        // handle base case
        unsigned char c = *(raster + (w * lowh) + loww);
        int val = (int)(c);
        return val;
    }

    if(level % 2 == 0) {
        left = from_raster_helper(level - 1, loww, lowh, highw, (lowh + highh) / 2, raster, pow, w, h, left, right);
        right = from_raster_helper(level - 1, loww, ((lowh + highh) / 2) + 1, highw, highh, raster, pow, w, h, left, right);
    }

    else {
        left = from_raster_helper(level - 1, loww, lowh, (loww + highw) / 2, highh, raster, pow, w, h, left, right);
        right = from_raster_helper(level - 1, ((loww + highw) / 2) + 1, lowh, highw, highh, raster, pow, w, h, left, right);
    }

    int ind = bdd_lookup(level, left, right);
    return ind;

}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    // find highest d such that 2^d <= max(w,h)
    int d = get_square_d(w, h);
    int pow = 1;
    for(int i = 0; i < d; i++) {
        pow *= 2;
    }
    int index = from_raster_helper(2 * d, 0, 0, pow - 1,  pow - 1, raster, pow, w, h, 0, 0);
    return bdd_nodes + index;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            unsigned char c = bdd_apply(node, i, j);
            *(raster + (w * i) + j) = c;
        }
    }
}

void serialize_bit_convert(int num, FILE *out) {
    int firstbyte = num << 24;
    firstbyte = firstbyte >> 24;

    int secondbyte = num << 16;
    secondbyte = secondbyte >> 24;

    int thirdbyte = num << 8;
    thirdbyte = thirdbyte >> 24;

    int fourthbyte = num >> 24;

    char firstchar = (char)(firstbyte);
    char secondchar = (char)(secondbyte);
    char thirdchar = (char)(thirdbyte);
    char fourthchar = (char)(fourthbyte);

    fputc(firstchar, out);
    fputc(secondchar, out);
    fputc(thirdchar, out);
    fputc(fourthchar, out);

}

int nodecount = 1;
int leftserial = 0, rightserial = 0, indexserial = 0;
int serializehelper(BDD_NODE *node, int index, FILE *out) {
    // post-order traversal
    // serialize left, right
    if(node->level == 0) {
        fputc('@', out);
        char c = (char)(index);
        fputc(c, out);
        int baseserial = 0;
        if(*(bdd_index_map + index) == 0) {
            *(bdd_index_map + index) = nodecount++;
            baseserial = nodecount - 1;
        }
        else {
            baseserial = *(bdd_index_map + index);
        }
        return baseserial;
    }

    if(*(bdd_index_map + node->left) == 0) {
        serializehelper(bdd_nodes + node->left, node->left, out);
    }

    if(*(bdd_index_map + node->right) == 0) {
        serializehelper(bdd_nodes + node->right, node->right, out);
    }


    int nodelevel = (int)((node -> level) - '0');
    char clevel = (char)('@' + nodelevel);
    fputc(clevel, out);

    if(*(bdd_index_map + node->left) == 0) {
        *(bdd_index_map + node->left) = nodecount++;
        leftserial = nodecount - 1;
    }

    else {
        leftserial = *(bdd_index_map + node->left);
    }

    if(*(bdd_index_map + node->right) == 0) {
        *(bdd_index_map + node->right) = nodecount++;
        rightserial = nodecount - 1;
    }
    else {
        rightserial = *(bdd_index_map + node->right);
    }

    if(*(bdd_index_map + index) == 0) {
        *(bdd_index_map + index) = nodecount++;
        indexserial = nodecount - 1;
    }
    else {
        indexserial = *(bdd_index_map + index);
    }

    serialize_bit_convert(leftserial, out);
    serialize_bit_convert(rightserial, out);

    return 0;
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    int nodeindex = bdd_lookup((int)((node -> level) - '0'), node->left, node->right);
    for(int i = 0; i < BDD_NODES_MAX; i++) {
        *(bdd_index_map + i) = 0;
    }
    serializehelper(node, nodeindex, out);
    return 0;
}

int serialnumber = 1;
BDD_NODE *bdd_deserialize_helper(FILE *in) {
    int c;
    while((c = fgetc(in)) != EOF) {
        // leaf node
        if((char)(c) == '@'){
            int leafval = fgetc(in);
            *(bdd_index_map + serialnumber) = leafval;
            serialnumber++;
        }
        // non-leaf node
        else {
            int level = c - '@';
            int leftserial, rightserial;
            fread(&leftserial, 4, 1, in);
            fread(&rightserial, 4, 1, in);

            int leftnodeindex = *(bdd_index_map + leftserial);
            int rightnodeindex = *(bdd_index_map + rightserial);
            int nodeindex = bdd_lookup(level, leftnodeindex, rightnodeindex);
            *(bdd_index_map + serialnumber) = nodeindex;
            serialnumber++;
        }
    }
    return bdd_nodes + *(bdd_index_map + serialnumber - 1);
}

// void dfs(BDD_NODE *node) {
//     if(node->level == 0) {
//         printf("%d\n", node->level);
//         return;
//     }
//     printf("%c %d %d\n", node->level, node->left, node->right);
//     dfs(bdd_nodes + node->left);
//     dfs(bdd_nodes + node->right);
// }

BDD_NODE *bdd_deserialize(FILE *in) {
    // TO BE IMPLEMENTED
    BDD_NODE *node = bdd_deserialize_helper(in);
    return node;
}

int get_bit_at_pos(int num, int pos) {
    return 1 & (num >> pos);
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    BDD_NODE *root = node;
    int pixel = 0;
    // printf("%c %d %d\n", root->level, r, c);
    while(root->level > 0) {
        // printf("%c %d %d\n", root->level, pixel, root->left);
        int level = root->level - '0';
        int bitpos = 0, bitval = 0;
        if(level % 2 == 0) {
            bitpos = (level - 2) / 2;
            bitval = get_bit_at_pos(r, bitpos);
        }
        else {
            bitpos = (level - 1) / 2;
            bitval = get_bit_at_pos(c, bitpos);
        }
        // printf("%d %d\n", bitpos, bitval);
        if(bitval == 1) {
            pixel = root->right;
            root = bdd_nodes + root->right;
        }
        else if(bitval == 0) {
            pixel = root->left;
            root = bdd_nodes + root->left;
        }
    }
    return pixel;
}

int bdd_map_helper(BDD_NODE *node, int index, unsigned char (*func)(unsigned char), int left, int right) {
    if(node->level == 0) {
        if(*(bdd_index_map + index) != -1) {
            return *(bdd_index_map + index);
        }
        int val = (*func)(index);
        *(bdd_index_map + index) = val;
        return val;
    }
    if(*(bdd_index_map + node->left) == -1) {
        left = bdd_map_helper(bdd_nodes + node->left, node->left, func, left, right);
    }
    else {
        left = *(bdd_index_map + node->left);
    }
    if(*(bdd_index_map + node->right) == -1) {
        right = bdd_map_helper(bdd_nodes + node->right, node->right, func, left, right);
    }
    else {
        right = *(bdd_index_map + node->right);
    }
    // printf("%c %d %d %d %d\n", node->level, node->left, node->right, left, right);
    int newindex = bdd_lookup((int)(node->level - '0'), left, right);
    *(bdd_index_map + index) = newindex;
    // printf("%d\n", newindex);
    return newindex;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    int currentindex = node - bdd_nodes;
    for(int i = 0; i < BDD_NODES_MAX; i++) {
        *(bdd_index_map + i) = -1;
    }
    int index = bdd_map_helper(node, currentindex, func, 0, 0);
    return bdd_nodes + index;
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    // TO BE IMPLEMENTED
    return NULL;
}
