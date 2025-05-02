# UCAS Binary File Layout

## Overview

The UCAS (Unreal Content Archive Storage) file format is part of Unreal Engine's IoStore container system. It stores the actual content/data of game assets, while the corresponding UTOC (Unreal Table of Contents) file contains the metadata and indexing information needed to access this content.

This document describes the binary layout of UCAS files based on analysis of the retoc codebase.

## File Structure

UCAS files have a relatively simple structure compared to UTOC files. They consist of a series of data blocks that store the actual content of game assets.

```
+------------------+
| Data Block 1     |
+------------------+
| Data Block 2     |
+------------------+
| ...              |
+------------------+
| Data Block N     |
+------------------+
```

There is no file header or other metadata in the UCAS file itself. All metadata, including offsets, lengths, and compression information, is stored in the corresponding UTOC file.

## Data Blocks

Each data block in the UCAS file corresponds to a chunk of data referenced by the UTOC file. The size of each block is determined by the `compression_block_size` value in the UTOC file, which is typically 0x10000 (65536) bytes.

### Block Layout

```
| Offset | Size     | Description                             |
| ------ | -------- | --------------------------------------- |
| 0x0000 | Variable | Block data (compressed or uncompressed) |
```

The size of each block can vary depending on whether it's compressed and the compression method used. The UTOC file contains the compressed and uncompressed sizes for each block.

### Block Alignment

If the UCAS file is encrypted, blocks are aligned to AES block boundaries (16 bytes). This alignment is necessary for AES encryption, which operates on 16-byte blocks.

## Accessing Data

To access data in a UCAS file, the following steps are performed:

1. Parse the UTOC file to get the chunk metadata.
2. Use the chunk ID to find the corresponding entry in the UTOC file.
3. Get the offset and length information from the UTOC file.
4. Calculate the block indices that contain the data:
   ```
   first_block_index = (offset / compression_block_size) as usize
   last_block_index = ((align_u64(offset + size, compression_block_size) - 1) / compression_block_size) as usize
   ```
5. Read the data from the UCAS file using the block indices.
6. If the data is compressed or encrypted, process it accordingly.

## Compression

UCAS files support several compression methods:

1. **Zlib**: A widely used compression algorithm.
2. **Zstd**: A newer compression algorithm with better compression ratios and speed.
3. **LZ4**: A fast compression algorithm with lower compression ratios.
4. **Oodle**: A proprietary compression algorithm developed by RAD Game Tools, known for its high performance in game assets.

The compression method is specified in the UTOC file for each block. The compression information includes:

- Compression method index
- Compressed size
- Uncompressed size

## Encryption

UCAS files can be encrypted using AES-256 in ECB mode. The encryption key is specified in the UTOC file and is identified by a GUID. Encrypted blocks are aligned to AES block boundaries (16 bytes).

The encryption process is as follows:

1. Read the encrypted data from the UCAS file.
2. Divide the data into 16-byte blocks.
3. Decrypt each block using the AES key.

## Partitioning

UCAS files can be partitioned, which means the content is split across multiple files. This is particularly useful for large games where a single UCAS file might be too large to handle efficiently.

### Partition Naming

When a UCAS file is partitioned, the partitions are named using a numeric suffix:

- `global.ucas.0`
- `global.ucas.1`
- `global.ucas.2`
- etc.

### Partition Access

When reading data from a partitioned UCAS file, the system needs to determine which partition contains the data:

1. The offset of the data is used to calculate the partition index:
   ```
   partition_index = (compression_block.offset / partition_size) as i32
   ```
2. The appropriate partition file is opened.
3. The data is read from the partition.

## Version Differences

The UCAS file format has evolved across different versions of Unreal Engine. While the basic structure remains the same, there are some differences in how the format is used and interpreted:

1. **UE4.26-UE4.27**:
   - Basic UCAS file format
   - Added support for directory indexing
   - Added support for partitioning

2. **UE5.0-UE5.3**:
   - Improved chunk lookup with perfect hashing
   - Added support for overflow in the perfect hash table
   - Better integration with the engine's asset loading system

3. **UE5.4-UE5.5**:
   - Added and later removed on-demand metadata
   - Replaced chunk hash with IO hash for better performance
   - Enhanced partitioning capabilities
   - Optimized for on-demand loading

## Binary Visualization

Here's a visual representation of a UCAS file with multiple data blocks:

```
+------------------------------------------+
| UCAS File                                |
+------------------------------------------+
| +--------------------------------------+ |
| | Data Block 1                         | |
| | (May be compressed and/or encrypted) | |
| +--------------------------------------+ |
| +--------------------------------------+ |
| | Data Block 2                         | |
| | (May be compressed and/or encrypted) | |
| +--------------------------------------+ |
| | ...                                  | |
| +--------------------------------------+ |
| | Data Block N                         | |
| | (May be compressed and/or encrypted) | |
| +--------------------------------------+ |
+------------------------------------------+
```

## Relationship with UTOC Files

UCAS files work in conjunction with UTOC files to form a complete IoStore container. The UTOC file contains the metadata and indexing information needed to access the content in the UCAS file.

For each chunk in the UCAS file, the UTOC file contains:

- Chunk ID
- Offset and length within the UCAS file
- Compression information
- Hash value for verification

This separation of concerns allows for efficient access to specific assets without having to scan the entire archive.
