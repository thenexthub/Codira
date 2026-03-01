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

/*
 * @file
 *
 * This file implements the class MacroCall.
 */
#ifndef CODIRA_MACROCALL_H
#define CODIRA_MACROCALL_H

#include <vector>
#include <variant>

#include "Codira/AST/Node.h"
#include "Codira/Frontend/CompilerInstance.h"

namespace Codira {
static const std::string SOURCE_PACKAGE = "sourcePackage";
static const std::string SOURCE_FILE = "sourceFile";
static const std::string SOURCE_LINE = "sourceLine";
static const std::vector<std::string> BUILD_IN_MACROS = {SOURCE_PACKAGE, SOURCE_FILE, SOURCE_LINE};
#ifdef _WIN32
const std::string LIB_SUFFIX = ".dll";
#elif defined(__APPLE__)
const std::string LIB_SUFFIX = ".dylib";
#else
const std::string LIB_SUFFIX = ".so";
#endif

// Short name for vector smart pointer.
template <typename T> using PtrVector = std::vector<OwnedPtr<T>>;
using DeclUPtrVector = PtrVector<AST::Decl>;
using TokenVector = std::vector<Token>;

/**
 * A Vector Target represents Macro replace location.
 */
template <typename T> struct VectorTarget {
    std::vector<T>* pointer;
    size_t loc; // Target location.
};

// Macro expansion replace node.
using PtrType =
    std::variant<OwnedPtr<AST::Expr>*,  // For those parent is an expr, like initializer in vardecl.
    OwnedPtr<AST::Node>*, // Match expr in match case other.
    VectorTarget<OwnedPtr<AST::Node>>, // For those parent is vector, like body in block.
    VectorTarget<OwnedPtr<AST::Expr>>, // Children in array lit and tuple lit.
    VectorTarget<OwnedPtr<AST::Decl>>, // Decls in class body, struct body.
    VectorTarget<OwnedPtr<AST::FuncParam>>>; // FuncParam in func paramlist.

enum class MacroKind {
    EXPR_KIND = 0, // @M(1+2)
    DECL_KIND = 1, // @M class A{}
    PARAM_KIND = 2, // func f (@M(a))
    UNINITIALIZED = 3,
};

enum class MacroEvalStatus : uint8_t {
    INIT = 0,       // Not ready to evaluate macrocall, child macrocalls should be evaluated first.
    READY = 1,      // Ready to evaluate macrocall.
    EVAL = 2,       // Evaluate macrocall.
    SUCCESS = 3,    // Evaluate macrocall successful.
    FAIL = 4,       // Evaluate macrocall failed.
    REEVAL = 5,     // ReEvaluate macroCall, becasue there are still macrocalls left After macro evaluated.
    FINISH = 6,     // No need to reEvaluate, because there are no macrocalls left After macro evaluated.
    ANNOTATION = 7, // Need change macrocall to decl with annotation.
    REEVALFAILED = 8,
};

enum class ItemKind {
    STRING = 1,
    INT = 2,
    BOOL = 3,
    TKS = 4,
};

struct ItemInfo {
    std::string key;
    ItemKind kind;
    std::string sValue;
    int64_t iValue;
    bool bValue;
    std::vector<Token> tValue;
};

struct ChildMessage {
    std::string childName;
    std::vector<ItemInfo> items;
};

class MacroCall {
public:
    explicit MacroCall(Ptr<AST::Node> node);
    ~MacroCall() {}
    std::vector<OwnedPtr<AST::Annotation>> GetAnnotations() const;
    /**
     * @brief Find valid macro function.
     *
     * @param instance Compiler instance for create diagnosis informations.
     * @return bool Returns true if the initialization is successful; otherwise, false.
     */
    bool ResolveMacroCall(CompilerInstance* instance);

    /**
     * @brief Find macro defined method from dynamic library.
     *
     * @param instance Compiler instance for create diagnosis informations.
     * @return bool Return true if find the method successful; otherwise, false.
     */
    bool FindMacroDefMethod(CompilerInstance* instance);
    inline Ptr<AST::MacroInvocation> GetInvocation() const
    {
        return invocation;
    }
    inline std::set<AST::Modifier> GetModifiers() const
    {
        return modifiers ? *modifiers : std::set<AST::Modifier>{};
    }
    inline Ptr<AST::Node> GetNode()
    {
        return node;
    }
    inline Position GetBeginPos() const
    {
        return begin;
    }
    inline Position GetEndPos() const
    {
        return end;
    }
    inline std::string GetFullName()
    {
        return invocation->fullName;
    }
    inline std::string GetIdentifier()
    {
        return invocation->identifier;
    }
    inline Ptr<const AST::FuncDecl> GetDefinition()
    {
        return definition;
    }
    inline bool HasAttribute()
    {
        return invocation->HasAttr();
    }
    inline std::string GetMacroInfo()
    {
        return "@" + GetFullName() + " in " + node->curFile->fileName +
            ":" + std::to_string(begin.line) + ":" + std::to_string(begin.column);
    }
    
    /**
     * @brief An inner macro call finds itself nested inside a particular outer macro call.
     *
     * @param parentStr The Outter macro name.
     * @param report Whether report error.
     * @return bool Returns true if the inner macro call is nested in the given outer macro call.
     */
    bool CheckParentContext(const char* parentStr, bool report);
    /**
     * @brief Report diagnostic.
     *
     * @param level Error level.
     * @param range Positon range for error code.
     * @param message Diagnostic message.
     * @param hint Diagnostic hint.
     */
    void DiagReport(const int level, const Range range, const char *message, const char* hint) const;
    /**
     * @brief An inner macro can also communicate with an outer macro.
     * When the inner macro executes, it calls the library function `setItem`
     *
     * @param key The key of macro context message.
     * @param value The value of macro context message.
     * @param type The of macro context message.
     */
    void SetItemMacroContext(char* key, void* value, uint8_t type);
    /**
     * @brief When the outer macro executes, it calls `getChildMessages`, getting messages.
     *
     * @param childrenStr The inner macro name that send messages.
     */
    void*** GetChildMessagesFromMacroContext(const char* childrenStr);

    PtrType replaceLoc; /**< Macro replace target Location */
    std::vector<MacroCall*> children;

    MacroEvalStatus status = MacroEvalStatus::INIT;
    bool isOuterMost{false};

    size_t threadId = 0;
    // Begin: For macrocall in string interpolation.
    bool isForInterpolation{false};
    TokenKind strKind = TokenKind::STRING_LITERAL;
    std::string newStr;
    // End: For macrocall in string interpolation.

    // For runtime invoke.
    void* invokeFunc{nullptr};
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    void* coroutineHandle{nullptr};
#endif
    bool isDataReady{false};

    bool useParentPos{false};

    /* The following functions and variables are used for macro with context */
    MacroCall* parentMacroCall {nullptr}; /** Record the current macrocall parent node. */
    CompilerInstance* ci{nullptr}; /** For call diag. */
    std::vector<void*> recordMacroInfo{};
    std::vector<std::vector<void*>> macroInfoVec{};

    bool hasSend{false};
    std::string methodName;     // macrodef method
    std::string packageName;    // macrodef package
    std::string libPath;        // macrodef lib path
    std::vector<std::string> parentNames;       // MacroContext: assertParentContext.
    std::vector<ItemInfo> items;                // MacroContext: setItem.
    std::vector<ChildMessage> childMessages;    // MacroContext: getChildMessages.
    std::vector<std::string> assertParents;     // MacroContext: assertParentContext failed parentName.
private:
    MacroKind kind;
    Ptr<AST::MacroInvocation> invocation{nullptr};
    Ptr<AST::Node> node{nullptr};
    Ptr<AST::FuncDecl> definition{nullptr};
    Position begin;
    Position end;
    std::set<AST::Modifier>* modifiers{nullptr};

    bool GetAllDeclsForMacroName(const std::string &macroName, std::vector<Ptr<AST::Decl>>& decls);
    bool GetValidFuncDecl(std::vector<Ptr<AST::Decl>>& decls);
    bool BindDefinition(const std::string &macroName);
    inline void BindDefinition(const Ptr<AST::FuncDecl> fd)
    {
        definition = fd;
    }

    bool BindInvokeFunc();

    /*
     * Recursive function to set recordMacroInfo info.
     */
    void TraverseMacroNode(
        MacroCall* macroNode, const std::string& childName, std::vector<std::vector<void*>>& macroInfos);
    bool TraverseParentMacroNode(MacroCall* mcNode, const std::string& parentName);
};

}

#endif
