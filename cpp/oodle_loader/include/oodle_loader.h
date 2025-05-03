#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace oodle {

/**
 * @brief Oodle compressor types
 */
enum class Compressor : int32_t {
    /// None = memcpy, pass through uncompressed bytes
    None = 3,

    /// Fast decompression and high compression ratios, amazing!
    Kraken = 8,
    /// Leviathan = Kraken's big brother with higher compression, slightly slower decompression.
    Leviathan = 13,
    /// Mermaid is between Kraken & Selkie - crazy fast, still decent compression.
    Mermaid = 9,
    /// Selkie is a super-fast relative of Mermaid. For maximum decode speed.
    Selkie = 11,
    /// Hydra, the many-headed beast = Leviathan, Kraken, Mermaid, or Selkie
    Hydra = 12,
};

/**
 * @brief Oodle compression levels
 */
enum class CompressionLevel : int32_t {
    /// don't compress, just copy raw bytes
    None = 0,
    /// super fast mode, lower compression ratio
    SuperFast = 1,
    /// fastest LZ mode with still decent compression ratio
    VeryFast = 2,
    /// fast - good for daily use
    Fast = 3,
    /// standard medium speed LZ mode
    Normal = 4,

    /// optimal parse level 1 (faster optimal encoder)
    Optimal1 = 5,
    /// optimal parse level 2 (recommended baseline optimal encoder)
    Optimal2 = 6,
    /// optimal parse level 3 (slower optimal encoder)
    Optimal3 = 7,
    /// optimal parse level 4 (very slow optimal encoder)
    Optimal4 = 8,
    /// optimal parse level 5 (don't care about encode speed, maximum compression)
    Optimal5 = 9,

    /// faster than SuperFast, less compression
    HyperFast1 = -1,
    /// faster than HyperFast1, less compression
    HyperFast2 = -2,
    /// faster than HyperFast2, less compression
    HyperFast3 = -3,
    /// fastest, less compression
    HyperFast4 = -4,
};

/**
 * @brief Error codes for Oodle operations
 */
enum class Error {
    None,
    HashMismatch,
    CompressionFailed,
    InitializationFailed,
    IoError,
    NetworkError,
    LibLoadingError
};

/**
 * @brief Result class for Oodle operations
 */
template<typename T>
class Result {
private:
    bool _success;
    T _value;
    Error _error;
    std::string _errorMessage;

public:
    Result(T value) : _success(true), _value(std::move(value)), _error(Error::None) {}
    Result(Error error, std::string errorMessage = "") 
        : _success(false), _error(error), _errorMessage(std::move(errorMessage)) {}

    bool isSuccess() const { return _success; }
    bool isError() const { return !_success; }
    
    const T& value() const { 
        if (!_success) {
            throw std::runtime_error("Attempted to access value of error result");
        }
        return _value; 
    }
    
    T&& moveValue() {
        if (!_success) {
            throw std::runtime_error("Attempted to access value of error result");
        }
        return std::move(_value);
    }
    
    Error error() const { return _error; }
    const std::string& errorMessage() const { return _errorMessage; }
};

/**
 * @brief Oodle library wrapper
 */
class OodleLibrary {
private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

    OodleLibrary();
    ~OodleLibrary();

public:
    /**
     * @brief Get the singleton instance of the Oodle library
     * @return Result containing a reference to the OodleLibrary instance or an error
     */
    static Result<OodleLibrary*> instance();

    /**
     * @brief Compress data using Oodle
     * @param input Data to compress
     * @param compressor Oodle compressor to use
     * @param compressionLevel Compression level to use
     * @return Result containing compressed data or an error
     */
    Result<std::vector<uint8_t>> compress(
        const std::vector<uint8_t>& input,
        Compressor compressor,
        CompressionLevel compressionLevel);

    /**
     * @brief Compress data using Oodle
     * @param input Pointer to data to compress
     * @param inputSize Size of data to compress
     * @param compressor Oodle compressor to use
     * @param compressionLevel Compression level to use
     * @return Result containing compressed data or an error
     */
    Result<std::vector<uint8_t>> compress(
        const uint8_t* input,
        size_t inputSize,
        Compressor compressor,
        CompressionLevel compressionLevel);

    /**
     * @brief Decompress data using Oodle
     * @param input Compressed data
     * @param outputSize Expected size of decompressed data
     * @return Result containing decompressed data or an error
     */
    Result<std::vector<uint8_t>> decompress(
        const std::vector<uint8_t>& input,
        size_t outputSize);

    /**
     * @brief Decompress data using Oodle
     * @param input Pointer to compressed data
     * @param inputSize Size of compressed data
     * @param output Pointer to output buffer
     * @param outputSize Size of output buffer
     * @return Result containing number of bytes decompressed or an error
     */
    Result<size_t> decompress(
        const uint8_t* input,
        size_t inputSize,
        uint8_t* output,
        size_t outputSize);

    /**
     * @brief Get the size needed for a compressed buffer
     * @param compressor Oodle compressor to use
     * @param rawSize Size of uncompressed data
     * @return Size needed for compressed buffer
     */
    size_t getCompressedBufferSizeNeeded(
        Compressor compressor,
        size_t rawSize);
};

} // namespace oodle
