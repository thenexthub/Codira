//===--- TestOptions.cpp --------------------------------------------------===//
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

#include "TestOptions.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"

using namespace toolchain::opt;
using namespace sourcekitd_test;
using toolchain::StringRef;

namespace {

// Create enum with OPT_xxx values for each option in Options.td.
enum Opt {
  OPT_INVALID = 0,
#define OPTION(...) TOOLCHAIN_MAKE_OPT_ID(__VA_ARGS__),
#include "Options.inc"
  LastOption
#undef OPTION
};

// Create prefix string literals used in Options.td.
#define PREFIX(NAME, VALUE)                                                    \
  constexpr toolchain::StringLiteral NAME##_init[] = VALUE;                         \
  constexpr toolchain::ArrayRef<toolchain::StringLiteral> NAME(                          \
      NAME##_init, std::size(NAME##_init) - 1);
#include "Options.inc"
#undef PREFIX

// Create table mapping all options defined in Options.td.
static const toolchain::opt::OptTable::Info InfoTable[] = {
#define OPTION(...) TOOLCHAIN_CONSTRUCT_OPT_INFO(__VA_ARGS__),
#include "Options.inc"
#undef OPTION
};

// Create OptTable class for parsing actual command line arguments
class TestOptTable : public toolchain::opt::GenericOptTable {
public:
  TestOptTable() : GenericOptTable(InfoTable) {}
};

} // end anonymous namespace

static std::pair<unsigned, unsigned> parseLineCol(StringRef LineCol) {
  unsigned Line, Col;
  size_t ColonIdx = LineCol.find(':');
  if (ColonIdx == StringRef::npos) {
    toolchain::errs() << "wrong pos format, it should be '<line>:<column>'\n";
    exit(1);
  }
  if (LineCol.substr(0, ColonIdx).getAsInteger(10, Line)) {
    toolchain::errs() << "wrong pos format, it should be '<line>:<column>'\n";
    exit(1);
  }
  if (LineCol.substr(ColonIdx+1).getAsInteger(10, Col)) {
    toolchain::errs() << "wrong pos format, it should be '<line>:<column>'\n";
    exit(1);
  }

  if (Line == 0 || Col == 0) {
    toolchain::errs() << "wrong pos format, line/col should start from 1\n";
    exit(1);
  }

  return { Line, Col };
}

bool TestOptions::parseArgs(toolchain::ArrayRef<const char *> Args) {
  if (Args.empty())
    return false;

  // Parse command line options using Options.td
  TestOptTable Table;
  unsigned MissingIndex;
  unsigned MissingCount;
  toolchain::opt::InputArgList ParsedArgs =
      Table.ParseArgs(Args, MissingIndex, MissingCount);
  if (MissingCount) {
    toolchain::errs() << "error: missing argument value for '"
        << ParsedArgs.getArgString(MissingIndex) << "', expected "
        << MissingCount << " argument(s)\n";
    return true;
  }

  for (auto InputArg : ParsedArgs) {
    switch (InputArg->getOption().getID()) {
    case OPT_req:
      Request = toolchain::StringSwitch<SourceKitRequest>(InputArg->getValue())
        .Case("version", SourceKitRequest::ProtocolVersion)
        .Case("compiler-version", SourceKitRequest::CompilerVersion)
        .Case("demangle", SourceKitRequest::DemangleNames)
        .Case("mangle", SourceKitRequest::MangleSimpleClasses)
        .Case("index", SourceKitRequest::Index)
        .Case("complete", SourceKitRequest::CodeComplete)
        .Case("complete.open", SourceKitRequest::CodeCompleteOpen)
        .Case("complete.close", SourceKitRequest::CodeCompleteClose)
        .Case("complete.update", SourceKitRequest::CodeCompleteUpdate)
        .Case("complete.cache.ondisk", SourceKitRequest::CodeCompleteCacheOnDisk)
        .Case("complete.setpopularapi", SourceKitRequest::CodeCompleteSetPopularAPI)
        .Case("typecontextinfo", SourceKitRequest::TypeContextInfo)
        .Case("conformingmethods", SourceKitRequest::ConformingMethodList)
        .Case("cursor", SourceKitRequest::CursorInfo)
        .Case("related-idents", SourceKitRequest::RelatedIdents)
        .Case("active-regions", SourceKitRequest::ActiveRegions)
        .Case("syntax-map", SourceKitRequest::SyntaxMap)
        .Case("structure", SourceKitRequest::Structure)
        .Case("format", SourceKitRequest::Format)
        .Case("expand-placeholder", SourceKitRequest::ExpandPlaceholder)
        .Case("doc-info", SourceKitRequest::DocInfo)
        .Case("sema", SourceKitRequest::SemanticInfo)
        .Case("interface-gen", SourceKitRequest::InterfaceGen)
        .Case("interface-gen-open", SourceKitRequest::InterfaceGenOpen)
        .Case("find-usr", SourceKitRequest::FindUSR)
        .Case("find-interface", SourceKitRequest::FindInterfaceDoc)
        .Case("open", SourceKitRequest::Open)
        .Case("close", SourceKitRequest::Close)
        .Case("edit", SourceKitRequest::Edit)
        .Case("print-annotations", SourceKitRequest::PrintAnnotations)
        .Case("print-diags", SourceKitRequest::PrintDiags)
        .Case("extract-comment", SourceKitRequest::ExtractComment)
        .Case("module-groups", SourceKitRequest::ModuleGroups)
        .Case("range", SourceKitRequest::RangeInfo)
        .Case("find-rename-ranges", SourceKitRequest::FindRenameRanges)
        .Case("find-local-rename-ranges", SourceKitRequest::FindLocalRenameRanges)
        .Case("translate", SourceKitRequest::NameTranslation)
        .Case("markup-xml", SourceKitRequest::MarkupToXML)
        .Case("stats", SourceKitRequest::Statistics)
        .Case("track-compiles", SourceKitRequest::EnableCompileNotifications)
        .Case("collect-type", SourceKitRequest::CollectExpressionType)
        .Case("collect-var-type", SourceKitRequest::CollectVariableType)
        .Case("global-config", SourceKitRequest::GlobalConfiguration)
        .Case("dependency-updated", SourceKitRequest::DependencyUpdated)
        .Case("diags", SourceKitRequest::Diagnostics)
        .Case("semantic-tokens", SourceKitRequest::SemanticTokens)
        .Case("compile", SourceKitRequest::Compile)
        .Case("compile.close", SourceKitRequest::CompileClose)
        .Case("syntactic-expandmacro", SourceKitRequest::SyntacticMacroExpansion)
        .Case("index-to-store", SourceKitRequest::IndexToStore)
#define SEMANTIC_REFACTORING(KIND, NAME, ID) .Case("refactoring." #ID, SourceKitRequest::KIND)
#include "language/Refactoring/RefactoringKinds.def"
        .Default(SourceKitRequest::None);

      if (Request == SourceKitRequest::None) {
        toolchain::errs() << "error: invalid request '" << InputArg->getValue()
                     << "'\nexpected one of "
                     << "- version\n"
                     << "- compiler-version\n"
                     << "- demangle\n"
                     << "- mangle\n"
                     << "- index\n"
                     << "- complete\n"
                     << "- complete.open\n"
                     << "- complete.close\n"
                     << "- complete.update\n"
                     << "- complete.cache.ondisk\n"
                     << "- complete.setpopularapi\n"
                     << "- typecontextinfo\n"
                     << "- conformingmethods\n"
                     << "- cursor\n"
                     << "- related-idents\n"
                     << "- syntax-map\n"
                     << "- structure\n"
                     << "- format\n"
                     << "- expand-placeholder\n"
                     << "- doc-info\n"
                     << "- sema\n"
                     << "- interface-gen\n"
                     << "- interface-gen-open\n"
                     << "- find-usr\n"
                     << "- find-interface\n"
                     << "- open\n"
                     << "- close\n"
                     << "- edit\n"
                     << "- print-annotations\n"
                     << "- print-diags\n"
                     << "- extract-comment\n"
                     << "- module-groups\n"
                     << "- range\n"
                     << "- find-rename-ranges\n"
                     << "- find-local-rename-ranges\n"
                     << "- translate\n"
                     << "- markup-xml\n"
                     << "- stats\n"
                     << "- track-compiles\n"
                     << "- collect-type\n"
                     << "- global-config\n"
                     << "- dependency-updated\n"
                     << "- syntactic-expandmacro\n"
                     << "- index-to-store\n"
#define SEMANTIC_REFACTORING(KIND, NAME, ID) << "- refactoring." #ID "\n"
#include "language/Refactoring/RefactoringKinds.def"
                        "\n";
        return true;
      }
      break;

    case OPT_help: {
      printHelp(false);
      return true;
    }

    case OPT_offset: {
      unsigned offset;
      if (StringRef(InputArg->getValue()).getAsInteger(10, offset)) {
        toolchain::errs() << "error: expected integer for 'offset'\n";
        return true;
      }

      Offset = offset;
      break;
    }

    case OPT_length:
      if (StringRef(InputArg->getValue()).getAsInteger(10, Length)) {
        toolchain::errs() << "error: expected integer for 'length'\n";
        return true;
      }
      break;

    case OPT_pos: {
      auto linecol = parseLineCol(InputArg->getValue());
      Line = linecol.first;
      Col = linecol.second;
      break;
    }

    case OPT_end_pos: {
      auto linecol = parseLineCol(InputArg->getValue());
      EndLine = linecol.first;
      EndCol = linecol.second;
      break;
    }

    case OPT_using_language_args: {
      UsingCodiraArgs = true;
      break;
    }

    case OPT_language_version:
      CodiraVersion = InputArg->getValue();
      break;

    case OPT_pass_version_as_string:
      PassVersionAsString = true;
      break;

    case OPT_line:
      if (StringRef(InputArg->getValue()).getAsInteger(10, Line)) {
        toolchain::errs() << "error: expected integer for 'line'\n";
        return true;
      }
      Col = 1;
      break;

    case OPT_replace:
      ReplaceText = InputArg->getValue();
      break;

    case OPT_module:
      ModuleName = InputArg->getValue();
      break;

    case OPT_group_name:
      ModuleGroupName = InputArg->getValue();
      break;

    case OPT_id:
      RequestId = InputArg->getValue();
      break;

    case OPT_interested_usr:
      InterestedUSR = InputArg->getValue();
      break;

    case OPT_header:
      HeaderPath = InputArg->getValue();
      break;

    case OPT_text_input:
      TextInputFile = InputArg->getValue();
      break;

    case OPT_usr:
      USR = InputArg->getValue();
      break;

    case OPT_pass_as_sourcetext:
      PassAsSourceText = true;
      break;

    case OPT_cache_path:
      CachePath = InputArg->getValue();
      break;

    case OPT_req_opts:
      for (auto item : InputArg->getValues())
        RequestOptions.push_back(item);
      break;

    case OPT_check_interface_is_ascii:
      CheckInterfaceIsASCII = true;
      break;

    case OPT_dont_print_request:
      PrintRequest = false;
      break;

    case OPT_print_response_as_json:
      PrintResponseAsJSON = true;
      break;

    case OPT_print_raw_response:
      PrintRawResponse = true;
      break;

    case OPT_dont_print_response:
      PrintResponse = false;
      break;

    case OPT_INPUT:
      SourceFile = InputArg->getValue();
      SourceText = std::nullopt;
      Inputs.push_back(InputArg->getValue());
      break;

    case OPT_primary_file:
      PrimaryFile = InputArg->getValue();
      break;

    case OPT_rename_spec:
      RenameSpecPath = InputArg->getValue();
      break;

    case OPT_json_request_path:
      JsonRequestPath = InputArg->getValue();
      break;

    case OPT_simplified_demangling:
      SimplifiedDemangling = true;
      break;

    case OPT_synthesized_extension:
      SynthesizedExtensions = true;
      break;

    case OPT_async:
      isAsyncRequest = true;
      break;

    case OPT_cursor_action:
      CollectActionables = true;
      break;

    case OPT_language_name:
      CodiraName = InputArg->getValue();
      break;

    case OPT_objc_name:
      ObjCName = InputArg->getValue();
      break;

    case OPT_objc_selector:
      ObjCSelector = InputArg->getValue();
      break;

    case OPT_name:
      Name = InputArg->getValue();
      break;

    case OPT_cancel_on_subsequent_request:
      unsigned Cancel;
      if (StringRef(InputArg->getValue()).getAsInteger(10, Cancel)) {
        toolchain::errs() << "error: expected integer for 'cancel-on-subsequent-request'\n";
        return true;
      }
      CancelOnSubsequentRequest = Cancel;
      break;

    case OPT_time_request:
      timeRequest = true;
      break;

    case OPT_measure_instructions:
      measureInstructions = true;
      break;

    case OPT_repeat_request:
      if (StringRef(InputArg->getValue()).getAsInteger(10, repeatRequest)) {
        toolchain::errs() << "error: expected integer for 'cancel-on-subsequent-request'\n";
        return true;
      } else if (repeatRequest < 1) {
        toolchain::errs() << "error: repeat-request must be >= 1\n";
        return true;
      }
      break;

    case OPT_vfs_files:
      VFSName = VFSName.value_or("in-memory-vfs");
      for (const char *vfsFile : InputArg->getValues()) {
        StringRef name, target;
        std::tie(name, target) = StringRef(vfsFile).split('=');
        toolchain::SmallString<64> nativeName;
        toolchain::sys::path::native(name, nativeName);
        bool passAsSourceText = target.consume_front("@");
        VFSFiles.try_emplace(nativeName.str(), VFSFile(target.str(), passAsSourceText));
      }
      break;

    case OPT_vfs_name:
      VFSName = InputArg->getValue();
      break;

    case OPT_index_store_path:
      IndexStorePath = InputArg->getValue();
      break;

    case OPT_index_unit_output_path:
      IndexUnitOutputPath = InputArg->getValue();
      break;

    case OPT_module_cache_path:
      ModuleCachePath = InputArg->getValue();
      break;

    case OPT_simulate_long_request:
      unsigned SimulatedDuration;
      if (StringRef(InputArg->getValue()).getAsInteger(10, SimulatedDuration)) {
        toolchain::errs() << "error: expected integer for 'simulate-long-request'\n";
        return true;
      }
      SimulateLongRequest = SimulatedDuration;
      break;

    case OPT_shell:
      ShellExecution = true;
      break;

    case OPT_cancel:
      CancelRequest = InputArg->getValue();
      break;

    case OPT_disable_implicit_concurrency_module_import:
      DisableImplicitConcurrencyModuleImport = true;
      break;

    case OPT_disable_implicit_string_processing_module_import:
      DisableImplicitStringProcessingModuleImport = true;
      break;

    case OPT_UNKNOWN:
      toolchain::errs() << "error: unknown argument: "
                   << InputArg->getAsString(ParsedArgs) << '\n'
                   << "Use -h or -help for assistance" << '\n';
      return true;
    }
  }

  if (Request == SourceKitRequest::InterfaceGenOpen && isAsyncRequest) {
    toolchain::errs()
        << "error: cannot use -async with interface-gen-open request\n";
    return true;
  }

  return false;
}

void TestOptions::printHelp(bool ShowHidden) const {

  // Based off of language/lib/Driver/Driver.cpp, at Driver::printHelp
  // FIXME: should we use IncludedFlagsBitmask and ExcludedFlagsBitmask?
  // Maybe not for modes such as Interactive, Batch, AutolinkExtract, etc,
  // as in Driver.cpp. But could be useful for extra info, like HelpHidden.

  TestOptTable Table;

  Table.printHelp(toolchain::outs(), "sourcekitd-test [options] <inputs>",
                  "SourceKit Testing Tool", ShowHidden);
}
