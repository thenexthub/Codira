# Copyright (c) NeXTHub Corporation. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# Author(-s): Tunjay Akbarli


language_compiler_sources(Optimizer
  SimplifyAllocRefDynamic.code
  SimplifyAllocStack.code
  SimplifyApply.code
  SimplifyBeginAndLoadBorrow.code
  SimplifyBeginCOWMutation.code
  SimplifyBranch.code
  SimplifyBuiltin.code
  SimplifyCheckedCast.code
  SimplifyClassifyBridgeObject.code
  SimplifyCondBranch.code
  SimplifyCondFail.code
  SimplifyConvertEscapeToNoEscape.code
  SimplifyCopyBlock.code
  SimplifyCopyValue.code
  SimplifyDebugStep.code
  SimplifyDestroyValue.code
  SimplifyDestructure.code
  SimplifyFixLifetime.code
  SimplifyGlobalValue.code
  SimplifyInitEnumDataAddr.code
  SimplifyKeyPath.code
  SimplifyLoad.code
  SimplifyMarkDependence.code
  SimplifyMisc.code
  SimplifyPartialApply.code
  SimplifyPointerToAddress.code
  SimplifyRefCasts.code
  SimplifyRetainReleaseValue.code
  SimplifyStrongRetainRelease.code
  SimplifyStructExtract.code
  SimplifySwitchEnum.code
  SimplifyTuple.code
  SimplifyTupleExtract.code
  SimplifyUncheckedAddrCast.code
  SimplifyUncheckedEnumData.code
  SimplifyValueToBridgeObject.code
  SimplifyWitnessMethod.code)
