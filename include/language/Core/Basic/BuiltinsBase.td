//===--- BuiltinsBase.td - common structured used by builtins -------------===//
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

// Attributes
// ==========

class Attribute<string mangling> {
  string Mangling = mangling;
}

class IndexedAttribute<string baseMangling, int I> : Attribute<baseMangling> {
  int Index = I;
}

class MultiIndexAttribute<string baseMangling, list<int> Is>
    : Attribute<baseMangling> {
  list<int> Indices = Is;
}

// Standard Attributes
// -------------------
def NoReturn : Attribute<"r">;

// Attributes from the gnu:: namespace
// -----------------------------------
def Const : Attribute<"c">;
def NoThrow : Attribute<"n">;
def Pure : Attribute<"U">;
def ReturnsTwice : Attribute<"j">;
//  FIXME: gcc has nonnull

// builtin-specific attributes
// ---------------------------

// Signature is meaningless, use custom typechecking.
def CustomTypeChecking : Attribute<"t">;

// Type is not important to semantic analysis and codegen; recognize as builtin
// even if type doesn't match signature, and don't warn if we can't be sure the
// type is right.
def IgnoreSignature : Attribute<"T">;

// Arguments are not evaluated for their side-effects.
def UnevaluatedArguments : Attribute<"u">;

// FIXME: This is misused in a lot of the places it is used currently.
// This function is equivalent to a library function without the __builtin_
// prefix. This is relevant for CodeGen; it should not be used if custom CodeGen
// is required for a builtin.
def FunctionWithBuiltinPrefix : Attribute<"F">;

def FunctionWithoutBuiltinPrefix : Attribute<"f">;

// const, but only when -fno-math-errno and FP exceptions are ignored.
def ConstIgnoringErrnoAndExceptions : Attribute<"e">;

// const when FP exceptions are ignored.
def ConstIgnoringExceptions : Attribute<"g">;

// This function requires a specific header or an explicit declaration.
def RequireDeclaration : Attribute<"h">;

// FIXME: Why is this not simply the min_vector_width attribute?
// Vector has to be at least N bits wide.
class RequiredVectorWidth<int N> : IndexedAttribute<"V", N>;

class PrintfFormat<int I> : IndexedAttribute<"p", I>;
class VPrintfFormat<int I> : IndexedAttribute<"P", I>;
class ScanfFormat<int I> : IndexedAttribute<"s", I>;
class VScanfFormat<int I> : IndexedAttribute<"S", I>;

// Other Attributes
// ----------------

// Builtin can be constant evaluated
def Constexpr : Attribute<"E">;
// Builtin is immediate and must be constant evaluated. Implies Constexpr, and will only be supported in C++20 mode.
def Consteval : Attribute<"EG">;

// Callback behavior: the first index argument is called with the arguments
// indicated by the remaining indices.
class Callback<list<int> ArgIndices> : MultiIndexAttribute<"C", ArgIndices>;

// Prefixes
// ========

class NamePrefix<string spelling> {
  string Spelling = spelling;
}

// Builtin kinds
// =============

class Builtin {
  list<string> Spellings;
  list<Attribute> Attributes = [];
  string Prototype;
  string Namespace;
  // On some platforms, some functions are actually macros. In that case we need
  // to #undef them.
  bit RequiresUndef = 0;
  // Enables builtins to generate `long long` outside of OpenCL and `long` inside.
  bit EnableOpenCLLong = 0;
  // Requires a common prefix to be prepended. Each generated set of builtins
  // can optionally extract one common prefix that is handled separately.
  NamePrefix RequiredNamePrefix;
}

class AtomicBuiltin : Builtin;

class LibBuiltin<string header, string languages = "ALL_LANGUAGES"> : Builtin {
  string Header = header;
  string Languages = languages;
  bit AddBuiltinPrefixedAlias = 0;
  bit OnlyBuiltinPrefixedAliasIsConstexpr = 0;
}

class MSLibBuiltin<string header> : LibBuiltin<header, "ALL_MS_LANGUAGES">;
class GNULibBuiltin<string header> : LibBuiltin<header, "ALL_GNU_LANGUAGES">;
class ObjCLibBuiltin<string header> : LibBuiltin<header, "OBJC_LANG">;
class CxxLibBuiltin<string header> : LibBuiltin<header, "CXX_LANG">;

class LangBuiltin<string languages> : Builtin {
  string Languages = languages;
}

class MSLangBuiltin : LangBuiltin<"ALL_MS_LANGUAGES">;
class CoroLangBuiltin : LangBuiltin<"COR_LANG">;
class OCLPipeLangBuiltin : LangBuiltin<"OCL_PIPE">;
class OCL_DSELangBuiltin : LangBuiltin<"OCL_DSE">;
class OCL_GASLangBuiltin : LangBuiltin<"OCL_GAS">;
class OCLLangBuiltin : LangBuiltin<"ALL_OCL_LANGUAGES">;

class TargetBuiltin : Builtin {
  string Features = "";
}
class TargetLibBuiltin : TargetBuiltin {
  string Header;
  string Languages = "ALL_LANGUAGES";
}

class Template<list<string> substitutions,
               list<string> affixes,
               bit as_prefix = 0> {
  list<string> Substitutions = substitutions;
  list<string> Affixes = affixes;
  bit AsPrefix = as_prefix;
}
