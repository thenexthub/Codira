/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file declares interpreter values.
 */

#ifndef CODIRA_CHIR_INTERRETER_INTERPREVERVALUE_H
#define CODIRA_CHIR_INTERRETER_INTERPREVERVALUE_H

#include <cstdint>
#include <variant>
#include <vector>

namespace Codira::CHIR::Interpreter {

struct IInvalid;
struct IUInt8;
struct IUInt16;
struct IUInt32;
struct IUInt64;
struct IUIntNat;
struct IInt8;
struct IInt16;
struct IInt32;
struct IInt64;
struct IIntNat;
struct IFloat16;
struct IFloat32;
struct IFloat64;
struct IRune;
struct IBool;
struct IUnit;
struct INullptr;
struct IPointer;
struct ITuple;
struct IArray;
struct IObject;
// These types correspond to the ones above, but where memory management is
// implemented by the user, they make the IValStack variant behave nicely
// (ie have trivial destructor and constructors)
struct ITuplePtr;
struct IArrayPtr;
struct IObjectPtr;

struct IFunc;

using IVal = std::variant<
    // Primitives
    IInvalid, // 0
    IUInt8,   // 1
    IUInt16,  // 2
    IUInt32,  // 3
    IUInt64,  // 4
    IUIntNat, // 5
    IInt8,    // 6
    IInt16,   // 7
    IInt32,   // 8
    IInt64,   // 9
    IIntNat,  // 10
    IFloat16, // 11
    IFloat32, // 12
    IFloat64, // 13
    IRune,    // 14
    IBool,    // 15
    IUnit,    // 16
    INullptr, // 17

    // Pointers (the values of atoms that point to values, see later about heap allocations)
    IPointer, // 18
    // Aggregates
    ITuple,  // 19
    IArray,  // 20
    IObject, // 21

    // Opaque things (functions, names)
    IFunc // 22
    >;

using IValStack = std::variant<
    // Primitives
    IInvalid, // 0
    IUInt8,   // 1
    IUInt16,  // 2
    IUInt32,  // 3
    IUInt64,  // 4
    IUIntNat, // 5
    IInt8,    // 6
    IInt16,   // 7
    IInt32,   // 8
    IInt64,   // 9
    IIntNat,  // 10
    IFloat16, // 11
    IFloat32, // 12
    IFloat64, // 13
    IRune,    // 14
    IBool,    // 15
    IUnit,    // 16
    INullptr, // 17

    // Pointers (the values of atoms that point to values, see later about heap allocations)
    IPointer, // 18
    // Aggregates
    ITuplePtr,  // 19
    IArrayPtr,  // 20
    IObjectPtr, // 21

    // Opaque things (functions, names)
    IFunc // 22
    >;

// Note: in the following structs we are not making the content field const, otherwise
// operator= becomes deleted. This is being used for instance in InterpreterStack::ArgsSet.
// In the future we can possibly eliminate this API method if there is a performance gain.

struct IInvalid {
};

struct IUInt8 {
    std::uint8_t content;
};
struct IUInt16 {
    std::uint16_t content;
};
struct IUInt32 {
    std::uint32_t content;
};
struct IUInt64 {
    std::uint64_t content;
};
struct IUIntNat {
    std::size_t content;
};
struct IInt8 {
    std::int8_t content;
};
struct IInt16 {
    std::int16_t content;
};
struct IInt32 {
    std::int32_t content;
};
struct IInt64 {
    std::int64_t content;
};
struct IIntNat {
#if (defined(__x86_64__) || defined(__aarch64__))
    std::int64_t content;
#else
    std::int32_t content;
#endif
};
struct IFloat16 {
    float content;
};
struct IFloat32 {
    float content;
};
struct IFloat64 {
    double content;
};
struct IRune {
    char32_t content;
};
struct IBool {
    bool content;
};
struct IUnit {
};
struct INullptr {
};
struct IPointer {
    IVal* content;
};
struct ITuple {
    std::vector<IVal> content;
};
struct IArray {
    std::vector<IVal> content;
};
struct IObject {
    std::uint32_t classId;
    std::vector<IVal> content;
};
struct ITuplePtr {
    std::vector<IVal>* contentPtr;
};
struct IArrayPtr {
    std::vector<IVal>* contentPtr;
};
struct IObjectPtr {
    std::uint32_t classId;
    std::vector<IVal>* contentPtr;
};
struct IFunc {
    std::size_t content; // program pointer to the function declaration
};

} // namespace Codira::CHIR::Interpreter

#endif // CODIRA_CHIR_INTERRETER_INTERPREVERVALUE_H
