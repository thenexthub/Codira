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
 * This file declares core handlers for implementation of Codira <-> Objective-C interopability.
 */

#ifndef CODIRA_SEMA_DESUGAR_OBJ_C_INTEROP_INTEROP_HANDLERS
#define CODIRA_SEMA_DESUGAR_OBJ_C_INTEROP_INTEROP_HANDLERS

#include "Context.h"
#include "NativeFFI/ObjC/Utils/Handler.h"

namespace Codira::Interop::ObjC {

/**
 * Finds all @ObjCMirror and @ObjCImpl declarations.
 */
class FindMirrors : public Handler<FindMirrors, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Performs all necessary syntax and semantic checks on @ObjCMirror declarations.
 */
class CheckMirrorTypes : public Handler<CheckMirrorTypes, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Performs all necessary syntax and semantic checks on @ObjCImpl declarations.
 */
class CheckImplTypes : public Handler<CheckImplTypes, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Creates and inserts field of type `NativeObjCId` in all hierarchy root @ObjCMirror declarations.
 *
 * `public var $obj: NativeObjCId`
 */
class InsertNativeHandleField : public Handler<InsertNativeHandleField, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Creates and inserts constructors skeletons for each @ObjCMirror class:
 *
 * `public init($obj: NativeObjCId)`
 */
class InsertMirrorCtorDecl : public Handler<InsertMirrorCtorDecl, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Creates and inserts constructors bodies for each @ObjCMirror class:
 *
 * ```codira
 * public init($obj: NativeObjCId) { // decl was inserted in `InsertMirrorCtorDecl`
 *     this.$obj = $obj
 * }
 * ```
 * or
 * ```codira
 * public init($obj: NativeObjCId) {
 *     super($obj)
 * }
 * ```
 */
class InsertMirrorCtorBody : public Handler<InsertMirrorCtorBody, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Creates and inserts finalizers (with special private field $hasInited) for each @ObjCMirror class:
 * ```codira
 * private var $hasInited: Bool = false
 * ~init() {
 *     unsafe { ObjCRuntime.release($obj) }
 * }
 * ```
 */
class InsertFinalizer : public Handler<InsertFinalizer, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Generates top-level `CODEImpl_ObjC_{ForeignName}_deleteCODEObject($registryId: RegistryId): Unit`
 * method for each @ObjCImpl declaration.
 */
class GenerateDeleteCODEObjectMethod : public Handler<GenerateDeleteCODEObjectMethod, InteropContext> {
public:
    explicit GenerateDeleteCODEObjectMethod()
    {
    }
    void HandleImpl(InteropContext& ctx);
};

/**
 * Desugars all members of every @ObjCMirror declarations.
 * Methods are desugared as follows:
 * 1. If method returns @ObjCMirror/@ObjCImpl value:
 * 1.1 Generates proper ObjCRuntime.msgSend call, e.g. ```unsafe { CFunc<(NativeObjCId, NativeObjCSel, ...ArgTypes) ->
 * NativeObjCId>(ObjCRuntime.msgSend)($obj, ObjCRuntime.registerName("${methodForeignName}"), ...args) }```. 1.2 Wraps
 * it with ObjCRuntime.withAutoreleasePoolObj call, e.g. ```ObjCRuntime.withAutoreleasePool{ => // msgSend call })```.
 * 1.3 Wraps it with ctor call of return value type, e.g. ```M(// ObjCRuntime.withAutoreleasePool call)```.
 *
 * 2. Else if method returns Objective-C compatible primitive type value:
 * 2.1 Generates proper ObjCRuntime.msgSend call, e.g. ```unsafe { CFunc<(NativeObjCId, NativeObjCSel, ...ArgTypes) ->
 * RetType>(ObjCRuntime.msgSend)($obj, ObjCRuntime.registerName("${methodForeignName}"), ...args) }```
 *
 * 3. Body is inserted in the ctor declaration generated on previous steps.
 *
 * Prop getters are desugared as follows:
 * 1. If prop has @ObjCMirror/@ObjCImpl value:
 * 1.1 Generates proper ObjCRuntime.msgSend call, e.g. ```unsafe { CFunc<(ObjCId, ObjCSel) ->
 * ObjCId>(ObjCRuntime.msgSend)($obj, ObjCRuntime.registerName("foo")) }```. 1.2 Wraps it with
 * ObjCRuntime.withAutoreleasePoolObj call, e.g. ```ObjCRuntime.withAutoreleasePool{ => // msgSend call })```. 1.3 Wraps
 * it with ObjCRuntime.getFromRegistry call, e.g. ```ObjCRuntime.getFromRegistry(...// ObjCRuntime.withAutoreleasePool
 * call)```.
 *
 * 2. Else if prop has Objective-C compatible primitive type value:
 * 2.1 Generates proper ObjCRuntime.msgSend call, e.g. ```unsafe { CFunc<(ObjCId, ObjCSel) ->
 * RetType>(ObjCRuntime.msgSend)($obj, ObjCRuntime.registerName("foo"), ...args) }```
 *
 * 3. Accessor body is inserted in the corresponding declaration generated on previous steps.
 *
 * Prop setters are desugared as follows:
 * Generates proper ObjCRuntime.msgSend call, e.g. ```unsafe { CFunc<(ObjCId, ObjCSel, ObjCId) ->
 * Unit>(ObjCRuntime.msgSend)($obj, ObjCRuntime.registerName("setFoo:"), value.$obj) }```.
 *
 * Fields are desugared as follows:
 * 1. On parse stage, all fields are replaced with corresponding prop declaration.
 * 2. Prop getter is essentially ObjCRuntime.getInstanceVariable call (getInstanceVariableObj for object types wrapped
 * with getFromRegistryByHandle call).
 * 3. Prop setter (@ObjCMirror field is always var) is essentially ObjCRuntime.setInstanceVariable
 * (setInstanceVariableObj for object type) call.
 */
class DesugarMirrors : public Handler<DesugarMirrors, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);

private:
    void DesugarTopLevelFunc(InteropContext& ctx, AST::FuncDecl& func);
    void DesugarMethod(InteropContext& ctx, AST::ClassLikeDecl& mirror, AST::FuncDecl& method);
    void DesugarCtor(InteropContext& ctx, AST::ClassLikeDecl& mirror, AST::FuncDecl& ctor);
    void DesugarProp(InteropContext& ctx, AST::ClassLikeDecl& mirror, AST::PropDecl& prop);
    void DesugarField(InteropContext& ctx, AST::ClassLikeDecl& mirror, AST::PropDecl& field);
};

/**
 * Does generation inside impl body. Inserts:
 * - A constructor (with extra handle parameter) for each user-defined constructor. It is callable from Objective-C
 */
class GenerateObjCImplMembers : public Handler<GenerateObjCImplMembers, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);

private:
    void GenerateCtor(InteropContext& ctx, AST::ClassDecl& target, AST::FuncDecl& from);
};

/**
 * Generates top-level `CODEImpl_ObjC_{ForeignName}($obj: NativeObjCId, ...): RegistryId` methods
 * for each @ObjCImpl ctor declaration (with first $obj param, as they have proper types for interop).
 */
class GenerateInitCODEObjectMethods : public Handler<GenerateInitCODEObjectMethods, InteropContext> {
public:
    explicit GenerateInitCODEObjectMethods()
    {
    }
    void HandleImpl(InteropContext& ctx);
};

/**
 * Desugars all members of every @ObjCImpl declaration.
 * Common steps are:
 * 1. All method/prop accessors bodies wrapped with `ObjC_ISL_withMethodEnv_ret<T>`/`ObjC_ISL_withMethodEnv_retObj`
 * method call.
 * 2. All expressions with Objective-C mirror/mirror subtype declarations involved are desugared to
 * `objc_msgSend`/`objc_msgSendSuper` calls.
 * 3. Desugared bodies are placed in the corresponding mirrors ($objC prefixed) members.
 * 4. Original members bodies are replaced with throwing `ObjC_ISL_UnreachableCodeException`.
 */
class DesugarImpls : public Handler<DesugarImpls, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);

private:
    void DesugarMethod(InteropContext& ctx, AST::ClassDecl& impl, AST::FuncDecl& method);
    void DesugarCtor(InteropContext& ctx, AST::ClassDecl& impl, AST::FuncDecl& ctor);
    void Desugar(InteropContext& ctx, AST::ClassDecl& impl, AST::PropDecl& prop);
};

/**
 * Generates top-level function wrappers for:
 * 1. Methods (static included)
 * 2. Props (getter and *setter static included)
 * 3. Fields (getter and *setter static included)
 *
 * for each @ObjCImpl declaration.
 *
 * It has to be done because we need a way to call them from Objective-C runtime.
 *
 * NOTE: setters are generated if and only if the prop/field is mutable.
 */
class GenerateWrappers : public Handler<GenerateWrappers, InteropContext> {
public:
    explicit GenerateWrappers()
    {
    }
    void HandleImpl(InteropContext& ctx);

private:
    void GenerateWrapper(InteropContext& ctx, AST::FuncDecl& method);
    // Generic methods for prop and field with SFINAE?
    void GenerateWrapper(InteropContext& ctx, AST::PropDecl& prop);
    void GenerateSetterWrapper(InteropContext& ctx, AST::PropDecl& prop);
    void GenerateWrapper(InteropContext& ctx, AST::VarDecl& field);
    void GenerateSetterWrapper(InteropContext& ctx, AST::VarDecl& field);
};

/**
 * Generates Objective-C glue code for all @ObjCImpl declarations.
 * For each valid @ObjCImpl would be generated the following:
 * 1. Header file ({ForeignName}.h)
 * 2. Implementation file ({ForeignName}.cpp)
 */
class GenerateGlueCode : public Handler<GenerateGlueCode, InteropContext> {
public:
    explicit GenerateGlueCode()
    {
    }
    void HandleImpl(InteropContext& ctx);
};

/**
 * Report errors on usage of ObjCPointer with ObjC-incompatible types
 */
class CheckObjCPointerTypeArguments : public Handler<CheckObjCPointerTypeArguments, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Rewrite pointer access performed by ObjCPointer methods to proper FFI calls
 */
class RewriteObjCPointerAccess : public Handler<RewriteObjCPointerAccess, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

/**
 * Drains all declarations generated on the previous step to their corresponding files which finishes the desugaring.
 */
class DrainGeneratedDecls : public Handler<DrainGeneratedDecls, InteropContext> {
public:
    void HandleImpl(InteropContext& ctx);
};

} // namespace Codira::Interop::ObjC

#endif // CODIRA_SEMA_DESUGAR_OBJ_C_INTEROP_INTEROP_HANDLERS
