# UTOC Perfect Hash System

## Overview

The Perfect Hash System is a feature introduced in the PerfectHash version (UE5.0) of the .utoc file format. It provides an efficient way to look up chunks by their ID, significantly improving performance for large containers. This document explains how the perfect hash system works and how it's implemented in the .utoc file format.

## Purpose

The perfect hash system serves several important purposes:

1. **Efficient Chunk Lookup**: Allows for O(1) lookup of chunks by their ID
2. **Reduced Memory Usage**: More efficient than a traditional hash table
3. **Improved Performance**: Faster access to chunks, especially in large containers

## What is a Perfect Hash?

A perfect hash function is a hash function that maps each input to a distinct output value, with no collisions. In the context of the .utoc file format, this means that each chunk ID maps to a unique index in the chunk array.

The perfect hash system in the .utoc file format uses a technique called "minimal perfect hashing," which creates a hash function that maps n keys to exactly n consecutive integers, typically in the range [0, n-1], with no collisions.

## Binary Structure

The perfect hash system in the .utoc file consists of two main components:

1. **Perfect Hash Seeds**: An array of 32-bit integers used to compute hash values
2. **Overflow Table**: An array of 32-bit integers for chunks that can't be perfectly hashed

```
Offset  Size    Type    Description
------  ------  ------  -----------
0x00    4*n     int32[] Perfect Hash Seeds (n = TocChunkPerfectHashSeedsCount)
varies  4*m     int32[] Overflow Table (m = TocChunksWithoutPerfectHashCount)
```

## How It Works

### Basic Algorithm

The perfect hash system works as follows:

1. **Seed Selection**: During .utoc creation, a set of seed values is chosen that allows for perfect hashing of most chunk IDs
2. **Hash Computation**: For each chunk ID, a hash value is computed using the seed values
3. **Index Lookup**: The hash value is used as an index into the chunk array
4. **Overflow Handling**: Chunks that can't be perfectly hashed are stored in an overflow table

### Hash Computation

The hash function used in the perfect hash system is a variant of the FNV-1a hash algorithm:

1. Start with an initial hash value
2. For each byte in the chunk ID:
   a. XOR the hash with the byte
   b. Multiply the hash by a prime number (typically 16777619)
3. Use the seed value to further modify the hash
4. Take the modulo of the hash with the number of chunks to get an index

### Overflow Handling

In the PerfectHashWithOverflow version (UE5.0+), chunks that can't be perfectly hashed are handled using an overflow table:

1. If a chunk can't be included in the perfect hash, it's added to the overflow table
2. The overflow table maps chunk IDs to their indices in the chunk array
3. When looking up a chunk, if it's not found using the perfect hash, the overflow table is checked

## Implementation Details

### Creating the Perfect Hash

Creating a perfect hash involves finding a set of seed values that allows for perfect hashing of the chunk IDs:

1. Start with a set of candidate seed values
2. For each seed value, compute the hash for each chunk ID
3. Check if there are any collisions (two chunk IDs mapping to the same index)
4. If there are collisions, try different seed values
5. Continue until a set of seed values is found that minimizes or eliminates collisions

### Chunk Lookup

Looking up a chunk by its ID involves:

1. Compute the hash of the chunk ID using the seed values
2. Use the hash as an index into the chunk array
3. If the chunk at that index doesn't match the requested ID, check the overflow table
4. If the chunk is found, return its data; otherwise, return an error

## Version Differences

The perfect hash system has evolved across versions of the .utoc file format:

### PerfectHash (UE5.0)
- Initial implementation of the perfect hash system
- Basic perfect hashing without overflow handling

### PerfectHashWithOverflow (UE5.0-5.3)
- Added overflow table for chunks that can't be perfectly hashed
- Improved handling of edge cases

### ReplaceIoChunkHashWithIoHash (UE5.5+)
- Modified the hash function to use a different algorithm
- Improved compatibility with other systems

## Performance Considerations

The perfect hash system provides significant performance benefits:

1. **Lookup Speed**: O(1) lookup time for most chunks
2. **Memory Efficiency**: More compact than a traditional hash table
3. **Scalability**: Performs well even with large numbers of chunks

However, there are some trade-offs:

1. **Creation Time**: Computing the perfect hash can be time-consuming
2. **Modification Difficulty**: Adding or removing chunks requires recomputing the perfect hash
3. **Overflow Overhead**: Handling overflow adds some complexity

## Implementation Considerations

When implementing a reader or writer for the perfect hash system:

1. **Hash Function**: Implement the correct hash function based on the .utoc version
2. **Seed Handling**: Properly use the seed values to compute hash values
3. **Overflow Handling**: Check the overflow table if a chunk isn't found using the perfect hash
4. **Version Compatibility**: Handle differences between versions of the perfect hash system

## Conclusion

The perfect hash system is a sophisticated feature of the .utoc file format that significantly improves chunk lookup performance. By understanding how it works and how to implement it, developers can efficiently access chunks in IoStore containers, even when dealing with large numbers of assets.
