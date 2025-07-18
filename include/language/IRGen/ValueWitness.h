//===--- ValueWitness.h - Enumeration of value witnesses --------*- C++ -*-===//
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
// This file defines the list of witnesses required to attest that a
// type is a value type.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_VALUEWITNESS_H
#define LANGUAGE_IRGEN_VALUEWITNESS_H

namespace language {
namespace irgen {

/// The members required to attest that a type is a value type.
///
/// Logically, there are three basic data operations we must support
/// on arbitrary types:
///   - initializing an object by copying another
///   - changing an object to be a copy of another
///   - destroying an object
///
/// As an optimization to permit efficient transfers of data, the
/// "copy" operations each have an analogous "take" operation which
/// implicitly destroys the source object.
///
/// Therefore there are five basic data operations:
///   initWithCopy(T*, T*)
///   initWithTake(T*, T*)
///   assignWithCopy(T*, T*)
///   assignWithTake(T*, T*)
///   destroy(T*)
///
/// As a further optimization, for every T*, there is a related
/// operation which replaces that T* with a B*, combinatorially.  This
/// makes 18 operations, except that some of these operations are
/// fairly unlikely and so do not merit optimized entries, due to
/// the common code patterns of the two use cases:
///   - Existential code usually doesn't work directly with T*s
///     because pointers into existential objects are not generally
///     reliable.
///   - Generic code works with T*s a fair amount, but it usually
///     doesn't have to deal with B*s after initialization
///     because initialization returns a reliable pointer.
/// This leads us to the following conclusions:
//    - Operations to copy a B* to a T* are very unlikely
///     to be used (-4 operations).
///   - Assignments involving two B*s are only likely in
///     existential code, where we won't have the right
///     typing guarantees to use them (-2 operations).
/// Furthermore, take-initializing a buffer from a buffer is just a
/// memcpy of the buffer (-1), and take-assigning a buffer from a
/// buffer is just a destroy and a memcpy (-1).
///
/// This leaves us with 12 data operations, to which we add the
/// meta-operation 'sizeAndAlign' for a total of 13.
enum class ValueWitness : unsigned {
#define WANT_ALL_VALUE_WITNESSES
#define VALUE_WITNESS(lowerId, upperId) upperId,
#define BEGIN_VALUE_WITNESS_RANGE(rangeId, upperId) First_##rangeId = upperId,
#define END_VALUE_WITNESS_RANGE(rangeId, upperId) Last_##rangeId = upperId,
#include "language/ABI/ValueWitness.def"
};

enum {
  NumRequiredValueWitnesses
    = unsigned(ValueWitness::Last_RequiredValueWitness) + 1,
  NumRequiredValueWitnessFunctions
    = unsigned(ValueWitness::Last_RequiredValueWitnessFunction) + 1,
  
  MaxNumValueWitnesses
    = unsigned(ValueWitness::Last_ValueWitness) + 1,
  MaxNumTypeLayoutWitnesses
    = unsigned(ValueWitness::Last_TypeLayoutWitness)
      - unsigned(ValueWitness::First_TypeLayoutWitness)
      + 1,
};

static inline bool isValueWitnessFunction(ValueWitness witness) {
#define WANT_ALL_VALUE_WITNESSES 1
#define FUNCTION_VALUE_WITNESS(name, Name, ret, args) \
  if (witness == ValueWitness::Name) \
    return true;
#define DATA_VALUE_WITNESS(name, Name, ty)
#include "language/ABI/ValueWitness.def"

  return false;
}

const char *getValueWitnessName(ValueWitness witness);

} // end namespace irgen
} // end namespace language

#endif
