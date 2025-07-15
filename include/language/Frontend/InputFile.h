//===--- InputFile.h --------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_FRONTEND_INPUTFILE_H
#define LANGUAGE_FRONTEND_INPUTFILE_H

#include "language/Basic/FileTypes.h"
#include "language/Basic/PrimarySpecificPaths.h"
#include "language/Basic/SupplementaryOutputPaths.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include <string>

namespace language {

/// An \c InputFile encapsulates information about an input passed to the
/// frontend.
///
/// Compiler inputs are usually passed on the command line without a leading
/// flag. However, there are clients that use the \c CompilerInvocation as
/// a library like LLDB and SourceKit that generate their own \c InputFile
/// instances programmatically. Note that an \c InputFile need not actually be
/// backed by a physical file, nor does its file name actually reflect its
/// contents. \c InputFile has a constructor that will try to figure out the file
/// type from the file name if none is provided, but many clients that
/// construct \c InputFile instances themselves may provide bogus file names
/// with pre-computed kinds. It is imperative that \c InputFile::getType be used
/// as a source of truth for this information.
///
/// \warning \c InputFile takes an unfortunately lax view of the ownership of
/// its primary data. It currently only owns the file name and a copy of any
/// assigned \c PrimarySpecificPaths outright. It is the responsibility of the
/// caller to ensure that an associated memory buffer outlives the \c InputFile.
class InputFile final {
  std::string Filename;
  file_types::ID FileID;
  toolchain::PointerIntPair<toolchain::MemoryBuffer *, 1, bool> BufferAndIsPrimary;
  PrimarySpecificPaths PSPs;

public:
  /// Constructs an input file from the provided data.
  ///
  /// \warning This entrypoint infers the type of the file from its extension
  /// and is therefore not suitable for most clients that use files synthesized
  /// from memory buffers. Use the overload of this constructor accepting a
  /// memory buffer and an explicit \c file_types::ID instead.
  InputFile(StringRef name, bool isPrimary,
            toolchain::MemoryBuffer *buffer = nullptr)
      : InputFile(name, isPrimary, buffer,
                  file_types::lookupTypeForExtension(
                      toolchain::sys::path::extension(name))) {}

  /// Constructs an input file from the provided data.
  InputFile(StringRef name, bool isPrimary, toolchain::MemoryBuffer *buffer,
            file_types::ID FileID)
      : Filename(
            convertBufferNameFromTOOLCHAIN_getFileOrSTDIN_toCodiraConventions(name)),
        FileID(FileID), BufferAndIsPrimary(buffer, isPrimary),
        PSPs(PrimarySpecificPaths()) {
    assert(!name.empty());
  }

public:
  /// Retrieves the type of this input file.
  file_types::ID getType() const { return FileID; };

  /// Retrieves whether this input file was passed as a primary to the frontend.
  bool isPrimary() const { return BufferAndIsPrimary.getInt(); }

  /// Retrieves the backing buffer for this input file, if any.
  toolchain::MemoryBuffer *getBuffer() const {
    return BufferAndIsPrimary.getPointer();
  }

  /// The name of this \c InputFile, or `-` if this input corresponds to the
  /// standard input stream.
  ///
  /// The returned file name is guaranteed not to be the empty string.
  const std::string &getFileName() const {
    assert(!Filename.empty());
    return Filename;
  }

  /// Return Codira-standard file name from a buffer name set by
  /// toolchain::MemoryBuffer::getFileOrSTDIN, which uses "<stdin>" instead of "-".
  static StringRef convertBufferNameFromTOOLCHAIN_getFileOrSTDIN_toCodiraConventions(
      StringRef filename) {
    return filename == "<stdin>" ? "-" : filename;
  }

  /// Retrieves the name of the output file corresponding to this input.
  ///
  /// If there is no such corresponding file, the result is the empty string.
  /// If there the resulting output should be directed to the standard output
  /// stream, the result is "-".
  std::string outputFilename() const { return PSPs.OutputFilename; }

  std::string indexUnitOutputFilename() const {
    if (!PSPs.IndexUnitOutputFilename.empty())
      return PSPs.IndexUnitOutputFilename;
    return outputFilename();
  }

  /// If there are explicit primary inputs (i.e. designated with -primary-input
  /// or -primary-filelist), the paths specific to those inputs (other than the
  /// input file path itself) are kept here. If there are no explicit primary
  /// inputs (for instance for whole module optimization), the corresponding
  /// paths are kept in the first input file.
  const PrimarySpecificPaths &getPrimarySpecificPaths() const { return PSPs; }

  void setPrimarySpecificPaths(PrimarySpecificPaths &&PSPs) {
    this->PSPs = std::move(PSPs);
  }

  // The next set of functions provides access to those primary-specific paths
  // accessed directly from an InputFile, as opposed to via
  // FrontendInputsAndOutputs. They merely make the call sites
  // a bit shorter. Add more forwarding methods as needed.

  StringRef getDependenciesFilePath() const {
    return getPrimarySpecificPaths().SupplementaryOutputs.DependenciesFilePath;
  }
  StringRef getLoadedModuleTracePath() const {
    return getPrimarySpecificPaths().SupplementaryOutputs.LoadedModuleTracePath;
  }
  StringRef getFineModuleTracePath() const {
    return getPrimarySpecificPaths().SupplementaryOutputs.FineModuleTracePath;
  }
  StringRef getSerializedDiagnosticsPath() const {
    return getPrimarySpecificPaths().SupplementaryOutputs
        .SerializedDiagnosticsPath;
  }
  StringRef getFixItsOutputPath() const {
    return getPrimarySpecificPaths().SupplementaryOutputs.FixItsOutputPath;
  }
};
} // namespace language

#endif // LANGUAGE_FRONTEND_INPUTFILE_H
