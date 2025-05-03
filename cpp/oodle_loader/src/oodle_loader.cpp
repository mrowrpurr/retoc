#include "oodle_loader.h"

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <mutex>

namespace oodle {

// Oodle function types
using OodleLZ_Compress = int64_t (*)(
    Compressor compressor,
    const uint8_t* rawBuf,
    size_t rawLen,
    uint8_t* compBuf,
    CompressionLevel level,
    void* pOptions,
    void* dictionaryBase,
    void* lrm,
    uint8_t* scratchMem,
    size_t scratchSize);

using OodleLZ_Decompress = int64_t (*)(
    const uint8_t* compBuf,
    size_t compBufSize,
    uint8_t* rawBuf,
    size_t rawLen,
    uint32_t fuzzSafe,
    uint32_t checkCRC,
    uint32_t verbosity,
    uint64_t decBufBase,
    size_t decBufSize,
    uint64_t fpCallback,
    uint64_t callbackUserData,
    uint8_t* decoderMemory,
    size_t decoderMemorySize,
    uint32_t threadPhase);

using OodleLZ_GetCompressedBufferSizeNeeded = size_t (*)(
    Compressor compressor,
    size_t rawSize);

using OodleCore_Plugins_SetPrintf = void (*)(void* printf);

// Oodle DLL information
static const char* OODLE_DLL_NAME = "oo2core_9_win64.dll";

// Implementation details for OodleLibrary
struct OodleLibrary::Impl {
    HMODULE library;
    OodleLZ_Compress compress;
    OodleLZ_Decompress decompress;
    OodleLZ_GetCompressedBufferSizeNeeded getCompressedBufferSizeNeeded;
    OodleCore_Plugins_SetPrintf setPrintf;

    Impl() : library(nullptr), compress(nullptr), decompress(nullptr), 
             getCompressedBufferSizeNeeded(nullptr), setPrintf(nullptr) {}
};

// Singleton instance and mutex
static OodleLibrary* s_instance = nullptr;
static std::mutex s_mutex;

// Helper function to load the Oodle DLL
Result<HMODULE> loadOodleDll() {
    // Get the path to the executable
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) == 0) {
        return Result<HMODULE>(Error::IoError, "Failed to get executable path");
    }

    // Get the directory of the executable
    std::string exeDir(exePath);
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash + 1);
    }

    // Construct the path to the Oodle DLL
    std::string dllPath = exeDir + OODLE_DLL_NAME;

    // Check if the DLL exists
    std::ifstream file(dllPath, std::ios::binary);
    if (!file.good()) {
        return Result<HMODULE>(Error::IoError, "Oodle DLL not found at: " + dllPath);
    }

    // Load the DLL
    HMODULE library = LoadLibraryA(dllPath.c_str());
    if (!library) {
        return Result<HMODULE>(Error::LibLoadingError, "Failed to load Oodle DLL");
    }

    return Result<HMODULE>(library);
}

// OodleLibrary constructor
OodleLibrary::OodleLibrary() : _impl(std::make_unique<Impl>()) {
    // Load the Oodle DLL
    auto result = loadOodleDll();
    if (result.isError()) {
        throw std::runtime_error("Failed to load Oodle DLL: " + result.errorMessage());
    }

    _impl->library = result.value();

    // Get function pointers
    _impl->compress = reinterpret_cast<OodleLZ_Compress>(
        GetProcAddress(_impl->library, "OodleLZ_Compress"));
    _impl->decompress = reinterpret_cast<OodleLZ_Decompress>(
        GetProcAddress(_impl->library, "OodleLZ_Decompress"));
    _impl->getCompressedBufferSizeNeeded = reinterpret_cast<OodleLZ_GetCompressedBufferSizeNeeded>(
        GetProcAddress(_impl->library, "OodleLZ_GetCompressedBufferSizeNeeded"));
    _impl->setPrintf = reinterpret_cast<OodleCore_Plugins_SetPrintf>(
        GetProcAddress(_impl->library, "OodleCore_Plugins_SetPrintf"));

    // Check if all function pointers were loaded
    if (!_impl->compress || !_impl->decompress || 
        !_impl->getCompressedBufferSizeNeeded || !_impl->setPrintf) {
        throw std::runtime_error("Failed to load Oodle function pointers");
    }

    // Silence Oodle logging
    _impl->setPrintf(nullptr);
}

// OodleLibrary destructor
OodleLibrary::~OodleLibrary() {
    if (_impl && _impl->library) {
        FreeLibrary(_impl->library);
        _impl->library = nullptr;
    }
}

// Get the singleton instance
Result<OodleLibrary*> OodleLibrary::instance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (!s_instance) {
        try {
            s_instance = new OodleLibrary();
        } catch (const std::exception& e) {
            return Result<OodleLibrary*>(Error::InitializationFailed, e.what());
        }
    }
    
    return Result<OodleLibrary*>(s_instance);
}

// Compress data (vector version)
Result<std::vector<uint8_t>> OodleLibrary::compress(
    const std::vector<uint8_t>& input,
    Compressor compressor,
    CompressionLevel compressionLevel) {
    
    return compress(input.data(), input.size(), compressor, compressionLevel);
}

// Compress data (pointer version)
Result<std::vector<uint8_t>> OodleLibrary::compress(
    const uint8_t* input,
    size_t inputSize,
    Compressor compressor,
    CompressionLevel compressionLevel) {
    
    // Get the required buffer size
    size_t bufferSize = getCompressedBufferSizeNeeded(compressor, inputSize);
    std::vector<uint8_t> buffer(bufferSize);
    
    // Compress the data
    int64_t compressedSize = _impl->compress(
        compressor,
        input,
        inputSize,
        buffer.data(),
        compressionLevel,
        nullptr,  // pOptions
        nullptr,  // dictionaryBase
        nullptr,  // lrm
        nullptr,  // scratchMem
        0         // scratchSize
    );
    
    if (compressedSize == -1) {
        return Result<std::vector<uint8_t>>(Error::CompressionFailed, "Oodle compression failed");
    }
    
    // Resize the buffer to the actual compressed size
    buffer.resize(static_cast<size_t>(compressedSize));
    
    return Result<std::vector<uint8_t>>(std::move(buffer));
}

// Decompress data (vector version)
Result<std::vector<uint8_t>> OodleLibrary::decompress(
    const std::vector<uint8_t>& input,
    size_t outputSize) {
    
    std::vector<uint8_t> output(outputSize);
    
    auto result = decompress(input.data(), input.size(), output.data(), outputSize);
    if (result.isError()) {
        return Result<std::vector<uint8_t>>(result.error(), result.errorMessage());
    }
    
    return Result<std::vector<uint8_t>>(std::move(output));
}

// Decompress data (pointer version)
Result<size_t> OodleLibrary::decompress(
    const uint8_t* input,
    size_t inputSize,
    uint8_t* output,
    size_t outputSize) {
    
    int64_t decompressedSize = _impl->decompress(
        input,
        inputSize,
        output,
        outputSize,
        1,          // fuzzSafe
        1,          // checkCRC
        0,          // verbosity
        0,          // decBufBase
        0,          // decBufSize
        0,          // fpCallback
        0,          // callbackUserData
        nullptr,    // decoderMemory
        0,          // decoderMemorySize
        3           // threadPhase
    );
    
    if (decompressedSize <= 0) {
        return Result<size_t>(Error::CompressionFailed, "Oodle decompression failed");
    }
    
    return Result<size_t>(static_cast<size_t>(decompressedSize));
}

// Get the size needed for a compressed buffer
size_t OodleLibrary::getCompressedBufferSizeNeeded(
    Compressor compressor,
    size_t rawSize) {
    
    return _impl->getCompressedBufferSizeNeeded(compressor, rawSize);
}

} // namespace oodle
