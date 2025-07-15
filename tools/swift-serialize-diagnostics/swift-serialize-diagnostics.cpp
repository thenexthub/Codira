//===--- language-serialize-diagnostics.cpp ----------------------------------===//
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
// Convert localization YAML files to a serialized format.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/STLExtras.h"
#include "language/Localization/LocalizationFormat.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Bitstream/BitstreamReader.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/EndianStream.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/OnDiskHashTable.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/YAMLParser.h"
#include "toolchain/Support/YAMLTraits.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdlib>

using namespace language;
using namespace language::diag;

namespace options {

static toolchain::cl::OptionCategory Category("language-serialize-diagnostics Options");

static toolchain::cl::opt<std::string>
    InputFilePath("input-file-path",
                  toolchain::cl::desc("Path to the `.strings` input file"),
                  toolchain::cl::cat(Category));

static toolchain::cl::opt<std::string>
    OutputDirectory("output-directory",
                    toolchain::cl::desc("Directory for the output file"),
                    toolchain::cl::cat(Category));

} // namespace options

int main(int argc, char *argv[]) {
  PROGRAM_START(argc, argv);

  toolchain::cl::HideUnrelatedOptions(options::Category);
  toolchain::cl::ParseCommandLineOptions(argc, argv,
                                    "Codira Serialize Diagnostics Tool\n");

  if (!toolchain::sys::fs::exists(options::InputFilePath)) {
    toolchain::errs() << "diagnostics file not found\n";
    return EXIT_FAILURE;
  }

  auto localeCode = toolchain::sys::path::filename(options::InputFilePath);
  toolchain::SmallString<128> SerializedFilePath(options::OutputDirectory);
  toolchain::sys::path::append(SerializedFilePath, localeCode);
  toolchain::sys::path::replace_extension(SerializedFilePath, ".db");

  SerializedLocalizationWriter Serializer;

  {
    assert(toolchain::sys::path::extension(options::InputFilePath) == ".strings");

    StringsLocalizationProducer strings(options::InputFilePath);

    strings.forEachAvailable(
        [&Serializer](language::DiagID id, toolchain::StringRef translation) {
          Serializer.insert(id, translation);
        });
  }

  if (Serializer.emit(SerializedFilePath.str())) {
    toolchain::errs() << "Cannot serialize diagnostic file "
                 << options::InputFilePath << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
