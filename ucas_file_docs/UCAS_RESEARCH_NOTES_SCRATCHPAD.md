# UCAS File Format Research Notes

## Overview

The UCAS (Unreal Content Archive Storage) file format is part of Unreal Engine's IoStore container system, introduced in Unreal Engine 5 as a replacement for the older .pak file system. UCAS files work in conjunction with UTOC (Unreal Table of Contents) files to form a complete IoStore container.

While UTOC files contain metadata and indexing information, UCAS files store the actual content/data of game assets. This separation allows for efficient access to specific assets without having to scan the entire archive.

## Key Characteristics from Documentation

From the documentation, I've gathered that UCAS files:

- Store the actual asset data in chunks
- Support compressed data using various methods (Zlib, Gzip, Zstd, LZ4, Oodle)
- Support encrypted data (AES-256)
- Are organized in blocks for efficient access
- Can be partitioned for better performance (e.g., global.ucas.0, global.ucas.1)

## Research Plan

1. Search for UCAS-related code in both retoc and repak projects
2. Analyze the file format structure from the source code
3. Identify how chunks are stored and accessed
4. Understand compression and encryption mechanisms
5. Document the binary file layout in detail
6. Create diagrams to visualize the file structure

## Source Code Findings

After examining the source code, I've found several key components related to UCAS files:

### FilePool

The `FilePool` struct in `file_pool.rs` is responsible for managing file handles to UCAS files. It provides a way to acquire and release file handles efficiently, which is important for parallel processing of UCAS files.

```rust
pub struct FilePool {
    inner: Arc<FilePoolInner>,
}

impl FilePool {
    pub fn new<P: Into<PathBuf>>(path: P, max_handles: usize) -> std::io::Result<Self> {
        let path = path.into();
        // open file once to verify we can
        fs::File::open(&path)?;

        Ok(FilePool {
            inner: Arc::new(FilePoolInner {
                path,
                state: Mutex::new(PoolState {
                    available_files: VecDeque::new(),
                    active_count: 0,
                }),
                max_handles,
                condvar: Condvar::new(),
            }),
        })
    }

    pub fn acquire(&self) -> std::io::Result<PooledFileHandle> {
        // Implementation for acquiring a file handle
    }
}
```

### Toc (Table of Contents)

The `Toc` struct in `main.rs` is responsible for parsing and managing the UTOC file, which contains metadata about the chunks stored in the UCAS file. It includes methods for reading chunks from the UCAS file based on the metadata in the UTOC file.

```rust
struct Toc {
    config: Arc<Config>,

    // serialized members
    chunks: Vec<FIoChunkId>,
    chunk_offset_lengths: Vec<FIoOffsetAndLength>,
    chunk_perfect_hash_seeds: Vec<i32>,
    chunk_indices_without_perfect_hash: Vec<i32>,
    compression_blocks: Vec<FIoStoreTocCompressedBlockEntry>,
    compression_methods: Vec<CompressionMethod>,
    signatures: Option<TocSignatures>,
    chunk_metas: Vec<FIoStoreTocEntryMeta>,

    // serialized in header
    version: EIoStoreTocVersion,
    container_id: FIoContainerId,
    compression_block_size: u32,
    partition_size: u64,
    partition_count: u32,
    encryption_key_guid: FGuid,
    container_flags: EIoContainerFlags,

    // transient indexes
    directory_index: FIoDirectoryIndexResource,
    file_map: HashMap<String, u32>,
    file_map_lower: HashMap<String, u32>,
    file_map_rev: HashMap<u32, String>,
    chunk_id_map: HashMap<FIoChunkId, u32>,
}
```

### Reading Data from UCAS Files

The `read` method in the `Toc` struct is responsible for reading chunks from the UCAS file:

```rust
fn read<C: Read + Seek>(&self, cas_stream: &mut C, toc_entry_index: u32) -> Result<Vec<u8>> {
    let offset_and_length = &self.chunk_offset_lengths[toc_entry_index as usize];
    let offset = offset_and_length.get_offset();
    let size = offset_and_length.get_length();

    let compression_block_size = self.compression_block_size;
    let first_block_index = (offset / compression_block_size as u64) as usize;
    let last_block_index = ((align_u64(offset + size, compression_block_size as u64) - 1)
        / compression_block_size as u64) as usize;

    let blocks = &self.compression_blocks[first_block_index..=last_block_index];
    let aes_key = if self.container_flags.contains(EIoContainerFlags::Encrypted) {
        Some(
            self.config
                .aes_keys
                .get(&self.encryption_key_guid)
                .with_context(|| {
                    format!(
                        "container is encrypted but no AES key for {:?} supplied",
                        self.encryption_key_guid
                    )
                })?,
        )
    } else {
        None
    };

    // Read and decompress/decrypt the data
    // ...
}
```

### IoStoreContainer

The `IoStoreContainer` struct in `iostore.rs` represents a container consisting of a UTOC file and its corresponding UCAS file. It provides methods for reading chunks from the container.

```rust
pub struct IoStoreContainer {
    name: String,
    path: PathBuf,
    toc: Toc,
    cas: FilePool,

    container_header: Option<FIoContainerHeader>,
}

impl IoStoreContainer {
    pub fn open<P: AsRef<Path>>(toc_path: P, config: Arc<Config>) -> Result<Self> {
        let path = toc_path.as_ref().to_path_buf();
        let toc: Toc = BufReader::new(fs::File::open(&path)?).de_ctx(config.clone())?;
        let cas = FilePool::new(path.with_extension("ucas"), rayon::max_num_threads())?;

        // ...
    }
}
```

### Compression and Encryption

The UCAS file format supports both compression and encryption:

- Compression is handled through various methods (Zlib, Gzip, Zstd, LZ4, Oodle)
- Encryption is done using AES-256
- The compression and encryption information is stored in the UTOC file

## Binary File Layout

Based on the source code analysis, the UCAS file format has a relatively simple structure compared to the UTOC file. It consists of a series of data blocks that store the actual content of game assets.

### Basic Structure

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

Each data block corresponds to a chunk of data referenced by the UTOC file. The UTOC file contains metadata about these chunks, including:

- Chunk IDs
- Offsets and lengths within the UCAS file
- Compression information
- Hash values for verification

### Data Blocks

Each data block in the UCAS file has the following characteristics:

1. **Size**: The size of each block is determined by the `compression_block_size` value in the UTOC file (typically 0x10000 or 65536 bytes).
2. **Alignment**: Blocks may be aligned for AES encryption (16-byte alignment).
3. **Content**: The actual binary data of the asset chunk.

### Accessing Data

To access data in a UCAS file:

1. The UTOC file is parsed to get the chunk metadata.
2. The chunk ID is used to find the corresponding entry in the UTOC file.
3. The offset and length information is used to locate the data in the UCAS file.
4. If the data is compressed or encrypted, it is processed accordingly.

### Writing Data

When writing data to a UCAS file:

1. The data is divided into blocks of size `compression_block_size`.
2. Each block is written to the UCAS file.
3. The offset, size, and hash information is recorded in the UTOC file.
4. If compression is used, each block is compressed before writing.
5. If encryption is used, each block is encrypted before writing.

## Compression and Encryption

The UCAS file format supports both compression and encryption to reduce file size and protect content.

### Compression

The following compression methods are supported:

1. **Zlib**: A widely used compression algorithm.
2. **Zstd**: A newer compression algorithm with better compression ratios and speed.
3. **LZ4**: A fast compression algorithm with lower compression ratios.
4. **Oodle**: A proprietary compression algorithm developed by RAD Game Tools, known for its high performance in game assets.

The compression method is specified in the UTOC file for each block. The compression information includes:

- Compression method index
- Compressed size
- Uncompressed size

```rust
// From compression.rs
pub fn decompress(compression: CompressionMethod, input: &[u8], output: &mut [u8]) -> Result<()> {
    match compression {
        CompressionMethod::Zlib => {
            flate2::read::ZlibDecoder::new(input).read_exact(output)?;
        }
        CompressionMethod::Zstd => {
            zstd::bulk::decompress_to_buffer(input, output)?;
        }
        CompressionMethod::LZ4 => {
            lz4_flex::block::decompress_into(input, output)?;
        }
        CompressionMethod::Oodle => {
            let status = liboodle::oodle()?.decompress(input, output);
            if status < 0 || status as usize != output.len() {
                bail!(
                    "Oodle decompression failed: expected {} output bytes, got {}",
                    output.len(),
                    status,
                );
            }
        }
    }
    Ok(())
}
```

### Encryption

Encryption is done using AES-256 in ECB mode. The encryption key is specified in the UTOC file and is identified by a GUID. Encrypted blocks are aligned to AES block boundaries (16 bytes).

```rust
// From main.rs (Toc::read method)
let aes_key = if self.container_flags.contains(EIoContainerFlags::Encrypted) {
    Some(
        self.config
            .aes_keys
            .get(&self.encryption_key_guid)
            .with_context(|| {
                format!(
                    "container is encrypted but no AES key for {:?} supplied",
                    self.encryption_key_guid
                )
            })?,
    )
} else {
    None
};

// ...

if let Some(key) = aes_key {
    let out = &mut out[..align_usize(uncompressed_size, 16)];
    cas_stream.read_exact(out)?;
    for block in out.chunks_mut(16) {
        key.0.decrypt_block(block.into());
    }
} else {
    cas_stream.read_exact(&mut out[..uncompressed_size])?;
}
```

## Partitioning

The UCAS file format supports partitioning, which allows the content to be split across multiple files. This is particularly useful for large games where a single UCAS file might be too large to handle efficiently.

### Partitioning Mechanism

Partitioning works by splitting the UCAS file into multiple files, each with a specific size limit. The partitioning information is stored in the UTOC file, which includes:

1. **Partition Size**: The maximum size of each partition.
2. **Partition Count**: The number of partitions.

```rust
// From main.rs (Toc struct)
struct Toc {
    // ...
    partition_size: u64,
    partition_count: u32,
    // ...
}
```

### Partition Naming

When a UCAS file is partitioned, the partitions are named using a numeric suffix:

- `global.ucas.0`
- `global.ucas.1`
- `global.ucas.2`
- etc.

### Accessing Partitioned Data

When reading data from a partitioned UCAS file, the system needs to determine which partition contains the data:

1. The offset of the data is used to calculate the partition index.
2. The appropriate partition file is opened.
3. The data is read from the partition.

```rust
// From main.rs (FIoStoreTocChunkInfo struct)
struct FIoStoreTocChunkInfo {
    // ...
    partition_index: i32,
    // ...
}
```

The partition index is calculated based on the offset of the compression block:

```rust
// From main.rs (Toc::get_chunk_info method)
partition_index = (compression_block.get_offset() / self.partition_size) as i32;
```

### Benefits of Partitioning

Partitioning provides several benefits:

1. **Improved Loading Performance**: Smaller files can be loaded more efficiently, especially on platforms with limited memory.
2. **Parallel Loading**: Multiple partitions can be loaded in parallel, improving loading times.
3. **Reduced Memory Usage**: The system can load only the partitions it needs, rather than the entire UCAS file.
4. **Better Caching**: Smaller files are more likely to be cached by the operating system.

## Version Differences

The UCAS file format has evolved across different versions of Unreal Engine. While the basic structure remains the same, there are some differences in how the format is used and interpreted.

### Version History

The UCAS file format is closely tied to the UTOC file format, which has the following version history:

| Version                      | UE Version | Key Features                        |
| ---------------------------- | ---------- | ----------------------------------- |
| Invalid                      | -          | Invalid version                     |
| Initial                      | UE4.26     | Initial specification               |
| DirectoryIndex               | UE4.26     | Added directory index               |
| PartitionSize                | UE4.27     | Added partition size                |
| PerfectHash                  | UE5.0      | Added perfect hash for chunk lookup |
| PerfectHashWithOverflow      | UE5.0-5.3  | Extended perfect hash with overflow |
| OnDemandMetaData             | UE5.4      | Added on-demand metadata            |
| RemovedOnDemandMetaData      | -          | Removed on-demand metadata          |
| ReplaceIoChunkHashWithIoHash | UE5.5      | Replaced chunk hash with IO hash    |

```rust
// From main.rs
#[derive(
    Debug, Default, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, FromRepr, Serialize, Deserialize,
)]
#[repr(u8)]
enum EIoStoreTocVersion {
    #[default]
    Invalid,
    Initial,
    DirectoryIndex,
    PartitionSize,
    PerfectHash,
    PerfectHashWithOverflow,
    OnDemandMetaData,
    RemovedOnDemandMetaData,
    ReplaceIoChunkHashWithIoHash,
}
```

### Key Version Changes

1. **UE4.26-UE4.27 (Initial, DirectoryIndex, PartitionSize)**:
   - Basic UCAS file format
   - Added support for directory indexing
   - Added support for partitioning

2. **UE5.0-UE5.3 (PerfectHash, PerfectHashWithOverflow)**:
   - Improved chunk lookup with perfect hashing
   - Added support for overflow in the perfect hash table
   - Better integration with the engine's asset loading system

3. **UE5.4-UE5.5 (OnDemandMetaData, RemovedOnDemandMetaData, ReplaceIoChunkHashWithIoHash)**:
   - Added and later removed on-demand metadata
   - Replaced chunk hash with IO hash for better performance
   - Enhanced partitioning capabilities
   - Optimized for on-demand loading

### Impact on UCAS Files

While these version changes primarily affect the UTOC file format, they also impact how UCAS files are accessed and interpreted:

1. **Partitioning**: The PartitionSize version added support for partitioning UCAS files.
2. **Chunk Hashing**: The ReplaceIoChunkHashWithIoHash version changed how chunks are hashed and verified.
3. **Chunk Lookup**: The PerfectHash and PerfectHashWithOverflow versions improved how chunks are looked up in the UTOC file.

### Compatibility

The retoc tool is designed to handle all versions of the UCAS file format, from UE4.26 to the latest UE5.5. This ensures compatibility across different engine versions.

```rust
// From iostore.rs (IoStoreContainer::open method)
pub fn open<P: AsRef<Path>>(toc_path: P, config: Arc<Config>) -> Result<Self> {
    let path = toc_path.as_ref().to_path_buf();
    let toc: Toc = BufReader::new(fs::File::open(&path)?).de_ctx(config.clone())?;
    let cas = FilePool::new(path.with_extension("ucas"), rayon::max_num_threads())?;
    // ...
}
```

## Notes on Implementation

Based on the analysis of the retoc codebase, here are some important implementation notes for working with UCAS files:

### Reading UCAS Files

When implementing a reader for UCAS files, consider the following:

1. **File Pooling**: The retoc implementation uses a file pool to manage file handles efficiently. This is important for parallel processing of UCAS files.

```rust
// From file_pool.rs
pub struct FilePool {
    inner: Arc<FilePoolInner>,
}

impl FilePool {
    pub fn new<P: Into<PathBuf>>(path: P, max_handles: usize) -> std::io::Result<Self> {
        // Implementation for creating a file pool
    }

    pub fn acquire(&self) -> std::io::Result<PooledFileHandle> {
        // Implementation for acquiring a file handle
    }
}
```

2. **Chunk Reading**: Reading chunks from a UCAS file involves several steps:
   - Finding the chunk in the UTOC file
   - Determining the offset and length of the chunk
   - Reading the data from the UCAS file
   - Decompressing and/or decrypting the data if necessary

```rust
// From main.rs (Toc::read method)
fn read<C: Read + Seek>(&self, cas_stream: &mut C, toc_entry_index: u32) -> Result<Vec<u8>> {
    // Implementation for reading a chunk from a UCAS file
}
```

3. **Partitioning**: If the UCAS file is partitioned, you need to determine which partition contains the data and open the appropriate file.

### Writing UCAS Files

When implementing a writer for UCAS files, consider the following:

1. **Block Size**: The default block size is 0x10000 (65536) bytes. This is the size of each data block in the UCAS file.

```rust
// From iostore_writer.rs (IoStoreWriter::new method)
pub(crate) fn new<P: AsRef<Path>>(
    toc_path: P,
    toc_version: EIoStoreTocVersion,
    container_header_version: Option<EIoContainerHeaderVersion>,
    mount_point: UEPathBuf,
) -> Result<Self> {
    // ...
    toc.compression_block_size = 0x10000;
    // ...
}
```

2. **Chunk Writing**: Writing chunks to a UCAS file involves several steps:
   - Dividing the data into blocks of size `compression_block_size`
   - Writing each block to the UCAS file
   - Recording the offset, size, and hash information in the UTOC file
   - Compressing and/or encrypting the data if necessary

```rust
// From iostore_writer.rs (IoStoreWriter::write_chunk method)
pub(crate) fn write_chunk(
    &mut self,
    chunk_id: FIoChunkId,
    path: Option<&UEPath>,
    data: &[u8],
) -> Result<()> {
    // Implementation for writing a chunk to a UCAS file
}
```

3. **Hashing**: Each chunk is hashed using Blake3 for verification.

```rust
// From iostore_writer.rs (IoStoreWriter::write_chunk method)
let mut hasher = blake3::Hasher::new();
for block in data.chunks(self.toc.compression_block_size as usize) {
    self.cas_stream.write_all(block)?;
    hasher.update(block);
    // ...
}
let hash = hasher.finalize();
```

### Performance Considerations

1. **Parallel Processing**: The retoc implementation uses Rayon for parallel processing of UCAS files. This is important for performance when working with large files.

```rust
// From iostore.rs (IoStoreContainer::open method)
let cas = FilePool::new(path.with_extension("ucas"), rayon::max_num_threads())?;
```

2. **Memory Management**: The retoc implementation uses a buffer pool to manage memory efficiently. This is important for performance when working with large files.

3. **Chunk Lookup**: The UTOC file uses a perfect hash table for efficient chunk lookup. This is important for performance when working with large files.

### Error Handling

1. **Missing Files**: The retoc implementation checks if the UCAS file exists before trying to open it.

```rust
// From file_pool.rs (FilePool::new method)
pub fn new<P: Into<PathBuf>>(path: P, max_handles: usize) -> std::io::Result<Self> {
    let path = path.into();
    // open file once to verify we can
    fs::File::open(&path)?;
    // ...
}
```

2. **Encryption Errors**: The retoc implementation checks if the UCAS file is encrypted and if the encryption key is available.

```rust
// From main.rs (Toc::read method)
let aes_key = if self.container_flags.contains(EIoContainerFlags::Encrypted) {
    Some(
        self.config
            .aes_keys
            .get(&self.encryption_key_guid)
            .with_context(|| {
                format!(
                    "container is encrypted but no AES key for {:?} supplied",
                    self.encryption_key_guid
                )
            })?,
    )
} else {
    None
};
```

3. **Compression Errors**: The retoc implementation checks if the compression method is supported and if the decompression was successful.

```rust
// From compression.rs (decompress function)
pub fn decompress(compression: CompressionMethod, input: &[u8], output: &mut [u8]) -> Result<()> {
    match compression {
        // ...
        CompressionMethod::Oodle => {
            let status = liboodle::oodle()?.decompress(input, output);
            if status < 0 || status as usize != output.len() {
                bail!(
                    "Oodle decompression failed: expected {} output bytes, got {}",
                    output.len(),
                    status,
                );
            }
        }
    }
    Ok(())
}
```
