//===-- EntryPointArgumentEmission.h - Emit function entries. -------------===//
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

#pragma once

namespace toolchain {
class Value;
}

namespace language {

class GenericRequirement;
class SILArgument;

namespace irgen {

class Explosion;
class LoadableTypeInfo;
class TypeInfo;

class EntryPointArgumentEmission {

public:
  virtual ~EntryPointArgumentEmission() {}
  virtual bool requiresIndirectResult(SILType retType) = 0;
  virtual toolchain::Value *getIndirectResultForFormallyDirectResult() = 0;
  virtual toolchain::Value *getIndirectResult(unsigned index) = 0;
  virtual toolchain::Value *getNextPolymorphicParameterAsMetadata() = 0;
  virtual toolchain::Value *
  getNextPolymorphicParameter(GenericRequirement &requirement) = 0;
};

class NativeCCEntryPointArgumentEmission
    : public virtual EntryPointArgumentEmission {

public:
  virtual void mapAsyncParameters() = 0;
  virtual toolchain::Value *getCallerErrorResultArgument() = 0;
  virtual toolchain::Value *getCallerTypedErrorResultArgument() = 0;
  virtual toolchain::Value *getContext() = 0;
  virtual Explosion getArgumentExplosion(unsigned index, unsigned size) = 0;
  virtual toolchain::Value *getSelfWitnessTable() = 0;
  virtual toolchain::Value *getSelfMetadata() = 0;
  virtual toolchain::Value *getCoroutineBuffer() = 0;
  virtual toolchain::Value *getCoroutineAllocator() = 0;
  Explosion
  explosionForObject(IRGenFunction &IGF, unsigned index, SILArgument *param,
                     SILType paramTy, const LoadableTypeInfo &loadableParamTI,
                     const LoadableTypeInfo &loadableArgTI,
                     std::function<Explosion(unsigned index, unsigned size)>
                         explosionForArgument);
};

} // end namespace irgen
} // end namespace language
