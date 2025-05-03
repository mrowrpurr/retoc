#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Ucas {

/**
 * @brief Error codes for UCAS operations
 */
enum class Error {
  None,
  FileNotFound,
  InvalidFormat,
  CompressionError,
  EncryptionError,
  IoError
};

/**
 * @brief Result class for UCAS operations
 */
template <typename T> class Result {
  bool _success;
  T _value;
  Error _error;
  std::string _errorMessage;

public:
  Result(T value)
      : _success(true), _value(std::move(value)), _error(Error::None) {}
  Result(Error error, std::string errorMessage = "")
      : _success(false), _error(error), _errorMessage(std::move(errorMessage)) {
  }

  bool isSuccess() const { return _success; }
  bool isError() const { return !_success; }

  const T &value() const {
    if (!_success) {
      throw std::runtime_error("Attempted to access value of error result");
    }
    return _value;
  }

  T &&moveValue() {
    if (!_success) {
      throw std::runtime_error("Attempted to access value of error result");
    }
    return std::move(_value);
  }

  Error error() const { return _error; }
  const std::string &errorMessage() const { return _errorMessage; }
};

// Template specialization for Result<void>
template <> class Result<void> {
  bool _success;
  Error _error;
  std::string _errorMessage;

public:
  Result() : _success(true), _error(Error::None) {}
  Result(Error error, std::string errorMessage = "")
      : _success(false), _error(error), _errorMessage(std::move(errorMessage)) {
  }

  bool isSuccess() const { return _success; }
  bool isError() const { return !_success; }

  Error error() const { return _error; }
  const std::string &errorMessage() const { return _errorMessage; }
};

/**
 * @brief Compression methods supported by .ucas files
 */
enum class CompressionMethod : uint8_t {
  None = 0,
  Zlib = 1,
  Gzip = 2,
  Zstd = 3,
  Oodle = 4
};

/**
 * @brief AES key for encrypted .ucas files
 */
class AesKey {
  std::array<uint8_t, 32> _key;

public:
  AesKey(const std::array<uint8_t, 32> &key);
  AesKey(std::string_view hexOrBase64Key);

  const std::array<uint8_t, 32> &getKey() const { return _key; }
};

/**
 * @brief Chunk information
 */
struct ChunkInfo {
  uint64_t id;
  uint64_t offset;
  uint64_t size;
  uint64_t uncompressedSize;
  CompressionMethod compressionMethod;
};

/**
 * @brief Reader for .ucas files
 */
class UcasReader {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  /**
   * @brief Construct a new UCAS Reader
   * @param path Path to the .ucas file
   * @param key Optional AES key for encrypted .ucas files
   */
  UcasReader(const std::filesystem::path &path,
             const std::optional<AesKey> &key = std::nullopt);
  ~UcasReader();

  /**
   * @brief Read a chunk from the UCAS file
   * @param offset Offset of the chunk in the UCAS file
   * @param size Size of the chunk in the UCAS file
   * @param compressionMethod Compression method used for the chunk
   * @param uncompressedSize Uncompressed size of the chunk
   * @return Result containing the chunk data or an error
   */
  Result<std::vector<uint8_t>> readChunk(uint64_t offset, uint64_t size,
                                         CompressionMethod compressionMethod,
                                         uint64_t uncompressedSize) const;

  /**
   * @brief Check if the UCAS file is encrypted
   * @return True if encrypted, false otherwise
   */
  bool isEncrypted() const;

  /**
   * @brief Get the size of the UCAS file
   * @return The size of the UCAS file in bytes
   */
  uint64_t getSize() const;
};

/**
 * @brief Writer for .ucas files
 */
class UcasWriter {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  /**
   * @brief Construct a new UCAS Writer
   * @param path Path to the output .ucas file
   * @param key Optional AES key for encrypted .ucas files
   */
  UcasWriter(const std::filesystem::path &path,
             const std::optional<AesKey> &key = std::nullopt);
  ~UcasWriter();

  /**
   * @brief Write a chunk to the UCAS file
   * @param data Chunk data
   * @param compressionMethod Compression method to use
   * @return Result containing the chunk information or an error
   */
  Result<ChunkInfo>
  writeChunk(const std::vector<uint8_t> &data,
             CompressionMethod compressionMethod = CompressionMethod::Zstd);

  /**
   * @brief Finalize the UCAS file
   * @return Result containing success or an error
   */
  Result<void> finalize();
};

} // namespace Ucas
