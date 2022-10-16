#pragma once

//
//  A huffman node is the basic data structure used as internal nodes in the huffman 
//  table.
//

class huffman_node {
public:

    //the left child index.
    byte left;
    //true if the left child is a leaf.
    bool leaf_left;

    //the right child index.
    byte right;
    //true if the right child is a leaf.
    bool leaf_right;
};

//
//  A huffman table stores the binary tree used to en/decode data.
//  It has 255 nodes, each with 2 child nodes, which can be either another internal
//  node, or a leaf.  The only important part of a leaf is the index of the leaf itself, which
//  represents the value of 8-bit uncompressed value that corresponds to the tree traversal that
//  led to the leaf.
//

class huffman_table {
public:
    huffman_table();
    ~huffman_table();

protected:
    //the internal nodes of the tree.
    huffman_node nodes[255];

    //each leaf's parent node.
    byte leaf_parents[256];

    //each node's parent node.
    byte node_parents[256];

public:
    //creates the tree with given the data
    void Init(byte *data, int data_length);

    int CompressedByteLength(byte leaf);
};


