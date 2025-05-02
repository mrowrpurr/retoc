# Encryption in Unreal Engine .pak Files

## Overview

Encryption is a key security feature in Unreal Engine's .pak file format, introduced in version 3 and enhanced in subsequent versions. This document provides detailed information about the encryption mechanisms used in .pak files, how they are implemented, and best practices for working with encrypted content.

## Encryption Features

The .pak file format supports several encryption features:

1. **Data Encryption**: Encrypts the actual file content
2. **Index Encryption**: Encrypts the file index (added in V4)
3. **Encryption Key GUID**: Identifies which encryption key to use (added in V7)

These features can be used independently or in combination to provide different levels of security.

## Encryption Algorithm

.pak files use the AES-256 encryption algorithm in ECB (Electronic Codebook) mode. AES (Advanced Encryption Standard) is a widely used symmetric encryption algorithm that provides strong security when implemented correctly.

### Key Characteristics

- **Algorithm**: AES-256
- **Mode**: ECB (Electronic Codebook)
- **Key Size**: 256 bits (32 bytes)
- **Block Size**: 16 bytes

### ECB Mode Considerations

ECB mode encrypts each block independently using the same key. This has some important implications:

- Identical plaintext blocks will encrypt to identical ciphertext blocks
- No initialization vector (IV) is used
- Each 16-byte block is encrypted separately

While ECB mode is generally not recommended for general-purpose encryption due to potential pattern leakage, it's used in .pak files for several reasons:

1. **Random Access**: Allows for decrypting specific blocks without decrypting the entire file
2. **Simplicity**: Easier to implement and manage without IVs
3. **Performance**: Faster and can be parallelized

## Implementation Details

### Data Encryption

When data encryption is enabled:

1. Each file's data is divided into 16-byte blocks
2. Each block is encrypted independently using AES-256 in ECB mode
3. If the last block is less than 16 bytes, it's padded to 16 bytes
4. The encrypted size is aligned to 16 bytes (AES block size)

The encryption flag in the file entry indicates whether the file data is encrypted.

### Index Encryption

When index encryption is enabled (V4+):

1. The entire index section is encrypted as a whole
2. The encrypted index flag in the footer indicates that the index is encrypted
3. The index must be decrypted before it can be parsed

Index encryption protects the file names, paths, and metadata, making it harder to determine what files are contained in the .pak archive.

### Encryption Key GUID

In version 7 and above, an encryption key GUID was added to the footer:

1. The GUID is a 128-bit (16-byte) identifier for the encryption key
2. It allows for identifying which key should be used to decrypt the content
3. This enables support for multiple encryption keys across different .pak files

The GUID is stored unencrypted in the footer, allowing tools to determine which key to use without having to try multiple keys.

## Encryption Process

### Encrypting a .pak File

When creating an encrypted .pak file:

1. **Decide on Encryption Strategy**:
   - Data encryption only
   - Index encryption only
   - Both data and index encryption

2. **Generate or Obtain Encryption Key**:
   - 256-bit (32-byte) key
   - Generate a GUID for the key (V7+)

3. **For Data Encryption**:
   - Set the encryption flag for each file entry
   - Encrypt each file's data in 16-byte blocks
   - Align encrypted data to 16-byte boundaries

4. **For Index Encryption**:
   - Set the encrypted index flag in the footer
   - Encrypt the entire index section
   - Update the index hash in the footer

### Decrypting a .pak File

When reading an encrypted .pak file:

1. **Read the Footer**:
   - Check if the index is encrypted
   - Get the encryption key GUID (V7+)

2. **Obtain the Correct Key**:
   - Use the GUID to identify which key to use (V7+)
   - Provide the key to the decryption routine

3. **Decrypt the Index (if encrypted)**:
   - Decrypt the entire index section
   - Parse the decrypted index

4. **For Each Encrypted File**:
   - Check the encryption flag in the file entry
   - Decrypt the file data in 16-byte blocks
   - Remove any padding from the last block

## Code Examples

### AES-256 ECB Encryption/Decryption

Here's a simplified example of how encryption and decryption might be implemented using OpenSSL:

```cpp
#include <openssl/aes.h>
#include <vector>
#include <cstring>

// Encrypt data using AES-256 in ECB mode
std::vector<uint8_t> EncryptAES256ECB(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    // Ensure key is 32 bytes (256 bits)
    if (key.size() != 32) {
        throw std::runtime_error("AES-256 requires a 32-byte key");
    }
    
    // Set up AES encryption key
    AES_KEY aesKey;
    AES_set_encrypt_key(key.data(), 256, &aesKey);
    
    // Calculate padded size (multiple of 16 bytes)
    size_t paddedSize = (data.size() + 15) & ~15;
    std::vector<uint8_t> encrypted(paddedSize);
    
    // Encrypt each 16-byte block
    for (size_t i = 0; i < paddedSize; i += 16) {
        // Create a block with padding if needed
        uint8_t block[16] = {0};
        size_t blockSize = std::min(size_t(16), data.size() - i);
        std::memcpy(block, data.data() + i, blockSize);
        
        // Encrypt the block
        AES_encrypt(block, encrypted.data() + i, &aesKey);
    }
    
    return encrypted;
}

// Decrypt data using AES-256 in ECB mode
std::vector<uint8_t> DecryptAES256ECB(const std::vector<uint8_t>& encrypted, const std::vector<uint8_t>& key, size_t originalSize) {
    // Ensure key is 32 bytes (256 bits)
    if (key.size() != 32) {
        throw std::runtime_error("AES-256 requires a 32-byte key");
    }
    
    // Ensure encrypted data is a multiple of 16 bytes
    if (encrypted.size() % 16 != 0) {
        throw std::runtime_error("Encrypted data size must be a multiple of 16 bytes");
    }
    
    // Set up AES decryption key
    AES_KEY aesKey;
    AES_set_decrypt_key(key.data(), 256, &aesKey);
    
    std::vector<uint8_t> decrypted(encrypted.size());
    
    // Decrypt each 16-byte block
    for (size_t i = 0; i < encrypted.size(); i += 16) {
        AES_decrypt(encrypted.data() + i, decrypted.data() + i, &aesKey);
    }
    
    // Resize to original size (remove padding)
    decrypted.resize(originalSize);
    
    return decrypted;
}
```

### Reading an Encrypted .pak File

Here's a simplified example of how to read an encrypted .pak file:

```cpp
bool ReadEncryptedPakFile(const std::string& pakFilePath, const std::vector<uint8_t>& key) {
    std::ifstream file(pakFilePath, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    
    // Read footer (assuming V11 format with 213-byte footer)
    file.seekg(fileSize - 213);
    
    // Read encryption GUID (16 bytes)
    std::vector<uint8_t> encryptionGuid(16);
    file.read(reinterpret_cast<char*>(encryptionGuid.data()), 16);
    
    // Read encrypted index flag (1 byte)
    uint8_t encryptedIndex;
    file.read(reinterpret_cast<char*>(&encryptedIndex), 1);
    
    // Read magic number (4 bytes)
    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), 4);
    if (magic != 0x5A6F12E1) {
        return false; // Not a valid .pak file
    }
    
    // Read version (4 bytes)
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), 4);
    
    // Read index offset and size (8 bytes each)
    uint64_t indexOffset, indexSize;
    file.read(reinterpret_cast<char*>(&indexOffset), 8);
    file.read(reinterpret_cast<char*>(&indexSize), 8);
    
    // Read index
    file.seekg(indexOffset);
    std::vector<uint8_t> indexData(indexSize);
    file.read(reinterpret_cast<char*>(indexData.data()), indexSize);
    
    // Decrypt index if needed
    if (encryptedIndex) {
        indexData = DecryptAES256ECB(indexData, key, indexSize);
    }
    
    // Parse index...
    // (Implementation depends on the specific version)
    
    return true;
}
```

## Key Management

Proper key management is crucial for the security of encrypted .pak files:

### Key Generation

Keys should be:
- Truly random (use a cryptographically secure random number generator)
- 256 bits (32 bytes) in length
- Unique for each project or release

### Key Storage

Keys should be stored securely:
- Never hardcode keys in source code
- Use secure key storage mechanisms provided by the platform
- Consider using key derivation from user credentials

### Key Distribution

For games that use encrypted .pak files:
- Keys may be embedded in the game executable
- Keys may be downloaded from a secure server
- Keys may be derived from user credentials or license information

### Key Rotation

Consider key rotation strategies:
- Different keys for different content
- Different keys for different releases
- Different keys for different platforms

## Best Practices

### When to Use Encryption

Encryption is useful for:
- Protecting proprietary assets
- Preventing unauthorized access to content
- Implementing DRM systems
- Securing pre-release content

### Performance Considerations

Encryption adds processing overhead:
- Decryption takes time, especially for large files
- Consider encrypting only sensitive content
- Test performance impact on target platforms

### Security Considerations

Be aware of security limitations:
- ECB mode can reveal patterns in the encrypted data
- Keys embedded in executables can be extracted
- Consider the security requirements of your specific use case

### Selective Encryption

Consider selective encryption strategies:
- Encrypt only sensitive files
- Use index encryption to hide file names
- Balance security needs with performance requirements

## Compatibility Considerations

### Version Compatibility

Different versions of the .pak file format have different encryption capabilities:
- V3: Basic data encryption
- V4: Added index encryption
- V7: Added encryption key GUID

### Platform Considerations

Ensure consistent encryption across platforms:
- Use the same encryption implementation on all platforms
- Test decryption on all target platforms
- Be aware of endianness issues

### Tool Support

Different tools have different levels of encryption support:
- UnrealPak: Full support for encryption
- repak: Support for reading encrypted .pak files
- Third-party tools: Varying levels of support

## Conclusion

Encryption is a powerful feature of the .pak file format that can help protect sensitive content. By understanding how encryption is implemented in .pak files and following best practices for key management, developers can effectively secure their game assets while maintaining performance and compatibility.

When implementing encryption for .pak files, it's important to carefully consider the security requirements, performance implications, and compatibility issues specific to your project.
