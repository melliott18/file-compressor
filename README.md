# file-compressor

<p>LZW file compressor/decompressor program written in C.</p>
<p>Author: Mitchell Elliott</p>
<p>Email: mitch.elliott@pacbell.net</p>

## Program Operation

### Compile:

```
make
make all: Build and compile everything
make clean: Remove object files
make infer: Run infer
```

### Run:

```
usage: main [-v] [-c] [-d] -i <input file> -o <output file>
```

### Notes

<p>Compression and decompression are mutually exclusive; only one may be chosen at a time.</p>
