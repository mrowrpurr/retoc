#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Pak {

/**
 * @brief Compression methods supported by .pak files
 */
enum class CompressionMethod : uint8_t {
  None = 0,
  Zlib = 1,
  Gzip = 2,
  Zstd = 3,
  LZ4 = 4,
  Oodle = 5
};

/**
 * @brief Pak file version
 */
enum class Version : uint32_t {
  Invalid = 0,
  V2 = 2,   // UE 4.0-4.2
  V3 = 3,   // UE 4.3-4.15
  V4 = 4,   // UE 4.16-4.19
  V5 = 5,   // UE 4.20
  V6 = 6,   // -
  V7 = 7,   // UE 4.21
  V8A = 8,  // UE 4.22
  V8B = 9,  // UE 4.23-4.24
  V9 = 10,  // UE 4.25
  V10 = 11, // -
  V11 = 12  // UE 4.26-5.3+
};

/**
 * @brief Compression block information
 */
struct CompressionBlock {
  uint64_t compressedStart;
  uint64_t compressedEnd;
};

/**
 * @brief File entry in a .pak file
 */
struct FileEntry {
  std::string path;
  uint64_t offset;
  uint64_t size;
  uint64_t uncompressedSize;
  CompressionMethod compressionMethod;
  std::array<uint8_t, 20> hash;
  std::vector<CompressionBlock> compressionBlocks;
  uint32_t compressionBlockSize;
  bool encrypted;
};

/**
 * @brief AES key for encrypted .pak files
 */
class AesKey {
  std::array<uint8_t, 32> _key;

public:
  AesKey(const std::array<uint8_t, 32> &key);
  AesKey(std::string_view hexOrBase64Key);

  const std::array<uint8_t, 32> &getKey() const { return _key; }
};

/**
 * @brief Error codes for pak operations
 */
enum class Error {
  None,
  FileNotFound,
  InvalidFormat,
  CompressionError,
  EncryptionError,
  HashMismatch,
  IoError
};

/**
 * @brief Result class for pak operations
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

/**
 * @brief Reader for .pak files
 */
class PakReader {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  /**
   * @brief Construct a new Pak Reader
   * @param path Path to the .pak file
   * @param key Optional AES key for encrypted .pak files
   */
  PakReader(const std::filesystem::path &path,
            const std::optional<AesKey> &key = std::nullopt);
  ~PakReader();

  /**
   * @brief Check if a file exists in the .pak
   * @param path Path to check
   * @return True if the file exists, false otherwise
   */
  bool fileExists(std::string_view path) const;

  /**
   * @brief Get a list of all files in the .pak
   * @return Vector of file paths
   */
  std::vector<std::string> getFileList() const;

  /**
   * @brief Extract a file from the .pak
   * @param path Path of the file to extract
   * @return Result containing the file data or an error
   */
  Result<std::vector<uint8_t>> extractFile(std::string_view path) const;

  /**
   * @brief Extract all files from the .pak
   * @param outputDir Directory to extract files to
   * @return Result containing the number of files extracted or an error
   */
  Result<size_t> extractAllFiles(const std::filesystem::path &outputDir) const;

  /**
   * @brief Get the version of the .pak file
   * @return The pak file version
   */
  Version getVersion() const;

  /**
   * @brief Check if the .pak file is encrypted
   * @return True if encrypted, false otherwise
   */
  bool isEncrypted() const;

  /**
   * @brief Get the mount point of the .pak file
   * @return The mount point
   */
  std::string getMountPoint() const;

  /**
   * @brief Get information about a file in the .pak
   * @param path Path of the file
   * @return Result containing the file entry or an error
   */
  Result<FileEntry> getFileEntry(std::string_view path) const;

  /**
   * @brief Get all file entries in the .pak
   * @return Vector of file entries
   */
  std::vector<FileEntry> getEntries() const;
};

/**
 * @brief Writer for .pak files
 */
class PakWriter {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  /**
   * @brief Construct a new Pak Writer
   * @param path Path to the output .pak file
   * @param version Version of the .pak file to create
   * @param mountPoint Mount point for the .pak file
   */
  PakWriter(const std::filesystem::path &path, Version version,
            std::string_view mountPoint);
  ~PakWriter();

  /**
   * @brief Add a file to the .pak
   * @param path Path within the .pak
   * @param data File data
   * @param compressionMethod Compression method to use
   * @return Result containing success or an error
   */
  Result<void>
  addFile(std::string_view path, const std::vector<uint8_t> &data,
          CompressionMethod compressionMethod = CompressionMethod::Zlib);

  /**
   * @brief Add a file from disk to the .pak
   * @param sourcePath Path to the file on disk
   * @param destPath Path within the .pak
   * @param compressionMethod Compression method to use
   * @return Result containing success or an error
   */
  Result<void> addFileFromDisk(
      const std::filesystem::path &sourcePath, std::string_view destPath,
      CompressionMethod compressionMethod = CompressionMethod::Zlib);

  /**
   * @brief Finalize the .pak file
   * @return Result containing success or an error
   */
  Result<void> finalize();
};

} // namespace Pak
