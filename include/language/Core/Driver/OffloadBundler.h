//===- OffloadBundler.h - File Bundling and Unbundling ----------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines an offload bundling API that bundles different files
/// that relate with the same source code but different targets into a single
/// one. Also the implements the opposite functionality, i.e. unbundle files
/// previous created by this API.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_DRIVER_OFFLOADBUNDLER_H
#define LANGUAGE_CORE_DRIVER_OFFLOADBUNDLER_H

#include "toolchain/Support/Compression.h"
#include "toolchain/Support/Error.h"
#include "toolchain/TargetParser/Triple.h"
#include <toolchain/Support/MemoryBuffer.h>
#include <string>
#include <vector>

namespace language::Core {

class OffloadBundlerConfig {
public:
  OffloadBundlerConfig();

  bool AllowNoHost = false;
  bool AllowMissingBundles = false;
  bool CheckInputArchive = false;
  bool PrintExternalCommands = false;
  bool HipOpenmpCompatible = false;
  bool Compress = false;
  bool Verbose = false;
  toolchain::compression::Format CompressionFormat;
  int CompressionLevel;
  uint16_t CompressedBundleVersion;

  unsigned BundleAlignment = 1;
  unsigned HostInputIndex = ~0u;

  std::string FilesType;
  std::string ObjcopyPath;

  // TODO: Convert these to toolchain::SmallVector
  std::vector<std::string> TargetNames;
  std::vector<std::string> InputFileNames;
  std::vector<std::string> OutputFileNames;
};

class OffloadBundler {
public:
  const OffloadBundlerConfig &BundlerConfig;

  // TODO: Add error checking from ClangOffloadBundler.cpp
  OffloadBundler(const OffloadBundlerConfig &BC) : BundlerConfig(BC) {}

  // List bundle IDs. Return true if an error was found.
  static toolchain::Error
  ListBundleIDsInFile(toolchain::StringRef InputFileName,
                      const OffloadBundlerConfig &BundlerConfig);

  toolchain::Error BundleFiles();
  toolchain::Error UnbundleFiles();
  toolchain::Error UnbundleArchive();
};

/// Obtain the offload kind, real machine triple, and an optional TargetID
/// out of the target information specified by the user.
/// Bundle Entry ID (or, Offload Target String) has following components:
///  * Offload Kind - Host, OpenMP, or HIP
///  * Triple - Standard LLVM Triple
///  * TargetID (Optional) - target ID, like gfx906:xnack+ or sm_30
struct OffloadTargetInfo {
  toolchain::StringRef OffloadKind;
  toolchain::Triple Triple;
  toolchain::StringRef TargetID;

  const OffloadBundlerConfig &BundlerConfig;

  OffloadTargetInfo(const toolchain::StringRef Target,
                    const OffloadBundlerConfig &BC);
  bool hasHostKind() const;
  bool isOffloadKindValid() const;
  bool isOffloadKindCompatible(const toolchain::StringRef TargetOffloadKind) const;
  bool isTripleValid() const;
  bool operator==(const OffloadTargetInfo &Target) const;
  std::string str() const;
};

// CompressedOffloadBundle represents the format for the compressed offload
// bundles.
//
// The format is as follows:
// - Magic Number (4 bytes) - A constant "CCOB".
// - Version (2 bytes)
// - Compression Method (2 bytes) - Uses the values from
// toolchain::compression::Format.
// - Total file size (4 bytes in V2, 8 bytes in V3).
// - Uncompressed Size (4 bytes in V1/V2, 8 bytes in V3).
// - Truncated MD5 Hash (8 bytes).
// - Compressed Data (variable length).
class CompressedOffloadBundle {
private:
  static inline const toolchain::StringRef MagicNumber = "CCOB";

public:
  struct CompressedBundleHeader {
    unsigned Version;
    toolchain::compression::Format CompressionFormat;
    std::optional<size_t> FileSize;
    size_t UncompressedFileSize;
    uint64_t Hash;

    static toolchain::Expected<CompressedBundleHeader> tryParse(toolchain::StringRef);
  };

  static inline const uint16_t DefaultVersion = 3;

  static toolchain::Expected<std::unique_ptr<toolchain::MemoryBuffer>>
  compress(toolchain::compression::Params P, const toolchain::MemoryBuffer &Input,
           uint16_t Version, bool Verbose = false);
  static toolchain::Expected<std::unique_ptr<toolchain::MemoryBuffer>>
  decompress(const toolchain::MemoryBuffer &Input, bool Verbose = false);
};

/// Check whether the bundle id is in the following format:
/// <kind>-<triple>[-<target id>[:target features]]
/// <triple> := <arch>-<vendor>-<os>-<env>
bool checkOffloadBundleID(const toolchain::StringRef Str);
} // namespace language::Core

#endif // LANGUAGE_CORE_DRIVER_OFFLOADBUNDLER_H
