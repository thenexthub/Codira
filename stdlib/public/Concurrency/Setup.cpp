//===--- Setup.cpp - Load-time setup code ---------------------------------===//
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

#include "language/Runtime/Metadata.h"

// Helper macros for figuring out the mangled name of a context descriptor.
#define DESCRIPTOR_MANGLING_SUFFIX_Structure Mn
#define DESCRIPTOR_MANGLING_SUFFIX_Class Mn
#define DESCRIPTOR_MANGLING_SUFFIX_Enum Mn
#define DESCRIPTOR_MANGLING_SUFFIX_Protocol Mp

#define DESCRIPTOR_MANGLING_SUFFIX_(X) X
#define DESCRIPTOR_MANGLING_SUFFIX(KIND)                                       \
  DESCRIPTOR_MANGLING_SUFFIX_(DESCRIPTOR_MANGLING_SUFFIX_##KIND)

#define DESCRIPTOR_MANGLING_(CHAR, SUFFIX) $sSc##CHAR##SUFFIX
#define DESCRIPTOR_MANGLING(CHAR, SUFFIX) DESCRIPTOR_MANGLING_(CHAR, SUFFIX)

// Declare context descriptors for all of the concurrency descriptors with
// standard manglings.
#define STANDARD_TYPE(KIND, MANGLING, TYPENAME)
#define STANDARD_TYPE_CONCURRENCY(KIND, MANGLING, TYPENAME)                    \
  extern "C" const language::ContextDescriptor DESCRIPTOR_MANGLING(               \
      MANGLING, DESCRIPTOR_MANGLING_SUFFIX(KIND));
#include "language/Demangling/StandardTypesMangling.def"

// Defined in Codira, redeclared here so we can register it with the runtime.
extern "C" LANGUAGE_CC(language)
bool _language_task_isCurrentGlobalActor(
    const language::Metadata *, const language::WitnessTable *);

// Register our type descriptors with standard manglings when the concurrency
// runtime is loaded. This allows the runtime to quickly resolve those standard
// manglings.
LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_BEGIN
__attribute__((constructor)) static void setupStandardConcurrencyDescriptors() {
  static const language::ConcurrencyStandardTypeDescriptors descriptors = {
#define STANDARD_TYPE(KIND, MANGLING, TYPENAME)
#define STANDARD_TYPE_CONCURRENCY(KIND, MANGLING, TYPENAME)                    \
  &DESCRIPTOR_MANGLING(MANGLING, DESCRIPTOR_MANGLING_SUFFIX(KIND)),
#include "language/Demangling/StandardTypesMangling.def"
  };
  _language_registerConcurrencyRuntime(
      &descriptors,
      &_language_task_isCurrentGlobalActor);
}
LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_END
