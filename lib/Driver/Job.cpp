//===--- Job.cpp - Command to Execute -------------------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/Basic/Range.h"
#include "language/Basic/STLExtras.h"
#include "language/Driver/Job.h"
#include "language/Driver/PrettyStackTrace.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Program.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;
using namespace language::driver;

CommandOutput::CommandOutput(StringRef dummyBase, OutputFileMap &dummyOFM)
    : Inputs({CommandInputPair(dummyBase, "")}), DerivedOutputMap(dummyOFM) {
  setAdditionalOutputForType(file_types::TY_CodiraDeps, dummyBase);
}

StringRef CommandOutput::getOutputForInputAndType(StringRef PrimaryInputFile,
                                                  file_types::ID Type) const {
  if (Type == file_types::TY_Nothing)
    return StringRef();
  auto const *M = DerivedOutputMap.getOutputMapForInput(PrimaryInputFile);
  if (!M)
    return StringRef();
  auto const Out = M->find(Type);
  if (Out == M->end())
    return StringRef();
  assert(!Out->second.empty());
  return StringRef(Out->second);
}

struct CommandOutputInvariantChecker {
  CommandOutput const &Out;
  CommandOutputInvariantChecker(CommandOutput const &CO) : Out(CO) {
#ifndef NDEBUG
    Out.checkInvariants();
#endif
  }
  ~CommandOutputInvariantChecker() {
#ifndef NDEBUG
    Out.checkInvariants();
#endif
  }
};

void CommandOutput::ensureEntry(StringRef PrimaryInputFile,
                                file_types::ID Type,
                                StringRef OutputFile,
                                bool Overwrite) {
  assert(!PrimaryInputFile.empty());
  assert(!OutputFile.empty());
  assert(Type != file_types::TY_Nothing);
  auto &M = DerivedOutputMap.getOrCreateOutputMapForInput(PrimaryInputFile);
  if (Overwrite) {
    M[Type] = OutputFile.str();
  } else {
    auto res = M.insert(std::make_pair(Type, OutputFile.str()));
    if (res.second) {
      // New entry, no need to compare.
    } else {
      // Existing entry, check equality with request.
      assert(res.first->getSecond() == OutputFile);
    }
  }
}

void CommandOutput::checkInvariants() const {
  file_types::forAllTypes([&](file_types::ID Type) {
      size_t numOutputsOfType = 0;
      for (auto const &I : Inputs) {
        // FIXME: At the moment, empty primary input names correspond to
        // corner cases in the driver where it is doing TY_Nothing work
        // and isn't even given a primary input; but at some point we
        // ought to enable storing derived OFM entries under the empty
        // name in general, for "whole build" additional outputs. They
        // are presently (arbitrarily and wrongly) stored in entries
        // associated with the first primary input of the CommandOutput
        // that they were derived from.
        assert(PrimaryOutputType == file_types::TY_Nothing || !I.Primary.empty());
        auto const *M = DerivedOutputMap.getOutputMapForInput(I.Primary);
        if (!M)
          continue;
        auto const Out = M->find(Type);
        if (Out == M->end())
          continue;
        assert(!Out->second.empty());
        ++numOutputsOfType;
      }
      assert(numOutputsOfType == 0 ||
             numOutputsOfType == 1 ||
             numOutputsOfType == Inputs.size());
    });
  assert(AdditionalOutputTypes.count(PrimaryOutputType) == 0);
}

bool CommandOutput::hasSameAdditionalOutputTypes(
    CommandOutput const &other) const {
  bool sameAdditionalOutputTypes = true;
  file_types::forAllTypes([&](file_types::ID Type) {
      bool a = AdditionalOutputTypes.count(Type) == 0;
      bool b = other.AdditionalOutputTypes.count(Type) == 0;
      if (a != b)
        sameAdditionalOutputTypes = false;
    });
  return sameAdditionalOutputTypes;
}

void CommandOutput::addOutputs(CommandOutput const &other) {
  CommandOutputInvariantChecker Check(*this);
  assert(PrimaryOutputType == other.PrimaryOutputType);
  assert(&DerivedOutputMap == &other.DerivedOutputMap);
  Inputs.append(other.Inputs.begin(),
                other.Inputs.end());
  // Should only be called with an empty AdditionalOutputTypes
  // or one populated with the same types as other.
  if (AdditionalOutputTypes.empty()) {
    AdditionalOutputTypes = other.AdditionalOutputTypes;
  } else {
    assert(hasSameAdditionalOutputTypes(other));
  }
}

CommandOutput::CommandOutput(file_types::ID PrimaryOutputType,
                             OutputFileMap &Derived)
    : PrimaryOutputType(PrimaryOutputType), DerivedOutputMap(Derived) {
  CommandOutputInvariantChecker Check(*this);
}

file_types::ID CommandOutput::getPrimaryOutputType() const {
  return PrimaryOutputType;
}

void CommandOutput::addPrimaryOutput(CommandInputPair Input,
                                     StringRef PrimaryOutputFile,
                                     StringRef IndexUnitOutputPath) {
  PrettyStackTraceDriverCommandOutputAddition CrashInfo(
    "primary", this, Input.Primary, PrimaryOutputType, PrimaryOutputFile);
  if (PrimaryOutputType == file_types::TY_Nothing) {
    // For TY_Nothing, we accumulate the inputs but do not add any outputs.
    // The invariant holds on either side of this action because all primary
    // outputs for this command will be absent (so the length == 0 case in the
    // invariant holds).
    CommandOutputInvariantChecker Check(*this);
    Inputs.push_back(Input);
    return;
  }
  // The invariant holds in the non-TY_Nothing case before an input is added and
  // _after the corresponding output is added_, but not inbetween. Don't try to
  // merge these two cases, they're different.
  CommandOutputInvariantChecker Check(*this);
  Inputs.push_back(Input);
  assert(!PrimaryOutputFile.empty());
  assert(AdditionalOutputTypes.count(PrimaryOutputType) == 0);
  ensureEntry(Input.Primary, PrimaryOutputType, PrimaryOutputFile, false);

  // If there is an overriding output path to record in the index store instead
  // of the primary output path, add it.
  if (!IndexUnitOutputPath.empty()) {
    ensureEntry(Input.Primary, file_types::TY_IndexUnitOutputPath,
                IndexUnitOutputPath, false);
  }
}

StringRef CommandOutput::getPrimaryOutputFilename() const {
  // FIXME: ideally this shouldn't exist, or should at least assert size() == 1,
  // and callers should handle cases with multiple primaries explicitly.
  assert(Inputs.size() >= 1);
  return getOutputForInputAndType(Inputs[0].Primary, PrimaryOutputType);
}

SmallVector<StringRef, 16> CommandOutput::getPrimaryOutputFilenames() const {
  SmallVector<StringRef, 16> V;
  size_t NonEmpty = 0;
  for (auto const &I : Inputs) {
    auto Out = getOutputForInputAndType(I.Primary, PrimaryOutputType);
    V.push_back(Out);
    if (!Out.empty())
      ++NonEmpty;
    assert(!Out.empty() || PrimaryOutputType == file_types::TY_Nothing);
  }
  assert(NonEmpty == 0 || NonEmpty == Inputs.size());
  return V;
}

SmallVector<StringRef, 16>
CommandOutput::getIndexUnitOutputFilenames() const {
  SmallVector<StringRef, 16> V;
  size_t NonEmpty = 0;
  for (auto const &I: Inputs) {
    auto Out = getOutputForInputAndType(I.Primary,
                                        file_types::TY_IndexUnitOutputPath);
    V.push_back(Out);
    if (!Out.empty())
      ++NonEmpty;
  }

  if (!NonEmpty || NonEmpty == V.size()) {
    if (!NonEmpty)
      V.clear();
    return V;
  }

  auto PrimaryOutputs = getPrimaryOutputFilenames();
  assert(PrimaryOutputs.size() == V.size());

  for (auto I: indices(V)) {
    if (V[I].empty())
      V[I] = PrimaryOutputs[I];
  }
  return V;
}

void CommandOutput::setAdditionalOutputForType(file_types::ID Type,
                                               StringRef OutputFilename) {
  PrettyStackTraceDriverCommandOutputAddition CrashInfo(
      "additional", this, Inputs[0].Primary, Type, OutputFilename);
  CommandOutputInvariantChecker Check(*this);
  assert(Inputs.size() >= 1);
  assert(!OutputFilename.empty());
  assert(Type != file_types::TY_Nothing);

  // If we're given an "additional" output with the same type as the primary,
  // and we've not yet had such an additional type added, we treat it as a
  // request to overwrite the primary choice (which happens early and is
  // sometimes just inferred) with a refined value (eg. -emit-module-path).
  bool Overwrite = Type == PrimaryOutputType;
  if (Overwrite) {
    assert(AdditionalOutputTypes.count(Type) == 0);
  } else {
    AdditionalOutputTypes.insert(Type);
  }
  ensureEntry(Inputs[0].Primary, Type, OutputFilename, Overwrite);
}

StringRef CommandOutput::getAdditionalOutputForType(file_types::ID Type) const {
  if (AdditionalOutputTypes.count(Type) == 0)
    return StringRef();
  assert(Inputs.size() >= 1);
  // FIXME: ideally this shouldn't associate the additional output with the
  // first primary, but with a specific primary (and/or possibly the primary "",
  // for build-wide outputs) specified by the caller.
  assert(Inputs.size() >= 1);
  return getOutputForInputAndType(Inputs[0].Primary, Type);
}

SmallVector<StringRef, 16>
CommandOutput::getAdditionalOutputsForType(file_types::ID Type) const {
  SmallVector<StringRef, 16> V;
  if (AdditionalOutputTypes.count(Type) != 0) {
    for (auto const &I : Inputs) {
      auto Out = getOutputForInputAndType(I.Primary, Type);
      // FIXME: In theory this should always be non-empty -- and V.size() would
      // always be either 0 or N like with primary outputs -- but in practice
      // WMO currently associates additional outputs with the _first primary_ in
      // a multi-primary job, which means that the 2nd..Nth primaries will have
      // an empty result from getOutputForInputAndType, and V.size() will be 1.
      if (!Out.empty())
        V.push_back(Out);
    }
  }
  assert(V.empty() || V.size() == 1 || V.size() == Inputs.size());
  return V;
}

bool CommandOutput::hasAdditionalOutputForType(file_types::ID type) const {
  return AdditionalOutputTypes.count(type);
}

StringRef CommandOutput::getAnyOutputForType(file_types::ID Type) const {
  if (PrimaryOutputType == Type)
    return getPrimaryOutputFilename();
  return getAdditionalOutputForType(Type);
}

const OutputFileMap &CommandOutput::getDerivedOutputMap() const {
  return DerivedOutputMap;
}

StringRef CommandOutput::getBaseInput(size_t Index) const {
  assert(Index < Inputs.size());
  return Inputs[Index].Base;
}

static void escapeAndPrintString(toolchain::raw_ostream &os, StringRef Str) {
  if (Str.empty()) {
    // Special-case the empty string.
    os << "\"\"";
    return;
  }

  bool NeedsEscape = Str.find_first_of(" \"\\$") != StringRef::npos;

  if (!NeedsEscape) {
    // This string doesn't have anything we need to escape, so print it directly
    os << Str;
    return;
  }

  // Quote and escape. This isn't really complete, but is good enough, and
  // matches how Clang's Command handles escaping arguments.
  os << '"';
  for (const char c : Str) {
    switch (c) {
    case '"':
    case '\\':
    case '$':
      // These characters need to be escaped.
      os << '\\';
      // Fall-through to the default case, since we still need to print the
      // character.
      TOOLCHAIN_FALLTHROUGH;
    default:
      os << c;
    }
  }
  os << '"';
}

void
CommandOutput::print(raw_ostream &out) const {
  out
    << "{\n"
    << "    PrimaryOutputType = " << file_types::getTypeName(PrimaryOutputType)
    << ";\n"
    << "    Inputs = [\n";
  interleave(Inputs,
             [&](CommandInputPair const &P) {
             out << "        CommandInputPair {\n"
                 << "            Base = ";
             escapeAndPrintString(out, P.Base);
             out << ", \n"
                 << "            Primary = ";
             escapeAndPrintString(out, P.Primary);
             out << "\n        }";
           },
           [&] { out << ",\n"; });
  out << "];\n"
      << "    DerivedOutputFileMap = {\n";
  DerivedOutputMap.dump(out, true);
  out << "\n    };\n}";
}

void
CommandOutput::dump() const {
  print(toolchain::errs());
  toolchain::errs() << '\n';
}

void CommandOutput::writeOutputFileMap(toolchain::raw_ostream &out) const {
  SmallVector<StringRef, 4> inputs;
  for (const CommandInputPair &IP : Inputs) {
    assert(IP.Base == IP.Primary && !IP.Base.empty() &&
           "output file maps won't work if these differ");
    inputs.push_back(IP.Primary);
  }
  getDerivedOutputMap().write(out, inputs);
}

Job::~Job() = default;

void Job::printArguments(raw_ostream &os,
                         const toolchain::opt::ArgStringList &Args) {
  interleave(Args,
             [&](const char *Arg) { escapeAndPrintString(os, Arg); },
             [&] { os << ' '; });
}

void Job::dump() const {
  printCommandLineAndEnvironment(toolchain::errs());
}

ArrayRef<const char *> Job::getArgumentsForTaskExecution() const {
  if (hasResponseFile()) {
    writeArgsToResponseFile();
    return getResponseFileArg();
  } else {
    return getArguments();
  }
}

void Job::printCommandLineAndEnvironment(raw_ostream &Stream,
                                         StringRef Terminator) const {
  printCommandLine(Stream, /*Terminator=*/"");
  if (!ExtraEnvironment.empty()) {
    Stream << "  #";
    for (auto &pair : ExtraEnvironment) {
      Stream << " " << pair.first << "=" << pair.second;
    }
  }
  Stream << "\n";
}

void Job::printCommandLine(raw_ostream &os, StringRef Terminator) const {
  escapeAndPrintString(os, Executable);
  os << ' ';
  if (hasResponseFile()) {
    printArguments(os, {ResponseFile->argString});
    os << " # ";
  }
  printArguments(os, Arguments);

  os << Terminator;
}

void Job::printSummary(raw_ostream &os) const {
  // Deciding how to describe our inputs is a bit subtle; if we are a Job built
  // from a JobAction that itself has InputActions sources, then we collect
  // those up. Otherwise it's more correct to talk about our inputs as the
  // outputs of our input-jobs.
  SmallVector<StringRef, 4> Inputs;
  SmallVector<StringRef, 4> Outputs = getOutput().getPrimaryOutputFilenames();

  for (const Action *A : getSource().getInputs())
    if (const auto *IA = dyn_cast<InputAction>(A))
      Inputs.push_back(IA->getInputArg().getValue());

  for (const Job *J : getInputs())
    for (StringRef f : J->getOutput().getPrimaryOutputFilenames())
      Inputs.push_back(f);

  size_t limit = 3;
  size_t actual_in = Inputs.size();
  size_t actual_out = Outputs.size();
  if (actual_in > limit) {
    Inputs.erase(Inputs.begin() + limit, Inputs.end());
  }
  if (actual_out > limit) {
    Outputs.erase(Outputs.begin() + limit, Outputs.end());
  }

  os << "{" << getSource().getClassName() << ": ";
  interleave(
      Outputs, [&](StringRef Arg) { os << toolchain::sys::path::filename(Arg); },
      [&] { os << ' '; });
  if (actual_out > limit) {
    os << " ... " << (actual_out-limit) << " more";
  }
  os << " <= ";
  interleave(
      Inputs, [&](StringRef Arg) { os << toolchain::sys::path::filename(Arg); },
      [&] { os << ' '; });
  if (actual_in > limit) {
    os << " ... " << (actual_in-limit) << " more";
  }
  os << "}";
}


bool Job::writeArgsToResponseFile() const {
  assert(hasResponseFile());
  std::error_code EC;
  toolchain::raw_fd_ostream OS(ResponseFile->path, EC, toolchain::sys::fs::OF_None);
  if (EC) {
    return true;
  }
  for (const char *arg : Arguments) {
    escapeAndPrintString(OS, arg);
    OS << " ";
  }
  OS.flush();
  return false;
}

StringRef Job::getFirstCodiraPrimaryInput() const {
  const JobAction &source = getSource();
  if (!isa<CompileJobAction>(source))
    return StringRef();
  const auto *firstInput = source.getInputs().front();
  if (auto *inputInput = dyn_cast<InputAction>(firstInput))
    return inputInput->getInputArg().getValue();
  return StringRef();
}
