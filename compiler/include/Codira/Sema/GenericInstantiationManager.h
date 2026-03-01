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
 * GenericInstantiationManager is the global manager to maintain the generic information.
 */

#ifndef CODIRA_SEMA_GENERIC_INSTANTIATION_MANAGER_H
#define CODIRA_SEMA_GENERIC_INSTANTIATION_MANAGER_H

#include "Codira/AST/Node.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Sema/CommonTypeAlias.h"

namespace Codira {
/** GenericInfo contains the information of the generic decl used to identify a generic decl when instantiated. */
struct GenericInfo {
    Ptr<AST::Decl> decl;      /**< The raw generic decl. */
    TypeSubst gTyToTyMap; /**< The map between the generic ty to instantiated ty. */
    GenericInfo(const Ptr<AST::Decl> decl, const TypeSubst& map) : decl(decl), gTyToTyMap(map)
    {
    }
};

/**
 * The class of generic instantiation manager is a global manager that
 * maintains cache of generic and instantiated decls' information.
 */
class GenericInstantiationManager {
public:
    explicit GenericInstantiationManager(CompilerInstance& ci);
    ~GenericInstantiationManager();
    /** Generic instantiation package entrance. */
    void GenericInstantiatePackage(AST::Package& pkg) const;
    /**
     * Get the instantiated decl corresponding to the genericInfo:
     * @param genericInfo [in] generic decl instantiation parameters.
     * @param pkg [in] current processing package. MUST given, if call this api outside genericInstantiation step.
     */
    Ptr<AST::Decl> GetInstantiatedDeclWithGenericInfo(const GenericInfo& genericInfo, AST::Package& pkg) const;
    /** Get set of instantiated decl of given generic decl */
    std::unordered_set<Ptr<AST::Decl>> GetInstantiatedDecls(const AST::Decl& genericDecl) const;

    /** Prepare for generic instantiation processing:
     *  1. clear all cache generated before.
     *  2. pre-build context cache.
     */
    void ResetGenericInstantiationStage() const;
    std::unordered_map<Ptr<const AST::Decl>, std::unordered_set<Ptr<AST::Decl>>> GetAllGenericToInsDecls() const;

    friend class MockUtils;

private:
    class InstantiatedExtendRecorder;
    class GenericInstantiationManagerImpl;
    std::unique_ptr<GenericInstantiationManagerImpl> impl;
};
} // namespace Codira
#endif
