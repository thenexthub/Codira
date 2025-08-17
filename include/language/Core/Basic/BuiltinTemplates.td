//===--- BuiltinTemplates.td - Clang builtin template aliases ---*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

class TemplateArg<string name> {
  string Name = name;
}

class Template<list<TemplateArg> args, string name> : TemplateArg<name> {
  list<TemplateArg> Args = args;
}

class Class<string name, bit is_variadic = 0> : TemplateArg<name> {
  bit IsVariadic = is_variadic;
}

class NTTP<string type_name, string name, bit is_variadic = 0> : TemplateArg<name> {
  string TypeName = type_name;
  bit IsVariadic = is_variadic;
}

class BuiltinNTTP<string type_name> : TemplateArg<""> {
  string TypeName = type_name;
}

def SizeT : BuiltinNTTP<"size_t"> {}
def Uint32T: BuiltinNTTP<"uint32_t"> {}

class BuiltinTemplate<list<TemplateArg> template_head> {
  list<TemplateArg> TemplateHead = template_head;
}

class CPlusPlusBuiltinTemplate<list<TemplateArg> template_head> : BuiltinTemplate<template_head>;

class HLSLBuiltinTemplate<list<TemplateArg> template_head> : BuiltinTemplate<template_head>;

// template <template <class T, T... Ints> IntSeq, class T, T N>
def __make_integer_seq : CPlusPlusBuiltinTemplate<
  [Template<[Class<"T">, NTTP<"T", "Ints", /*is_variadic=*/1>], "IntSeq">, Class<"T">, NTTP<"T", "N">]>;

// template <size_t, class... T>
def __type_pack_element : CPlusPlusBuiltinTemplate<
  [SizeT, Class<"T", /*is_variadic=*/1>]>;

// template <template <class... Args> BaseTemplate,
//           template <class TypeMember> HasTypeMember,
//           class HasNoTypeMember
//           class... Ts>
def __builtin_common_type : CPlusPlusBuiltinTemplate<
  [Template<[Class<"Args", /*is_variadic=*/1>], "BaseTemplate">,
   Template<[Class<"TypeMember">], "HasTypeMember">,
   Class<"HasNoTypeMember">,
   Class<"Ts", /*is_variadic=*/1>]>;

// template <uint32_t Opcode,
//           uint32_t Size,
//           uint32_t Alignment,
//           typename ...Operands>
def __hlsl_spirv_type : HLSLBuiltinTemplate<
[Uint32T, Uint32T, Uint32T, Class<"Operands", /*is_variadic=*/1>]>;
