#include "trie.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

//
// Constructor for a TrieNode.
//
// code:   The TrieNode's unique code.
//
TrieNode *trie_node_create(uint16_t code) {
  TrieNode *t = (TrieNode *)malloc(sizeof(TrieNode));
  if (t) {
    for (uint16_t i = 0; i < 256; i++) {
      t->children[i] = NULL;
    }
    t->code = code;
  }
  return t;
}

//
// Destructor for a TrieNode.
//
// n:  The TrieNode to be freed.
//
void trie_node_delete(TrieNode *n) {
  if (n != NULL) {
    for (uint16_t i = 0; i < 256; i++) {
      n->children[i] = NULL;
    }
    free(n);
    n = NULL;
  }
  return;
}

//
// Creates and initializes a new Trie.
// All ASCII characters are initially added as the root's children.
//
TrieNode *trie_create() {
  TrieNode *t = (TrieNode *)malloc(sizeof(TrieNode));
  if (t) {
    for (uint16_t i = 0; i < 256; i++) {
      t->children[i] = trie_node_create(i);
    }
    t->code = UINT16_MAX;
  }
  return t;
}

//
// Resets a trie back to its original state of only ASCII characters.
//
// root: The root of the trie to reset.
//
void trie_reset(TrieNode *root) {
  if (root != NULL) {
    for (uint16_t i = 0; i < 256; i++) {
      TrieNode *t = trie_step(root, i);
      if (t != NULL) {
        trie_delete(t);
      }
    }
    for (uint16_t i = 0; i < 256; i++) {
      root->children[i] = trie_node_create(i);
    }
  }
  return;
}

//
// Frees memory allocated for an entire trie.
//
// n: The root of the trie to free.
//
void trie_delete(TrieNode *n) {
  if (n != NULL) {
    for (uint16_t i = 0; i < 256; i++) {
      trie_delete(n->children[i]);
      n->children[i] = NULL;
    }
    trie_node_delete(n);
    n = NULL;
  }
  return;
}

//
// Steps down to child the TrieNode that represents the symbol.
// Returns NULL if the symbol doesn't exist.
//
// n:     The TrieNode to step down from.
// sym:   The symbol to step to.
//
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
  if (n != NULL) {
    TrieNode *t = n->children[sym];
    if (t != NULL) {
      return t;
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}
