//===--- ImplementTypeIDZone.h - Implement a TypeID Zone --------*- C++ -*-===//
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
//  This file should be #included to implement the TypeIDs for a given zone
//  in a C++ file.
//  Two macros should be #define'd before inclusion, and will be #undef'd at
//  the end of this file:
//
//    LANGUAGE_TYPEID_ZONE: The ID number of the Zone being defined, which must
//    be unique. 0 is reserved for basic C and LLVM types; 255 is reserved
//    for test cases.
//
//    LANGUAGE_TYPEID_HEADER: A (quoted) name of the header to be
//    included to define the types in the zone.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_TYPEID_ZONE
#  error Must define the value of the TypeID zone with the given name.
#endif

#ifndef LANGUAGE_TYPEID_HEADER
#  error Must define the TypeID header name with LANGUAGE_TYPEID_HEADER
#endif

// Define a TypeID where the type name and internal name are the same.
#define LANGUAGE_REQUEST(Zone, Type, Sig, Caching, LocOptions) LANGUAGE_TYPEID_NAMED(Type, Type)
#define LANGUAGE_TYPEID(Type) LANGUAGE_TYPEID_NAMED(Type, Type)

// Out-of-line definitions.
#define LANGUAGE_TYPEID_NAMED(Type, Name)            \
  const uint64_t TypeID<Type>::value;

#define LANGUAGE_TYPEID_TEMPLATE1_NAMED(Template, Name, Param1, Arg1)

#include LANGUAGE_TYPEID_HEADER

#undef LANGUAGE_REQUEST

#undef LANGUAGE_TYPEID_NAMED
#undef LANGUAGE_TYPEID_TEMPLATE1_NAMED

#undef LANGUAGE_TYPEID
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER
