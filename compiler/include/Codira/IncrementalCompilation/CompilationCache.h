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

#ifndef CODIRA_COMPILATION_CACHE_H
#define CODIRA_COMPILATION_CACHE_H

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Codira/AST/Node.h"

namespace Codira {
using RawMangledName = std::string;
using RawMangled2DeclMap = std::unordered_map<RawMangledName, Ptr<const AST::Decl>>;
// A map to record the CHIR optimizations effects on incremental compilation, where the key
// is the raw mangle name of the source decl, and the value is the set of the raw mangle name
// of the polluted decls
using OptEffectStrMap = std::unordered_map<RawMangledName, std::unordered_set<RawMangledName>>;
// no need to have the type defined here
using OptEffectNodeMap = std::unordered_map<Ptr<AST::Decl>, std::unordered_set<Ptr<AST::Decl>>>;
using SrcImportedDepMap = std::unordered_map<Ptr<const AST::Decl>, std::set<Ptr<const AST::Decl>>>;
// record the raw mangle to it's virual func wrapper mangled name map
using VirtualWrapperDepMap = std::unordered_map<RawMangledName, std::string>;
// record the raw mangle to it's var init func mangled name map
using VarInitDepMap = std::unordered_map<RawMangledName, std::string>;

/********************* Compilation Cache In Parser *******************/
/// Describes the file and decl index of a decl. Used to analyse if the relative order among top leve or member decls
/// are changed since last compilation.
struct GlobalVarIndex {
    std::string file;
    int id;

    bool operator<(const GlobalVarIndex& other) const
    {
        return file == other.file && id < other.id;
    }
};
[[maybe_unused]] std::ostream& operator<<(std::ostream& out, const GlobalVarIndex& id);

/// Base class necessary to describe all info of a decl
struct DeclCacheBase {
    size_t sigHash{0}; // the API of a decl (e.g. name of named parameter)
    size_t srcUse{0};   // the ABI (e.g. foreign, @Annotation) and source usage of a decl (e.g. public)
    size_t bodyHash{0}; // the body hash of a decl that has no impact on API (e.g. @Overflow, line no)
                        // for type decl, bodyHash records accessibility and constraints
    uint8_t astKind{0};
    // is global var or varwithpattern (excluding var in varwithpattern), or is static var, used in gvid
    // property can have member function decl
    bool isGV{false};
    GlobalVarIndex gvid{"", 0};
    std::vector<struct MemberDeclCache> members;
    std::string cgMangle; // mangle names for codegen
};

struct MemberDeclCache : DeclCacheBase {
    RawMangledName rawMangle;
};

struct TopLevelDeclCache : DeclCacheBase {
    std::vector<RawMangledName> extends;

    size_t instVarHash{0};
    size_t virtHash{0}; // order of virtual member decls
};

using ASTCache = std::unordered_map<RawMangledName, TopLevelDeclCache>;

/*********************************************************************/

/********************* Compilation Cache In Sema *********************/

/*
 * Definitions:
 * name usages:
 * api usage (any usage of a type decl):
 *     rawMangledName: A in let a: A
 *     qualifier: pkg1.pkg2.pkg3 in let b: pkg1.pkg2.pkg3.A
 * name usage:
 *     unqualified usage (not right to any . operator):
 *         f in f()
 *         a in return a
 *     qualified usage (right to . operator):
 *         decl member accessing usage (left is an type object or typeName):
 *             a in obj.a
 *         package qualified usage (left is a package name):
 *             p1.p2 in p1.p2.f
 *             f in p2.f
 */
// Approximately records the usage of a name of a class/package decl
struct NameUsage {
    std::set<RawMangledName> parentDecls;    // RawMangledName of used parent type decl
    std::set<std::string> packageQualifiers; // Eg: for 'p1.p2.A', identifier is 'A', qualifier is 'p1.p2'.
                                             // only top level non-type decls may have this kind of usage
    bool hasUnqualifiedUsage{false};
    bool hasUnqualifiedUsageOfImported{false};
};

struct UseInfo {
    std::set<RawMangledName> usedDecls;
    std::map<std::string, NameUsage> usedNames;
};

struct SemaUsage {
    UseInfo apiUsages;
    UseInfo bodyUsages;
    std::set<RawMangledName> boxedTypes;
};

struct SemaRelation {
    std::set<RawMangledName> inherits;
    std::set<RawMangledName> extends;
    std::set<RawMangledName> extendedInterfaces;
};

struct SemanticInfo {
    // Record what decls and expressions are used in toplevel/member's. Do not care about removed decl's internal usage.
    // map 'toplevel/member-> usage'.
    std::unordered_map<Ptr<const AST::Decl>, SemaUsage> usages;
    // Record 'type->(inherits, extends, extend interfaces).
    std::unordered_map<RawMangledName, SemaRelation> relations;
    // Record for 'builtin type -> (extends, extend interfaces)'.
    std::unordered_map<std::string, SemaRelation> builtInTypeRelations;
    // Record for user defined decl -> compiler_add_decl_mangle.
    std::unordered_map<RawMangledName, std::set<std::string>> compilerAddedUsages;
};

/*********************************************************************/

// decls groupby file orderby gvid
using FileMap = std::unordered_map<std::string, std::vector<const AST::Decl*>>; // the gvid is written in the Decl
using CachedFileMap = std::unordered_map<std::string, std::vector<RawMangledName>>;
    // the gvid value is not needed as they are sorted

/// All cache info of an instance of incremental compilation. Some are to be stored for further compilations,
/// and some are used for further analysis of this compilation.
struct CompilationCache {
    uint64_t specs = 0;
    uint64_t lambdaCounter = 0;
    uint64_t stringLiteralCounter = 0;
    uint64_t envClassCounter = 0;
    std::vector<std::string> compileArgs;
    std::vector<std::pair<Ptr<const AST::Decl>, std::vector<Ptr<const AST::Decl>>>> varAndFuncDep;
    OptEffectStrMap chirOptInfo;
    VirtualWrapperDepMap virtualFuncDep;
    VarInitDepMap varInitDepMap;
    std::set<std::string> ccOutFuncs; // raw mangled name of gloable or member funcs had closure convert in chir
    SemanticInfo semaInfo;
    ASTCache curPkgASTCache;
    CachedFileMap fileMap;
    ASTCache importedASTCache;
    std::vector<std::string> bitcodeFilesName;
};

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
bool WriteCache(const struct ::Codira::AST::Package& pkg, CompilationCache&& cachedInfo,
    std::vector<const AST::Decl*>&& order, const std::string& path);
#endif
} // namespace Codira
#endif // CODIRA_COMPILATION_CACHE_H
