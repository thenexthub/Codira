//===--- DefineTypeIDZone.h - Define a TypeID Zone --------------*- C++ -*-===//
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
//  This file should be #included to define the TypeIDs for a given zone.
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
#define LANGUAGE_TYPEID(Type) LANGUAGE_TYPEID_NAMED(Type, Type)
#define LANGUAGE_REQUEST(Zone, Type, Sig, Caching, LocOptions) LANGUAGE_TYPEID_NAMED(Type, Type)

// First pass: put all of the names into an enum so we get values for them.
template<> struct TypeIDZoneTypes<Zone::LANGUAGE_TYPEID_ZONE> {
  enum Types : uint8_t {
#define LANGUAGE_TYPEID_NAMED(Type, Name) Name,
#define LANGUAGE_TYPEID_TEMPLATE1_NAMED(Template, Name, Param1, Arg1) Name,
#define LANGUAGE_TYPEID_TEMPLATE2_NAMED(Template, Name, Param1, Arg1, Param2, Arg2) Name,
#include LANGUAGE_TYPEID_HEADER
#undef LANGUAGE_TYPEID_NAMED
#undef LANGUAGE_TYPEID_TEMPLATE1_NAMED
#undef LANGUAGE_TYPEID_TEMPLATE2_NAMED
    Count
  };
};

// Second pass: create specializations of TypeID for these types.
#define LANGUAGE_TYPEID_NAMED(Type, Name)                       \
template<> struct TypeID<Type> {                             \
  static constexpr Zone zone = Zone::LANGUAGE_TYPEID_ZONE;      \
  static const uint8_t zoneID = static_cast<uint8_t>(zone);  \
  static const uint8_t localID =                             \
    TypeIDZoneTypes<Zone::LANGUAGE_TYPEID_ZONE>::Name;          \
                                                             \
  static const uint64_t value = formTypeID(zoneID, localID); \
                                                             \
  static toolchain::StringRef getName() { return #Name; }         \
};

#define LANGUAGE_TYPEID_TEMPLATE1_NAMED(Template, Name, Param1, Arg1)    \
template<Param1> struct TypeID<Template<Arg1>> {                      \
private:                                                              \
  static const uint64_t templateID =                                  \
    formTypeID(static_cast<uint8_t>(Zone::LANGUAGE_TYPEID_ZONE),         \
               TypeIDZoneTypes<Zone::LANGUAGE_TYPEID_ZONE>::Name);       \
                                                                      \
public:                                                               \
  static const uint64_t value =                                       \
    (TypeID<Arg1>::value << 16) | templateID;                         \
                                                                      \
  static std::string getName() {                                      \
    return std::string(#Name) + "<" + TypeID<Arg1>::getName() + ">";  \
  }                                                                   \
};                                                                    \
                                                                      \
template<Param1> const uint64_t TypeID<Template<Arg1>>::value;

#define LANGUAGE_TYPEID_TEMPLATE2_NAMED(Template, Name, Param1, Arg1, Param2, Arg2) \
template<Param1, Param2> struct TypeID<Template<Arg1, Arg2>> {        \
private:                                                              \
  static const uint64_t templateID =                                  \
    formTypeID(static_cast<uint8_t>(Zone::LANGUAGE_TYPEID_ZONE),         \
               TypeIDZoneTypes<Zone::LANGUAGE_TYPEID_ZONE>::Name);       \
                                                                      \
public:                                                               \
  static const uint64_t value =                                       \
    (TypeID<Arg1>::value << 32) |                                     \
    (TypeID<Arg2>::value << 16) |                                     \
    templateID;                                                       \
                                                                      \
  static std::string getName() {                                      \
    return std::string(#Name) + "<" + TypeID<Arg1>::getName() +       \
        ", " + TypeID<Arg2>::getName() + ">";                         \
  }                                                                   \
};                                                                    \
                                                                      \
template<Param1, Param2> const uint64_t TypeID<Template<Arg1, Arg2>>::value;

#include LANGUAGE_TYPEID_HEADER

#undef LANGUAGE_REQUEST

#undef LANGUAGE_TYPEID_NAMED
#undef LANGUAGE_TYPEID_TEMPLATE1_NAMED
#undef LANGUAGE_TYPEID_TEMPLATE2_NAMED

#undef LANGUAGE_TYPEID
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER
