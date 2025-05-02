# UTOC and UCAS Relationship

## Overview

The .utoc (Unreal Table of Contents) and .ucas (Unreal Content Archive Storage) files work together as a pair to form a complete IoStore container. This document explains the relationship between these two file formats, how they interact, and how they're used together in the Unreal Engine IoStore system.

## Purpose

The separation of metadata and content into .utoc and .ucas files serves several important purposes:

1. **Efficient Metadata Access**: The smaller .utoc file can be loaded quickly to access metadata without loading the entire content
2. **Reduced Memory Usage**: Only the necessary chunks need to be loaded from the .ucas file
3. **Parallel Loading**: Multiple .ucas files can be loaded in parallel
4. **Partitioning**: Content can be split across multiple .ucas files for better organization and loading

## File Naming Convention

.utoc and .ucas files follow a specific naming convention:

1. **Base Name**: Both files share the same base name (e.g., "global")
2. **Extension**: The files have different extensions (.utoc and .ucas)
3. **Partitioning**: Partitioned .ucas files have a numeric suffix (e.g., "global.ucas.0", "global.ucas.1")

Examples:
- global.utoc + global.ucas
- pakchunk0-Windows.utoc + pakchunk0-Windows.ucas
- global.utoc + global.ucas.0 + global.ucas.1 + global.ucas.2 (partitioned)

## Data Flow

The data flow between .utoc and .ucas files works as follows:

1. **Lookup**: The engine looks up a file path or chunk ID in the .utoc file
2. **Metadata Retrieval**: The .utoc file provides metadata about the chunk (offset, size, compression, etc.)
3. **Content Access**: The engine uses the metadata to access the chunk data in the .ucas file
4. **Decompression**: If the chunk is compressed, the engine decompresses it using the information from the .utoc file

## UTOC File Role

The .utoc file serves as the index and metadata repository for the IoStore container:

1. **Chunk IDs**: Maps chunk IDs to offsets and lengths in the .ucas file
2. **Directory Index**: Provides a hierarchical view of the file structure
3. **Compression Information**: Describes how chunks are compressed
4. **Container Header**: Contains metadata about packages in the container
5. **Perfect Hash**: Provides efficient lookup of chunks by ID

## UCAS File Role

The .ucas file serves as the content storage for the IoStore container:

1. **Chunk Data**: Stores the actual data for each chunk
2. **Compressed Blocks**: Contains compressed blocks of data
3. **Raw Storage**: Provides raw storage for uncompressed data
4. **Partitioning**: Can be split into multiple files for better organization

## Reading Process

The process of reading a chunk from an IoStore container involves both files:

1. **Path to Chunk ID**: Convert a file path to a chunk ID using the directory index in the .utoc file
2. **Chunk ID to Metadata**: Look up the chunk ID in the .utoc file to get its metadata
3. **Metadata to Content**: Use the metadata to locate and read the chunk data from the .ucas file
4. **Content Processing**: Decompress and/or decrypt the chunk data if necessary

## Writing Process

The process of writing a chunk to an IoStore container also involves both files:

1. **Content Preparation**: Compress and/or encrypt the chunk data if necessary
2. **Content Writing**: Write the chunk data to the .ucas file
3. **Metadata Creation**: Create metadata for the chunk (offset, size, compression, etc.)
4. **Metadata Writing**: Write the metadata to the .utoc file
5. **Directory Updating**: Update the directory index in the .utoc file if necessary

## Partitioning

In newer versions of the IoStore system, .ucas files can be partitioned into multiple files:

1. **Partition Size**: The maximum size of each partition (specified in the .utoc file)
2. **Partition Count**: The number of partitions (specified in the .utoc file)
3. **Partition Index**: Each compression block includes a partition index
4. **File Naming**: Partitioned files are named with a numeric suffix (e.g., "global.ucas.0")

Partitioning provides several benefits:
- Reduced file size for better handling by file systems
- Parallel loading of multiple partitions
- Better organization of content

## Encryption

Both .utoc and .ucas files can be encrypted:

1. **UTOC Encryption**: The directory index in the .utoc file can be encrypted
2. **UCAS Encryption**: The data blocks in the .ucas file can be encrypted
3. **Encryption Key**: Both files use the same encryption key, identified by a GUID in the .utoc file

## Version Compatibility

The .utoc and .ucas files must be compatible with each other:

1. **Version Matching**: Both files must use the same version of the IoStore format
2. **Container ID**: Both files must have the same container ID
3. **Chunk Consistency**: Chunks referenced in the .utoc file must exist in the .ucas file

## Implementation Considerations

When implementing a reader or writer for IoStore containers:

1. **File Handling**: Open and manage both files together
2. **Partitioning Support**: Handle partitioned .ucas files correctly
3. **Error Handling**: Handle missing or corrupted files gracefully
4. **Version Compatibility**: Ensure compatibility between .utoc and .ucas files

## Example Workflow

Here's an example workflow for accessing a file in an IoStore container:

1. **Container Opening**: Open the .utoc file and read its header
2. **File Lookup**: Look up the file path in the directory index
3. **Chunk Identification**: Identify the chunk ID for the file
4. **Metadata Retrieval**: Get the chunk's offset, size, and compression information
5. **UCAS Opening**: Open the appropriate .ucas file (or partition)
6. **Data Reading**: Read the compressed data from the .ucas file
7. **Decompression**: Decompress the data using the specified compression method
8. **Data Usage**: Use the decompressed data as needed

## Conclusion

The .utoc and .ucas files work together as a complementary pair, with the .utoc file providing the metadata and indexing information needed to efficiently access the content stored in the .ucas file. Understanding this relationship is essential for working with IoStore containers in Unreal Engine.
