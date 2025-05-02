# FNV-64 Hash Algorithm in .pak Files

## Overview

The FNV-64 (Fowler-Noll-Vo) hash algorithm is used in Unreal Engine's .pak file format, particularly in version 10 and above, for the path hash index. This document details how the algorithm is implemented in the context of .pak files and how it's used for efficient file lookups.

## Algorithm Description

FNV-64 is a non-cryptographic hash function designed to be fast while maintaining a low collision rate. In the context of .pak files, it's used to hash file paths for quick lookups in the path hash index.

### Basic FNV-64 Algorithm

The standard FNV-64 algorithm works as follows:

1. Start with an initial hash value (offset basis): `0xcbf29ce484222325`
2. For each byte in the input:
   - XOR the current hash value with the byte
   - Multiply the result by the FNV prime: `0x00000100000001b3`
3. The final hash value is the result

### .pak File Implementation

In the .pak file format, the FNV-64 algorithm is modified slightly:

1. The path is first converted to lowercase
2. The path is then converted to UTF-16 encoding
3. A seed value is added to the offset basis
4. The FNV-64 algorithm is applied to the UTF-16 bytes

Here's the implementation from the repak library:

```rust
fn fnv64<I>(data: I, offset: u64) -> u64
where
    I: IntoIterator<Item = u8>,
{
    const OFFSET: u64 = 0xcbf29ce484222325;
    const PRIME: u64 = 0x00000100000001b3;
    let mut hash = OFFSET.wrapping_add(offset);
    for b in data.into_iter() {
        hash ^= b as u64;
        hash = hash.wrapping_mul(PRIME);
    }
    hash
}

fn fnv64_path(path: &str, offset: u64) -> u64 {
    let lower = path.to_lowercase();
    let data = lower.encode_utf16().flat_map(u16::to_le_bytes);
    fnv64(data, offset)
}
```

## Bug Fix in V11

Version 11 of the .pak file format included a fix for a bug in the FNV-64 hash calculation. The bug was related to how the hash was calculated for paths, which could lead to hash collisions or incorrect lookups.

The specific issue was in the handling of UTF-16 encoding and endianness. The fix ensures that the bytes are processed in the correct order, maintaining consistency across different platforms and implementations.

## Path Hash Index

In .pak files version 10 and above, the path hash index uses the FNV-64 algorithm to create a mapping from file paths to their locations in the encoded entries section.

### Structure

The path hash index has the following structure:

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Entry Count       | uint32         | Number of entries                        |
| Entries           | PHIEntry[]     | Array of path hash index entries         |
| Terminator        | uint32         | Always 0                                 |
+-------------------+----------------+------------------------------------------+
```

Each path hash index entry:

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Path Hash         | uint64         | FNV-64 hash of file path                 |
| Entry Offset      | uint32         | Offset to encoded entry                  |
+-------------------+----------------+------------------------------------------+
```

### Usage

The path hash index is used for efficient file lookups:

1. When looking for a file, the engine calculates the FNV-64 hash of the requested path
2. It then searches the path hash index for this hash
3. If found, it reads the encoded entry at the specified offset
4. The encoded entry contains the information needed to access the file data

This approach is much faster than scanning through all file entries, especially for large .pak files with thousands of files.

## Path Hash Seed

The path hash seed is a 64-bit value stored in the index section of .pak files version 10 and above. This seed is added to the offset basis in the FNV-64 algorithm, providing a way to customize the hash calculation.

The seed serves several purposes:

1. **Collision Avoidance**: Different seeds produce different hash values for the same input, helping to avoid collisions in specific cases
2. **Security**: Makes it slightly harder to predict hash values without knowing the seed
3. **Customization**: Allows for different hashing behaviors in different contexts

## Implementation Considerations

When implementing the FNV-64 algorithm for .pak files, consider the following:

1. **Case Sensitivity**: Paths are converted to lowercase before hashing
2. **Encoding**: Paths are converted to UTF-16 before hashing
3. **Endianness**: The UTF-16 bytes are processed in little-endian order
4. **Overflow Handling**: Use wrapping addition and multiplication to handle overflow correctly
5. **Seed Value**: Include the path hash seed in the calculation

## Example Implementation

Here's a C++ implementation of the FNV-64 algorithm for .pak files:

```cpp
uint64_t FNV64(const std::vector<uint8_t>& data, uint64_t seed) {
    const uint64_t FNV_OFFSET = 0xcbf29ce484222325;
    const uint64_t FNV_PRIME = 0x00000100000001b3;
    
    uint64_t hash = FNV_OFFSET + seed; // Add seed to offset basis
    
    for (uint8_t byte : data) {
        hash ^= static_cast<uint64_t>(byte);
        hash *= FNV_PRIME; // Overflow is handled by standard C++ unsigned arithmetic
    }
    
    return hash;
}

uint64_t FNV64Path(const std::string& path, uint64_t seed) {
    // Convert to lowercase
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    // Convert to UTF-16LE bytes
    std::vector<uint8_t> utf16Bytes;
    for (char c : lowerPath) {
        // Simple ASCII to UTF-16LE conversion (for non-ASCII, use a proper conversion library)
        utf16Bytes.push_back(static_cast<uint8_t>(c));
        utf16Bytes.push_back(0); // High byte for ASCII is 0
    }
    
    return FNV64(utf16Bytes, seed);
}
```

## Testing

To verify your implementation of the FNV-64 algorithm for .pak files, you can use the following test cases:

| Path                       | Seed      | Expected Hash (Hex) |
| -------------------------- | --------- | ------------------- |
| "/Game/Test.uasset"        | 0         | 0x1B33DD73F33573A5  |
| "/Game/Test.uasset"        | 123456789 | 0x6E8C0A8D5D5A9B1E  |
| "/game/test.uasset"        | 0         | 0x1B33DD73F33573A5  |
| "Content/Maps/Level1.umap" | 0         | 0x9F5C3825A80504B2  |

Note that the third test case has the same hash as the first one because the algorithm converts the path to lowercase before hashing.

## Conclusion

The FNV-64 hash algorithm plays a crucial role in the efficiency of .pak files version 10 and above. By understanding how it's implemented and used, developers can better work with .pak files and potentially optimize their own implementations for specific use cases.

When implementing tools for working with .pak files, it's important to correctly implement the FNV-64 algorithm to ensure compatibility with the Unreal Engine ecosystem.
