//===--- TypeID.h - Simple Type Identification ------------------*- C++ -*-===//
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
//  This file defines the TypeID template, which provides a numeric
//  encoding of (static) type information for use as a simple replacement
//  for run-time type information.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_TYPEID_H
#define LANGUAGE_BASIC_TYPEID_H

// NOTE: Most of these includes are for CTypeIDZone.def and DefineTypeIDZone.h.
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/TinyPtrVector.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace language {

enum class Zone : uint8_t {
#define LANGUAGE_TYPEID_ZONE(Name, Id) Name = Id,
#include "language/Basic/TypeIDZones.def"
#undef LANGUAGE_TYPEID_ZONE
};

static_assert(std::is_same<std::underlying_type<Zone>::type, uint8_t>::value,
              "underlying type is no longer uint8_t!");

/// Form a unique 64-bit integer value describing the type `T`.
///
/// This template needs to be specialized for every type that can
/// participate in this kind of run-time type information, e.g., so
/// that it can be stored in a request.
template<typename T>
struct TypeID;

/// Template whose specializations provide the set of type IDs within a
/// given zone.
template<Zone Zone> struct TypeIDZoneTypes;

/// Form a type ID given a zone and type value.
constexpr uint64_t formTypeID(uint8_t zone, uint8_t type) {
  return (uint64_t(zone) << 8) | uint64_t(type);
}

namespace evaluator {
/// The return type of requests that execute side effects.
///
/// In general, it is not appropriate to use the request evaluator framework to
/// execute a request for the sake of its side effects. However, there are
/// operations we would currently like to be requests because it makes modelling
/// some aspect of their implementation particularly nice. For example, an
/// operation that emits diagnostics to run some checking code in a primary
/// file may be desirable to requestify because it should be run only once per
/// declaration, but it has no coherent return value. Another category of
/// side-effecting requests are those that adapt existing parts of the compiler that
/// do not yet behave in a "functional" manner to have a functional interface. Consider
/// the request to run the SIL Optimizer. In theory, it should be a request that takes in
/// a SILModule and returns a SILModule. In practice, it is a request that executes
/// effects against a SILModule.
///
/// To make these requests stand out - partially in the hope we can return and
/// refactor them to behave in a more well-structured manner, partially because
/// they cannot return \c void or we will get template substitution failures - we
/// annotate them as computing an \c evaluator::SideEffect.
using SideEffect = std::tuple<>;
}

// Define the C type zone (zone 0).
#define LANGUAGE_TYPEID_ZONE C
#define LANGUAGE_TYPEID_HEADER "language/Basic/CTypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"

} // end namespace language

#endif // LANGUAGE_BASIC_TYPEID_H
