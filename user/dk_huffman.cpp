#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned char byte;

#include "dk_huffman.h"


huffman_table::huffman_table() {

}

huffman_table::~huffman_table() {

}

//creates the tree with given the data
void huffman_table::Init(byte *data, int data_length) {
    //first count the number of occurences of each byte value.
    int data_counts[256];

    for (int i = 0; i < 256; i++) data_counts[i] = 0;
    for (i = 0; i < data_length; i++) data_counts[data[i]]++;

    //as we build the tree, we need to keep counts associated with each node.
    int node_counts[255];
    for (i = 0; i < 255; i++) {
        node_counts[i] = 0;
    }

    //we need to build the entire tree by pairing up the 2 smallest nodes/leaves under a new internal node
    //until all internal nodes are used.  We use nodes starting with the last one in the array, so that
    //the first one will contain the top node of the tree.
    //the first node we will use is the last one, index 254.
    int next_open_node = 254;

    //as we put the leaves into the tree, we must know the nodes that are used as their parents.
    //we also use this parent index to know if the leaf has been added to a node yet.
    //initialize our node parent indices now also.
    for (i = 0; i < 256; i++) {
        leaf_parents[i] = 0;
        node_parents[i] = 0;
    }

    //make nodes until all nodes are used.
    while (next_open_node >= 0) {
        //find the smallest 2 nodes or leaves, in any possible combination of the 2 types.
        int smallest_index;
        int smaller_index;

        int smallest_count;
        int smaller_count;

        //true if smallest/smaller are leaves.
        bool smallest_is_leaf;
        bool smaller_is_leaf;

        //find any 2 nodes/leaves to use as the initial values of the smallest/smaller indices.
        bool found_smallest = false;
        bool found_smaller = false;

        //look in our nodes for our first 2.
        for (int i = next_open_node + 1; i < 255 && found_smaller == false; i++) {
            //check if this node is available.
            if (node_parents[i] == 0) {
                //this node is available.
                if (found_smallest == false) {
                    //use this as our smallest.
                    found_smallest = true;
                    smallest_index = i;
                    smallest_is_leaf = false;
                    smallest_count = node_counts[i];
                }
                else if (found_smaller == false) {
                    //use this as our smaller.
                    found_smaller = true;
                    smaller_index = i;
                    smaller_is_leaf = false;
                    smaller_count = node_counts[i];
                }
            }
        }

        //look for our initial 2 in the leaves.
        for (i = 0; i < 256 && found_smaller == false; i++) {
            //check if this leaf is available.
            if (leaf_parents[i] == 0) {
                //this leaf is available.
                if (found_smallest == false) {
                    //use this as our smallest.
                    found_smallest = true;
                    smallest_index = i;
                    smallest_is_leaf = true;
                    smallest_count = data_counts[i];
                }
                else if (found_smaller == false) {
                    //use this as our smaller.
                    found_smaller = true;
                    smaller_index = i;
                    smaller_is_leaf = true;
                    smaller_count = data_counts[i];
                }
            }
        }

        //now go through the remaining nodes/leaves and look for the real smallest and smaller ones.
        int next_index_to_check;
        bool next_index_is_leaf;

        if (smaller_is_leaf == true) {
            //we start searching for the smallest node/leaf in the leaf array.
            next_index_is_leaf = true;
            next_index_to_check = smaller_index + 1;
        }
        else {
            //we should start searching for the smallest node/leaf in the node array, but
            //first make sure that our initial smaller node/leaf is not the last node.
            if (smaller_is_leaf == false && smaller_index == 254) {
                //start searching at the first leaf.
                next_index_is_leaf = true;
                next_index_to_check = 0;
            }
            else {
                //start searching at the node after the initial smaller node.
                next_index_is_leaf = false;
                next_index_to_check = smaller_index + 1;
            }
        }

        //compare the smallest and the smaller values and make sure there are not actually more of the smallest
        //that of the smaller.
        if (smallest_count > smaller_count) {
            //switch them.
            int save_index = smallest_index;
            bool save_is_leaf = smallest_is_leaf;
            int save_count = smallest_count;

            smallest_index = smaller_index;
            smallest_is_leaf = smaller_is_leaf;
            smallest_count = smaller_count;

            smaller_index = save_index;
            smaller_is_leaf = save_is_leaf;
            smaller_count = save_count;
        }

        //search through the nodes for new values for smallest/smaller
        for (; next_index_is_leaf == false; next_index_to_check++) {
            //make sure this node hasn't been parented yet.
            if (node_parents[next_index_to_check] == 0) {
                //this node hasnt been parented yet, and can be considered when looking for the smallest
                //count.

                //check the count for this node.
                if (node_counts[next_index_to_check] < smallest_count) {
                    //this leaf is out new smallest.  move the existing smallest to smaller.
                    smaller_index = smallest_index;
                    smaller_is_leaf = smallest_is_leaf;
                    smaller_count = smallest_count;

                    //save this leaf as our smallest.
                    smallest_index = next_index_to_check;
                    smallest_is_leaf = false;
                    smallest_count = node_counts[next_index_to_check];
                }
                //else, check if it is at least smaller than smaller.
                else if (node_counts[next_index_to_check] < smaller_count) {
                    //this node is the new smaller.
                    smaller_index = next_index_to_check;
                    smaller_is_leaf = false;
                    smaller_count = node_counts[next_index_to_check];
                }
            }

            //check if the node we just checked is the last node.
            if (next_index_to_check >= 254) {
                //we just checked the last one, go on to the leaves.
                next_index_to_check = 0;
                next_index_is_leaf = true;
                break;
            }
        }

        //go through leaves.
        for (; next_index_to_check < 256; next_index_to_check++) {
            //check if this leaf has been used already.
            if (leaf_parents[next_index_to_check] != 0) continue;

            //this leaf is unused so far, check if it is our new smallest/smaller.

            //check if it is smaller than smallest
            if (data_counts[next_index_to_check] < smallest_count) {
                //this leaf is out new smallest.  move the existing smallest to smaller.
                smaller_index = smallest_index;
                smaller_is_leaf = smallest_is_leaf;
                smaller_count = smallest_count;

                //save this leaf as our smallest.
                smallest_index = next_index_to_check;
                smallest_is_leaf = true;
                smallest_count = data_counts[next_index_to_check];
            }
            //else, check if it is at least smaller than smaller.
            else if (data_counts[next_index_to_check] < smaller_count) {
                //this leaf is the new smaller.
                smaller_index = next_index_to_check;
                smaller_is_leaf = true;
                smaller_count = data_counts[next_index_to_check];
            }
        }

        //we have the smallest and 2nd smallest nodes/leaves.
        //make a new node that joins these two.  we put the smallest index on the right.
        nodes[next_open_node].left = smaller_index;
        nodes[next_open_node].leaf_left = smaller_is_leaf;

        nodes[next_open_node].right = smallest_index;
        nodes[next_open_node].leaf_right = smallest_is_leaf;

        //set up the parenting of the two children just added as children to this new node.
        //check if the left child is a leaf or a node.
        //at the same time, compute the count of the new node.
        if (nodes[next_open_node].leaf_left == true) {
            //left child is a leaf.
            leaf_parents[nodes[next_open_node].left] = next_open_node;
            node_counts[next_open_node] += data_counts[nodes[next_open_node].left];
        }
        else {
            //left child is a node.
            node_parents[nodes[next_open_node].left] = next_open_node;
            node_counts[next_open_node] += node_counts[nodes[next_open_node].left];
        }
        
        //check if right child is a leaf or a node.
        if (nodes[next_open_node].leaf_right == true) {
            //right child is a leaf.
            leaf_parents[nodes[next_open_node].right] = next_open_node;
            node_counts[next_open_node] += data_counts[nodes[next_open_node].right];
        }
        else {
            //right child is a node.
            node_parents[nodes[next_open_node].right] = next_open_node;
            node_counts[next_open_node] += node_counts[nodes[next_open_node].right];
        }

        //do it all over again for the next node.
        next_open_node--;
    }
}

int huffman_table::CompressedByteLength(byte leaf) {
    //we must walk up the tree from the given leaf and count how many steps it takes us to get
    //to the top of the tree, node 0.
    int height = 1;
    int parent = leaf_parents[leaf];

    while (parent != 0) {
        //go to the next higher node.
        parent = node_parents[parent];
        height++;
    }

    //return the height;
    return height;
}






/*
#include <math.h>
extern "C" void main_huffman(byte *data, int len);

void main_huffman(byte *data, int len) {
    //build a huffman table from the data.
    huffman_table table;
    table.Init(data, len);

    //get the heights of each of the byte values.
    int height[256];
    for (int i = 0; i < 256; i++) {
        height[i] = table.CompressedByteLength(i);
    }

    //compute the length of the data when compressed.
    int compressed_size = 0;
    for (i = 0; i < len; i++) {
        compressed_size += height[data[i]];
    }

    compressed_size >>= 3;

    printf("Compressed size: %d\n", compressed_size);
}

*/