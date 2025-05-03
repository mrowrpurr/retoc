#include <CLI/CLI.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>

#include "liboodle.h"
#include "pak.h"
#include "ucas.h"
#include "utoc.h"

int main(int argc, char **argv) {
  // Setup logging
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
  spdlog::set_level(spdlog::level::info);

  // Setup CLI
  CLI::App app{"Unreal Engine PAK/UTOC/UCAS File Utility"};
  app.set_version_flag("--version", "1.0.0");
  app.require_subcommand(1);

  // Set failure behavior to show help
  app.failure_message(CLI::FailureMessage::help);

  // Configure all subcommands to show help on validation errors
  auto configureSubcommand = [](CLI::App *cmd) {
    cmd->failure_message(CLI::FailureMessage::help);
  };

  // Add subcommands for each file type
  auto pak_cmd = app.add_subcommand("pak", "PAK file operations");
  configureSubcommand(pak_cmd);
  pak_cmd->require_subcommand(1);

  auto utoc_cmd = app.add_subcommand("utoc", "UTOC file operations");
  configureSubcommand(utoc_cmd);
  utoc_cmd->require_subcommand(1);

  auto ucas_cmd = app.add_subcommand("ucas", "UCAS file operations");
  configureSubcommand(ucas_cmd);
  ucas_cmd->require_subcommand(1);

  auto convert_cmd =
      app.add_subcommand("convert", "Asset conversion operations");
  configureSubcommand(convert_cmd);
  convert_cmd->require_subcommand(1);

  // PAK subcommands
  std::string pak_file;
  std::string pak_output_dir;
  std::string pak_aes_key;
  bool pak_list_only = false;

  auto pak_list = pak_cmd->add_subcommand("list", "List files in a PAK file");
  pak_list->add_option("file", pak_file, "PAK file to list")->required();
  pak_list->add_option("--key", pak_aes_key, "AES key for encrypted PAK files");

  pak_list->callback([&]() {
    spdlog::info("Listing files in PAK file: {}", pak_file);

    try {
      // Create a PAK reader
      std::optional<Pak::AesKey> key_opt;
      if (!pak_aes_key.empty()) {
        key_opt = Pak::AesKey(pak_aes_key);
        spdlog::info("Using AES key: {}", pak_aes_key);
      }

      auto reader = Pak::PakReader(pak_file, key_opt);

      // Get PAK info
      auto version = reader.getVersion();
      auto mountPoint = reader.getMountPoint();
      auto encrypted = reader.isEncrypted();

      spdlog::info("PAK Version: {}", static_cast<int>(version));
      spdlog::info("Mount Point: {}", mountPoint);
      spdlog::info("Encrypted: {}", encrypted ? "Yes" : "No");

      // List files
      auto files = reader.getFileList();
      spdlog::info("Found {} files in PAK", files.size());

      for (const auto &path : files) {
        auto entry_result = reader.getFileEntry(path);
        if (entry_result.isError()) {
          spdlog::info("  {}", path);
        } else {
          const auto &entry = entry_result.value();
          std::string compression;
          switch (entry.compressionMethod) {
          case Pak::CompressionMethod::None:
            compression = "None";
            break;
          case Pak::CompressionMethod::Zlib:
            compression = "Zlib";
            break;
          case Pak::CompressionMethod::Gzip:
            compression = "Gzip";
            break;
          case Pak::CompressionMethod::Zstd:
            compression = "Zstd";
            break;
          case Pak::CompressionMethod::LZ4:
            compression = "LZ4";
            break;
          case Pak::CompressionMethod::Oodle:
            compression = "Oodle";
            break;
          default:
            compression = "Unknown";
            break;
          }

          spdlog::info("  {} (Size: {}, Compressed: {}, Method: {})", path,
                       entry.uncompressedSize, entry.size, compression);
        }
      }

      spdlog::info("PAK listing complete");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  auto pak_info =
      pak_cmd->add_subcommand("info", "Display information about a PAK file");
  pak_info->add_option("file", pak_file, "PAK file to analyze")->required();
  pak_info->add_option("--key", pak_aes_key, "AES key for encrypted PAK files");

  pak_info->callback([&]() {
    spdlog::info("Analyzing PAK file: {}", pak_file);

    try {
      // Create a PAK reader
      std::optional<Pak::AesKey> key_opt;
      if (!pak_aes_key.empty()) {
        key_opt = Pak::AesKey(pak_aes_key);
        spdlog::info("Using AES key: {}", pak_aes_key);
      }

      auto reader = Pak::PakReader(pak_file, key_opt);

      // Get PAK info
      auto version = reader.getVersion();
      auto mountPoint = reader.getMountPoint();
      auto encrypted = reader.isEncrypted();
      auto entries = reader.getEntries();

      // Calculate total size
      uint64_t totalSize = 0;
      uint64_t totalCompressedSize = 0;
      for (const auto &entry : entries) {
        totalSize += entry.uncompressedSize;
        totalCompressedSize += entry.size;
      }

      // Display info
      spdlog::info("PAK Version: {}", static_cast<int>(version));
      spdlog::info("Mount Point: {}", mountPoint);
      spdlog::info("Encrypted: {}", encrypted ? "Yes" : "No");
      spdlog::info("File Count: {}", entries.size());
      spdlog::info("Total Size: {} bytes", totalSize);
      spdlog::info("Total Compressed Size: {} bytes", totalCompressedSize);

      if (totalSize > 0) {
        double compressionRatio = static_cast<double>(totalCompressedSize) /
                                  static_cast<double>(totalSize);
        spdlog::info("Compression Ratio: {:.2f}%", compressionRatio * 100.0);
      }

      // Count files by compression method
      std::unordered_map<Pak::CompressionMethod, size_t> compressionCounts;
      for (const auto &entry : entries) {
        compressionCounts[entry.compressionMethod]++;
      }

      spdlog::info("Compression Methods:");
      for (const auto &[method, count] : compressionCounts) {
        std::string methodName;
        switch (method) {
        case Pak::CompressionMethod::None:
          methodName = "None";
          break;
        case Pak::CompressionMethod::Zlib:
          methodName = "Zlib";
          break;
        case Pak::CompressionMethod::Gzip:
          methodName = "Gzip";
          break;
        case Pak::CompressionMethod::Zstd:
          methodName = "Zstd";
          break;
        case Pak::CompressionMethod::LZ4:
          methodName = "LZ4";
          break;
        case Pak::CompressionMethod::Oodle:
          methodName = "Oodle";
          break;
        default:
          methodName = "Unknown";
          break;
        }

        spdlog::info("  {}: {} files", methodName, count);
      }

      spdlog::info("PAK analysis complete");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  auto pak_extract =
      pak_cmd->add_subcommand("extract", "Extract files from a PAK file");
  pak_extract->add_option("file", pak_file, "PAK file to extract")->required();
  pak_extract->add_option("output", pak_output_dir, "Output directory")
      ->required();
  pak_extract->add_option("--key", pak_aes_key,
                          "AES key for encrypted PAK files");
  pak_extract->add_flag("--list-only", pak_list_only,
                        "Only list files, don't extract");

  pak_extract->callback([&]() {
    spdlog::info("Extracting PAK file: {}", pak_file);

    try {
      // Create a PAK reader
      std::optional<Pak::AesKey> key_opt;
      if (!pak_aes_key.empty()) {
        key_opt = Pak::AesKey(pak_aes_key);
        spdlog::info("Using AES key: {}", pak_aes_key);
      }

      auto reader = Pak::PakReader(pak_file, key_opt);

      // List files
      auto files = reader.getFileList();
      spdlog::info("Found {} files in PAK", files.size());

      for (const auto &path : files) {
        spdlog::info("  {}", path);

        if (!pak_list_only) {
          // Extract file
          auto extract_result = reader.extractFile(path);
          if (extract_result.isError()) {
            spdlog::error("Failed to extract {}: {}", path,
                          extract_result.errorMessage());
          } else {
            // Write to file
            std::filesystem::create_directories(pak_output_dir);
            std::string output_path =
                (std::filesystem::path(pak_output_dir) / path).string();
            std::filesystem::create_directories(
                std::filesystem::path(output_path).parent_path());

            std::ofstream out(output_path, std::ios::binary);
            if (!out) {
              spdlog::error("Failed to create output file: {}", output_path);
              continue;
            }

            const auto &data = extract_result.value();
            out.write(reinterpret_cast<const char *>(data.data()), data.size());
            out.close();

            spdlog::info("    Wrote {} bytes to {}", data.size(), output_path);
          }
        }
      }

      spdlog::info("PAK extraction complete");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  // UTOC subcommands
  std::string utoc_file;
  std::string utoc_output_dir;
  std::string utoc_aes_key;
  std::string ucas_file;

  auto utoc_list =
      utoc_cmd->add_subcommand("list", "List chunks in a UTOC file");
  utoc_list->add_option("file", utoc_file, "UTOC file to list")->required();
  utoc_list->add_option("--key", utoc_aes_key,
                        "AES key for encrypted UTOC files");

  utoc_list->callback([&]() {
    spdlog::info("Listing chunks in UTOC file: {}", utoc_file);

    try {
      // Create a UTOC reader
      std::optional<Utoc::AesKey> key_opt;
      if (!utoc_aes_key.empty()) {
        key_opt = Utoc::AesKey(utoc_aes_key);
        spdlog::info("Using AES key: {}", utoc_aes_key);
      }

      auto reader = Utoc::UtocReader(utoc_file, std::nullopt, key_opt);

      // Get UTOC info
      const auto &header = reader.getContainerHeader();
      spdlog::info("UTOC Version: {}", static_cast<int>(header.version));
      spdlog::info("Mount Point: {}", header.mountPoint);
      spdlog::info("Encrypted: {}", header.encrypted ? "Yes" : "No");

      // Check if the UTOC file has a Directory Index
      auto filePathMap = reader.getFilePathMap();
      bool hasDirectoryIndex = !filePathMap.empty();

      if (hasDirectoryIndex) {
        // Display file paths
        spdlog::info("Found {} files in UTOC", filePathMap.size());

        for (const auto &[path, chunk_id] : filePathMap) {
          auto chunk_info_result = reader.getChunkInfo(chunk_id);
          if (chunk_info_result.isError()) {
            spdlog::info("  {} (Chunk: {})", path, chunk_id.toString());
          } else {
            const auto &chunk_info = chunk_info_result.value();
            std::string compression;
            switch (chunk_info.compressionMethod) {
            case Utoc::CompressionMethod::None:
              compression = "None";
              break;
            case Utoc::CompressionMethod::Zlib:
              compression = "Zlib";
              break;
            case Utoc::CompressionMethod::Gzip:
              compression = "Gzip";
              break;
            case Utoc::CompressionMethod::Zstd:
              compression = "Zstd";
              break;
            case Utoc::CompressionMethod::Oodle:
              compression = "Oodle";
              break;
            default:
              compression = "Unknown";
              break;
            }

            std::string type;
            switch (chunk_info.type) {
            case Utoc::ChunkType::Unknown:
              type = "Unknown";
              break;
            case Utoc::ChunkType::Embedded:
              type = "Embedded";
              break;
            case Utoc::ChunkType::Compressed:
              type = "Compressed";
              break;
            case Utoc::ChunkType::Raw:
              type = "Raw";
              break;
            default:
              type = "Unknown";
              break;
            }

            spdlog::info("  {} (Chunk: {}, Size: {}, Compressed: {}, Method: "
                         "{}, Type: {})",
                         path, chunk_id.toString(), chunk_info.uncompressedSize,
                         chunk_info.size, compression, type);
          }
        }
      } else {
        // No Directory Index, display chunks
        auto chunks = reader.getChunkList();
        spdlog::info("Found {} chunks in UTOC (No Directory Index)",
                     chunks.size());

        for (const auto &chunk_id : chunks) {
          auto chunk_info_result = reader.getChunkInfo(chunk_id);
          if (chunk_info_result.isError()) {
            spdlog::info("  Chunk {}", chunk_id.toString());
          } else {
            const auto &chunk_info = chunk_info_result.value();
            std::string compression;
            switch (chunk_info.compressionMethod) {
            case Utoc::CompressionMethod::None:
              compression = "None";
              break;
            case Utoc::CompressionMethod::Zlib:
              compression = "Zlib";
              break;
            case Utoc::CompressionMethod::Gzip:
              compression = "Gzip";
              break;
            case Utoc::CompressionMethod::Zstd:
              compression = "Zstd";
              break;
            case Utoc::CompressionMethod::Oodle:
              compression = "Oodle";
              break;
            default:
              compression = "Unknown";
              break;
            }

            std::string type;
            switch (chunk_info.type) {
            case Utoc::ChunkType::Unknown:
              type = "Unknown";
              break;
            case Utoc::ChunkType::Embedded:
              type = "Embedded";
              break;
            case Utoc::ChunkType::Compressed:
              type = "Compressed";
              break;
            case Utoc::ChunkType::Raw:
              type = "Raw";
              break;
            default:
              type = "Unknown";
              break;
            }

            spdlog::info(
                "  Chunk {} (Size: {}, Compressed: {}, Method: {}, Type: {})",
                chunk_id.toString(), chunk_info.uncompressedSize,
                chunk_info.size, compression, type);
          }
        }
      }

      spdlog::info("UTOC listing complete");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  auto utoc_info =
      utoc_cmd->add_subcommand("info", "Display information about a UTOC file");
  utoc_info->add_option("file", utoc_file, "UTOC file to analyze")->required();
  utoc_info->add_option("--key", utoc_aes_key,
                        "AES key for encrypted UTOC files");

  utoc_info->callback([&]() {
    spdlog::info("Analyzing UTOC file: {}", utoc_file);

    try {
      // Create a UTOC reader
      std::optional<Utoc::AesKey> key_opt;
      if (!utoc_aes_key.empty()) {
        key_opt = Utoc::AesKey(utoc_aes_key);
        spdlog::info("Using AES key: {}", utoc_aes_key);
      }

      auto reader = Utoc::UtocReader(utoc_file, std::nullopt, key_opt);

      // Get UTOC info
      const auto &header = reader.getContainerHeader();
      spdlog::info("UTOC Version: {}", static_cast<int>(header.version));
      spdlog::info("Chunk Count: {}", header.entryCount);
      spdlog::info("Total Size: {} bytes", header.containerSize);
      spdlog::info("Encrypted: {}", header.encrypted ? "Yes" : "No");

      spdlog::info("UTOC analysis complete");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  auto utoc_extract = utoc_cmd->add_subcommand(
      "extract", "Extract chunks from a UTOC/UCAS file pair");
  utoc_extract->add_option("utoc", utoc_file, "UTOC file to extract from")
      ->required();
  utoc_extract
      ->add_option("ucas", ucas_file, "UCAS file containing the chunk data")
      ->required();
  utoc_extract->add_option("output", utoc_output_dir, "Output directory")
      ->required();
  utoc_extract->add_option("--key", utoc_aes_key,
                           "AES key for encrypted files");

  utoc_extract->callback([&]() {
    spdlog::info("Extracting from UTOC file: {} and UCAS file: {}", utoc_file,
                 ucas_file);

    try {
      // Create a UTOC reader
      std::optional<Utoc::AesKey> utoc_key_opt;
      if (!utoc_aes_key.empty()) {
        utoc_key_opt = Utoc::AesKey(utoc_aes_key);
        spdlog::info("Using AES key: {}", utoc_aes_key);
      }

      auto utoc_reader = Utoc::UtocReader(
          utoc_file, std::make_optional(std::filesystem::path(ucas_file)),
          utoc_key_opt);

      // Create a UCAS reader
      std::optional<Ucas::AesKey> ucas_key_opt;
      if (!utoc_aes_key.empty()) {
        ucas_key_opt = Ucas::AesKey(utoc_aes_key);
      }

      auto ucas_reader = Ucas::UcasReader(ucas_file, ucas_key_opt);

      // Get chunks
      auto chunk_ids = utoc_reader.getChunkList();
      spdlog::info("Found {} chunks in UTOC", chunk_ids.size());

      // Create output directory
      std::filesystem::create_directories(utoc_output_dir);

      // Extract each chunk
      for (const auto &chunk_id : chunk_ids) {
        spdlog::info("Extracting chunk {}", chunk_id.toString());

        // Get chunk info
        auto chunk_info_result = utoc_reader.getChunkInfo(chunk_id);
        if (chunk_info_result.isError()) {
          spdlog::error("Failed to get chunk info: {}",
                        chunk_info_result.errorMessage());
          continue;
        }

        auto chunk_info = chunk_info_result.value();

        // Get chunk data from UCAS
        auto chunk_result = ucas_reader.readChunk(
            chunk_info.offset, chunk_info.size,
            static_cast<Ucas::CompressionMethod>(
                static_cast<uint8_t>(chunk_info.compressionMethod)),
            chunk_info.uncompressedSize);

        if (chunk_result.isError()) {
          spdlog::error("Failed to read chunk {}: {}", chunk_id.toString(),
                        chunk_result.errorMessage());
          continue;
        }

        // Write chunk to file
        std::string chunk_file = (std::filesystem::path(utoc_output_dir) /
                                  ("chunk_" + chunk_id.toString() + ".bin"))
                                     .string();
        std::ofstream out(chunk_file, std::ios::binary);
        if (!out) {
          spdlog::error("Failed to create output file: {}", chunk_file);
          continue;
        }

        const auto &data = chunk_result.value();
        out.write(reinterpret_cast<const char *>(data.data()), data.size());
        out.close();

        spdlog::info("  Wrote {} bytes to {}", data.size(), chunk_file);
      }

      spdlog::info("UTOC/UCAS extraction complete");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  // UCAS subcommands
  std::string ucas_output_file;
  std::string input_file;
  std::string compression_method = "zstd";

  auto ucas_info =
      ucas_cmd->add_subcommand("info", "Display information about a UCAS file");
  ucas_info->add_option("file", ucas_file, "UCAS file to analyze")->required();
  ucas_info->add_option("--key", utoc_aes_key,
                        "AES key for encrypted UCAS files");

  ucas_info->callback([&]() {
    spdlog::info("Analyzing UCAS file: {}", ucas_file);

    try {
      // Create a UCAS reader
      std::optional<Ucas::AesKey> key_opt;
      if (!utoc_aes_key.empty()) {
        key_opt = Ucas::AesKey(utoc_aes_key);
        spdlog::info("Using AES key: {}", utoc_aes_key);
      }

      auto reader = Ucas::UcasReader(ucas_file, key_opt);

      // Get UCAS info
      auto encrypted = reader.isEncrypted();
      auto size = reader.getSize();

      // Display info
      spdlog::info("UCAS File: {}", ucas_file);
      spdlog::info("Encrypted: {}", encrypted ? "Yes" : "No");
      spdlog::info("Size: {} bytes", size);

      spdlog::info("UCAS analysis complete");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  auto ucas_create =
      ucas_cmd->add_subcommand("create", "Create a new UCAS file");
  ucas_create->add_option("output", ucas_output_file, "Output UCAS file")
      ->required();
  ucas_create->add_option("--key", utoc_aes_key,
                          "AES key for encrypted UCAS files");

  ucas_create->callback([&]() {
    spdlog::info("Creating UCAS file: {}", ucas_output_file);

    try {
      // Create a UCAS writer
      std::optional<Ucas::AesKey> key_opt;
      if (!utoc_aes_key.empty()) {
        key_opt = Ucas::AesKey(utoc_aes_key);
        spdlog::info("Using AES key: {}", utoc_aes_key);
      }

      auto writer = Ucas::UcasWriter(ucas_output_file, key_opt);

      spdlog::info("UCAS file created successfully");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  auto ucas_add = ucas_cmd->add_subcommand("add", "Add a file to a UCAS file");
  ucas_add->add_option("ucas", ucas_file, "UCAS file to add to")->required();
  ucas_add->add_option("file", input_file, "File to add")->required();
  ucas_add->add_option("--compression", compression_method,
                       "Compression method (none, zlib, gzip, zstd, oodle)");
  ucas_add->add_option("--key", utoc_aes_key,
                       "AES key for encrypted UCAS files");

  ucas_add->callback([&]() {
    spdlog::info("Adding file {} to UCAS file: {}", input_file, ucas_file);

    try {
      // Map compression method string to enum
      Ucas::CompressionMethod method = Ucas::CompressionMethod::Zstd;
      if (compression_method == "none") {
        method = Ucas::CompressionMethod::None;
      } else if (compression_method == "zlib") {
        method = Ucas::CompressionMethod::Zlib;
      } else if (compression_method == "gzip") {
        method = Ucas::CompressionMethod::Gzip;
      } else if (compression_method == "zstd") {
        method = Ucas::CompressionMethod::Zstd;
      } else if (compression_method == "oodle") {
        method = Ucas::CompressionMethod::Oodle;
      } else {
        spdlog::error("Unknown compression method: {}", compression_method);
        return 1;
      }

      // Create a UCAS writer
      std::optional<Ucas::AesKey> key_opt;
      if (!utoc_aes_key.empty()) {
        key_opt = Ucas::AesKey(utoc_aes_key);
        spdlog::info("Using AES key: {}", utoc_aes_key);
      }

      auto writer = Ucas::UcasWriter(ucas_file, key_opt);

      // Read input file
      std::ifstream in(input_file, std::ios::binary);
      if (!in) {
        spdlog::error("Failed to open input file: {}", input_file);
        return 1;
      }

      in.seekg(0, std::ios::end);
      size_t size = in.tellg();
      in.seekg(0, std::ios::beg);

      std::vector<uint8_t> data(size);
      in.read(reinterpret_cast<char *>(data.data()), size);
      in.close();

      spdlog::info("Read {} bytes from {}", size, input_file);

      // Add to UCAS
      auto result = writer.writeChunk(data, method);
      if (result.isError()) {
        spdlog::error("Failed to write chunk: {}", result.errorMessage());
        return 1;
      }

      auto chunk_info = result.value();
      spdlog::info(
          "Added chunk: id={}, offset={}, size={}, uncompressed_size={}",
          chunk_info.id, chunk_info.offset, chunk_info.size,
          chunk_info.uncompressedSize);

      // Finalize
      auto finalize_result = writer.finalize();
      if (finalize_result.isError()) {
        spdlog::error("Failed to finalize UCAS file: {}",
                      finalize_result.errorMessage());
        return 1;
      }

      spdlog::info("UCAS file updated successfully");
    } catch (const std::exception &e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    return 0;
  });

  // Apply configureSubcommand to existing subcommands
  configureSubcommand(pak_list);
  configureSubcommand(pak_info);
  configureSubcommand(pak_extract);
  configureSubcommand(utoc_list);
  configureSubcommand(utoc_info);
  configureSubcommand(utoc_extract);
  configureSubcommand(ucas_info);
  configureSubcommand(ucas_create);
  configureSubcommand(ucas_add);

  // Convert subcommands (placeholder for future implementation)
  convert_cmd->add_subcommand("pak-to-utoc", "Convert PAK files to UTOC/UCAS");
  convert_cmd->add_subcommand("utoc-to-pak", "Convert UTOC/UCAS to PAK files");
  convert_cmd->add_subcommand("legacy-to-zen",
                              "Convert legacy assets to Zen assets");
  convert_cmd->add_subcommand("zen-to-legacy",
                              "Convert Zen assets to legacy assets");

  // Apply the same configuration to all nested subcommands
  for (auto *subcmd : {pak_cmd, utoc_cmd, ucas_cmd, convert_cmd}) {
    for (auto *cmd : subcmd->get_subcommands()) {
      configureSubcommand(cmd);
    }
  }

  // Parse command line
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  return 0;
}
