//===--- CodiraTargetInfo.cpp ----------------------------------------------===//
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
// This file defines the CodiraTargetInfo abstract base class. This class
// provides an interface to target-dependent attributes of interest to Codira.
//
//===----------------------------------------------------------------------===//

#include "CodiraTargetInfo.h"
#include "IRGenModule.h"
#include "toolchain/TargetParser/Triple.h"
#include "toolchain/IR/DataLayout.h"
#include "language/ABI/System.h"
#include "language/AST/ASTContext.h"
#include "language/AST/IRGenOptions.h"
#include "language/Basic/Platform.h"

using namespace language;
using namespace irgen;

/// Initialize a bit vector to be equal to the given bit-mask.
static void setToMask(SpareBitVector &bits, unsigned size, uint64_t mask) {
  bits.clear();
  bits.add(size, mask);
}

/// Configures target-specific information for arm64 platforms.
static void configureARM64(IRGenModule &IGM, const toolchain::Triple &triple,
                           CodiraTargetInfo &target) {
  if (triple.isAndroid()) {
    setToMask(target.PointerSpareBits, 64,
              LANGUAGE_ABI_ANDROID_ARM64_LANGUAGE_SPARE_BITS_MASK);
    setToMask(target.ObjCPointerReservedBits, 64,
              LANGUAGE_ABI_ANDROID_ARM64_OBJC_RESERVED_BITS_MASK);
  } else {
    setToMask(target.PointerSpareBits, 64,
              LANGUAGE_ABI_ARM64_LANGUAGE_SPARE_BITS_MASK);
    setToMask(target.ObjCPointerReservedBits, 64,
              LANGUAGE_ABI_ARM64_OBJC_RESERVED_BITS_MASK);
  }
  setToMask(target.IsObjCPointerBit, 64, LANGUAGE_ABI_ARM64_IS_OBJC_BIT);

  if (triple.isOSDarwin()) {
    target.LeastValidPointerValue =
      LANGUAGE_ABI_DARWIN_ARM64_LEAST_VALID_POINTER;
  }

  // arm64 has no special objc_msgSend variants, not even stret.
  target.ObjCUseStret = false;

  // arm64 requires marker assembly for objc_retainAutoreleasedReturnValue.
  target.ObjCRetainAutoreleasedReturnValueMarker =
    "mov\tfp, fp\t\t// marker for objc_retainAutoreleaseReturnValue";

  // arm64 requires ISA-masking.
  target.ObjCUseISAMask = true;

  // arm64 tops out at 56 effective bits of address space and reserves the high
  // half for the kernel.
  target.CodiraRetainIgnoresNegativeValues = true;

  target.UsableCodiraAsyncContextAddrIntrinsic = true;
}

/// Configures target-specific information for arm64_32 platforms.
static void configureARM64_32(IRGenModule &IGM, const toolchain::Triple &triple,
                              CodiraTargetInfo &target) {
  setToMask(target.PointerSpareBits, 32,
            LANGUAGE_ABI_ARM_LANGUAGE_SPARE_BITS_MASK);

  // arm64_32 has no special objc_msgSend variants, not even stret.
  target.ObjCUseStret = false;

  // arm64_32 requires marker assembly for objc_retainAutoreleasedReturnValue.
  target.ObjCRetainAutoreleasedReturnValueMarker =
    "mov\tfp, fp\t\t// marker for objc_retainAutoreleaseReturnValue";

  setToMask(target.IsObjCPointerBit, 32, LANGUAGE_ABI_ARM_IS_OBJC_BIT);

  target.ObjCHasOpaqueISAs = true;
}

/// Configures target-specific information for x86-64 platforms.
static void configureX86_64(IRGenModule &IGM, const toolchain::Triple &triple,
                            CodiraTargetInfo &target) {
  setToMask(target.PointerSpareBits, 64,
            LANGUAGE_ABI_X86_64_LANGUAGE_SPARE_BITS_MASK);
  setToMask(target.IsObjCPointerBit, 64, LANGUAGE_ABI_X86_64_IS_OBJC_BIT);

  if (triple.isSimulatorEnvironment()) {
    setToMask(target.ObjCPointerReservedBits, 64,
              LANGUAGE_ABI_X86_64_SIMULATOR_OBJC_RESERVED_BITS_MASK);
  } else {
    setToMask(target.ObjCPointerReservedBits, 64,
              LANGUAGE_ABI_X86_64_OBJC_RESERVED_BITS_MASK);
  }

  if (triple.isOSDarwin()) {
    target.LeastValidPointerValue =
      LANGUAGE_ABI_DARWIN_X86_64_LEAST_VALID_POINTER;
  }

  // x86-64 has every objc_msgSend variant known to humankind.
  target.ObjCUseFPRet = true;
  target.ObjCUseFP2Ret = true;

  // x86-64 requires ISA-masking.
  target.ObjCUseISAMask = true;
  
  // x86-64 only has 48 effective bits of address space and reserves the high
  // half for the kernel.
  target.CodiraRetainIgnoresNegativeValues = true;

  target.UsableCodiraAsyncContextAddrIntrinsic = true;
}

/// Configures target-specific information for 32-bit x86 platforms.
static void configureX86(IRGenModule &IGM, const toolchain::Triple &triple,
                         CodiraTargetInfo &target) {
  setToMask(target.PointerSpareBits, 32,
            LANGUAGE_ABI_I386_LANGUAGE_SPARE_BITS_MASK);

  // x86 uses objc_msgSend_fpret but not objc_msgSend_fp2ret.
  target.ObjCUseFPRet = true;
  setToMask(target.IsObjCPointerBit, 32, LANGUAGE_ABI_I386_IS_OBJC_BIT);
}

/// Configures target-specific information for 32-bit arm platforms.
static void configureARM(IRGenModule &IGM, const toolchain::Triple &triple,
                         CodiraTargetInfo &target) {
  setToMask(target.PointerSpareBits, 32,
            LANGUAGE_ABI_ARM_LANGUAGE_SPARE_BITS_MASK);

  // ARM requires marker assembly for objc_retainAutoreleasedReturnValue.
  target.ObjCRetainAutoreleasedReturnValueMarker =
    "mov\tr7, r7\t\t// marker for objc_retainAutoreleaseReturnValue";

  // armv7k has opaque ISAs which must go through the ObjC runtime.
  if (triple.getSubArch() == toolchain::Triple::SubArchType::ARMSubArch_v7k)
    target.ObjCHasOpaqueISAs = true;

  setToMask(target.IsObjCPointerBit, 32, LANGUAGE_ABI_ARM_IS_OBJC_BIT);
}

/// Configures target-specific information for powerpc platforms.
static void configurePowerPC(IRGenModule &IGM, const toolchain::Triple &triple,
                               CodiraTargetInfo &target) {
  setToMask(target.PointerSpareBits, 32,
            LANGUAGE_ABI_POWERPC_LANGUAGE_SPARE_BITS_MASK);
}

/// Configures target-specific information for powerpc64 platforms.
static void configurePowerPC64(IRGenModule &IGM, const toolchain::Triple &triple,
                               CodiraTargetInfo &target) {
  setToMask(target.PointerSpareBits, 64,
            LANGUAGE_ABI_POWERPC64_LANGUAGE_SPARE_BITS_MASK);
}

/// Configures target-specific information for SystemZ platforms.
static void configureSystemZ(IRGenModule &IGM, const toolchain::Triple &triple,
                             CodiraTargetInfo &target) {
  setToMask(target.PointerSpareBits, 64,
            LANGUAGE_ABI_S390X_LANGUAGE_SPARE_BITS_MASK);
  setToMask(target.ObjCPointerReservedBits, 64,
            LANGUAGE_ABI_S390X_OBJC_RESERVED_BITS_MASK);
  setToMask(target.IsObjCPointerBit, 64, LANGUAGE_ABI_S390X_IS_OBJC_BIT);
  target.CodiraRetainIgnoresNegativeValues = true;
}

/// Configures target-specific information for wasm32 platforms.
static void configureWasm32(IRGenModule &IGM, const toolchain::Triple &triple,
                            CodiraTargetInfo &target) {
  target.LeastValidPointerValue =
    LANGUAGE_ABI_WASM32_LEAST_VALID_POINTER;
}

/// Configure a default target.
CodiraTargetInfo::CodiraTargetInfo(
  toolchain::Triple::ObjectFormatType outputObjectFormat,
  unsigned numPointerBits)
  : OutputObjectFormat(outputObjectFormat),
    HeapObjectAlignment(numPointerBits / 8),
    LeastValidPointerValue(LANGUAGE_ABI_DEFAULT_LEAST_VALID_POINTER)
{
  setToMask(PointerSpareBits, numPointerBits,
            LANGUAGE_ABI_DEFAULT_LANGUAGE_SPARE_BITS_MASK);
  setToMask(ObjCPointerReservedBits, numPointerBits,
            LANGUAGE_ABI_DEFAULT_OBJC_RESERVED_BITS_MASK);
  setToMask(FunctionPointerSpareBits, numPointerBits,
            LANGUAGE_ABI_DEFAULT_FUNCTION_SPARE_BITS_MASK);
  if (numPointerBits == 64) {
    ReferencePoisonDebugValue =
      LANGUAGE_ABI_DEFAULT_REFERENCE_POISON_DEBUG_VALUE_64;
  } else {
    ReferencePoisonDebugValue =
      LANGUAGE_ABI_DEFAULT_REFERENCE_POISON_DEBUG_VALUE_32;
  }
}

CodiraTargetInfo CodiraTargetInfo::get(IRGenModule &IGM) {
  const toolchain::Triple &triple = IGM.Context.LangOpts.Target;
  auto pointerSize = IGM.DataLayout.getPointerSizeInBits();

  // Prepare generic target information.
  CodiraTargetInfo target(triple.getObjectFormat(), pointerSize);
  
  // On Apple platforms, we implement "once" using dispatch_once,
  // which exposes a barrier-free inline path with -1 as the "done" value.
  if (triple.isOSDarwin())
    target.OnceDonePredicateValue = -1L;
  // Other platforms use std::call_once() and we don't
  // assume that they have a barrier-free inline fast path.
  
  switch (triple.getArch()) {
  case toolchain::Triple::x86_64:
    configureX86_64(IGM, triple, target);
    break;

  case toolchain::Triple::x86:
    configureX86(IGM, triple, target);
    break;

  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
    configureARM(IGM, triple, target);
    break;

  case toolchain::Triple::aarch64:
  case toolchain::Triple::aarch64_32:
    if (triple.getArchName() == "arm64_32")
      configureARM64_32(IGM, triple, target);
    else
      configureARM64(IGM, triple, target);
    break;
  
  case toolchain::Triple::ppc:
    configurePowerPC(IGM, triple, target);
    break;

  case toolchain::Triple::ppc64:
  case toolchain::Triple::ppc64le:
    configurePowerPC64(IGM, triple, target);
    break;

  case toolchain::Triple::systemz:
    configureSystemZ(IGM, triple, target);
    break;
  case toolchain::Triple::wasm32:
    configureWasm32(IGM, triple, target);
    break;

  default:
    // FIXME: Complain here? Default target info is unlikely to be correct.
    break;
  }

  if (IGM.getOptions().CustomLeastValidPointerValue != 0)
    target.LeastValidPointerValue = IGM.getOptions().CustomLeastValidPointerValue;

  return target;
}

bool CodiraTargetInfo::hasObjCTaggedPointers() const {
  return ObjCPointerReservedBits.any();
}
