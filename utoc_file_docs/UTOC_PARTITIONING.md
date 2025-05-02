# UTOC Partitioning System

## Overview

The Partitioning System is a feature introduced in the PartitionSize version (UE4.27) of the .utoc file format. It allows for splitting the content of an IoStore container across multiple .ucas files, providing better organization and more efficient loading. This document explains how the partitioning system works and how it's implemented in the .utoc file format.

## Purpose

The partitioning system serves several important purposes:

1. **File Size Management**: Keeps individual .ucas files below a certain size
2. **Parallel Loading**: Allows for loading multiple partitions in parallel
3. **Content Organization**: Enables better organization of content
4. **Platform Compatibility**: Addresses file size limitations on some platforms
5. **Incremental Updates**: Facilitates updating only specific partitions

## Binary Structure

The partitioning information in the .utoc file is stored in the header:

```
Offset  Size    Type    Description
------  ------  ------  -----------
0x34    4       uint32  PartitionCount
0x58    8       uint64  PartitionSize
```

The `PartitionCount` field specifies the number of partitions, and the `PartitionSize` field specifies the maximum size of each partition in bytes.

## Partition Indexing

Each compression block in the .utoc file includes a partition index, which is derived from its offset:

```
PartitionIndex = BlockOffset / PartitionSize
```

This index determines which .ucas partition file contains the block.

## File Naming Convention

Partitioned .ucas files follow a specific naming convention:

1. **Base Name**: The same base name as the .utoc file (e.g., "global")
2. **Extension**: The .ucas extension
3. **Partition Index**: A numeric suffix indicating the partition index (e.g., ".0", ".1", ".2")

Examples:
- global.ucas.0
- global.ucas.1
- global.ucas.2

## How It Works

### Writing to Partitioned Files

When writing chunks to a partitioned IoStore container:

1. Calculate the partition index for each block based on its offset
2. Create a separate .ucas file for each partition
3. Write blocks to the appropriate partition file
4. Update the .utoc file with the partition count and size

### Reading from Partitioned Files

When reading chunks from a partitioned IoStore container:

1. Look up the chunk in the .utoc file to get its offset and size
2. Calculate the partition index for each block that contains the chunk
3. Open the appropriate .ucas partition file(s)
4. Read the blocks from the partition file(s)
5. Decompress and assemble the chunk data

## Cross-Partition Chunks

A single chunk may span multiple partitions if it crosses a partition boundary. In this case:

1. The chunk's blocks are split across multiple partition files
2. Each block is stored in the partition corresponding to its offset
3. When reading the chunk, blocks must be read from all relevant partitions

## Partition Size Selection

The choice of partition size depends on several factors:

1. **File System Limitations**: Some file systems have maximum file size limits
2. **Memory Constraints**: Smaller partitions require less memory to process
3. **Loading Performance**: Larger partitions may reduce the overhead of opening multiple files
4. **Parallel Loading**: Smaller partitions allow for more parallelism

Typical partition sizes range from 1 GB to 4 GB.

## Implementation Details

### Partition Calculation

To calculate which partition contains a block:

```
PartitionIndex = BlockOffset / PartitionSize
```

To calculate the offset within a partition:

```
OffsetInPartition = BlockOffset % PartitionSize
```

### File Handling

When implementing partitioning:

1. **File Opening**: Open only the necessary partition files
2. **Offset Translation**: Translate global offsets to partition-relative offsets
3. **Cross-Partition Handling**: Handle chunks that span multiple partitions
4. **Error Handling**: Handle missing or corrupted partition files

## Version Differences

The partitioning system was introduced in the PartitionSize version (UE4.27) of the .utoc file format and has remained relatively stable since then.

## Performance Considerations

Partitioning affects performance in several ways:

1. **Parallel Loading**: Multiple partitions can be loaded in parallel
2. **File Opening Overhead**: Opening multiple files adds some overhead
3. **Memory Usage**: Smaller partitions require less memory to process
4. **Seek Performance**: Smaller files may have better seek performance

## Implementation Considerations

When implementing a reader or writer for partitioned IoStore containers:

1. **Partition Management**: Efficiently manage multiple partition files
2. **Offset Translation**: Correctly translate between global and partition-relative offsets
3. **Cross-Partition Chunks**: Handle chunks that span multiple partitions
4. **Parallel Processing**: Consider loading partitions in parallel for better performance

## Example

Here's an example of a partitioned IoStore container:

```
global.utoc           # Table of Contents (metadata)
global.ucas.0         # First partition (0 to PartitionSize - 1)
global.ucas.1         # Second partition (PartitionSize to 2*PartitionSize - 1)
global.ucas.2         # Third partition (2*PartitionSize to 3*PartitionSize - 1)
```

With a partition size of 2 GB:
- Blocks with offsets 0 to 2,147,483,647 are in global.ucas.0
- Blocks with offsets 2,147,483,648 to 4,294,967,295 are in global.ucas.1
- Blocks with offsets 4,294,967,296 to 6,442,450,943 are in global.ucas.2

## Conclusion

The partitioning system is a valuable feature of the .utoc file format that allows for better management of large IoStore containers. By understanding how it works and how to implement it, developers can effectively work with partitioned containers, improving loading performance and addressing file size limitations.
