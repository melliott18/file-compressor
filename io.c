#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "io.h"
#include "word.h"

uint64_t byte_index = 0;
uint64_t byte_index2 = BLOCK_SIZE;
uint64_t char_index = BLOCK_SIZE;
uint64_t char_index2 = 0;
uint64_t bit_index = 0;
uint64_t bit_index2 = (BLOCK_SIZE * 8);
uint64_t flush_index = 0;
uint8_t char_buffer[BLOCK_SIZE];
uint8_t bit_buffer[BLOCK_SIZE];
uint8_t bit_buffer2[BLOCK_SIZE];
uint8_t code_buffer[BLOCK_SIZE];
uint8_t bit_array[BLOCK_SIZE];
uint16_t ncode;
uint64_t file_bit_index = 0;
uint64_t infile_size = 0;
uint64_t outfile_size = 0;
uint64_t buffer_write_count = 0;
uint64_t iteration = 0;
uint16_t num_bits;
uint16_t difference;
uint16_t number;
uint16_t reverse;

//
// Reads a file header from the input file.
//
// infile:     Input file descriptor.
// header:     The file header struct to read into.
//
void read_header(int infile, FileHeader *header) {
  lseek(infile, 0, SEEK_SET);
  read(infile, header, sizeof(FileHeader));
  if (header->magic != MAGIC) {
  	printf("%s\n", "Error: file has incorrect magic number.");
    exit(EXIT_FAILURE);
  }
  return;
}

//
// Writes a file header to the output file and returns original file size.
//
// outfile:    Output file descriptor.
// header:     FileHeader to set values in and write.
//
void write_header(int outfile, FileHeader *header) {
  header->magic = MAGIC;
  lseek(outfile, 0, SEEK_SET);
  write(outfile, header, sizeof(FileHeader));
  return;
}

//
// Returns next character, or byte, from the input file.
// 4KB of characters are read when needed into a character buffer.
// This is only called once for each byte in the input file.
//
// infile:     Input file descriptor
//
uint8_t next_char(int infile) {
  if (char_index == BLOCK_SIZE) {
    read(infile, char_buffer, sizeof(char_buffer));
    char_index = 0;
  }
  char_index++;
  return char_buffer[char_index - 1];
}

//
// Buffers a code into the code buffer for writing.
// The binary of the code is placed into the buffer in reverse.
//
// outfile:   Output file descriptor.
// code:      The code to buffer.
// bit_len:   The length in bits of the code.
//
void buffer_code(int outfile, uint16_t code, uint8_t bit_len) { 
  FileHeader *out_header = malloc(sizeof(FileHeader));
  if (iteration == 0) {
    read_header(outfile, out_header);
    outfile_size = out_header->file_size;
  }
  num_bits = log2(code) + 1;
  difference = bit_len - num_bits;
  number = code;
  reverse = 0;      
  while (number > 0) { 
    reverse <<= 1; 
    reverse |= (number & 1);
    number >>= 1;          
  } 
  reverse <<= difference;
  for (int i = bit_len - 1; i >= 0; i--) {
    if (bit_index == (BLOCK_SIZE * 8)) {
      write(outfile, bit_buffer, sizeof(bit_buffer));
      memset(bit_buffer, 0, BLOCK_SIZE);
      bit_index = 0;
      buffer_write_count++;
    }
    int j = reverse >> i;
    if (j & 1) {
      bit_buffer[bit_index / 8] |= (1 << (bit_index % 8));
    } else {
      bit_buffer[bit_index / 8] &= ~(1 << (bit_index % 8));
    }
    bit_index++;
    file_bit_index++;
  }
  byte_index = ((bit_index - 1) / 8) + 1;
  iteration++;
  free(out_header);
  out_header = NULL;
  return;
}

//
// Flushes any remaining codes in the buffer to the output file.
//
// outfile:    Output file descriptor.
//
void flush_codes(int outfile) {
  write(outfile, bit_buffer, sizeof(bit_buffer));
  return;
}

//
// Reads and returns next code in the input file.
// 4KB worth of codes are read into a code buffer.
// Called until main decompression loop decodes all characters.
//
// infile:     Input file descriptor.
// bit_len:    The length in bits of the code to read.
//
uint16_t next_code(int infile, uint8_t bit_len) {
  for (int i = bit_len - 1; i >= 0; i--) {
  	if (bit_index2 == (BLOCK_SIZE * 8)) {
      read(infile, bit_buffer2, sizeof(bit_buffer2));
      byte_index2 = 0;
      bit_index2 = 0;
    }
  	bit_array[i] = 1 & (bit_buffer2[bit_index2 / 8] >> (bit_index2 % 8));
  	bit_index2++;
  }
  ncode = 0;
  int j = 0;
  for (int i = bit_len - 1; i >= 0; i--) {
    if (bit_array[i] == 1) {
    	ncode += pow(2, j);
    }
    j++;
  }
  for (int i = 0; i < bit_len; i++) {
  	bit_array[i] = 0;
  }
  return ncode;
}

//
// Adds a word into the character buffer to write to the output file.
// Buffer is written when it's filled with 4KB of characters.
//
// outfile:    Output file descriptor.
// word:       Word to output.
// word_len:   Length of word to output.
//
void buffer_word(int outfile, Word *w) {
  for (uint64_t i = 0; i < w->length; i++) {
    if (char_index2 == BLOCK_SIZE) {
      write(outfile, char_buffer, sizeof(char_buffer));
      memset(char_buffer, 0, BLOCK_SIZE);
      char_index2 = 0;
    }
    char_buffer[char_index2] = w->word[i];
    char_index2++;
  }
  return;
}

//
// Flushes the character buffer to the output file if not empty.
//
// outfile:    Output file descriptor.
//
void flush_words(int outfile) {
  if (char_index2 != 0) {
    write(outfile, char_buffer, sizeof(char_buffer));
    char_index2 = 0;
  }
  return;
}

