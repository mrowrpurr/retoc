#include "ucas.h"

#include <fstream>
#include <iostream>
#include <lz4.h>
#include <memory>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <zlib.h>
#include <zstd.h>

#include "liboodle.h"

namespace Ucas {

// Helper function to convert hex string to bytes
static std::vector<uint8_t> hexToBytes(std::string_view hex) {
  std::vector<uint8_t> bytes;
  bytes.reserve(hex.length() / 2);

  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byteString = std::string(hex.substr(i, 2));
    uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
    bytes.push_back(byte);
  }

  return bytes;
}

// Helper function to convert base64 string to bytes
static std::vector<uint8_t> base64ToBytes(std::string_view base64) {
  // This is a simplified implementation
  // In a real implementation, you would use a proper base64 decoder
  // For now, we'll just return an empty vector
  return {};
}

// AesKey implementation
AesKey::AesKey(const std::array<uint8_t, 32> &key) : _key(key) {}

AesKey::AesKey(std::string_view hexOrBase64Key) {
  std::vector<uint8_t> keyBytes;

  if (hexOrBase64Key.length() == 64) {
    // Assume hex
    keyBytes = hexToBytes(hexOrBase64Key);
  } else {
    // Assume base64
    keyBytes = base64ToBytes(hexOrBase64Key);
  }

  if (keyBytes.size() != 32) {
    throw std::runtime_error("Invalid key length");
  }

  std::copy(keyBytes.begin(), keyBytes.end(), _key.begin());
}

// UcasReader implementation
struct UcasReader::Impl {
  std::filesystem::path path;
  std::optional<AesKey> key;
  std::ifstream file;
  bool encrypted;
  uint64_t fileSize;

  Impl(const std::filesystem::path &path, const std::optional<AesKey> &key)
      : path(path), key(key), encrypted(false), fileSize(0) {

    // Open the file
    file.open(path, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Failed to open file: " + path.string());
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Check if the file is encrypted
    // This is a simplified implementation
    // In a real implementation, you would check for encryption markers
    encrypted = false;
  }
};

UcasReader::UcasReader(const std::filesystem::path &path,
                       const std::optional<AesKey> &key)
    : _impl(std::make_unique<Impl>(path, key)) {}

UcasReader::~UcasReader() = default;

Result<std::vector<uint8_t>>
UcasReader::readChunk(uint64_t offset, uint64_t size,
                      CompressionMethod compressionMethod,
                      uint64_t uncompressedSize) const {
  // Check if the offset and size are valid
  if (offset + size > _impl->fileSize) {
    return Result<std::vector<uint8_t>>(Error::IoError,
                                        "Invalid offset or size");
  }

  // Read the chunk data
  std::vector<uint8_t> compressedData(size);
  _impl->file.seekg(offset);
  _impl->file.read(reinterpret_cast<char *>(compressedData.data()), size);

  // Decrypt the data if necessary
  if (_impl->encrypted) {
    if (!_impl->key) {
      return Result<std::vector<uint8_t>>(Error::EncryptionError,
                                          "Encrypted file but no key provided");
    }

    // This is a simplified implementation
    // In a real implementation, you would decrypt the data
    // For now, we'll just return an error
    return Result<std::vector<uint8_t>>(Error::EncryptionError,
                                        "Decryption not implemented");
  }

  // Decompress the data if necessary
  if (compressionMethod != CompressionMethod::None) {
    std::vector<uint8_t> decompressedData(uncompressedSize);

    switch (compressionMethod) {
    case CompressionMethod::Zlib: {
      // Decompress with zlib
      z_stream stream;
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      stream.avail_in = static_cast<uInt>(compressedData.size());
      stream.next_in = compressedData.data();
      stream.avail_out = static_cast<uInt>(decompressedData.size());
      stream.next_out = decompressedData.data();

      int ret = inflateInit(&stream);
      if (ret != Z_OK) {
        return Result<std::vector<uint8_t>>(Error::CompressionError,
                                            "Failed to initialize zlib");
      }

      ret = inflate(&stream, Z_FINISH);
      inflateEnd(&stream);

      if (ret != Z_STREAM_END) {
        return Result<std::vector<uint8_t>>(Error::CompressionError,
                                            "Failed to decompress with zlib");
      }

      return Result<std::vector<uint8_t>>(std::move(decompressedData));
    }
    case CompressionMethod::Gzip: {
      // Decompress with gzip
      // This is a simplified implementation
      // In a real implementation, you would use the gzip API
      // For now, we'll just return an error
      return Result<std::vector<uint8_t>>(Error::CompressionError,
                                          "Gzip decompression not implemented");
    }
    case CompressionMethod::Zstd: {
      // Decompress with zstd
      size_t result =
          ZSTD_decompress(decompressedData.data(), decompressedData.size(),
                          compressedData.data(), compressedData.size());
      if (ZSTD_isError(result)) {
        return Result<std::vector<uint8_t>>(Error::CompressionError,
                                            "Failed to decompress with zstd");
      }

      return Result<std::vector<uint8_t>>(std::move(decompressedData));
    }
    case CompressionMethod::Oodle: {
      // Decompress with Oodle
      // This is a simplified implementation
      // In a real implementation, you would use the Oodle API
      // For now, we'll just return an error
      return Result<std::vector<uint8_t>>(
          Error::CompressionError, "Oodle decompression not implemented");
    }
    default:
      return Result<std::vector<uint8_t>>(Error::CompressionError,
                                          "Unknown compression method");
    }
  }

  // No compression, just return the data
  return Result<std::vector<uint8_t>>(std::move(compressedData));
}

bool UcasReader::isEncrypted() const { return _impl->encrypted; }

uint64_t UcasReader::getSize() const { return _impl->fileSize; }

// UcasWriter implementation
struct UcasWriter::Impl {
  std::filesystem::path path;
  std::optional<AesKey> key;
  std::ofstream file;
  bool encrypted;
  bool finalized;
  uint64_t currentOffset;

  Impl(const std::filesystem::path &path, const std::optional<AesKey> &key)
      : path(path), key(key), encrypted(key.has_value()), finalized(false),
        currentOffset(0) {

    // Create the directory if it doesn't exist
    std::filesystem::create_directories(path.parent_path());

    // Open the file
    file.open(path, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Failed to create file: " + path.string());
    }

    // Write a placeholder header
    // In a real implementation, you would write the actual header
    // and update it when finalizing the file
    file.seekp(0);
    file.write("UCAS", 4);
    currentOffset = 4;
  }

  ~Impl() {
    if (!finalized) {
      try {
        finalize();
      } catch (...) {
        // Ignore errors during destruction
      }
    }
  }

  Result<void> finalize() {
    if (finalized) {
      return Result<void>();
    }

    // This is a simplified implementation
    // In a real implementation, you would update the header with the chunk
    // entries

    file.close();
    finalized = true;

    return Result<void>();
  }
};

UcasWriter::UcasWriter(const std::filesystem::path &path,
                       const std::optional<AesKey> &key)
    : _impl(std::make_unique<Impl>(path, key)) {}

UcasWriter::~UcasWriter() = default;

Result<ChunkInfo> UcasWriter::writeChunk(const std::vector<uint8_t> &data,
                                         CompressionMethod compressionMethod) {
  if (_impl->finalized) {
    return Result<ChunkInfo>(Error::IoError, "UCAS file has been finalized");
  }

  // Compress the data if necessary
  std::vector<uint8_t> compressedData;
  uint64_t uncompressedSize = data.size();

  if (compressionMethod != CompressionMethod::None) {
    switch (compressionMethod) {
    case CompressionMethod::Zlib: {
      // Compress with zlib
      uLongf compressedSize = compressBound(data.size());
      compressedData.resize(compressedSize);

      int ret = compress(compressedData.data(), &compressedSize, data.data(),
                         data.size());
      if (ret != Z_OK) {
        return Result<ChunkInfo>(Error::CompressionError,
                                 "Failed to compress with zlib");
      }

      compressedData.resize(compressedSize);
      break;
    }
    case CompressionMethod::Gzip: {
      // Compress with gzip
      // This is a simplified implementation
      // In a real implementation, you would use the gzip API
      // For now, we'll just return an error
      return Result<ChunkInfo>(Error::CompressionError,
                               "Gzip compression not implemented");
    }
    case CompressionMethod::Zstd: {
      // Compress with zstd
      size_t compressedSize = ZSTD_compressBound(data.size());
      compressedData.resize(compressedSize);

      size_t result =
          ZSTD_compress(compressedData.data(), compressedData.size(),
                        data.data(), data.size(), 1);
      if (ZSTD_isError(result)) {
        return Result<ChunkInfo>(Error::CompressionError,
                                 "Failed to compress with zstd");
      }

      compressedData.resize(result);
      break;
    }
    case CompressionMethod::Oodle: {
      // Compress with Oodle
      // This is a simplified implementation
      // In a real implementation, you would use the Oodle API
      // For now, we'll just return an error
      return Result<ChunkInfo>(Error::CompressionError,
                               "Oodle compression not implemented");
    }
    default:
      return Result<ChunkInfo>(Error::CompressionError,
                               "Unknown compression method");
    }
  } else {
    // No compression, just use the original data
    compressedData = data;
  }

  // Encrypt the data if necessary
  if (_impl->encrypted) {
    // This is a simplified implementation
    // In a real implementation, you would encrypt the data
    // For now, we'll just return an error
    return Result<ChunkInfo>(Error::EncryptionError,
                             "Encryption not implemented");
  }

  // Write the data to the file
  _impl->file.seekp(_impl->currentOffset);
  _impl->file.write(reinterpret_cast<const char *>(compressedData.data()),
                    compressedData.size());

  // Create the chunk info
  ChunkInfo chunkInfo;
  chunkInfo.id = 0; // This would be set by the caller
  chunkInfo.offset = _impl->currentOffset;
  chunkInfo.size = compressedData.size();
  chunkInfo.uncompressedSize = uncompressedSize;
  chunkInfo.compressionMethod = compressionMethod;

  // Update the current offset
  _impl->currentOffset += compressedData.size();

  return Result<ChunkInfo>(chunkInfo);
}

Result<void> UcasWriter::finalize() { return _impl->finalize(); }

} // namespace Ucas
