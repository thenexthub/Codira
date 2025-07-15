//===--- OptimizationRemark.cpp - Optimization diagnostics ------*- C++ -*-===//
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
/// \file
/// This file defines the remark type and the emitter class that passes can use
/// to emit optimization diagnostics.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-opt-remarks"

#include "language/SIL/OptimizationRemark.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsSIL.h"
#include "language/AST/SemanticAttrs.h"
#include "language/Demangling/Demangler.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/InstructionUtils.h"
#include "language/SIL/MemAccessUtils.h"
#include "language/SIL/Projection.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILRemarkStreamer.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;
using namespace OptRemark;

Argument::Argument(StringRef key, int n)
    : key(ArgumentKeyKind::Default, key), val(toolchain::itostr(n)) {}

Argument::Argument(StringRef key, long n)
    : key(ArgumentKeyKind::Default, key), val(toolchain::itostr(n)) {}

Argument::Argument(StringRef key, long long n)
    : key(ArgumentKeyKind::Default, key), val(toolchain::itostr(n)) {}

Argument::Argument(StringRef key, unsigned n)
    : key(ArgumentKeyKind::Default, key), val(toolchain::utostr(n)) {}

Argument::Argument(StringRef key, unsigned long n)
    : key(ArgumentKeyKind::Default, key), val(toolchain::utostr(n)) {}

Argument::Argument(StringRef key, unsigned long long n)
    : key(ArgumentKeyKind::Default, key), val(toolchain::utostr(n)) {}

Argument::Argument(ArgumentKey key, SILFunction *f) : key(key) {
  auto options = Demangle::DemangleOptions::SimplifiedUIDemangleOptions();
  // Enable module names so that we have a way of filtering out
  // stdlib-related remarks.
  options.DisplayModuleNames = true;

  val = (Twine("\"") + Demangle::demangleSymbolAsString(f->getName(), options) +
         "\"")
            .str();

  if (f->hasLocation())
    loc = f->getLocation().getSourceLoc();
}

Argument::Argument(StringRef key, SILType ty)
    : key(ArgumentKeyKind::Default, key) {
  toolchain::raw_string_ostream stream(val);
  PrintOptions subPrinter = PrintOptions::printSIL();
  ty.getASTType().print(stream, subPrinter);
}

Argument::Argument(StringRef key, CanType ty)
    : key(ArgumentKeyKind::Default, key) {
  toolchain::raw_string_ostream stream(val);
  ty.print(stream);
}

template <typename DerivedT>
std::string Remark<DerivedT>::getMsg() const {
  std::string str;
  toolchain::raw_string_ostream stream(str);
  // Go through our args and if we are not emitting for diagnostics *OR* we are
  // emitting for diagnostics and this argument is not intended to be emitted as
  // a diagnostic separate from our main remark, emit the arg value here.
  for (const Argument &arg : args) {
    if (arg.key.kind.isSeparateDiagnostic())
      continue;
    stream << arg.val;
  }

  return stream.str();
}

template <typename DerivedT>
std::string Remark<DerivedT>::getDebugMsg() const {
  std::string str;
  toolchain::raw_string_ostream stream(str);

  if (indentDebugWidth)
    stream << std::string(" ", indentDebugWidth);

  for (const Argument &arg : args)
    stream << arg.val;

  stream << "\n";
  return stream.str();
}

static bool hasForceEmitSemanticAttr(SILFunction &fn, StringRef passName) {
  return toolchain::any_of(fn.getSemanticsAttrs(), [&](const std::string &str) {
    auto ref = StringRef(str);

    // First try to chomp the prefix.
    if (!ref.consume_front(semantics::FORCE_EMIT_OPT_REMARK_PREFIX))
      return false;

    // Then see if we only have the prefix. Then always return true the user
    // wants /all/ remarks.
    if (ref.empty())
      return true;

    // Otherwise, lets try to chomp the '.' and then the passName.
    if (!ref.consume_front(".") || !ref.consume_front(passName))
      return false;

    return ref.empty();
  });
}

static bool isMethodWithForceEmitSemanticAttrNominalType(SILFunction &fn) {
  if (!fn.hasSelfParam() || fn.getArguments().empty())
    return false;

  auto selfType = fn.getSelfArgument()->getType();
  auto *nomType = selfType.getNominalOrBoundGenericNominal();
  if (!nomType)
    return false;
  return nomType->shouldEmitAssemblyVisionRemarksOnMethods();
}

Emitter::Emitter(StringRef passName, SILFunction &fn)
    : fn(fn), passName(passName),
      passedEnabled(
          hasForceEmitSemanticAttr(fn, passName) ||
          isMethodWithForceEmitSemanticAttrNominalType(fn) ||
          (fn.getASTContext().LangOpts.OptimizationRemarkPassedPattern &&
           fn.getASTContext().LangOpts.OptimizationRemarkPassedPattern->match(
               passName))),
      missedEnabled(
          hasForceEmitSemanticAttr(fn, passName) ||
          isMethodWithForceEmitSemanticAttrNominalType(fn) ||
          (fn.getASTContext().LangOpts.OptimizationRemarkMissedPattern &&
           fn.getASTContext().LangOpts.OptimizationRemarkMissedPattern->match(
               passName))) {}

static SourceLoc getLocForPresentation(SILLocation loc,
                                       SourceLocPresentationKind kind) {
  if (!loc)
    return SourceLoc();
  switch (kind) {
  case SourceLocPresentationKind::StartRange:
    return loc.getSourceLoc();
  case SourceLocPresentationKind::EndRange:
    return loc.getEndSourceLoc();
  }
  toolchain_unreachable("covered switch");
}

static bool instHasInferrableLoc(SILInstruction &inst) {
  if (isa<DeallocStackInst>(inst) || isa<StrongRetainInst>(inst) ||
      isa<StrongReleaseInst>(inst) || isa<RetainValueInst>(inst) ||
      isa<ReleaseValueInst>(inst) || isa<EndAccessInst>(inst))
    return false;

  // Ignore empty tuples
  if (auto *tup = dyn_cast<TupleInst>(&inst))
    return tup->getNumOperands() != 0;

  return true;
}

/// The user has passed us an instruction that for some reason has a source loc
/// that can not be used. Search down the current block for an instruction with
/// a valid source loc and use that instead.
static SourceLoc
inferOptRemarkSearchForwards(SILInstruction &i,
                             SourceLocPresentationKind presentationKind) {
  for (auto &inst :
       toolchain::make_range(std::next(i.getIterator()), i.getParent()->end())) {
    if (!instHasInferrableLoc(inst))
      continue;
    // Skip instructions without a loc we care about since we move it around.
    auto newLoc = getLocForPresentation(inst.getLoc(), presentationKind);
    if (auto inlinedLoc = inst.getDebugScope()->getOutermostInlineLocation())
      newLoc = getLocForPresentation(inlinedLoc, presentationKind);
    if (newLoc.isValid()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Inferring loc from " << inst);
      return newLoc;
    }
  }

  return SourceLoc();
}

/// The user has passed us an instruction that for some reason has a source loc
/// that can not be used. Search up the current block for an instruction with
/// a valid SILLocation and use the end SourceLoc of the SourceRange for the
/// instruction.
static SourceLoc
inferOptRemarkSearchBackwards(SILInstruction &i,
                              SourceLocPresentationKind presentationKind) {
  for (auto &inst : toolchain::make_range(std::next(i.getReverseIterator()),
                                     i.getParent()->rend())) {
    if (!instHasInferrableLoc(inst))
      continue;
    auto loc = inst.getLoc();
    if (auto inlinedLoc = inst.getDebugScope()->getOutermostInlineLocation())
      loc = inlinedLoc;
    if (auto result = getLocForPresentation(loc, presentationKind)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Inferring loc from " << inst);
      return result;
    }
  }

  return SourceLoc();
}

static toolchain::cl::opt<bool> IgnoreAlwaysInferForTesting(
    "sil-opt-remark-ignore-always-infer", toolchain::cl::Hidden,
    toolchain::cl::init(false),
    toolchain::cl::desc(
        "Disables always infer source loc behavior for testing purposes"));

// Attempt to infer a SourceLoc for \p i using heuristics specified by \p
// inferBehavior.
//
// NOTE: We distinguish in between situations where we always must infer
// (retain, release) and other situations where we are ok with original source
// locs if we are not inlined (alloc_ref, alloc_stack).
SourceLoc language::OptRemark::inferOptRemarkSourceLoc(
    SILInstruction &i, SourceLocInferenceBehavior inferBehavior,
    SourceLocPresentationKind presentationKind) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Begin infer source loc for: " << i);
  // If we are only supposed to infer in inline contexts, see if we have a valid
  // loc and if that loc is an inlined call site.
  auto loc = i.getLoc();
  if (!(bool(inferBehavior & SourceLocInferenceBehavior::AlwaysInfer) &&
        !IgnoreAlwaysInferForTesting)) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Testing insts own source loc?!\n");
    if (loc.getSourceLoc().isValid()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found initial valid loc!\n");
      if (!(i.getDebugScope() && i.getDebugScope()->InlinedCallSite)) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found debug scope!\n");
        return getLocForPresentation(loc, presentationKind);
      } else {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "Did not find debug scope!\n");
      }
    } else {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Failed to find initial valid loc!\n");
    }
  }

  if (bool(inferBehavior & SourceLocInferenceBehavior::ForwardScan)) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Inferring Source Loc Forward!\n");
    SourceLoc newLoc = inferOptRemarkSearchForwards(i, presentationKind);
    if (newLoc.isValid()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found loc!\n");
      return newLoc;
    }
  }

  if (bool(inferBehavior & SourceLocInferenceBehavior::BackwardScan)) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Inferring Source Loc Backwards!\n");
    SourceLoc newLoc = inferOptRemarkSearchBackwards(i, presentationKind);
    if (newLoc.isValid()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found loc!\n");
      return newLoc;
    }
  }

  if (bool(inferBehavior & SourceLocInferenceBehavior::ForwardScan2nd)) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Inferring Source Loc Forward Scan 2nd!\n");
    SourceLoc newLoc = inferOptRemarkSearchForwards(i, presentationKind);
    if (newLoc.isValid()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found loc!\n");
      return newLoc;
    }
  }

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Failed to find good loc!\n");
  return SourceLoc();
}

template <typename RemarkT, typename... ArgTypes>
static void emitRemark(SILFunction &fn, const Remark<RemarkT> &remark,
                       Diag<ArgTypes...> id, bool diagEnabled) {
  if (remark.getLocation().isInvalid())
    return;

  auto &module = fn.getModule();
  if (auto *remarkStreamer = module.getSILRemarkStreamer())
    remarkStreamer->emit(remark);

  // If diagnostics are enabled, first emit the main diagnostic and then loop
  // through our arguments and allow the arguments to add additional diagnostics
  // if they want.
  if (!diagEnabled && !fn.hasSemanticsAttrThatStartsWith(
                          semantics::FORCE_EMIT_OPT_REMARK_PREFIX))
    return;

  auto &de = module.getASTContext().Diags;
  de.diagnoseWithNotes(
      de.diagnose(remark.getLocation(), id, remark.getMsg()), [&]() {
        for (auto &arg : remark.getArgs()) {
          switch (arg.key.kind) {
          case ArgumentKeyKind::Default:
            continue;
          case ArgumentKeyKind::Note:
            de.diagnose(arg.loc, diag::opt_remark_note, arg.val);
            continue;
          case ArgumentKeyKind::ParentLocNote:
            de.diagnose(remark.getLocation(), diag::opt_remark_note, arg.val);
            continue;
          }
          toolchain_unreachable("Unhandled case?!");
        }
      });
}

void Emitter::emit(const RemarkPassed &remark) {
  emitRemark(fn, remark, diag::opt_remark_passed, isEnabled<RemarkPassed>());
}

void Emitter::emit(const RemarkMissed &remark) {
  emitRemark(fn, remark, diag::opt_remark_missed, isEnabled<RemarkMissed>());
}

void Emitter::emitDebug(const RemarkPassed &remark) {
  toolchain::dbgs() << remark.getDebugMsg();
}

void Emitter::emitDebug(const RemarkMissed &remark) {
  toolchain::dbgs() << remark.getDebugMsg();
}
