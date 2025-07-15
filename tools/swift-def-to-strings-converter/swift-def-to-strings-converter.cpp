//===--- language-def-to-strings-converter.cpp -------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Create a .strings file from the diagnostic messages text in `.def` files.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/Compiler.h"
#include "language/Localization/LocalizationFormat.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/EndianStream.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdlib>
#include <string>
#include <system_error>

static constexpr const char *const diagnosticID[] = {
#define DIAG(KIND, ID, Group, Options, Text, Signature) #ID,
#include "language/AST/DiagnosticsAll.def"
};

static constexpr const char *const diagnosticMessages[] = {
#define DIAG(KIND, ID, Group, Options, Text, Signature) Text,
#include "language/AST/DiagnosticsAll.def"
};

enum LocalDiagID : uint32_t {
#define DIAG(KIND, ID, Group, Options, Text, Signature) ID,
#include "language/AST/DiagnosticsAll.def"
  NumDiags
};

namespace options {

static toolchain::cl::OptionCategory
    Category("language-def-to-strings-converter Options");

static toolchain::cl::opt<std::string>
    OutputDirectory("output-directory",
                    toolchain::cl::desc("Directory for the output file"),
                    toolchain::cl::cat(Category));

static toolchain::cl::opt<std::string>
    OutputFilename("output-filename",
                   toolchain::cl::desc("Filename for the output file"),
                   toolchain::cl::cat(Category));

} // namespace options

int main(int argc, char *argv[]) {
  PROGRAM_START(argc, argv);

  toolchain::cl::HideUnrelatedOptions(options::Category);
  toolchain::cl::ParseCommandLineOptions(argc, argv,
                                    "Codira `.def` to `.strings` Converter\n");

  toolchain::SmallString<128> LocalizedFilePath;
  if (options::OutputFilename.empty()) {
    // The default language for localization is English
    std::string defaultLocaleCode = "en";
    LocalizedFilePath = options::OutputDirectory;
    toolchain::sys::path::append(LocalizedFilePath, defaultLocaleCode);
    toolchain::sys::path::replace_extension(LocalizedFilePath, ".strings");
  } else {
    LocalizedFilePath = options::OutputFilename;
  }

  std::error_code error;
  toolchain::raw_fd_ostream OS(LocalizedFilePath.str(), error,
                          toolchain::sys::fs::OF_None);

  if (OS.has_error() || error) {
    toolchain::errs() << "Error has occurred while trying to write to "
                 << LocalizedFilePath.str()
                 << " with error code: " << error.message() << "\n";
    return EXIT_FAILURE;
  }

  toolchain::ArrayRef<const char *> ids(diagnosticID, LocalDiagID::NumDiags);
  toolchain::ArrayRef<const char *> messages(diagnosticMessages,
                                        LocalDiagID::NumDiags);

  language::diag::DefToStringsConverter converter(ids, messages);
  converter.convert(OS);

  return EXIT_SUCCESS;
}
