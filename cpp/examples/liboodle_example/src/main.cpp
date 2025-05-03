#include <iostream>
#include <liboodle.h>
#include <string>
#include <vector>

int main() {
  // Get the Oodle library instance
  auto oodleResult = oodle::OodleLibrary::instance();
  if (oodleResult.isError()) {
    std::cerr << "Failed to initialize Oodle: " << oodleResult.errorMessage()
              << std::endl;
    return 1;
  }

  oodle::OodleLibrary *oodle = oodleResult.value();

  // Test data to compress
  const std::string testData =
      "In tools and when compressing large inputs in one call, consider using "
      "OodleXLZ_Compress_AsyncAndWait (in the Oodle2 Ext lib) instead to get "
      "parallelism. "
      "Alternatively, chop the data into small fixed size chunks (we recommend "
      "at least 256KiB, "
      "i.e. 262144 bytes) and call compress on each of them, which decreases "
      "compression ratio "
      "but makes for trivial parallel compression and decompression.";

  std::vector<uint8_t> inputData(testData.begin(), testData.end());

  // Compress the data
  auto compressResult = oodle->compress(inputData, oodle::Compressor::Mermaid,
                                        oodle::CompressionLevel::Optimal5);

  if (compressResult.isError()) {
    std::cerr << "Compression failed: " << compressResult.errorMessage()
              << std::endl;
    return 1;
  }

  std::vector<uint8_t> compressedData = compressResult.moveValue();

  // Print compression stats
  std::cout << "Original size: " << inputData.size() << " bytes" << std::endl;
  std::cout << "Compressed size: " << compressedData.size() << " bytes"
            << std::endl;
  std::cout << "Compression ratio: "
            << static_cast<float>(inputData.size()) / compressedData.size()
            << std::endl;

  // Decompress the data
  auto decompressResult = oodle->decompress(compressedData, inputData.size());

  if (decompressResult.isError()) {
    std::cerr << "Decompression failed: " << decompressResult.errorMessage()
              << std::endl;
    return 1;
  }

  std::vector<uint8_t> decompressedData = decompressResult.moveValue();

  // Verify the decompressed data matches the original
  bool matches = (inputData.size() == decompressedData.size());
  if (matches) {
    for (size_t i = 0; i < inputData.size(); ++i) {
      if (inputData[i] != decompressedData[i]) {
        matches = false;
        break;
      }
    }
  }

  if (matches) {
    std::cout << "Decompression successful: data matches original" << std::endl;
  } else {
    std::cerr << "Decompression failed: data does not match original"
              << std::endl;
    return 1;
  }

  // Test with raw pointers
  auto compressResult2 =
      oodle->compress(inputData.data(), inputData.size(),
                      oodle::Compressor::Kraken, oodle::CompressionLevel::Fast);

  if (compressResult2.isError()) {
    std::cerr << "Compression (pointer version) failed: "
              << compressResult2.errorMessage() << std::endl;
    return 1;
  }

  std::vector<uint8_t> compressedData2 = compressResult2.moveValue();

  // Print compression stats for the second method
  std::cout << "\nCompression with Kraken/Fast:" << std::endl;
  std::cout << "Original size: " << inputData.size() << " bytes" << std::endl;
  std::cout << "Compressed size: " << compressedData2.size() << " bytes"
            << std::endl;
  std::cout << "Compression ratio: "
            << static_cast<float>(inputData.size()) / compressedData2.size()
            << std::endl;

  // Decompress using the raw pointer version
  std::vector<uint8_t> decompressedData2(inputData.size());
  auto decompressResult2 =
      oodle->decompress(compressedData2.data(), compressedData2.size(),
                        decompressedData2.data(), decompressedData2.size());

  if (decompressResult2.isError()) {
    std::cerr << "Decompression (pointer version) failed: "
              << decompressResult2.errorMessage() << std::endl;
    return 1;
  }

  // Verify the decompressed data matches the original
  matches = (inputData.size() == decompressedData2.size());
  if (matches) {
    for (size_t i = 0; i < inputData.size(); ++i) {
      if (inputData[i] != decompressedData2[i]) {
        matches = false;
        break;
      }
    }
  }

  if (matches) {
    std::cout
        << "Decompression (pointer version) successful: data matches original"
        << std::endl;
  } else {
    std::cerr << "Decompression (pointer version) failed: data does not match "
                 "original"
              << std::endl;
    return 1;
  }

  return 0;
}
