# PAK File Format Research Notes

## Source Code Analysis

I've analyzed the following source files from the repak library:

1. `lib.rs` - Main library definitions, version enums
2. `pak.rs` - Main implementation for handling .pak files
3. `footer.rs` - Footer structure and handling
4. `entry.rs` - File entry structure and handling
5. `data.rs` - Data handling and compression
6. `error.rs` - Error handling
7. `ext.rs` - Extension traits for reading/writing

## Key Findings

### Magic Number
- The .pak file format uses a magic number of `0x5A6F12E1` to identify valid .pak files

### Version Information
- Multiple versions exist (V0-V11)
- Each version has specific features and capabilities
- Version major categories:
  - `Unknown` (V0)
  - `Initial` (V1)
  - `NoTimestamps` (V2)
  - `CompressionEncryption` (V3)
  - `IndexEncryption` (V4)
  - `RelativeChunkOffsets` (V5)
  - `DeleteRecords` (V6)
  - `EncryptionKeyGuid` (V7)
  - `FNameBasedCompression` (V8A, V8B)
  - `FrozenIndex` (V9)
  - `PathHashIndex` (V10)
  - `Fnv64BugFix` (V11)

### File Structure
- Header with magic number and version
- File entries containing metadata and data
- Index for quick file lookup
- Footer with information about the index location and encryption

### Compression
- Multiple compression methods supported:
  - Zlib
  - Gzip
  - Oodle
  - Zstd
  - LZ4
- Compression can be applied to individual files
- Compression block size is configurable

### Encryption
- AES encryption support
- Can encrypt just the index or the entire file
- Encryption key GUID for key identification

### Index Structure
- Mount point for virtual file paths
- File entries with paths, offsets, sizes
- Path hash index for faster lookups (V10+)
- Full directory index for directory structure

### Entry Structure
- Offset within the .pak file
- Compressed and uncompressed sizes
- Compression method used
- Hash for verification
- Blocks for chunked data
- Flags for encryption and deletion status

## Questions to Address in Documentation

1. What is the exact binary layout of a .pak file?
2. How does the index structure work in different versions?
3. How are file entries encoded in different versions?
4. How does the path hash index work?
5. What is the full directory index structure?
6. How does encryption work in practice?
7. What are the compression block structures?
8. How do the different versions affect compatibility?

## Next Steps

1. Create a detailed binary file layout document
2. Document the version differences in detail
3. Create diagrams for the file structure
4. Document the encryption and compression methods
5. Create a comprehensive reference for the .pak file format
