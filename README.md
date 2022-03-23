# HDF5-FUSE
-----------------------------------------------------------
A mountable HDF5 viewer based on FUSE3 library.

## Installation & Usage
- First, replace `HDF5_DIR` in Makefile to your HDF5 root
- Then, run `make`
- Finally, run `./h5fuse [FUSE Options] <MOUNT_POINT> <YOUR_HDF5_FILE>`

## Features
- Almost Readonly now, except it supports `mkdir` (to create groups in your HDF5 file).
- All groups are reflected to a directory
- To access data, you can use `cat DATASET_PATH >& YOUR_BIN_FILE.bin` to dump the raw binary data
  from specified datasets. And you can then use `hexdump` to obtain the human-readable outputs.

