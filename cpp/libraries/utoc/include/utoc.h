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

namespace Utoc {

/**
 * @brief UTOC file version
 */
enum class Version : uint32_t {
  Invalid = 0,
  Initial = 1,
  DirectoryIndex = 2,
  PartitionSize = 3,
  PerfectHash = 4,
  PerfectHashWithOverflow = 5,
  RelativePaths = 6
};

/**
 * @brief Compression methods supported by .utoc/.ucas files
 */
enum class CompressionMethod : uint8_t {
  None = 0,
  Zlib = 1,
  Gzip = 2,
  Zstd = 3,
  Oodle = 4
};

/**
 * @brief Chunk type
 */
enum class ChunkType : uint8_t {
  Unknown = 0,
  Embedded = 1,
  Compressed = 2,
  Raw = 3
};

/**
 * @brief Chunk ID format
 */
struct ChunkId {
  uint64_t id;

  ChunkId() : id(0) {}
  explicit ChunkId(uint64_t id) : id(id) {}

  bool operator==(const ChunkId &other) const { return id == other.id; }
  bool operator!=(const ChunkId &other) const { return id != other.id; }

  // Extract components from the chunk ID
  uint32_t getHash() const;
  uint8_t getIndex() const;
  uint8_t getType() const;
  uint16_t getFlags() const;

  // Create a chunk ID from components
  static ChunkId create(uint32_t hash, uint8_t index, uint8_t type,
                        uint16_t flags);

  // Convert to string
  std::string toString() const;
};

/**
 * @brief Chunk information
 */
struct ChunkInfo {
  ChunkId id;
  uint64_t offset;
  uint64_t size;
  uint64_t uncompressedSize;
  CompressionMethod compressionMethod;
  ChunkType type;
  std::vector<uint8_t> data; // Only used for embedded chunks
};

/**
 * @brief Container header information
 */
struct ContainerHeader {
  Version version;
  uint32_t headerSize;
  uint32_t entryCount;
  uint32_t compressedBlockCount;
  uint32_t compressionBlockSize;
  uint32_t directoryIndexSize;
  uint32_t partitionCount;
  uint64_t containerSize;
  std::string mountPoint;
  bool encrypted;
  std::array<uint8_t, 32> encryptionKeyGuid;
};

/**
 * @brief Error codes for UTOC operations
 */
enum class Error {
  None,
  FileNotFound,
  InvalidFormat,
  CompressionError,
  EncryptionError,
  HashMismatch,
  IoError,
  UcasNotFound
};

/**
 * @brief Result class for UTOC operations
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
 * @brief AES key for encrypted .utoc/.ucas files
 */
class AesKey {
  std::array<uint8_t, 32> _key;

public:
  AesKey(const std::array<uint8_t, 32> &key);
  AesKey(std::string_view hexOrBase64Key);

  const std::array<uint8_t, 32> &getKey() const { return _key; }
};

/**
 * @brief Reader for .utoc files
 */
class UtocReader {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  /**
   * @brief Construct a new UTOC Reader
   * @param utocPath Path to the .utoc file
   * @param ucasPath Optional path to the .ucas file. If not provided, it will
   * be inferred from the .utoc path
   * @param key Optional AES key for encrypted .utoc/.ucas files
   */
  UtocReader(
      const std::filesystem::path &utocPath,
      const std::optional<std::filesystem::path> &ucasPath = std::nullopt,
      const std::optional<AesKey> &key = std::nullopt);
  ~UtocReader();

  /**
   * @brief Check if a chunk exists in the UTOC
   * @param chunkId Chunk ID to check
   * @return True if the chunk exists, false otherwise
   */
  bool chunkExists(const ChunkId &chunkId) const;

  /**
   * @brief Get a list of all chunks in the UTOC
   * @return Vector of chunk IDs
   */
  std::vector<ChunkId> getChunkList() const;

  /**
   * @brief Extract a chunk from the UTOC/UCAS
   * @param chunkId Chunk ID to extract
   * @return Result containing the chunk data or an error
   */
  Result<std::vector<uint8_t>> extractChunk(const ChunkId &chunkId) const;

  /**
   * @brief Extract all chunks from the UTOC/UCAS
   * @param outputDir Directory to extract chunks to
   * @return Result containing the number of chunks extracted or an error
   */
  Result<size_t> extractAllChunks(const std::filesystem::path &outputDir) const;

  /**
   * @brief Get the version of the UTOC file
   * @return The UTOC file version
   */
  Version getVersion() const;

  /**
   * @brief Check if the UTOC/UCAS files are encrypted
   * @return True if encrypted, false otherwise
   */
  bool isEncrypted() const;

  /**
   * @brief Get the mount point of the UTOC file
   * @return The mount point
   */
  std::string getMountPoint() const;

  /**
   * @brief Get information about a chunk in the UTOC
   * @param chunkId Chunk ID to get information for
   * @return Result containing the chunk information or an error
   */
  Result<ChunkInfo> getChunkInfo(const ChunkId &chunkId) const;

  /**
   * @brief Get the container header information
   * @return The container header information
   */
  const ContainerHeader &getContainerHeader() const;
};

/**
 * @brief Writer for .utoc/.ucas files
 */
class UtocWriter {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  /**
   * @brief Construct a new UTOC Writer
   * @param utocPath Path to the output .utoc file
   * @param ucasPath Optional path to the output .ucas file. If not provided, it
   * will be inferred from the .utoc path
   * @param version Version of the UTOC file to create
   * @param mountPoint Mount point for the UTOC file
   */
  UtocWriter(
      const std::filesystem::path &utocPath,
      const std::optional<std::filesystem::path> &ucasPath = std::nullopt,
      Version version = Version::PerfectHashWithOverflow,
      std::string_view mountPoint = "/");
  ~UtocWriter();

  /**
   * @brief Add a chunk to the UTOC/UCAS
   * @param chunkId Chunk ID
   * @param data Chunk data
   * @param compressionMethod Compression method to use
   * @param type Chunk type
   * @return Result containing success or an error
   */
  Result<void>
  addChunk(const ChunkId &chunkId, const std::vector<uint8_t> &data,
           CompressionMethod compressionMethod = CompressionMethod::Zstd,
           ChunkType type = ChunkType::Compressed);

  /**
   * @brief Add a chunk from disk to the UTOC/UCAS
   * @param chunkId Chunk ID
   * @param sourcePath Path to the file on disk
   * @param compressionMethod Compression method to use
   * @param type Chunk type
   * @return Result containing success or an error
   */
  Result<void> addChunkFromDisk(
      const ChunkId &chunkId, const std::filesystem::path &sourcePath,
      CompressionMethod compressionMethod = CompressionMethod::Zstd,
      ChunkType type = ChunkType::Compressed);

  /**
   * @brief Finalize the UTOC/UCAS files
   * @return Result containing success or an error
   */
  Result<void> finalize();
};

} // namespace Utoc

// Hash function for ChunkId to use in unordered containers
namespace std {
template <> struct hash<Utoc::ChunkId> {
  size_t operator()(const Utoc::ChunkId &chunkId) const {
    return static_cast<size_t>(chunkId.id);
  }
};
} // namespace std
