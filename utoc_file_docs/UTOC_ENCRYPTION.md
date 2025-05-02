# UTOC Encryption System

## Overview

The Encryption System is an important feature of the .utoc file format that provides security for game assets. It allows for the encryption of both metadata in the .utoc file and content in the .ucas file. This document explains how the encryption system works and how it's implemented in the .utoc file format.

## Purpose

The encryption system serves several important purposes:

1. **Content Protection**: Protects game assets from unauthorized access
2. **IP Security**: Safeguards intellectual property
3. **Anti-Tampering**: Prevents modification of game assets
4. **Selective Encryption**: Allows for encryption of specific components (directory index, chunk data)

## Binary Structure

The encryption information in the .utoc file is stored in the header:

```
Offset  Size    Type    Description
------  ------  ------  -----------
0x40    16      FGuid   EncryptionKeyGuid
0x50    1       uint8   ContainerFlags (bit 1 = Encrypted)
```

The `EncryptionKeyGuid` is a 16-byte GUID that identifies the encryption key to use. The `Encrypted` bit in the `ContainerFlags` indicates whether the container uses encryption.

## Encryption Algorithm

The IoStore system uses AES-256 encryption in CBC mode for both the .utoc and .ucas files:

1. **AES-256**: Advanced Encryption Standard with a 256-bit key
2. **CBC Mode**: Cipher Block Chaining mode for block encryption
3. **Block Size**: 16 bytes (128 bits)
4. **Key Identification**: Keys are identified by GUID rather than being stored in the file

## Encrypted Components

The following components can be encrypted:

1. **Directory Index**: The hierarchical file structure in the .utoc file
2. **Chunk Data**: The actual content data in the .ucas file

## Encryption Process

### Directory Index Encryption

The directory index is encrypted as follows:

1. The directory index is serialized to a binary blob
2. The blob is padded to a multiple of 16 bytes (AES block size)
3. The blob is encrypted using AES-256 with the key identified by the EncryptionKeyGuid
4. The encrypted blob is stored in the .utoc file

### Chunk Data Encryption

Chunk data in the .ucas file is encrypted as follows:

1. The chunk data is compressed (if compression is enabled)
2. The compressed data is padded to a multiple of 16 bytes
3. The data is encrypted using AES-256 with the key identified by the EncryptionKeyGuid
4. The encrypted data is stored in the .ucas file

## Decryption Process

### Directory Index Decryption

To decrypt the directory index:

1. Read the encrypted blob from the .utoc file
2. Obtain the encryption key using the EncryptionKeyGuid
3. Decrypt the blob using AES-256
4. Deserialize the decrypted blob to obtain the directory index

### Chunk Data Decryption

To decrypt chunk data:

1. Read the encrypted data from the .ucas file
2. Obtain the encryption key using the EncryptionKeyGuid
3. Decrypt the data using AES-256
4. Decompress the decrypted data (if it was compressed)

## Key Management

The encryption system uses a key management approach where:

1. **Key Storage**: Keys are not stored in the .utoc or .ucas files
2. **Key Identification**: Keys are identified by GUID
3. **Key Provision**: Keys must be provided by the application using the files
4. **Key Rotation**: Different containers can use different keys

This approach enhances security by separating the encrypted content from the encryption keys.

## Implementation Details

### Encryption Key GUID

The `EncryptionKeyGuid` is a 16-byte structure:

```
Offset  Size    Type    Description
------  ------  ------  -----------
0x00    4       uint32  A
0x04    4       uint32  B
0x08    4       uint32  C
0x0C    4       uint32  D
```

This GUID uniquely identifies the encryption key to use.

### Alignment and Padding

Encrypted data must be aligned to AES block boundaries (16 bytes):

1. **Input Alignment**: Data to be encrypted must be padded to a multiple of 16 bytes
2. **Output Alignment**: Encrypted data will be a multiple of 16 bytes
3. **Padding Method**: Typically zero padding is used

### Encryption Flags

The `ContainerFlags` in the .utoc header includes an `Encrypted` bit (bit 1):

```
Bit     Flag        Description
------  ----------  -----------
1       Encrypted   Container has encrypted data
```

When this bit is set, the directory index and potentially chunk data are encrypted.

## Reading Encrypted Data

To read encrypted data from an IoStore container:

1. Check if the `Encrypted` bit is set in the `ContainerFlags`
2. If set, obtain the encryption key using the `EncryptionKeyGuid`
3. Decrypt the directory index to locate chunks
4. When reading a chunk, check if it's encrypted
5. If the chunk is encrypted, decrypt it before decompression

## Writing Encrypted Data

To write encrypted data to an IoStore container:

1. Set the `Encrypted` bit in the `ContainerFlags`
2. Set the `EncryptionKeyGuid` to identify the encryption key
3. Encrypt the directory index before writing it
4. When writing a chunk, encrypt it after compression
5. Ensure all encrypted data is aligned to 16-byte boundaries

## Performance Considerations

Encryption adds overhead to both reading and writing operations:

1. **CPU Usage**: Encryption and decryption require CPU resources
2. **Memory Usage**: Encrypted data must be processed in memory
3. **Alignment Overhead**: Padding to 16-byte boundaries can increase file size
4. **Key Lookup**: Looking up keys by GUID adds some overhead

## Security Considerations

When implementing encryption for IoStore containers:

1. **Key Security**: Protect encryption keys from unauthorized access
2. **Key Distribution**: Securely distribute keys to authorized users
3. **Key Rotation**: Consider rotating keys periodically
4. **Implementation Security**: Use secure implementations of AES-256

## Version Differences

The encryption system has remained relatively stable across versions of the .utoc file format, with most changes affecting other aspects of the format.

## Implementation Considerations

When implementing a reader or writer for encrypted IoStore containers:

1. **AES Implementation**: Use a secure and efficient implementation of AES-256
2. **Key Management**: Implement a secure system for managing encryption keys
3. **Error Handling**: Handle encryption and decryption errors gracefully
4. **Performance Optimization**: Optimize encryption and decryption for performance

## Conclusion

The encryption system is an important security feature of the .utoc file format that protects game assets from unauthorized access. By understanding how it works and how to implement it, developers can securely store and access assets in IoStore containers.
