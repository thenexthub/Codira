//===- FrontendOptions.h ----------------------------------------*- C++ -*-===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_FRONTENDOPTIONS_H
#define LANGUAGE_COMPABILITY_FRONTEND_FRONTENDOPTIONS_H

#include "language/Compability/Lower/EnvironmentDefault.h"
#include "language/Compability/Parser/characters.h"
#include "language/Compability/Parser/unparse.h"
#include "language/Compability/Support/Fortran-features.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/MemoryBuffer.h"
#include <cstdint>
#include <string>

namespace language::Compability::frontend {

enum ActionKind {
  /// -test-io mode
  InputOutputTest,

  /// -E mode
  PrintPreprocessedInput,

  /// -fsyntax-only
  ParseSyntaxOnly,

  /// Emit FIR mlir file
  EmitFIR,

  /// Emit HLFIR mlir file
  EmitHLFIR,

  /// Emit an .ll file
  EmitLLVM,

  /// Emit a .bc file
  EmitLLVMBitcode,

  /// Emit a .o file.
  EmitObj,

  /// Emit a .s file.
  EmitAssembly,

  /// Parse, unparse the parse-tree and output a Fortran source file
  DebugUnparse,

  /// Parse, unparse the parse-tree and output a Fortran source file, skip the
  /// semantic checks
  DebugUnparseNoSema,

  /// Parse, resolve the sybmols, unparse the parse-tree and then output a
  /// Fortran source file
  DebugUnparseWithSymbols,

  /// Parse, run semantics, and output a Fortran source file preceded
  /// by all the necessary modules (transitively)
  DebugUnparseWithModules,

  /// Parse, run semantics and then output symbols from semantics
  DebugDumpSymbols,

  /// Parse, run semantics and then output the parse tree
  DebugDumpParseTree,

  /// Parse, run semantics and then output the pre-fir parse tree
  DebugDumpPFT,

  /// Parse, run semantics and then output the parse tree and symbols
  DebugDumpAll,

  /// Parse and then output the parse tree, skip the semantic checks
  DebugDumpParseTreeNoSema,

  /// Dump provenance
  DebugDumpProvenance,

  /// Parse then output the parsing log
  DebugDumpParsingLog,

  /// Parse then output the number of objects in the parse tree and the overall
  /// size
  DebugMeasureParseTree,

  /// Parse, run semantics and then output the pre-FIR tree
  DebugPreFIRTree,

  /// `-fget-definition`
  GetDefinition,

  /// Parse, run semantics and then dump symbol sources map
  GetSymbolsSources,

  /// Only execute frontend initialization
  InitOnly,

  /// Run a plugin action
  PluginAction
};

/// \param suffix The file extension
/// \return True if the file extension should be processed as fixed form
bool isFixedFormSuffix(toolchain::StringRef suffix);

/// \param suffix The file extension
/// \return True if the file extension should be processed as free form
bool isFreeFormSuffix(toolchain::StringRef suffix);

/// \param suffix The file extension
/// \return True if the file should be preprocessed
bool isToBePreprocessed(toolchain::StringRef suffix);

/// \param suffix The file extension
/// \return True if the file contains CUDA Fortran
bool isCUDAFortranSuffix(toolchain::StringRef suffix);

enum class Language : uint8_t {
  Unknown,

  /// MLIR: we accept this so that we can run the optimizer on it, and compile
  /// it to LLVM IR, assembly or object code.
  MLIR,

  /// LLVM IR: we accept this so that we can run the optimizer on it,
  /// and compile it to assembly or object code.
  LLVM_IR,

  /// @{ Languages that the frontend can parse and compile.
  Fortran,
  /// @}
};

// Source file layout
enum class FortranForm {
  /// The user has not specified a form. Base the form off the file extension.
  Unknown,

  /// -ffree-form
  FixedForm,

  /// -ffixed-form
  FreeForm
};

/// The kind of a file that we've been handed as an input.
class InputKind {
private:
  Language lang;

public:
  /// The input file format.
  enum Format { Source, ModuleMap, Precompiled };

  constexpr InputKind(Language l = Language::Unknown) : lang(l) {}

  Language getLanguage() const { return static_cast<Language>(lang); }

  /// Is the input kind fully-unknown?
  bool isUnknown() const { return lang == Language::Unknown; }
};

/// An input file for the front end.
class FrontendInputFile {
  /// The file name, or "-" to read from standard input.
  std::string file;

  /// The input, if it comes from a buffer rather than a file. This object
  /// does not own the buffer, and the caller is responsible for ensuring
  /// that it outlives any users.
  const toolchain::MemoryBuffer *buffer = nullptr;

  /// The kind of input, atm it contains language
  InputKind kind;

  /// Is this input file in fixed-form format? This is simply derived from the
  /// file extension and should not be altered by consumers. For input from
  /// stdin this is never modified.
  bool isFixedForm = false;

  /// Must this file be preprocessed? Note that in Flang the preprocessor is
  /// always run. This flag is used to control whether predefined and command
  /// line preprocessor macros are enabled or not. In practice, this is
  /// sufficient to implement gfortran`s logic controlled with `-cpp/-nocpp`.
  unsigned mustBePreprocessed : 1;

  /// Whether to enable CUDA Fortran language extensions
  bool isCUDAFortran{false};

public:
  FrontendInputFile() = default;
  FrontendInputFile(toolchain::StringRef file, InputKind inKind)
      : file(file.str()), kind(inKind) {

    // Based on the extension, decide whether this is a fixed or free form
    // file.
    auto pathDotIndex{file.rfind(".")};
    std::string pathSuffix{file.substr(pathDotIndex + 1)};
    isFixedForm = isFixedFormSuffix(pathSuffix);
    mustBePreprocessed = isToBePreprocessed(pathSuffix);
    isCUDAFortran = isCUDAFortranSuffix(pathSuffix);
  }

  FrontendInputFile(const toolchain::MemoryBuffer *memBuf, InputKind inKind)
      : buffer(memBuf), kind(inKind) {}

  InputKind getKind() const { return kind; }

  bool isEmpty() const { return file.empty() && buffer == nullptr; }
  bool isFile() const { return (buffer == nullptr); }
  bool getIsFixedForm() const { return isFixedForm; }
  bool getMustBePreprocessed() const { return mustBePreprocessed; }
  bool getIsCUDAFortran() const { return isCUDAFortran; }

  toolchain::StringRef getFile() const {
    assert(isFile());
    return file;
  }

  const toolchain::MemoryBuffer *getBuffer() const {
    assert(buffer && "Requested buffer, but it is empty!");
    return buffer;
  }
};

/// FrontendOptions - Options for controlling the behavior of the frontend.
struct FrontendOptions {
  FrontendOptions()
      : showHelp(false), showVersion(false), instrumentedParse(false),
        showColors(false), printSupportedCPUs(false),
        needProvenanceRangeToCharBlockMappings(false) {}

  /// Show the -help text.
  unsigned showHelp : 1;

  /// Show the -version text.
  unsigned showVersion : 1;

  /// Instrument the parse to get a more verbose log
  unsigned instrumentedParse : 1;

  /// Enable color diagnostics.
  unsigned showColors : 1;

  /// Print the supported cpus for the current target
  unsigned printSupportedCPUs : 1;

  /// Enable Provenance to character-stream mapping. Allows e.g. IDEs to find
  /// symbols based on source-code location. This is not needed in regular
  /// compilation.
  unsigned needProvenanceRangeToCharBlockMappings : 1;

  /// Input values from `-fget-definition`
  struct GetDefinitionVals {
    unsigned line;
    unsigned startColumn;
    unsigned endColumn;
  };
  GetDefinitionVals getDefVals;

  /// The input files and their types.
  std::vector<FrontendInputFile> inputs;

  /// The output file, if any.
  std::string outputFile;

  /// The frontend action to perform.
  frontend::ActionKind programAction = ParseSyntaxOnly;

  // The form to process files in, if specified.
  FortranForm fortranForm = FortranForm::Unknown;

  // Default values for environment variables to be set by the runtime.
  std::vector<language::Compability::lower::EnvironmentDefault> envDefaults;

  // The column after which characters are ignored in fixed form lines in the
  // source file.
  int fixedFormColumns = 72;

  /// The input kind, either specified via -x argument or deduced from the input
  /// file name.
  InputKind dashX;

  // Language features
  common::LanguageFeatureControl features;

  // Source file encoding
  language::Compability::parser::Encoding encoding{language::Compability::parser::Encoding::UTF_8};

  /// The list of plugins to load.
  std::vector<std::string> plugins;

  /// The name of the action to run when using a plugin action.
  std::string actionName;

  /// A list of arguments to forward to LLVM's option processing; this
  /// should only be used for debugging and experimental features.
  std::vector<std::string> toolchainArgs;

  /// A list of arguments to forward to MLIR's option processing; this
  /// should only be used for debugging and experimental features.
  std::vector<std::string> mlirArgs;

  // Return the appropriate input kind for a file extension. For example,
  /// "*.f" would return Language::Fortran.
  ///
  /// \return The input kind for the extension, or Language::Unknown if the
  /// extension is not recognized.
  static InputKind getInputKindForExtension(toolchain::StringRef extension);
};
} // namespace language::Compability::frontend

#endif // FORTRAN_FRONTEND_FRONTENDOPTIONS_H
