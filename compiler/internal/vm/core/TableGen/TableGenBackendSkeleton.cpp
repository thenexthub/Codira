//===- TableGenBackendSkeleton.cpp - Skeleton TableGen backend --*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This Tablegen backend emits ...
//
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/StringRef.h"
#include "vm/core/TableGen/TableGenBackend.h"

#define DEBUG_TYPE "skeleton-emitter"

namespace vm::core {
class RecordKeeper;
class raw_ostream;
} // namespace vm::core

using namespace vm::core;

namespace {

// Any helper data structures can be defined here. Some backends use
// structs to collect information from the records.

class SkeletonEmitter {
private:
  const RecordKeeper &Records;

public:
  SkeletonEmitter(const RecordKeeper &RK) : Records(RK) {}

  void run(raw_ostream &OS);
}; // emitter class

} // anonymous namespace

void SkeletonEmitter::run(raw_ostream &OS) {
  emitSourceFileHeader("Skeleton data structures", OS);

  (void)Records; // To suppress unused variable warning; remove on use.
}

// Choose either option A or B.

//===----------------------------------------------------------------------===//
// Option A: Register the backed as class <SkeletonEmitter>
static TableGen::Emitter::OptClass<SkeletonEmitter>
    X("gen-skeleton-class", "Generate example skeleton class");

//===----------------------------------------------------------------------===//
// Option B: Register "EmitSkeleton" directly
// The emitter entry may be private scope.
static void EmitSkeleton(const RecordKeeper &RK, raw_ostream &OS) {
  // Instantiate the emitter class and invoke run().
  SkeletonEmitter(RK).run(OS);
}

static TableGen::Emitter::Opt Y("gen-skeleton-entry", EmitSkeleton,
                                "Generate example skeleton entry");
