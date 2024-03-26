#include "io.h"
#include "trie.h"
#include "word.h"
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define OPTIONS "vcdi:o:"

TrieNode *root;
TrieNode *curr_node;
TrieNode *next_node;
WordTable *table;
Word *curr_entry;
Word *prev_entry;
Word *missing_entry;
uint8_t *curr_word;
uint8_t *prev_word;
uint8_t *new_word;
bool reset;
uint8_t curr_char;
uint16_t curr_code;
uint16_t prev_code;
uint64_t curr_len;
uint64_t prev_len;
uint64_t new_len;
uint16_t next_avail_code;
uint64_t bit_len;
uint64_t encoded_chars;
uint64_t decoded_chars;
uint64_t original_file_size;
uint64_t compressed_file_size;
float compression_ratio;
uint64_t longest_word_length;
int maxLevel;

extern char *strdup(const char *);

void find_longest_word(TrieNode *n, int level, int *maxLevel) {
  if (n != NULL) {
    for (uint16_t i = 0; i < 256; i++) {
      find_longest_word(n->children[i], ++level, maxLevel);
      --level;
      if (level > *maxLevel) {
        *maxLevel = level;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  int infile = 0;
  int outfile = 1;
  int option = 0;
  int v = 0;
  int compression = 0;
  int decompression = 0;

  FileHeader *FH = malloc(sizeof(FileHeader));
  struct stat fileStat;

  if (argc > 8) {
    printf("%s\n", "Error: too many arguments!");
    exit(EXIT_FAILURE);
  }

  // getopt()
  while ((option = getopt(argc, argv, OPTIONS)) != -1) {
    switch (option) {
    case 'v': // Verbose option
      v = 1;
      break;
    case 'c': // Perform compression
      compression = 1;
      break;
    case 'd': // Perform decompression
      decompression = 1;
      break;
    case 'i': // Sets the input file (default is stdin)
      if ((infile = open(optarg, O_RDONLY)) < -1) {
        printf("Unable to read from file %s\n", optarg);
        exit(EXIT_FAILURE);
      }

      if (fstat(infile, &fileStat) < 0) {
        exit(EXIT_FAILURE);
      }

      if (FH != NULL) {
        FH->file_size = fileStat.st_size;
        FH->protection = fileStat.st_mode;
        FH->padding = UINT16_MAX;
      }

      break;
    case 'o': // Sets the output file (default is stdout)
      if ((outfile = open(optarg, O_RDWR | O_CREAT | O_TRUNC, 0600)) < -1) {
        printf("Unable to read or write to file %s\n", optarg);
        exit(EXIT_FAILURE);
      }
      break;
    default:
      printf("%s\n", "Error: invalid argument!");
      exit(EXIT_FAILURE);
    }
  }

  if (compression && decompression) {
    printf("%s\n", "Error: Cannot compress and decompress at same time.");
    exit(EXIT_FAILURE);
  }

  if (compression) {
    write_header(outfile, FH);
    root = trie_create();
    curr_node = root;
    next_node = NULL;
    next_avail_code = 256;
    encoded_chars = 0;
    longest_word_length = 0;

    while (encoded_chars != FH->file_size) {
      curr_char = next_char(infile);
      next_node = trie_step(curr_node, curr_char);
      if (encoded_chars == 0 || next_node != NULL) {
        curr_node = next_node;
      } else {
        bit_len = log2(next_avail_code) + 1;
        curr_code = curr_node->code;
        compressed_file_size += bit_len;
        buffer_code(outfile, curr_code, bit_len);
        curr_node->children[curr_char] = trie_node_create(next_avail_code);
        curr_node = root->children[curr_char];
        next_avail_code++;
      }
      if (next_avail_code == UINT16_MAX) {
        trie_delete(root);
        root = NULL;
        root = trie_create();
        curr_node = root->children[curr_char];
        next_avail_code = 256;
      }
      encoded_chars++;
    }

    if (curr_node != NULL) {
      curr_code = curr_node->code;
      compressed_file_size += bit_len;
      buffer_code(outfile, curr_code, bit_len);
    }

    flush_codes(outfile);

    if (root != NULL) {
      int maxLevel = -1;
      find_longest_word(root, 0, &maxLevel);
      longest_word_length = maxLevel;
      trie_delete(root);
    }

    original_file_size = encoded_chars;
  }

  if (decompression) {
    read_header(infile, FH);
    table = wt_create();
    next_avail_code = 256;
    reset = false;
    curr_word = calloc(4096, sizeof(uint8_t));
    prev_word = calloc(4096, sizeof(uint8_t));
    new_word = calloc(4096, sizeof(uint8_t));
    curr_len = 0;
    prev_len = 0;
    new_len = 0;
    decoded_chars = 0;
    longest_word_length = 0;

    while (decoded_chars != FH->file_size) {
      bit_len = log2(next_avail_code + 1) + 1;
      compressed_file_size += bit_len;
      curr_code = next_code(infile, bit_len);
      if (curr_code == 0) {
        curr_entry = NULL;
      } else {
        curr_entry = table->entries[curr_code];
      }

      if (decoded_chars == 0 || reset) {
        buffer_word(outfile, curr_entry);
        for (uint64_t i = 0; i < curr_entry->length; i++) {
          prev_word[i] = curr_entry->word[i];
        }
        prev_len = curr_entry->length;
        reset = false;
        decoded_chars++;
      } else if (curr_entry != NULL) {
        for (uint64_t i = 0; i < curr_entry->length; i++) {
          curr_word[i] = curr_entry->word[i];
        }
        curr_len = curr_entry->length;
        prev_entry = table->entries[prev_code];
        for (uint64_t i = 0; i < prev_entry->length; i++) {
          prev_word[i] = prev_entry->word[i];
        }
        prev_len = prev_entry->length;
        for (uint64_t i = 0; i < prev_entry->length; i++) {
          new_word[i] = prev_entry->word[i];
        }
        new_word[prev_len] = curr_word[0];
        new_len = prev_len + 1;
        table->entries[next_avail_code] = word_create(new_word, new_len);
        next_avail_code++;
        buffer_word(outfile, curr_entry);
        decoded_chars += curr_entry->length;
      } else {
        prev_entry = table->entries[prev_code];
        for (uint64_t i = 0; i < prev_entry->length; i++) {
          prev_word[i] = prev_entry->word[i];
        }
        prev_len = prev_entry->length;
        for (uint64_t i = 0; i < prev_entry->length; i++) {
          curr_word[i] = prev_entry->word[i];
        }
        curr_word[prev_len] = prev_word[0];
        curr_len = prev_len + 1;
        missing_entry = word_create(curr_word, curr_len);
        table->entries[next_avail_code] = missing_entry;
        next_avail_code++;
        buffer_word(outfile, missing_entry);
        decoded_chars += missing_entry->length;
      }

      prev_code = curr_code;

      if (next_avail_code == UINT16_MAX - 1) {
        wt_reset(table);
        next_avail_code = 256;
        reset = true;
      }
    }
    flush_words(outfile);
    free(curr_word);
    free(prev_word);
    free(new_word);

    if (table != NULL) {
      for (int i = 0; i < UINT16_MAX; i++) {
        if (table->entries[i] != NULL) {
          if (table->entries[i]->length > longest_word_length) {
            longest_word_length = table->entries[i]->length;
          }
        }
      }
      wt_delete(table);
    }

    original_file_size = decoded_chars;
  }

  compressed_file_size = (compressed_file_size / 8) + 25;
  compression_ratio
      = 100 * (1 - ((float)compressed_file_size / (float)original_file_size));

  if (compression && v) {
    printf("%s: %llu\n", "Original file size", encoded_chars);
    printf("%s: %llu\n", "Compressed file size", compressed_file_size);
    printf("%s: %.4f%s\n", "Compression ratio", compression_ratio, "%");
    printf("%s: %llu\n", "longest_word_length", longest_word_length);
  }

  if (decompression && v) {
    printf("%s: %llu\n", "Original file size", decoded_chars);
    printf("%s: %llu\n", "Compressed file size", compressed_file_size);
    printf("%s: %.4f%s\n", "Compression ratio", compression_ratio, "%");
    printf("%s: %llu\n", "longest_word_length", longest_word_length);
  }

  free(FH);
  FH = NULL;

  if (close(infile) < 0) {
    printf("%s\n", "Error: unable to close the file");
    exit(EXIT_FAILURE);
  }

  if (close(outfile) < 0) {
    printf("%s\n", "Error: unable to close the file");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
