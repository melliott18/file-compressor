#ifndef __TRIE_H__
#define __TRIE_H__

#include <inttypes.h>
#include <stdbool.h>

// Typedef TrieNode to be just TrieNode.
typedef struct TrieNode TrieNode;

//
// Struct definition for a TrieNode.
//
// children:  An array of TrieNode pointers.
// code:      The TrieNode's unique code.
//
struct TrieNode {
  TrieNode *children[256];
  uint16_t code;
};

//
// Constructor for a TrieNode.
//
// code:   The TrieNode's unique code.
//
TrieNode *trie_node_create(uint16_t code);

//
// Destructor for a TrieNode.
//
// n:  The TrieNode to be freed.
//
void trie_node_delete(TrieNode *n);

//
// Creates and initializes a new Trie.
// All ASCII characters are initially added as the root's children.
//
TrieNode *trie_create();

//
// Resets a trie back to its original state of only ASCII characters. 
//
// root: The root of the trie to reset.
//
void trie_reset(TrieNode *root);

//
// Frees memory allocated for an entire trie.
//
// n: The root of the trie to free.
//
void trie_delete(TrieNode *n);

//
// Steps down to child the TrieNode that represents the symbol.
// Returns NULL if the symbol doesn't exist.
//
// n:     The TrieNode to step down from.
// sym:   The symbol to step to.
//
TrieNode *trie_step(TrieNode *n, uint8_t sym);

#endif

