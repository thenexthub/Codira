//===--- language-refactor.cpp - Test driver for local refactoring --*- C++ -*-==//
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

#include "language/Basic/Defer.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/Refactoring/Refactoring.h"
#include "language/IDE/Utils.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/FileSystem.h"

#include <regex>

using namespace language;
using namespace language::ide;

namespace options {
static toolchain::cl::opt<RefactoringKind>
Action(toolchain::cl::desc("kind:"), toolchain::cl::init(RefactoringKind::None),
       toolchain::cl::values(
           clEnumValN(RefactoringKind::LocalRename,
                      "rename", "Perform rename refactoring"),
           clEnumValN(RefactoringKind::ExtractExpr,
                      "extract-expr", "Perform extract expression refactoring"),
           clEnumValN(RefactoringKind::ExtractRepeatedExpr,
                      "extract-repeat", "Perform extract repeated expression refactoring"),
           clEnumValN(RefactoringKind::FillProtocolStub,
                      "fill-stub", "Perform fill protocol stub refactoring"),
           clEnumValN(RefactoringKind::ExpandDefault,
                      "expand-default", "Perform expand default statement refactoring"),
           clEnumValN(RefactoringKind::ExpandSwitchCases,
                      "expand-switch-cases", "Perform switch cases expand refactoring"),
           clEnumValN(RefactoringKind::LocalizeString,
                      "localize-string", "Perform string localization refactoring"),
           clEnumValN(RefactoringKind::CollapseNestedIfStmt,
                      "collapse-nested-if", "Perform collapse nested if statements"),
           clEnumValN(RefactoringKind::ConvertToDoCatch,
                      "convert-to-do-catch", "Perform force try to do try catch refactoring"),
           clEnumValN(RefactoringKind::SimplifyNumberLiteral,
                      "simplify-long-number", "Perform simplify long number literal refactoring"),
           clEnumValN(RefactoringKind::ConvertStringsConcatenationToInterpolation,
                      "strings-concatenation-to-interpolation", "Perform strings concatenation to interpolation refactoring"),
           clEnumValN(RefactoringKind::ExpandTernaryExpr,
                      "expand-ternary-expr", "Perform expand ternary expression"),
           clEnumValN(RefactoringKind::ConvertToTernaryExpr,
                      "convert-to-ternary-expr", "Perform convert to ternary expression"),
		       clEnumValN(RefactoringKind::ConvertIfLetExprToGuardExpr,
                      "convert-to-guard", "Perform convert to guard expression"),
           clEnumValN(RefactoringKind::ConvertGuardExprToIfLetExpr,
                      "convert-to-iflet", "Perform convert to iflet expression"),
           clEnumValN(RefactoringKind::ExtractFunction,
                      "extract-function", "Perform extract function refactoring"),
           clEnumValN(RefactoringKind::MoveMembersToExtension,
                      "move-to-extension", "Move selected members to an extension"),
           clEnumValN(RefactoringKind::GlobalRename,
                      "syntactic-rename", "Perform syntactic rename"),
           clEnumValN(RefactoringKind::FindGlobalRenameRanges,
                      "find-rename-ranges", "Find detailed ranges for syntactic rename"),
           clEnumValN(RefactoringKind::FindLocalRenameRanges,
                      "find-local-rename-ranges", "Find detailed ranges for local rename"),
           clEnumValN(RefactoringKind::TrailingClosure,
                      "trailingclosure", "Perform trailing closure refactoring"),
           clEnumValN(RefactoringKind::ReplaceBodiesWithFatalError,
                      "replace-bodies-with-fatalError", "Perform trailing closure refactoring"),
           clEnumValN(RefactoringKind::MemberwiseInitLocalRefactoring, "memberwise-init", "Generate member wise initializer"),
           clEnumValN(RefactoringKind::AddEquatableConformance, "add-equatable-conformance", "Add Equatable conformance"),
           clEnumValN(RefactoringKind::AddExplicitCodableImplementation, "add-explicit-codable-implementation", "Add Explicit Codable Implementation"),
           clEnumValN(RefactoringKind::ConvertToComputedProperty,
                      "convert-to-computed-property", "Convert from field initialization to computed property"),
           clEnumValN(RefactoringKind::ConvertToSwitchStmt, "convert-to-switch-stmt", "Perform convert to switch statement"),
           clEnumValN(RefactoringKind::ConvertCallToAsyncAlternative,
                      "convert-call-to-async-alternative", "Convert call to use its async alternative (if any)"),
           clEnumValN(RefactoringKind::ConvertToAsync,
                      "convert-to-async", "Convert the entire function to async"),
           clEnumValN(RefactoringKind::AddAsyncAlternative,
                      "add-async-alternative", "Add an async alternative of a function taking a callback"),
           clEnumValN(RefactoringKind::AddAsyncWrapper,
                      "add-async-wrapper", "Add an async alternative that forwards onto the function taking a callback")));


static toolchain::cl::opt<std::string>
ModuleName("module-name", toolchain::cl::desc("The module name of the given test."),
            toolchain::cl::init("language_refactor"));

static toolchain::cl::opt<std::string>
SourceFilename("source-filename", toolchain::cl::desc("Name of the source file"));

static toolchain::cl::list<std::string>
InputFilenames(toolchain::cl::Positional, toolchain::cl::desc("[input files...]"),
               toolchain::cl::ZeroOrMore);

static toolchain::cl::opt<std::string>
    RewrittenOutputFile("rewritten-output-file",
                        toolchain::cl::desc("Name of the rewritten output file"));

static toolchain::cl::opt<std::string>
LineColumnPair("pos", toolchain::cl::desc("Line:Column pair or /*label*/"));

static toolchain::cl::opt<std::string>
EndLineColumnPair("end-pos", toolchain::cl::desc("Line:Column pair or /*label*/"));

static toolchain::cl::opt<std::string>
NewName("new-name", toolchain::cl::desc("A given name to rename to"),
        toolchain::cl::init("new_name"));

static toolchain::cl::opt<std::string>
OldName("old-name", toolchain::cl::desc("The expected existing name"),
        toolchain::cl::init("old_name"));

static toolchain::cl::opt<bool>
IsFunctionLike("is-function-like", toolchain::cl::desc("The symbol being renamed is function-like"),
               toolchain::cl::ValueDisallowed);

static toolchain::cl::opt<bool>
IsNonProtocolType("is-non-protocol-type",
                  toolchain::cl::desc("The symbol being renamed is a type and not a protocol"));

static toolchain::cl::opt<bool> EnableExperimentalConcurrency(
    "enable-experimental-concurrency",
    toolchain::cl::desc("Whether to enable experimental concurrency or not"));

static toolchain::cl::opt<std::string>
    SDK("sdk", toolchain::cl::desc("Path to the SDK to build against"));

static toolchain::cl::list<std::string>
    ImportPaths("I",
                toolchain::cl::desc("Add a directory to the import search path"));

static toolchain::cl::opt<std::string>
Triple("target", toolchain::cl::desc("target triple"));

static toolchain::cl::opt<std::string> ResourceDir(
    "resource-dir",
    toolchain::cl::desc("The directory that holds the compiler resource files"));

enum class DumpType {
  REWRITTEN,
  JSON,
  TEXT
};
static toolchain::cl::opt<DumpType> DumpIn(
    toolchain::cl::desc("Dump edits to stdout as:"),
    toolchain::cl::init(DumpType::REWRITTEN),
    toolchain::cl::values(
        clEnumValN(DumpType::REWRITTEN, "dump-rewritten",
                   "rewritten file"),
        clEnumValN(DumpType::JSON, "dump-json",
                   "JSON"),
        clEnumValN(DumpType::TEXT, "dump-text",
                   "text")));

static toolchain::cl::opt<bool>
AvailableActions("actions",
            toolchain::cl::desc("Dump the available refactoring kind for a given range"),
            toolchain::cl::init(false));
}

bool doesFileExist(StringRef Path) {
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
    toolchain::MemoryBuffer::getFile(Path);
  if(FileBufOrErr)
    return true;
  return false;
}

struct RefactorLoc {
  unsigned Line;
  unsigned Column;
  RenameLocUsage Usage;
};

RenameLocUsage convertToNameUsage(StringRef RoleString) {
  if (RoleString == "unknown")
    return RenameLocUsage::Unknown;
  if (RoleString == "def")
    return RenameLocUsage::Definition;
  if (RoleString == "ref")
    return RenameLocUsage::Reference;
  if (RoleString == "call")
    return RenameLocUsage::Call;
  toolchain_unreachable("unhandled role string");
}

std::vector<RefactorLoc> getLocsByLabelOrPosition(StringRef LabelOrLineCol,
                                                         std::string &Buffer) {

  std::vector<RefactorLoc> LocResults;
  if (LabelOrLineCol.empty())
    return LocResults;

  if (LabelOrLineCol.contains(':')) {
    auto LineCol = parseLineCol(LabelOrLineCol);
    if (LineCol.has_value()) {
      LocResults.push_back({LineCol.value().first, LineCol.value().second,
                            RenameLocUsage::Unknown});
    } else {
      toolchain::errs() << "cannot parse position pair.";
    }
    return LocResults;
  }

  std::smatch Matches;
  // Intended to match comments like below where the "+offset" and ":usage"
  // are defaulted to 0 and ref respectively
  // /*name+offset:usage*/
  const std::regex LabelRegex("/\\*([^ *:+]+)(?:\\+(\\d+))?(?:\\:([^ *]+))?\\*/|\\n");

  std::string::const_iterator SearchStart(Buffer.cbegin());
  unsigned Line = 1;
  unsigned Column = 1;
  while (std::regex_search(SearchStart, Buffer.cend(), Matches, LabelRegex)) {
    auto EndOffset = Matches.position(0) + Matches.length(0);
    LANGUAGE_DEFER { SearchStart += EndOffset; };
    if (!Matches[1].matched) {
      ++Line;
      Column = 1;
      continue;
    }
    Column += EndOffset;
    if (LabelOrLineCol == Matches[1].str()) {
      unsigned ColumnOffset = 0;
      if (Matches[2].length() > 0 && !toolchain::to_integer(Matches[2].str(), ColumnOffset))
        continue; // bad column offset
      auto Usage = RenameLocUsage::Reference;
      if (Matches[3].length() > 0)
        Usage = convertToNameUsage(Matches[3].str());
      LocResults.push_back({Line, Column + ColumnOffset, Usage});
    }
  }
  return LocResults;
}

std::vector<RenameLoc> getRenameLocs(unsigned BufferID, SourceManager &SM,
                                     ArrayRef<RefactorLoc> Locs,
                                     StringRef OldName, StringRef NewName) {
  std::vector<RenameLoc> Renames;
  toolchain::transform(Locs, std::back_inserter(Renames),
                  [&](const RefactorLoc &Loc) -> RenameLoc {
                    return {Loc.Line, Loc.Column, Loc.Usage, OldName};
                  });
  return Renames;
}

RangeConfig getRange(unsigned BufferID, SourceManager &SM,
                                   RefactorLoc Start,
                                   RefactorLoc End) {
  RangeConfig Range;
  SourceLoc EndLoc = SM.getLocForLineCol(BufferID, End.Line, End.Column);
  Range.BufferID = BufferID;
  Range.Line = Start.Line;
  Range.Column = Start.Column;
  Range.Length = SM.getByteDistance(Range.getStart(SM), EndLoc);

  assert(Range.getEnd(SM) == EndLoc);
  return Range;
}

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// getMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement getMainExecutable
// without being given the address of a function in the main executable).
void anchorForGetMainExecutable() {}

static StringRef syntacticRenameRangeTag(RefactoringRangeKind Kind) {
  switch (Kind) {
  case RefactoringRangeKind::BaseName:
    return "base";
  case RefactoringRangeKind::KeywordBaseName:
    return "keywordBase";
  case RefactoringRangeKind::ParameterName:
    return "param";
  case RefactoringRangeKind::NoncollapsibleParameterName:
    return "noncollapsibleparam";
  case RefactoringRangeKind::DeclArgumentLabel:
    return "arglabel";
  case RefactoringRangeKind::CallArgumentLabel:
    return "callarg";
  case RefactoringRangeKind::CallArgumentColon:
    return "callcolon";
  case RefactoringRangeKind::CallArgumentCombined:
    return "callcombo";
  case RefactoringRangeKind::SelectorArgumentLabel:
    return "sel";
  }
  toolchain_unreachable("unhandled kind");
}

static int printSyntacticRenameRanges(
    CancellableResult<std::vector<SyntacticRenameRangeDetails>> RenameRanges,
    SourceManager &SM, unsigned BufferId, toolchain::raw_ostream &OS) {
  switch (RenameRanges.getKind()) {
  case CancellableResultKind::Success:
    break; // Handled below
  case CancellableResultKind::Failure:
    OS << RenameRanges.getError();
    return 1;
  case CancellableResultKind::Cancelled:
    OS << "<cancelled>";
    return 1;
  }
  SourceEditOutputConsumer outputConsumer(SM, BufferId, OS);
  for (auto RangeDetails : RenameRanges.getResult()) {
    if (RangeDetails.Type == RegionType::Mismatch ||
        RangeDetails.Type == RegionType::Unmatched) {
      continue;
    }
    for (const auto &Range : RangeDetails.Ranges) {
      std::string NewText;
      toolchain::raw_string_ostream OS(NewText);
      StringRef Tag = syntacticRenameRangeTag(Range.RangeKind);
      OS << "<" << Tag;
      if (Range.Index.has_value())
        OS << " index=" << *Range.Index;
      OS << ">" << Range.Range.str() << "</" << Tag << ">";
      Replacement Repl{/*Path=*/{}, Range.Range, /*BufferName=*/{}, OS.str(),
                       /*RegionsWorthNote=*/{}};
      outputConsumer.SourceEditConsumer::accept(SM, Repl);
    }
  }
  return RenameRanges.getResult().empty();
}

int main(int argc, char *argv[]) {
  PROGRAM_START(argc, argv);
  INITIALIZE_LLVM();
  toolchain::cl::ParseCommandLineOptions(argc, argv, "Codira refactor\n");
  if (options::SourceFilename.empty()) {
    toolchain::errs() << "cannot find source filename\n";
    return 1;
  }
  if (!doesFileExist(options::SourceFilename)) {
    toolchain::errs() << "cannot open source file.\n";
    return 1;
  }

  CompilerInvocation Invocation;
  Invocation.setMainExecutablePath(
    toolchain::sys::fs::getMainExecutable(argv[0],
    reinterpret_cast<void *>(&anchorForGetMainExecutable)));

  Invocation.setSDKPath(options::SDK);
  std::vector<SearchPathOptions::SearchPath> ImportPaths;
  for (const auto &path : options::ImportPaths) {
    ImportPaths.push_back({path, /*isSystem=*/false});
  }
  Invocation.setImportSearchPaths(ImportPaths);
  if (!options::Triple.empty())
    Invocation.setTargetTriple(options::Triple);

  if (!options::ResourceDir.empty())
    Invocation.setRuntimeResourcePath(options::ResourceDir);

  Invocation.getFrontendOptions().InputsAndOutputs.addInputFile(
      options::SourceFilename);
  Invocation.getFrontendOptions().RequestedAction = FrontendOptions::ActionType::Typecheck;
  Invocation.getLangOptions().AttachCommentsToDecls = true;
  Invocation.getLangOptions().CollectParsedToken = true;
  Invocation.getLangOptions().DisableAvailabilityChecking = true;

  if (options::EnableExperimentalConcurrency)
    Invocation.getLangOptions().EnableExperimentalConcurrency = true;

  for (auto FileName : options::InputFilenames)
    Invocation.getFrontendOptions().InputsAndOutputs.addInputFile(FileName);
  Invocation.setModuleName(options::ModuleName);
  CompilerInstance CI;
  // Display diagnostics to stderr.
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);
  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    toolchain::errs() << InstanceSetupError << '\n';
    return 1;
  }
  registerIDERequestFunctions(CI.getASTContext().evaluator);
  switch (options::Action) {
    case RefactoringKind::GlobalRename:
    case RefactoringKind::FindGlobalRenameRanges:
      // No type-checking required.
      break;
    default:
      CI.performSema();
  }

  SourceFile *SF = nullptr;
  for (auto Unit : CI.getMainModule()->getFiles()) {
    if (auto Current = dyn_cast<SourceFile>(Unit)) {
      if (Current->getFilename() == options::SourceFilename)
        SF = Current;
    }
    if (SF)
      break;
  }
  assert(SF && "no source file?");

  SourceManager &SM = SF->getASTContext().SourceMgr;
  unsigned BufferID = SF->getBufferID();
  std::string Buffer = SM.getRangeForBuffer(BufferID).str().str();

  auto Start = getLocsByLabelOrPosition(options::LineColumnPair, Buffer);
  if (Start.empty()) {
    toolchain::errs() << "cannot parse position pair.\n";
    return 1;
  }

  RefactorLoc EndLoc = Start.front();
  if (options::EndLineColumnPair.getNumOccurrences() == 1) {
    auto End = getLocsByLabelOrPosition(options::EndLineColumnPair, Buffer);
    if (End.size() > 1) {
      toolchain::errs() << "only a single start and end position may be specified.";
      return 1;
    }
    EndLoc = End.front();
  }

  RefactorLoc &StartLoc = Start.front();


  if (options::Action == RefactoringKind::FindLocalRenameRanges) {
    RangeConfig Range = getRange(BufferID, SM, StartLoc, EndLoc);
    auto SyntacticRenameRanges = findLocalRenameRanges(SF, Range);
    return printSyntacticRenameRanges(SyntacticRenameRanges, SM, BufferID,
                                      toolchain::outs());
  }

  if (options::Action == RefactoringKind::GlobalRename ||
      options::Action == RefactoringKind::FindGlobalRenameRanges) {
    if (!options::OldName.getNumOccurrences()) {
      toolchain::errs() << "old-name must be specified.";
      return 1;
    }
    if (options::Action == RefactoringKind::GlobalRename && !options::NewName.getNumOccurrences()) {
      toolchain::errs() << "new-name must be specified.";
      return 1;
    }

    std::string NewName = options::NewName;
    if (!options::NewName.getNumOccurrences()) {
      // Unlike other operations, we don't want to provide a default new_name,
      // since we don't want to validate the new name.
      NewName = "";
    }

    std::vector<RenameLoc> RenameLocs =
        getRenameLocs(BufferID, SM, Start, options::OldName, NewName);

    if (options::Action != RefactoringKind::FindGlobalRenameRanges) {
      toolchain_unreachable("unexpected refactoring kind");
    }
    auto SyntacticRenameRanges =
        findSyntacticRenameRanges(SF, RenameLocs, NewName);
    return printSyntacticRenameRanges(SyntacticRenameRanges, SM, BufferID,
                                      toolchain::outs());
  }

  RangeConfig Range = getRange(BufferID, SM, StartLoc, EndLoc);

  if (options::Action == RefactoringKind::None) {
    bool CollectRangeStartRefactorings = false;
    auto Refactorings = collectRefactorings(
        SF, Range, CollectRangeStartRefactorings, {&PrintDiags});
    toolchain::outs() << "Action begins\n";
    for (auto Info : Refactorings) {
      if (Info.AvailableKind == RefactorAvailableKind::Available) {
        toolchain::outs() << getDescriptiveRefactoringKindName(Info.Kind) << "\n";
      }
    }
    toolchain::outs() << "Action ends\n";
    return 0;
  }

  RefactoringOptions RefactoringConfig(options::Action);
  RefactoringConfig.Range = Range;
  RefactoringConfig.PreferredName = options::NewName;
  std::string Error;

  StringRef RewrittenOutputFile = options::RewrittenOutputFile;
  if (RewrittenOutputFile.empty())
    RewrittenOutputFile = "-";
  std::error_code EC;
  toolchain::raw_fd_ostream RewriteStream(RewrittenOutputFile, EC);
  if (RewriteStream.has_error()) {
    toolchain::errs() << "Could not open rewritten output file";
    return 1;
  }

  SmallVector<std::unique_ptr<SourceEditConsumer>> Consumers;
  if (!options::RewrittenOutputFile.empty() ||
      options::DumpIn == options::DumpType::REWRITTEN) {
    Consumers.emplace_back(new SourceEditOutputConsumer(
        SF->getASTContext().SourceMgr, BufferID, RewriteStream));
  }
  switch (options::DumpIn) {
  case options::DumpType::REWRITTEN:
    // Already added
    break;
  case options::DumpType::JSON:
    Consumers.emplace_back(new SourceEditJsonConsumer(toolchain::outs()));
    break;
  case options::DumpType::TEXT:
    Consumers.emplace_back(new SourceEditTextConsumer(toolchain::outs()));
    break;
  }

  BroadcastingSourceEditConsumer BroadcastConsumer(Consumers);
  return refactorCodiraModule(CI.getMainModule(), RefactoringConfig,
                             BroadcastConsumer, PrintDiags);
}
