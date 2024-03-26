#include "word.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Constructor for a Word.
//
// word: 	The byte array.
// length: 	The length of the word.
//
Word *word_create(uint8_t *word, uint64_t length) {
  Word *w = (Word *)malloc(sizeof(Word));
  if (w) {
    w->word = calloc(length, sizeof(uint8_t));
    for (uint64_t i = 0; i < length; i++) {
      if (w->word != NULL) {
        w->word[i] = word[i];
      }
    }
    w->length = length;
  }
  return w;
}

//
// Destructor for a Word.
//
// w:  The Word.
//
void word_delete(Word *w) {
  if (w != NULL) {
    free(w->word);
    w->word = NULL;
  }
  return;
}

//
// Constructor for a WordTable.
// Is initialized with all ASCII characters.
//
WordTable *wt_create(void) {
  WordTable *wt = (WordTable *)malloc(sizeof(WordTable));
  if (wt) {
    for (uint16_t i = 0; i < UINT16_MAX; i++) {
      if (i <= UINT8_MAX) {
        uint8_t *word = calloc(1, sizeof(uint8_t));
        if (word != NULL) {
          word[0] = (uint8_t)i;
          wt->entries[i] = word_create(word, 1);
          free(word);
        }
      } else {
        wt->entries[i] = NULL;
      }
    }
  }
  return wt;
}

//
// Resets a word table to its original state of just ASCII characters.
//
// wt: The word table to reset.
//
void wt_reset(WordTable *wt) {
  if (wt != NULL) {
    for (uint16_t i = UINT8_MAX + 1; i < UINT16_MAX; i++) {
      if (wt->entries[i] != NULL) {
        word_delete(wt->entries[i]);
      }
    }
  }
  return;
}

//
// Destructor for a WordTable.
//
// wt:  The WordTable.
//
void wt_delete(WordTable *wt) {
  if (wt != NULL) {
    for (uint16_t i = 0; i < UINT16_MAX; i++) {
      if (wt->entries[i] != NULL) {
        word_delete(wt->entries[i]);
        free(wt->entries[i]);
        wt->entries[i] = NULL;
      }
    }
    free(wt);
    wt = NULL;
  }
  return;
}
