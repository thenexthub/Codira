//===--- TypeLayoutVerifier.cpp -------------------------------------------===//
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
// This file defines a generator that produces code to verify that IRGen's
// static assumptions about data layout for a Codira type correspond to the
// runtime's understanding of data layout.
//
//===----------------------------------------------------------------------===//

#include "toolchain/IR/Function.h"
#include "toolchain/IR/Module.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticsIRGen.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILModule.h"
#include "EnumPayload.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "GenOpaque.h"
#include "GenType.h"
#include "FixedTypeInfo.h"

using namespace language;
using namespace irgen;

IRGenTypeVerifierFunction::IRGenTypeVerifierFunction(IRGenModule &IGM,
                                                     toolchain::Function *f)
    : IRGenFunction(IGM, f),
      VerifierFn(IGM.getVerifyTypeLayoutAttributeFunctionPointer()) {
  // Verifier functions are always artificial.
  if (IGM.DebugInfo)
    IGM.DebugInfo->emitArtificialFunction(*this, f);
}

void
IRGenTypeVerifierFunction::emit(ArrayRef<CanType> formalTypes) {
  auto getSizeConstant = [&](Size sz) -> toolchain::Constant * {
    return toolchain::ConstantInt::get(IGM.SizeTy, sz.getValue());
  };
  auto getAlignmentMaskConstant = [&](Alignment a) -> toolchain::Constant * {
    return toolchain::ConstantInt::get(IGM.SizeTy, a.getValue() - 1);
  };
  auto getBoolConstant = [&](bool b) -> toolchain::Constant * {
    return toolchain::ConstantInt::get(IGM.Int1Ty, b);
  };

  SmallString<20> numberBuf;

  for (auto formalType : formalTypes) {
    // Runtime type metadata always represents the maximal abstraction level of
    // the type.
    auto maxAbstraction = AbstractionPattern::getOpaque();
    auto layoutType = IGM.getLoweredType(maxAbstraction, formalType);
    auto &ti = getTypeInfo(layoutType);
    auto metadata = emitTypeMetadataRef(formalType);

    // Check type metrics for fixed-layout types.
    // If there's no fixed type info, we rely on the runtime for type metrics,
    // so there's no compile-time values to validate against.
    if (auto *fixedTI = dyn_cast<FixedTypeInfo>(&ti)) {
      // Check that the fixed layout matches the runtime layout.
      verifyValues(metadata,
             emitLoadOfSize(*this, layoutType),
             getSizeConstant(fixedTI->getFixedSize()),
             "size");
      verifyValues(metadata,
             emitLoadOfAlignmentMask(*this, layoutType),
             getAlignmentMaskConstant(fixedTI->getFixedAlignment()),
             "alignment mask");
      verifyValues(metadata,
             emitLoadOfStride(*this, layoutType),
             getSizeConstant(fixedTI->getFixedStride()),
             "stride");
      verifyValues(metadata,
             emitLoadOfIsInline(*this, layoutType),
             getBoolConstant(fixedTI->getFixedPacking(IGM)
                               == FixedPacking::OffsetZero),
             "is-inline bit");
      verifyValues(metadata,
             emitLoadOfIsTriviallyDestroyable(*this, layoutType),
             getBoolConstant(fixedTI->isTriviallyDestroyable(ResilienceExpansion::Maximal)),
             "is-trivially-destructible bit");
      verifyValues(metadata,
             emitLoadOfIsBitwiseTakable(*this, layoutType),
             getBoolConstant(fixedTI->isBitwiseTakable(ResilienceExpansion::Maximal)),
             "is-bitwise-takable bit");
      unsigned xiCount = fixedTI->getFixedExtraInhabitantCount(IGM);
      verifyValues(metadata,
             emitLoadOfExtraInhabitantCount(*this, layoutType),
             IGM.getInt32(xiCount),
             "extra inhabitant count");

      // Check extra inhabitants.
      if (xiCount > 0) {
        // Verify that the extra inhabitant representations are consistent.
        
        // TODO: Update for EnumPayload implementation changes.
        auto xiBuf = createAlloca(fixedTI->getStorageType(),
                                      fixedTI->getFixedAlignment(),
                                      "extra-inhabitant");
        auto fixedXIBuf = createAlloca(fixedTI->getStorageType(),
                                           fixedTI->getFixedAlignment(),
                                           "extra-inhabitant");
        auto xiOpaque = Builder.CreateElementBitCast(xiBuf, IGM.OpaqueTy);
        auto fixedXIOpaque =
            Builder.CreateElementBitCast(fixedXIBuf, IGM.OpaqueTy);
        auto xiMask = fixedTI->getFixedExtraInhabitantMask(IGM);
        auto xiSchema = EnumPayloadSchema(xiMask.getBitWidth());

        auto maxXiCount = std::min(xiCount, 256u);
        auto numCases = toolchain::ConstantInt::get(IGM.Int32Ty, maxXiCount);

        // TODO: Randomize the set of extra inhabitants we check.
        unsigned bits = fixedTI->getFixedSize().getValueInBits();
        for (unsigned i = 0, e = maxXiCount; i < e; ++i) {
          // Initialize the buffer with junk, to help ensure we're insensitive to
          // insignificant bits.
          // TODO: Randomize the filler.
          Builder.CreateMemSet(xiBuf.getAddress(),
                                   toolchain::ConstantInt::get(IGM.Int8Ty, 0x5A),
                                   fixedTI->getFixedSize().getValue(),
                                   toolchain::MaybeAlign(fixedTI->getFixedAlignment().getValue()));
          
          // Ask the runtime to store an extra inhabitant.
          auto tag = toolchain::ConstantInt::get(IGM.Int32Ty, i+1);
          emitStoreEnumTagSinglePayloadCall(*this, layoutType, tag,
                                            numCases, xiOpaque);
          
          // Compare the stored extra inhabitant against the fixed extra
          // inhabitant pattern.
          auto fixedXIValue
             = fixedTI->getFixedExtraInhabitantValue(IGM, bits, i);
          auto fixedXIPayload =
            EnumPayload::fromBitPattern(IGM, fixedXIValue,
                                        xiSchema);
          fixedXIPayload.store(*this, fixedXIBuf);
          
          auto runtimeXIPayload = EnumPayload::load(*this, xiBuf, xiSchema);
          runtimeXIPayload.emitApplyAndMask(*this, xiMask);
          runtimeXIPayload.store(*this, xiBuf);

          numberBuf.clear();
          {
            toolchain::raw_svector_ostream os(numberBuf);
            os << i;
          }
          
          verifyBuffers(metadata, xiBuf, fixedXIBuf, fixedTI->getFixedSize(),
                 toolchain::Twine("stored extra inhabitant ") + numberBuf.str());
          
          // Now ask the runtime to identify the fixed extra inhabitant value.
          // Mask in junk to make sure the runtime correctly ignores it.
          // TODO: Randomize the filler.
          auto xiFill = ~APInt(fixedXIValue.getBitWidth(), 0);
          xiFill &= ~xiMask;
          fixedXIValue |= xiFill;

          auto maskedXIPayload = EnumPayload::fromBitPattern(IGM,
            fixedXIValue, xiSchema);
          maskedXIPayload.store(*this, fixedXIBuf);
          
          auto runtimeTag =
            emitGetEnumTagSinglePayloadCall(*this, layoutType, numCases,
                                            fixedXIOpaque);
          verifyValues(metadata,
                       runtimeTag, tag,
                       toolchain::Twine("extra inhabitant tag calculation ")
                         + numberBuf.str());
        }
      }
    }
    ti.verify(*this, metadata, layoutType);
  }

  Builder.CreateRetVoid();
}

void
IRGenTypeVerifierFunction::verifyValues(toolchain::Value *typeMetadata,
                                        toolchain::Value *runtimeVal,
                                        toolchain::Value *staticVal,
                                        const toolchain::Twine &description) {
  assert(runtimeVal->getType() == staticVal->getType());
  // Get or create buffers for the arguments.
  VerifierArgumentBuffers bufs;
  auto foundBufs = VerifierArgBufs.find(runtimeVal->getType());
  if (foundBufs != VerifierArgBufs.end()) {
    bufs = foundBufs->second;
  } else {
    Address runtimeBuf = createAlloca(runtimeVal->getType(),
                                          IGM.getPointerAlignment(),
                                          "runtime");
    Address staticBuf = createAlloca(staticVal->getType(),
                                         IGM.getPointerAlignment(),
                                         "static");
    bufs = {runtimeBuf, staticBuf};
    VerifierArgBufs[runtimeVal->getType()] = bufs;
  }
  
  Builder.CreateStore(runtimeVal, bufs.runtimeBuf);
  Builder.CreateStore(staticVal, bufs.staticBuf);
  
  auto runtimePtr = Builder.CreateBitCast(bufs.runtimeBuf.getAddress(),
                                              IGM.Int8PtrTy);
  auto staticPtr = Builder.CreateBitCast(bufs.staticBuf.getAddress(),
                                             IGM.Int8PtrTy);
  auto count = toolchain::ConstantInt::get(IGM.SizeTy,
                IGM.DataLayout.getTypeStoreSize(runtimeVal->getType()));
  auto msg
    = IGM.getAddrOfGlobalString(description.str());
  
  Builder.CreateCall(
      VerifierFn, {typeMetadata, runtimePtr, staticPtr, count, msg});
}

void
IRGenTypeVerifierFunction::verifyBuffers(toolchain::Value *typeMetadata,
                                         Address runtimeBuf,
                                         Address staticBuf,
                                         Size size,
                                         const toolchain::Twine &description) {
  auto runtimePtr = Builder.CreateBitCast(runtimeBuf.getAddress(),
                                          IGM.Int8PtrTy);
  auto staticPtr = Builder.CreateBitCast(staticBuf.getAddress(),
                                         IGM.Int8PtrTy);
  auto count = toolchain::ConstantInt::get(IGM.SizeTy,
                                      size.getValue());
  auto msg
    = IGM.getAddrOfGlobalString(description.str());

  Builder.CreateCall(
      VerifierFn, {typeMetadata, runtimePtr, staticPtr, count, msg});
}

void IRGenModule::emitTypeVerifier() {
  // Look up the types to verify.
  
  SmallVector<CanType, 4> TypesToVerify;
  for (auto name : IRGen.Opts.VerifyTypeLayoutNames) {
    // Look up the name in the module.
    SmallVector<ValueDecl*, 1> lookup;
    language::ModuleDecl *M = getCodiraModule();
    M->lookupMember(lookup, M, DeclName(Context.getIdentifier(name)),
                    Identifier());
    if (lookup.empty()) {
      Context.Diags.diagnose(SourceLoc(), diag::type_to_verify_not_found,
                             name);
      continue;
    }
    
    TypeDecl *typeDecl = nullptr;
    for (auto decl : lookup) {
      if (auto td = dyn_cast<TypeDecl>(decl)) {
        if (typeDecl) {
          Context.Diags.diagnose(SourceLoc(), diag::type_to_verify_ambiguous,
                                 name);
          goto next;
        }
        typeDecl = td;
        break;
      }
    }
    if (!typeDecl) {
      Context.Diags.diagnose(SourceLoc(), diag::type_to_verify_not_found, name);
      continue;
    }
    
    {
      auto type = typeDecl->getDeclaredInterfaceType();
      if (type->hasTypeParameter()) {
        Context.Diags.diagnose(SourceLoc(), diag::type_to_verify_dependent,
                               name);
        continue;
      }
      
      TypesToVerify.push_back(type->getCanonicalType());
    }
  next:;
  }
  if (TypesToVerify.empty())
    return;

  // Find the entry point.
  SILFunction *EntryPoint = getSILModule().lookUpFunction(
      getSILModule().getASTContext().getEntryPointFunctionName());

  if (!EntryPoint)
    return;
  
  toolchain::Function *EntryFunction = Module.getFunction(EntryPoint->getName());
  if (!EntryFunction)
    return;
  
  // Create a new function to contain our logic.
  auto fnTy = toolchain::FunctionType::get(VoidTy, /*varArg*/ false);
  auto VerifierFunction = toolchain::Function::Create(fnTy,
                                             toolchain::GlobalValue::PrivateLinkage,
                                             "type_verifier",
                                             getModule());
  VerifierFunction->setAttributes(constructInitialAttributes());
  
  // Insert a call into the entry function.
  {
    toolchain::BasicBlock *EntryBB = &EntryFunction->getEntryBlock();
    toolchain::BasicBlock::iterator IP = EntryBB->getFirstInsertionPt();
    IRBuilder Builder(getLLVMContext(), DebugInfo != nullptr);
    Builder.toolchain::IRBuilderBase::SetInsertPoint(EntryBB, IP);
    if (DebugInfo)
      DebugInfo->setEntryPointLoc(Builder);
    Builder.CreateCall(fnTy, VerifierFunction, {});
  }

  IRGenTypeVerifierFunction VerifierIGF(*this, VerifierFunction);
  VerifierIGF.emit(TypesToVerify);
}
