# UTOC Detailed Information

## Introduction

The .utoc (Unreal Table of Contents) file format is a key component of Unreal Engine's IoStore container system. It works in conjunction with .ucas (Unreal Content Archive Storage) files to provide an efficient and feature-rich asset storage solution. This document provides detailed information about the .utoc file format, its purpose, functionality, and implementation details.

## Purpose and Role

The .utoc file serves as the index and metadata repository for the IoStore container system. Its primary purposes are:

1. **Asset Indexing**: Provides a mapping between virtual file paths and chunks of data in the .ucas file
2. **Chunk Metadata**: Stores information about each chunk, including its type, size, and hash
3. **Compression Information**: Contains details about how chunks are compressed
4. **Directory Structure**: Maintains a hierarchical representation of the file structure
5. **Encryption Support**: Provides information for decrypting encrypted chunks
6. **Partitioning Information**: Contains details about how the .ucas file is partitioned

The .utoc file is designed to be small and efficient to load, allowing for quick access to the larger asset data stored in the .ucas file.

## Relationship with .ucas Files

The .utoc and .ucas files work together as a pair:

- The .utoc file contains metadata and indexing information
- The .ucas file contains the actual asset data in chunks

When the engine needs to access an asset, it:
1. Looks up the asset's path in the .utoc file's directory index
2. Finds the corresponding chunk ID
3. Uses the chunk ID to locate the chunk's offset and length in the .ucas file
4. Reads the chunk data from the .ucas file
5. Decompresses and/or decrypts the chunk data if necessary

This separation of concerns allows for efficient access to specific assets without having to scan the entire .ucas file.

## Key Components

### Chunk System

The IoStore system organizes data into "chunks," which are the basic units of storage. Each chunk:

- Has a unique 12-byte ID
- Belongs to a specific type (e.g., ExportBundleData, BulkData, ShaderCodeLibrary)
- Can be compressed and/or encrypted
- Has a hash for verification

The chunk ID encodes:
- Package ID (8 bytes): For asset chunks, this is a hash of the package name
- Chunk index (2 bytes): Used to distinguish between multiple chunks of the same type in a package
- Chunk type (1 byte): Indicates what kind of data the chunk contains
- Version information (1 byte): Contains version bits for compatibility

### Directory Index

The directory index provides a hierarchical representation of the file structure, mapping virtual file paths to chunk IDs. It consists of:

1. **Mount Point**: The base path where the container is mounted (e.g., "../../../")
2. **Directory Entries**: A tree structure of directories
3. **File Entries**: Information about files, including their chunk IDs
4. **String Table**: A collection of strings used for file and directory names

The directory index allows the engine to quickly find the chunk ID for a given file path without having to scan the entire container.

### Compression System

The .utoc file supports multiple compression methods:

- Zlib
- Gzip
- Zstd
- LZ4
- Oodle

Each compression block in the .ucas file is described by an entry in the .utoc file, which includes:
- Offset in the .ucas file
- Compressed size
- Uncompressed size
- Compression method index

The compression system allows for efficient storage while maintaining fast access to assets.

### Perfect Hash System

In versions >= PerfectHash (UE5.0), the .utoc file uses a perfect hash table for efficient chunk lookup. This system consists of:

1. **Perfect Hash Seeds**: Used to compute hash values for chunk IDs
2. **Overflow Table**: Handles hash collisions for chunks that can't be perfectly hashed

The perfect hash system allows for O(1) lookup of chunks by their ID, significantly improving performance for large containers.

## Version Evolution

The .utoc file format has evolved across Unreal Engine versions to add new features and improve performance:

### UE4.26 (Initial)
- Basic .utoc file format with essential features
- Support for compression and encryption
- Simple chunk lookup

### UE4.26-UE4.27 (DirectoryIndex, PartitionSize)
- Added directory index for file path lookup
- Added support for partitioned .ucas files

### UE5.0-UE5.3 (PerfectHash, PerfectHashWithOverflow)
- Added perfect hash table for efficient chunk lookup
- Extended perfect hash with overflow for handling collisions
- Improved performance for large containers

### UE5.4-UE5.5 (OnDemandMetaData, RemovedOnDemandMetaData, ReplaceIoChunkHashWithIoHash)
- Added on-demand metadata loading
- Later removed on-demand metadata
- Replaced chunk hash with IO hash for better compatibility

## Container Header

The container header is a special chunk (ChunkType = ContainerHeader) that contains information about the packages in the container:

1. **Package IDs**: List of package IDs in the container
2. **Store Entries**: Metadata about each package
3. **Package Redirects**: Information about package redirects
4. **Localized Packages**: Support for localized versions of packages

The container header has its own version system:

| Version                 | UE Version | Key Features                     |
| ----------------------- | ---------- | -------------------------------- |
| PreInitial              | -          | Pre-initial version              |
| Initial                 | UE4.26-27  | Initial specification            |
| LocalizedPackages       | UE5.0      | Added localized packages support |
| OptionalSegmentPackages | UE5.1-5.2  | Added optional segment packages  |
| NoExportInfo            | UE5.3-5.4  | Removed export information       |
| SoftPackageReferences   | UE5.5+     | Added soft package references    |

## Reading Process

The process of reading a .utoc file involves:

1. **Reading the header**: Parse the FIoStoreTocHeader to get basic information about the container
2. **Reading chunk IDs**: Load the array of chunk IDs
3. **Reading chunk offsets and lengths**: Load the location and size of each chunk
4. **Reading the hash map**: Load the perfect hash table (if present)
5. **Reading compression blocks**: Load information about compressed blocks
6. **Reading compression methods**: Load the list of compression methods
7. **Reading signatures**: Load signatures if the container is signed
8. **Reading the directory index**: Load the file structure
9. **Reading chunk metadata**: Load hash and flags for each chunk

Once the .utoc file is loaded, the engine can efficiently access chunks in the .ucas file as needed.

## Writing Process

Creating a .utoc file involves:

1. **Preparing chunk data**: Organize the data into chunks
2. **Compressing chunks**: Apply compression if desired
3. **Writing chunks to .ucas**: Write the chunk data to the .ucas file
4. **Building the directory index**: Create the file structure
5. **Creating the hash map**: Generate the perfect hash table
6. **Writing the .utoc file**: Write all the metadata and indexing information

## Encryption

The .utoc file supports AES-256 encryption for both the directory index and the chunk data:

1. **Directory Index Encryption**: The directory index can be encrypted to protect file paths
2. **Chunk Data Encryption**: Individual chunks in the .ucas file can be encrypted
3. **Encryption Key GUID**: The .utoc file stores a GUID that identifies the encryption key

Encrypted blocks are aligned to AES block boundaries (16 bytes) for efficient decryption.

## Partitioning

In newer versions, the .ucas file can be partitioned into multiple files:

1. **Partition Size**: The maximum size of each partition
2. **Partition Count**: The number of partitions
3. **Partition Index**: Each compression block includes a partition index

Partitioning allows for better parallel loading and reduced file size, which can be beneficial for certain platforms and distribution methods.

## Implementation Considerations

When implementing a reader or writer for the .utoc file format, consider the following:

1. **Version Compatibility**: Handle different versions of the format
2. **Efficient Chunk Lookup**: Use the perfect hash table for fast chunk access
3. **Memory Management**: Load only the necessary parts of the .utoc file
4. **Compression Support**: Implement the required compression methods
5. **Encryption Handling**: Support AES-256 decryption
6. **Partitioned Files**: Handle multiple .ucas partition files

## Conclusion

The .utoc file format is a sophisticated and efficient solution for managing game assets in Unreal Engine. By understanding its structure and functionality, developers can effectively work with IoStore containers, whether for modding existing games or creating tools for asset management.
