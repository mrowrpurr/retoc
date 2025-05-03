#include "pak.h"

#include <ankerl/unordered_dense.h>
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

namespace Pak {

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

// FNV-64 hash implementation for path hashing
static uint64_t fnv64(const void *data, size_t size, uint64_t seed = 0) {
  const uint64_t FNV_OFFSET = 0xcbf29ce484222325;
  const uint64_t FNV_PRIME = 0x00000100000001b3;

  uint64_t hash = FNV_OFFSET + seed;
  const uint8_t *bytes = static_cast<const uint8_t *>(data);

  for (size_t i = 0; i < size; ++i) {
    hash ^= bytes[i];
    hash *= FNV_PRIME;
  }

  return hash;
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

// PakReader implementation
struct PakReader::Impl {
  std::filesystem::path filePath;
  std::optional<AesKey> key;
  std::ifstream file;
  Version version;
  bool encrypted;
  std::string mountPoint;
  std::unordered_map<std::string, FileEntry> entries;

  Impl(const std::filesystem::path &path, const std::optional<AesKey> &key)
      : filePath(path), key(key), encrypted(false), version(Version::Invalid) {

    // Open the file
    file.open(path, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Failed to open file: " + path.string());
    }

    // Read the header
    readHeader();
  }

  void readHeader() {
    // This is a simplified implementation
    // In a real implementation, you would read the actual header
    // and parse the file entries

    // For now, we'll just set some default values
    version = Version::V11;
    encrypted = false;
    mountPoint = "/";

    // Add some dummy entries for testing
    FileEntry entry;
    entry.path = "test.txt";
    entry.offset = 0;
    entry.size = 0;
    entry.uncompressedSize = 0;
    entry.compressionMethod = CompressionMethod::None;
    entry.compressionBlockSize = 0;
    entry.encrypted = false;

    entries[entry.path] = entry;
  }
};

PakReader::PakReader(const std::filesystem::path &path,
                     const std::optional<AesKey> &key)
    : _impl(std::make_unique<Impl>(path, key)) {}

PakReader::~PakReader() = default;

bool PakReader::fileExists(std::string_view path) const {
  return _impl->entries.count(std::string(path)) > 0;
}

std::vector<std::string> PakReader::getFileList() const {
  std::vector<std::string> files;
  files.reserve(_impl->entries.size());

  for (const auto &[path, _] : _impl->entries) {
    files.push_back(path);
  }

  return files;
}

Result<std::vector<uint8_t>>
PakReader::extractFile(std::string_view path) const {
  auto it = _impl->entries.find(std::string(path));
  if (it == _impl->entries.end()) {
    return Result<std::vector<uint8_t>>(Error::FileNotFound,
                                        "File not found: " + std::string(path));
  }

  const FileEntry &entry = it->second;

  // This is a simplified implementation
  // In a real implementation, you would read the file data from the pak file,
  // decompress it if necessary, and decrypt it if necessary

  // For now, we'll just return an empty vector
  return Result<std::vector<uint8_t>>(std::vector<uint8_t>());
}

Result<size_t>
PakReader::extractAllFiles(const std::filesystem::path &outputDir) const {
  size_t extractedCount = 0;

  for (const auto &[path, _] : _impl->entries) {
    auto result = extractFile(path);
    if (result.isError()) {
      return Result<size_t>(result.error(), result.errorMessage());
    }

    std::filesystem::path outputPath = outputDir / path;
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

Version PakReader::getVersion() const { return _impl->version; }

bool PakReader::isEncrypted() const { return _impl->encrypted; }

std::string PakReader::getMountPoint() const { return _impl->mountPoint; }

Result<FileEntry> PakReader::getFileEntry(std::string_view path) const {
  auto it = _impl->entries.find(std::string(path));
  if (it == _impl->entries.end()) {
    return Result<FileEntry>(Error::FileNotFound,
                             "File not found: " + std::string(path));
  }

  return Result<FileEntry>(it->second);
}

std::vector<FileEntry> PakReader::getEntries() const {
  std::vector<FileEntry> result;
  result.reserve(_impl->entries.size());

  for (const auto &[_, entry] : _impl->entries) {
    result.push_back(entry);
  }

  return result;
}

// PakWriter implementation
struct PakWriter::Impl {
  std::filesystem::path filePath;
  Version version;
  std::string mountPoint;
  std::ofstream file;
  std::unordered_map<std::string, FileEntry> entries;
  bool finalized;

  Impl(const std::filesystem::path &path, Version version,
       std::string_view mountPoint)
      : filePath(path), version(version), mountPoint(mountPoint),
        finalized(false) {

    // Create the directory if it doesn't exist
    std::filesystem::create_directories(path.parent_path());

    // Open the file
    file.open(path.string(), std::ios::binary);
    if (!file) {
      throw std::runtime_error("Failed to create file: " + path.string());
    }

    // Write a placeholder header
    // In a real implementation, you would write the actual header
    // and update it when finalizing the pak file
    file.seekp(0);
    file.write("PAK", 3);
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
    // In a real implementation, you would update the header with the file
    // entries

    file.close();
    finalized = true;

    return Result<void>();
  }
};

PakWriter::PakWriter(const std::filesystem::path &path, Version version,
                     std::string_view mountPoint)
    : _impl(std::make_unique<Impl>(path, version, mountPoint)) {}

PakWriter::~PakWriter() = default;

Result<void> PakWriter::addFile(std::string_view path,
                                const std::vector<uint8_t> &data,
                                CompressionMethod compressionMethod) {
  if (_impl->finalized) {
    return Result<void>(Error::IoError, "Pak file has been finalized");
  }

  // This is a simplified implementation
  // In a real implementation, you would compress the data if necessary,
  // calculate the hash, and write the data to the pak file

  // For now, we'll just add the entry to the map
  FileEntry entry;
  entry.path = std::string(path);
  entry.offset = 0;
  entry.size = data.size();
  entry.uncompressedSize = data.size();
  entry.compressionMethod = compressionMethod;
  entry.compressionBlockSize = 0;
  entry.encrypted = false;

  _impl->entries[entry.path] = entry;

  return Result<void>();
}

Result<void> PakWriter::addFileFromDisk(const std::filesystem::path &sourcePath,
                                        std::string_view destPath,
                                        CompressionMethod compressionMethod) {
  if (_impl->finalized) {
    return Result<void>(Error::IoError, "Pak file has been finalized");
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

    // Add the file to the pak
    return addFile(destPath, data, compressionMethod);
  } catch (const std::exception &e) {
    return Result<void>(Error::IoError,
                        "Error reading file: " + std::string(e.what()));
  }
}

Result<void> PakWriter::finalize() { return _impl->finalize(); }

} // namespace Pak
