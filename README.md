# Huffman Compression Tool

This repository contains C++ code for a linux app that compresses files and decompresses them based on the huffman encoding technique. It is a direct application done of the technique applied on the frequency of each char (or rather each individual byte) and it uses C functions for low level file I/O.

The C++ executable only work on individual files, they cannot be applied to folders, so I also made a bash script which tars the given arguments then applies the huffman compression, and a script for decompressing and untaring. If you want to run the two scripts you will need to compile the C++ files and modify the executable paths inside the scripts as they currently are my working directories.