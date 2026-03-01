/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include <stdlib.h>
#include "Codira/Basic/UGTypeKind.h"

#if defined(__linux__)
#define TYPE_INFO_SECTION ".codemetadata.typeinfo"
#define TYPE_TEMPLATE_SECTION ".codemetadata.typetemplate"
#elif defined(_WIN32) && defined(__MINGW64__)
#define TYPE_INFO_SECTION ".codeti"
#define TYPE_TEMPLATE_SECTION ".codett"
#elif defined(__APPLE__)
#define TYPE_INFO_SECTION "__CODE_METADATA,__codetypeinfo"
#define TYPE_TEMPLATE_SECTION "__CODE_METADATA,__codetemplate"
#else
#define TYPE_INFO_SECTION ""
#define TYPE_TEMPLATE_SECTION ""
#error "Unknown OS，TYPE_INFO_SECTION is empty"
#endif

#define SANITIZE_ATTR __attribute__((no_sanitize_address))

#define DECLARE_TYPE_INFO(name, initializer)                                                                           \
    TypeInfo name __attribute__((section(TYPE_INFO_SECTION))) __attribute__((aligned(16))) SANITIZE_ATTR = initializer;

#define DECLARE_TYPE_TEMPLATE(name, initializer)                                                                       \
    TypeTemplate name __attribute__((section(TYPE_TEMPLATE_SECTION))) __attribute__((aligned(16))) SANITIZE_ATTR =     \
        initializer;

typedef struct TypeInfo* (*FnPtrType)(int32_t, struct TypeInfo**);

#pragma pack(push, 8)
typedef struct TypeTemplate {
    const char* name;
    int8_t typeKind;      // Reference types: class, interface, array (reference) Use enum values
    uint8_t flag;         // Mark, hasRefField, hasFinalize, monitor, waitQueue，Use 0-3 bit
    uint16_t fieldsNum;   // Number of member variables
    uint16_t typeArgsNum; // Number of generic parameters
    // Member variables
    FnPtrType* fieldsFns;
    // Parent class
    FnPtrType superFn;
    void* finalizer;
    // Reflects relevant information, controlled by the reflection compilation switch
    void* reflection;
    void** extensionDefPtr;
    uint16_t inheritedClassNum;
} TypeTemplate;
#pragma pack(pop)

#pragma pack(push, 8)
typedef struct TypeInfo {
    const char* name;
    int8_t type;  // Type, unit, bool, int, float, struct, class, interface, array, function, varray use enum values
    uint8_t flag; // Mark, hasRefField, hasFinalize, monitor, waitQueue, use bits 0-3
    uint16_t fieldsNum; // Number of member variables
    uint32_t size;      // Memory size (Byte)
    const void* gcTib;  // Indicates which fields of the current object are references for GC to recursively scan
    uint32_t uuid;
    uint8_t align;      // Align
    uint8_t typeArgNum; // Number of generic parameters
    uint16_t inheritedClassNum;
    uint32_t* offsets;
    union {
        // If `typeArgNum` is greater than 0, it indicates `sourceGeneric`. Otherwise, it indicates `finalizer`.
        const TypeTemplate* sourceGeneric; // Original generic template
        void* finalizer;
    };
    // Generic parameter list
    const struct TypeInfo** typeArgs;
    // List of member variables
    const struct TypeInfo** fields;
    // Parent class
    const struct TypeInfo* super;
    void** extensionDefPtr;
    void* mtable;
    // Reflection metadata information is controlled by the reflection compilation switch.
    void* reflection;
} TypeInfo;
#pragma pack(pop)

// GlobalVarible for TypeInfo defined in core package
DECLARE_TYPE_INFO(Int64_ti,
    ((TypeInfo){
        .name = "Int64",
        .type = UG_INT64,
        .flag = 0,
        .fieldsNum = 0,
        .size = 8,
        .gcTib = NULL,
        .uuid = 0,
        .align = 8,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Nothing_ti,
    ((TypeInfo){
        .name = "Nothing",
        .type = UG_NOTHING,
        .flag = 0,
        .fieldsNum = 0,
        .size = 0,
        .gcTib = NULL,
        .uuid = 0,
        .align = 1,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Unit_ti,
    ((TypeInfo){
        .name = "Unit",
        .type = UG_UNIT,
        .flag = 0,
        .fieldsNum = 0,
        .size = 0,
        .gcTib = NULL,
        .uuid = 0,
        .align = 1,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Bool_ti,
    ((TypeInfo){
        .name = "Bool",
        .type = UG_BOOLEAN,
        .flag = 0,
        .fieldsNum = 0,
        .size = 1,
        .gcTib = NULL,
        .uuid = 0,
        .align = 1,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Rune_ti,
    ((TypeInfo){
        .name = "Rune",
        .type = UG_RUNE,
        .flag = 0,
        .fieldsNum = 0,
        .size = 4,
        .gcTib = NULL,
        .uuid = 0,
        .align = 4,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(UInt8_ti,
    ((TypeInfo){
        .name = "UInt8",
        .type = UG_UINT8,
        .flag = 0,
        .fieldsNum = 0,
        .size = 1,
        .gcTib = NULL,
        .uuid = 0,
        .align = 1,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(UInt16_ti,
    ((TypeInfo){
        .name = "UInt16",
        .type = UG_UINT16,
        .flag = 0,
        .fieldsNum = 0,
        .size = 2,
        .gcTib = NULL,
        .uuid = 0,
        .align = 2,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(UInt32_ti,
    ((TypeInfo){
        .name = "UInt32",
        .type = UG_UINT32,
        .flag = 0,
        .fieldsNum = 0,
        .size = 4,
        .gcTib = NULL,
        .uuid = 0,
        .align = 4,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(UInt64_ti,
    ((TypeInfo){
        .name = "UInt64",
        .type = UG_UINT64,
        .flag = 0,
        .fieldsNum = 0,
        .size = 8,
        .gcTib = NULL,
        .uuid = 0,
        .align = 8,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(UIntNative_ti, ((TypeInfo) {
    .name = "UIntNative", .type = UG_UINT_NATIVE, .flag = 0, .fieldsNum = 0,
#if (defined(__x86_64__) || defined(__aarch64__))
    .size = 8U,
#else
    .size = 4U,
#endif
    .gcTib = NULL, .uuid = 0,
#if (defined(__x86_64__) || defined(__aarch64__))
    .align = 8U,
#else
    .align = 4U,
#endif
    .typeArgNum = 0, .inheritedClassNum = 0, .offsets = NULL, .finalizer = NULL, .typeArgs = NULL, .fields = NULL,
    .super = NULL, .extensionDefPtr = NULL, .mtable = NULL, .reflection = NULL,
}))

DECLARE_TYPE_INFO(Int8_ti,
    ((TypeInfo){
        .name = "Int8",
        .type = UG_INT8,
        .flag = 0,
        .fieldsNum = 0,
        .size = 1,
        .gcTib = NULL,
        .uuid = 0,
        .align = 1,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Int16_ti,
    ((TypeInfo){
        .name = "Int16",
        .type = UG_INT16,
        .flag = 0,
        .fieldsNum = 0,
        .size = 2,
        .gcTib = NULL,
        .uuid = 0,
        .align = 2,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Int32_ti,
    ((TypeInfo){
        .name = "Int32",
        .type = UG_INT32,
        .flag = 0,
        .fieldsNum = 0,
        .size = 4,
        .gcTib = NULL,
        .uuid = 0,
        .align = 4,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(IntNative_ti, ((TypeInfo) {
    .name = "IntNative", .type = UG_INT_NATIVE, .flag = 0, .fieldsNum = 0,
#if (defined(__x86_64__) || defined(__aarch64__))
    .size = 8U,
#else
    .size = 4U,
#endif
    .gcTib = NULL, .uuid = 0,
#if (defined(__x86_64__) || defined(__aarch64__))
    .align = 8U,
#else
    .align = 4U,
#endif
    .typeArgNum = 0, .inheritedClassNum = 0, .offsets = NULL, .finalizer = NULL, .typeArgs = NULL, .fields = NULL,
    .super = NULL, .extensionDefPtr = NULL, .mtable = NULL, .reflection = NULL,
}))

DECLARE_TYPE_INFO(Float16_ti,
    ((TypeInfo){
        .name = "Float16",
        .type = UG_FLOAT16,
        .flag = 0,
        .fieldsNum = 0,
        .size = 2,
        .gcTib = NULL,
        .uuid = 0,
        .align = 2,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Float32_ti,
    ((TypeInfo){
        .name = "Float32",
        .type = UG_FLOAT32,
        .flag = 0,
        .fieldsNum = 0,
        .size = 4,
        .gcTib = NULL,
        .uuid = 0,
        .align = 4,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

DECLARE_TYPE_INFO(Float64_ti,
    ((TypeInfo){
        .name = "Float64",
        .type = UG_FLOAT64,
        .flag = 0,
        .fieldsNum = 0,
        .size = 8,
        .gcTib = NULL,
        .uuid = 0,
        .align = 8,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))

#ifdef __arm__
DECLARE_TYPE_INFO(CString_ti,
    ((TypeInfo){
        .name = "CString",
        .type = UG_CSTRING,
        .flag = 0,
        .fieldsNum = 0,
        .size = 4,
        .gcTib = NULL,
        .uuid = 0,
        .align = 4,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))
#else
DECLARE_TYPE_INFO(CString_ti,
    ((TypeInfo){
        .name = "CString",
        .type = UG_CSTRING,
        .flag = 0,
        .fieldsNum = 0,
        .size = 8,
        .gcTib = NULL,
        .uuid = 0,
        .align = 8,
        .typeArgNum = 0,
        .inheritedClassNum = 0,
        .offsets = NULL,
        .finalizer = NULL,
        .typeArgs = NULL,
        .fields = NULL,
        .super = NULL,
        .extensionDefPtr = NULL,
        .mtable = NULL,
        .reflection = NULL,
    }))
#endif

// GlobalVarible for TypeTemplate defined in core package
DECLARE_TYPE_TEMPLATE(Tuple_tt,
    ((TypeTemplate){
        .name = "Tuple",
        .typeKind = UG_TUPLE,
        .flag = 0,
        .fieldsNum = 0,
        .typeArgsNum = 0,
        .fieldsFns = NULL,
        .superFn = NULL,
        .finalizer = NULL,
        .reflection = NULL,
        .extensionDefPtr = NULL,
        .inheritedClassNum = 0,
    }))

DECLARE_TYPE_TEMPLATE(Box_tt,
    ((TypeTemplate){.name = "Box",
        .typeKind = UG_CLASS,
        .flag = 0,
        .fieldsNum = 0,
        .typeArgsNum = 0,
        .fieldsFns = NULL,
        .superFn = NULL,
        .finalizer = NULL,
        .reflection = NULL}))

DECLARE_TYPE_TEMPLATE(VArray_tt,
    ((TypeTemplate){
        .name = "VArray",
        .typeKind = UG_VARRAY,
        .flag = 0,
        .fieldsNum = 0,
        .typeArgsNum = 0,
        .fieldsFns = NULL,
        .superFn = NULL,
        .finalizer = NULL,
        .reflection = NULL,
        .extensionDefPtr = NULL,
        .inheritedClassNum = 0,
    }))

DECLARE_TYPE_TEMPLATE(RawArray_tt,
    ((TypeTemplate){
        .name = "RawArray",
        .typeKind = UG_RAWARRAY,
        .flag = 0,
        .fieldsNum = 0,
        .typeArgsNum = 1,
        .fieldsFns = NULL,
        .superFn = NULL,
        .finalizer = NULL,
        .reflection = NULL,
        .extensionDefPtr = NULL,
        .inheritedClassNum = 0,
    }))

DECLARE_TYPE_TEMPLATE(CPointer_tt,
    ((TypeTemplate){
        .name = "CPointer",
        .typeKind = UG_CPOINTER,
        .flag = 0,
        .fieldsNum = 0,
        .typeArgsNum = 1,
        .fieldsFns = NULL,
        .superFn = NULL,
        .finalizer = NULL,
        .reflection = NULL,
        .extensionDefPtr = NULL,
        .inheritedClassNum = 0,
    }))

DECLARE_TYPE_TEMPLATE(CFunc_tt,
    ((TypeTemplate){
        .name = "CFunc",
        .typeKind = UG_CFUNC,
        .flag = 0,
        .fieldsNum = 0,
        .typeArgsNum = 1,
        .fieldsFns = NULL,
        .superFn = NULL,
        .finalizer = NULL,
        .reflection = NULL,
        .extensionDefPtr = NULL,
        .inheritedClassNum = 0,
    }))

static TypeInfo* ClosureTTField0Fn(int32_t argNum, TypeInfo** typeArgs)
{
    return &Int64_ti;
}

static TypeInfo* ClosureTTField1Fn(int32_t argNum, TypeInfo** typeArgs)
{
    return &Int64_ti;
}

static FnPtrType g_closureTTFieldFns[] = {ClosureTTField0Fn, ClosureTTField1Fn};

DECLARE_TYPE_TEMPLATE(Closure_tt,
    ((TypeTemplate){
        .name = "Closure",
        .typeKind = UG_FUNC,
        .flag = 0,
        .fieldsNum = 2,
        .typeArgsNum = 1,
        .fieldsFns = g_closureTTFieldFns,
        .superFn = NULL,
        .finalizer = NULL,
        .reflection = NULL,
        .extensionDefPtr = NULL,
        .inheritedClassNum = 0,
    }))
