//===--- language-demangle.cpp - Codira Demangler app -------------------------===//
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
// This is the entry point.
//
//===----------------------------------------------------------------------===//

#include "language/Demangling/Demangle.h"
#include "language/Demangling/ManglingFlavor.h"
#include "language/Demangling/ManglingMacros.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/PrettyStackTrace.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/raw_ostream.h"

// For std::rand, to work around a bug if main()'s first function call passes
// argv[0].
#if defined(__CYGWIN__)
#include <cstdlib>
#endif

#include <iostream>

using namespace language::Demangle;

static toolchain::cl::opt<bool>
ExpandMode("expand",
               toolchain::cl::desc("Expand mode (show node structure of the demangling)"));

static toolchain::cl::opt<bool>
CompactMode("compact",
          toolchain::cl::desc("Compact mode (only emit the demangled names)"));

static toolchain::cl::opt<bool>
TreeOnly("tree-only",
           toolchain::cl::desc("Tree-only mode (do not show the demangled string)"));

static toolchain::cl::opt<bool>
DemangleType("type",
           toolchain::cl::desc("Demangle a runtime type string"));

static toolchain::cl::opt<bool>
StripSpecialization("strip-specialization",
           toolchain::cl::desc("Remangle the origin of a specialized function"));

static toolchain::cl::opt<bool>
RemangleMode("test-remangle",
           toolchain::cl::desc("Remangle test mode (show the remangled string)"));

static toolchain::cl::opt<bool>
RemangleRtMode("remangle-objc-rt",
           toolchain::cl::desc("Remangle to the ObjC runtime name mangling scheme"));

static toolchain::cl::opt<bool>
RemangleNew("remangle-new",
           toolchain::cl::desc("Remangle the symbol with new mangling scheme"));

static toolchain::cl::opt<bool>
DisableSugar("no-sugar",
           toolchain::cl::desc("No sugar mode (disable common language idioms such as ? and [] from the output)"));

static toolchain::cl::opt<bool>
Simplified("simplified",
           toolchain::cl::desc("Don't display module names or implicit self types"));

static toolchain::cl::opt<bool>
Classify("classify",
           toolchain::cl::desc("Display symbol classification characters"));

/// Options that are primarily used for testing.
/// \{
static toolchain::cl::opt<bool> DisplayLocalNameContexts(
    "display-local-name-contexts", toolchain::cl::init(true),
    toolchain::cl::desc("Qualify local names"),
    toolchain::cl::Hidden);

static toolchain::cl::opt<bool> DisplayStdlibModule(
    "display-stdlib-module", toolchain::cl::init(true),
    toolchain::cl::desc("Qualify types originating from the Codira standard library"),
    toolchain::cl::Hidden);

static toolchain::cl::opt<bool> DisplayObjCModule(
    "display-objc-module", toolchain::cl::init(true),
    toolchain::cl::desc("Qualify types originating from the __ObjC module"),
    toolchain::cl::Hidden);

static toolchain::cl::opt<std::string> HidingModule(
    "hiding-module",
    toolchain::cl::desc("Don't qualify types originating from this module"),
    toolchain::cl::Hidden);

static toolchain::cl::opt<bool>
    ShowClosureSignature("show-closure-signature", toolchain::cl::init(true),
                         toolchain::cl::desc("Show type signature of closures"),
                         toolchain::cl::Hidden);
/// \}


static toolchain::cl::list<std::string>
InputNames(toolchain::cl::Positional, toolchain::cl::desc("[mangled name...]"),
               toolchain::cl::ZeroOrMore);

static toolchain::StringRef substrBefore(toolchain::StringRef whole,
                                    toolchain::StringRef part) {
  return whole.slice(0, part.data() - whole.data());
}

static toolchain::StringRef substrAfter(toolchain::StringRef whole,
                                   toolchain::StringRef part) {
  return whole.substr((part.data() - whole.data()) + part.size());
}

static void stripSpecialization(NodePointer Node) {
  if (Node->getKind() != Node::Kind::Global)
    return;
  switch (Node->getFirstChild()->getKind()) {
    case Node::Kind::FunctionSignatureSpecialization:
    case Node::Kind::GenericSpecialization:
    case Node::Kind::GenericSpecializationPrespecialized:
    case Node::Kind::GenericSpecializationNotReAbstracted:
    case Node::Kind::GenericPartialSpecialization:
    case Node::Kind::GenericPartialSpecializationNotReAbstracted:
      Node->removeChildAt(0);
      break;
    default:
      break;
  }
}

static void demangle(toolchain::raw_ostream &os, toolchain::StringRef name,
                     language::Demangle::Context &DCtx,
                     const language::Demangle::DemangleOptions &options) {
  bool hadLeadingUnderscore = false;
  if (name.starts_with("__")) {
    hadLeadingUnderscore = true;
    name = name.substr(1);
  }
  language::Demangle::NodePointer pointer;
  if (DemangleType) {
    pointer = DCtx.demangleTypeAsNode(name);
    if (!pointer) {
      os << "<<invalid type>>";
      return;
    }
  } else {
    pointer = DCtx.demangleSymbolAsNode(name);
  }

  if (ExpandMode || TreeOnly) {
    os << "Demangling for " << name << '\n';
    os << getNodeTreeAsString(pointer);
  }

  language::Mangle::ManglingFlavor ManglingFlavor = language::Mangle::ManglingFlavor::Default;
  if (name.starts_with(MANGLING_PREFIX_EMBEDDED_STR)) {
    ManglingFlavor = language::Mangle::ManglingFlavor::Embedded;
  }

  if (RemangleMode) {
    std::string remangled;
    if (!pointer || !(name.starts_with(MANGLING_PREFIX_STR) ||
                      name.starts_with(MANGLING_PREFIX_EMBEDDED_STR) ||
                      name.starts_with("_S"))) {
      // Just reprint the original mangled name if it didn't demangle or is in
      // the old mangling scheme.
      // This makes it easier to share the same database between the
      // mangling and demangling tests.
      remangled = name.str();
    } else {
      auto mangling = language::Demangle::mangleNode(pointer, ManglingFlavor);
      if (!mangling.isSuccess()) {
        toolchain::errs() << "Error: (" << mangling.error().code << ":"
                     << mangling.error().line << ") unable to re-mangle "
                     << name << '\n';
        exit(1);
      }
      remangled = mangling.result();
      unsigned prefixLen = language::Demangle::getManglingPrefixLength(remangled);
      assert(prefixLen > 0);
      // Replace the prefix if we remangled with a different prefix.
      if (!name.starts_with(remangled.substr(0, prefixLen))) {
        unsigned namePrefixLen =
          language::Demangle::getManglingPrefixLength(name);
        assert(namePrefixLen > 0);
        remangled = name.take_front(namePrefixLen).str() +
                      remangled.substr(prefixLen);
      }
      if (name != remangled) {
        toolchain::errs() << "Error: re-mangled name \n  " << remangled
                     << "\ndoes not match original name\n  " << name << '\n';
        exit(1);
      }
    }
    if (hadLeadingUnderscore) os << '_';
    os << remangled;
    return;
  } else if (RemangleRtMode) {
    std::string remangled = name.str();
    if (pointer) {
      auto mangling = language::Demangle::mangleNodeOld(pointer);
      if (!mangling.isSuccess()) {
        toolchain::errs() << "Error: (" << mangling.error().code << ":"
                     << mangling.error().line << ") unable to re-mangle "
                     << name << '\n';
        exit(1);
      }
      remangled = mangling.result();
    }
    os << remangled;
    return;
  }
  if (!TreeOnly) {
    if (RemangleNew) {
      if (!pointer) {
        toolchain::errs() << "Error: unable to de-mangle " << name << '\n';
        exit(1);
      }
      auto mangling = language::Demangle::mangleNode(pointer, ManglingFlavor);
      if (!mangling.isSuccess()) {
        toolchain::errs() << "Error: (" << mangling.error().code << ":"
                     << mangling.error().line << ") unable to re-mangle "
                     << name << '\n';
        exit(1);
      }
      std::string remangled = mangling.result();
      os << remangled;
      return;
    }
    if (StripSpecialization) {
      stripSpecialization(pointer);
      auto mangling = language::Demangle::mangleNode(pointer, ManglingFlavor);
      if (!mangling.isSuccess()) {
        toolchain::errs() << "Error: (" << mangling.error().code << ":"
                     << mangling.error().line << ") unable to re-mangle "
                     << name << '\n';
        exit(1);
      }
      std::string remangled = mangling.result();
      os << remangled;
      return;
    }
    std::string string = language::Demangle::nodeToString(pointer, options);
    if (!CompactMode)
      os << name << " ---> ";

    if (Classify) {
      std::string Classifications;
      if (!language::Demangle::isCodiraSymbol(name))
        Classifications += 'N';
      if (DCtx.isThunkSymbol(name)) {
        if (!Classifications.empty())
          Classifications += ',';
        Classifications += "T:";
        Classifications += DCtx.getThunkTarget(name);
      } else {
        assert(DCtx.getThunkTarget(name).empty());
      }
      if (pointer && !DCtx.hasCodiraCallingConvention(name)) {
        if (!Classifications.empty())
          Classifications += ',';
        Classifications += 'C';
      }
      if (!Classifications.empty())
        os << '{' << Classifications << "} ";
    }
    os << (string.empty() ? name : toolchain::StringRef(string));
  }
  DCtx.clear();
}

static bool isValidInMangling(char ch) {
  return (ch == '_' || ch == '$' || ch == '.'
          || (ch >= 'a' && ch <= 'z')
          || (ch >= 'A' && ch <= 'Z')
          || (ch >= '0' && ch <= '9'));
}

static bool findMaybeMangled(toolchain::StringRef input, toolchain::StringRef &match) {
  const char *ptr = input.data();
  size_t len = input.size();
  const char *end = ptr + len;
  enum {
    Start,
    SeenUnderscore,
    SeenDollar,
    FoundPrefix
  } state = Start;
  const char *matchStart = nullptr;

  // Find _T, $S, $s, _$S, _$s, @__languagemacro_ followed by a valid mangled string
  while (ptr < end) {
    switch (state) {
    case Start:
      while (ptr < end) {
        char ch = *ptr++;

        if (ch == '_') {
          state = SeenUnderscore;
          matchStart = ptr - 1;
          break;
        } else if (ch == '$') {
          state = SeenDollar;
          matchStart = ptr - 1;
          break;
        } else if (ch == '@' &&
                   toolchain::StringRef(ptr, end - ptr).starts_with("__languagemacro_")){
          matchStart = ptr - 1;
          ptr = ptr + strlen("__languagemacro_");
          state = FoundPrefix;
          break;
        }
      }
      break;

    case SeenUnderscore:
      while (ptr < end) {
        char ch = *ptr++;

        if (ch == 'T') {
          state = FoundPrefix;
          break;
        } else if (ch == '$') {
          state = SeenDollar;
          break;
        } else if (ch == '_') {
          matchStart = ptr - 1;
        } else {
          state = Start;
          break;
        }
      }
      break;

    case SeenDollar:
      while (ptr < end) {
        char ch = *ptr++;

        if (ch == 'S' || ch == 's' || ch == 'e') {
          state = FoundPrefix;
          break;
        } else if (ch == '_') {
          state = SeenUnderscore;
          matchStart = ptr - 1;
          break;
        } else if (ch == '$') {
          matchStart = ptr - 1;
        } else {
          state = Start;
          break;
        }
      }
      break;

    case FoundPrefix:
      {
        const char *mangled = ptr;

        while (ptr < end && isValidInMangling(*ptr))
          ++ptr;

        if (ptr == mangled) {
          state = Start;
          break;
        }

        match = toolchain::StringRef(matchStart, ptr - matchStart);
        return true;
      }
    }
  }

  return false;
}

static int demangleSTDIN(const language::Demangle::DemangleOptions &options) {
  language::Demangle::Context DCtx;
  for (std::string mangled; std::getline(std::cin, mangled);) {
    toolchain::StringRef inputContents(mangled);
    toolchain::StringRef match;

    while (findMaybeMangled(inputContents, match)) {
      toolchain::outs() << substrBefore(inputContents, match);
      demangle(toolchain::outs(), match, DCtx, options);
      inputContents = substrAfter(inputContents, match);
    }

    toolchain::outs() << inputContents << '\n';
  }

  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
#if defined(__CYGWIN__)
  // Cygwin clang 3.5.2 with '-O3' generates CRASHING BINARY,
  // if main()'s first function call is passing argv[0].
  std::rand();
#endif
  toolchain::cl::ParseCommandLineOptions(argc, argv);

  language::Demangle::DemangleOptions options;
  options.SynthesizeSugarOnTypes = !DisableSugar;
  if (Simplified)
    options = language::Demangle::DemangleOptions::SimplifiedUIDemangleOptions();
  options.DisplayStdlibModule = DisplayStdlibModule;
  options.DisplayObjCModule = DisplayObjCModule;
  options.HidingCurrentModule = HidingModule;
  options.DisplayLocalNameContexts = DisplayLocalNameContexts;
  options.ShowClosureSignature = ShowClosureSignature;

  if (InputNames.empty()) {
    CompactMode = true;
    if (DemangleType) {
      toolchain::errs() << "The option -type cannot be used to demangle stdin.\n";
      return EXIT_FAILURE;
    }
    return demangleSTDIN(options);
  } else {
    language::Demangle::Context DCtx;
    for (toolchain::StringRef name : InputNames) {
      if (name == "_") {
        toolchain::errs() << "warning: input symbol '_' is likely the result of "
                        "variable expansion by the shell. Ensure the argument "
                        "is quoted or escaped.\n";
        continue;
      }
      if (name == "") {
        toolchain::errs() << "warning: empty input symbol is likely the result of "
                        "variable expansion by the shell. Ensure the argument "
                        "is quoted or escaped.\n";
        continue;
      }
      if (!DemangleType && (name.starts_with("S") || name.starts_with("s") || name.starts_with("e"))) {
        std::string correctedName = std::string("$") + name.str();
        demangle(toolchain::outs(), correctedName, DCtx, options);
      } else {
        demangle(toolchain::outs(), name, DCtx, options);
      }
      toolchain::outs() << '\n';
    }

    return EXIT_SUCCESS;
  }
}
