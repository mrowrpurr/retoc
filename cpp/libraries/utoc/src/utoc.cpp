#include "utoc.h"

#include <blake3.h>
#include <fstream>
#include <iostream>
#include <lz4.h>
#include <memory>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <unordered_map>
#include <zlib.h>
#include <zstd.h>

#include "liboodle.h"

namespace Utoc {

// ChunkId implementation
uint32_t ChunkId::getHash() const {
  return static_cast<uint32_t>(id & 0xFFFFFFFF);
}

uint8_t ChunkId::getIndex() const {
  return static_cast<uint8_t>((id >> 32) & 0xFF);
}

uint8_t ChunkId::getType() const {
  return static_cast<uint8_t>((id >> 40) & 0xFF);
}

uint16_t ChunkId::getFlags() const {
  return static_cast<uint16_t>((id >> 48) & 0xFFFF);
}

ChunkId ChunkId::create(uint32_t hash, uint8_t index, uint8_t type,
                        uint16_t flags) {
  uint64_t id = hash;
  id |= static_cast<uint64_t>(index) << 32;
  id |= static_cast<uint64_t>(type) << 40;
  id |= static_cast<uint64_t>(flags) << 48;
  return ChunkId(id);
}

std::string ChunkId::toString() const {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%08X-%02X-%02X-%04X", getHash(), getIndex(),
           getType(), getFlags());
  return std::string(buffer);
}

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

// UtocReader implementation
struct UtocReader::Impl {
  std::filesystem::path utocPath;
  std::filesystem::path ucasPath;
  std::optional<AesKey> key;
  std::ifstream utocFile;
  std::ifstream ucasFile;
  ContainerHeader header;
  std::unordered_map<ChunkId, ChunkInfo> chunks;

  Impl(const std::filesystem::path &utocPath,
       const std::optional<std::filesystem::path> &ucasPath,
       const std::optional<AesKey> &key)
      : utocPath(utocPath), key(key) {

    // Determine UCAS path if not provided
    if (ucasPath) {
      this->ucasPath = *ucasPath;
    } else {
      this->ucasPath = utocPath;
      this->ucasPath.replace_extension(".ucas");
    }

    // Open the UTOC file
    utocFile.open(utocPath, std::ios::binary);
    if (!utocFile) {
      throw std::runtime_error("Failed to open UTOC file: " +
                               utocPath.string());
    }

    // Open the UCAS file
    ucasFile.open(this->ucasPath, std::ios::binary);
    if (!ucasFile) {
      throw std::runtime_error("Failed to open UCAS file: " +
                               this->ucasPath.string());
    }

    // Read the header
    readHeader();
  }

  void readHeader() {
    // This is a simplified implementation
    // In a real implementation, you would read the actual header
    // and parse the chunk entries

    // For now, we'll just set some default values
    header.version = Version::PerfectHashWithOverflow;
    header.headerSize = 0;
    header.entryCount = 0;
    header.compressedBlockCount = 0;
    header.compressionBlockSize = 0;
    header.directoryIndexSize = 0;
    header.partitionCount = 0;
    header.containerSize = 0;
    header.mountPoint = "/";
    header.encrypted = false;
    header.encryptionKeyGuid.fill(0);

    // Add some dummy chunks for testing
    ChunkId chunkId = ChunkId::create(0x12345678, 0, 0, 0);
    ChunkInfo chunkInfo;
    chunkInfo.id = chunkId;
    chunkInfo.offset = 0;
    chunkInfo.size = 0;
    chunkInfo.uncompressedSize = 0;
    chunkInfo.compressionMethod = CompressionMethod::None;
    chunkInfo.type = ChunkType::Raw;

    chunks[chunkId] = chunkInfo;
  }
};

UtocReader::UtocReader(const std::filesystem::path &utocPath,
                       const std::optional<std::filesystem::path> &ucasPath,
                       const std::optional<AesKey> &key)
    : _impl(std::make_unique<Impl>(utocPath, ucasPath, key)) {}

UtocReader::~UtocReader() = default;

bool UtocReader::chunkExists(const ChunkId &chunkId) const {
  return _impl->chunks.count(chunkId) > 0;
}

std::vector<ChunkId> UtocReader::getChunkList() const {
  std::vector<ChunkId> chunkIds;
  chunkIds.reserve(_impl->chunks.size());

  for (const auto &[id, _] : _impl->chunks) {
    chunkIds.push_back(id);
  }

  return chunkIds;
}

Result<std::vector<uint8_t>>
UtocReader::extractChunk(const ChunkId &chunkId) const {
  auto it = _impl->chunks.find(chunkId);
  if (it == _impl->chunks.end()) {
    return Result<std::vector<uint8_t>>(
        Error::FileNotFound, "Chunk not found: " + chunkId.toString());
  }

  const ChunkInfo &chunkInfo = it->second;

  // This is a simplified implementation
  // In a real implementation, you would read the chunk data from the UCAS file,
  // decompress it if necessary, and decrypt it if necessary

  // For now, we'll just return an empty vector
  return Result<std::vector<uint8_t>>(std::vector<uint8_t>());
}

Result<size_t>
UtocReader::extractAllChunks(const std::filesystem::path &outputDir) const {
  size_t extractedCount = 0;

  for (const auto &[id, _] : _impl->chunks) {
    auto result = extractChunk(id);
    if (result.isError()) {
      return Result<size_t>(result.error(), result.errorMessage());
    }

    std::filesystem::path outputPath = outputDir / id.toString();
    std::filesystem::create_directories(outputPath.parent_path());

    std::ofstream outFile(outputPath.string(), std::ios::binary);
    if (!outFile) {
      return Result<size_t>(Error::IoError,
                            "Failed to create file: " + outputPath.string());
    }

    const auto &data = result.value();
    outFile.write(reinterpret_cast<const char *>(data.data()), data.size());

    extractedCount++;
  }

  return Result<size_t>(extractedCount);
}

Version UtocReader::getVersion() const { return _impl->header.version; }

bool UtocReader::isEncrypted() const { return _impl->header.encrypted; }

std::string UtocReader::getMountPoint() const {
  return _impl->header.mountPoint;
}

Result<ChunkInfo> UtocReader::getChunkInfo(const ChunkId &chunkId) const {
  auto it = _impl->chunks.find(chunkId);
  if (it == _impl->chunks.end()) {
    return Result<ChunkInfo>(Error::FileNotFound,
                             "Chunk not found: " + chunkId.toString());
  }

  return Result<ChunkInfo>(it->second);
}

const ContainerHeader &UtocReader::getContainerHeader() const {
  return _impl->header;
}

// UtocWriter implementation
struct UtocWriter::Impl {
  std::filesystem::path utocPath;
  std::filesystem::path ucasPath;
  Version version;
  std::string mountPoint;
  std::ofstream utocFile;
  std::ofstream ucasFile;
  std::unordered_map<ChunkId, ChunkInfo> chunks;
  bool finalized;

  Impl(const std::filesystem::path &utocPath,
       const std::optional<std::filesystem::path> &ucasPath, Version version,
       std::string_view mountPoint)
      : utocPath(utocPath), version(version), mountPoint(mountPoint),
        finalized(false) {

    // Determine UCAS path if not provided
    if (ucasPath) {
      this->ucasPath = *ucasPath;
    } else {
      this->ucasPath = utocPath;
      this->ucasPath.replace_extension(".ucas");
    }

    // Create the directories if they don't exist
    std::filesystem::create_directories(utocPath.parent_path());
    std::filesystem::create_directories(this->ucasPath.parent_path());

    // Open the UTOC file
    utocFile.open(utocPath.string(), std::ios::binary);
    if (!utocFile) {
      throw std::runtime_error("Failed to create UTOC file: " +
                               utocPath.string());
    }

    // Open the UCAS file
    ucasFile.open(this->ucasPath.string(), std::ios::binary);
    if (!ucasFile) {
      throw std::runtime_error("Failed to create UCAS file: " +
                               this->ucasPath.string());
    }

    // Write placeholder headers
    // In a real implementation, you would write the actual headers
    // and update them when finalizing the files
    utocFile.seekp(0);
    utocFile.write("UTOC", 4);

    ucasFile.seekp(0);
    ucasFile.write("UCAS", 4);
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
    // In a real implementation, you would update the headers with the chunk
    // entries

    utocFile.close();
    ucasFile.close();
    finalized = true;

    return Result<void>();
  }
};

UtocWriter::UtocWriter(const std::filesystem::path &utocPath,
                       const std::optional<std::filesystem::path> &ucasPath,
                       Version version, std::string_view mountPoint)
    : _impl(std::make_unique<Impl>(utocPath, ucasPath, version, mountPoint)) {}

UtocWriter::~UtocWriter() = default;

Result<void> UtocWriter::addChunk(const ChunkId &chunkId,
                                  const std::vector<uint8_t> &data,
                                  CompressionMethod compressionMethod,
                                  ChunkType type) {
  if (_impl->finalized) {
    return Result<void>(Error::IoError, "UTOC/UCAS files have been finalized");
  }

  // This is a simplified implementation
  // In a real implementation, you would compress the data if necessary,
  // calculate the hash, and write the data to the UCAS file

  // For now, we'll just add the chunk to the map
  ChunkInfo chunkInfo;
  chunkInfo.id = chunkId;
  chunkInfo.offset = 0;
  chunkInfo.size = data.size();
  chunkInfo.uncompressedSize = data.size();
  chunkInfo.compressionMethod = compressionMethod;
  chunkInfo.type = type;

  if (type == ChunkType::Embedded) {
    chunkInfo.data = data;
  }

  _impl->chunks[chunkId] = chunkInfo;

  return Result<void>();
}

Result<void> UtocWriter::addChunkFromDisk(
    const ChunkId &chunkId, const std::filesystem::path &sourcePath,
    CompressionMethod compressionMethod, ChunkType type) {
  if (_impl->finalized) {
    return Result<void>(Error::IoError, "UTOC/UCAS files have been finalized");
  }

  try {
    // Check if file exists
    if (!std::filesystem::exists(sourcePath)) {
      return Result<void>(Error::FileNotFound,
                          "File not found: " + sourcePath.string());
    }

    // Get file size
    std::uintmax_t fileSize = std::filesystem::file_size(sourcePath);

    // Read file contents
    std::vector<uint8_t> data(static_cast<size_t>(fileSize));

    FILE *file = fopen(sourcePath.string().c_str(), "rb");
    if (!file) {
      return Result<void>(Error::IoError,
                          "Failed to open file: " + sourcePath.string());
    }

    size_t bytesRead =
        fread(data.data(), 1, static_cast<size_t>(fileSize), file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize)) {
      return Result<void>(Error::IoError,
                          "Failed to read file: " + sourcePath.string());
    }

    // Add the chunk
    return addChunk(chunkId, data, compressionMethod, type);
  } catch (const std::exception &e) {
    return Result<void>(Error::IoError,
                        "Error reading file: " + std::string(e.what()));
  }
}

Result<void> UtocWriter::finalize() { return _impl->finalize(); }

} // namespace Utoc
