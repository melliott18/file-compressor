#ifndef __WORD_H__
#define __WORD_H__

#include <inttypes.h>

//
// Struct definition for a Word.
// A Word in LZW is just a byte array.
//
// word: 	The word byte array.
// length: 	The length of the word.
//
typedef struct Word {
  uint8_t *word;
  uint64_t length;
} Word;

//
// Struct definition of a WordTable.
// Each index is a Word.
//
// entries:   An array of Word pointers.
//
typedef struct WordTable {
  Word *entries[UINT16_MAX];
} WordTable;

//
// Constructor for a Word.
//
// word: 	The byte array.
// length: 	The length of the word.
//
Word *word_create(uint8_t *word, uint64_t length);

//
// Destructor for a Word.
//
// w:  The Word.
//
void word_delete(Word *w);

//
// Constructor for a WordTable.
// Is initialized with all ASCII characters.
//
WordTable *wt_create(void);

//
// Resets a word table to its original state of just ASCII characters.
//
// wt: The word table to reset.
//
void wt_reset(WordTable *wt);

//
// Destructor for a WordTable.
//
// wt:  The WordTable.
//
void wt_delete(WordTable *wt);

#endif

