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
  AccessUtilsTest.code
  AddressUtils.code
  BorrowUtils.code
  SpecializationCloner.code
  Devirtualization.code
  EscapeUtils.code
  ForwardingUtils.code
  FunctionSignatureTransforms.code
  GenericSpecialization.code
  LifetimeDependenceUtils.code
  LocalVariableUtils.code
  OptUtils.code
  OwnershipLiveness.code
  PhiUpdater.code
  SSAUpdater.code
  StaticInitCloner.code
  Test.code
  Verifier.code
)
