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
 * Implements GenericInstantiationManager API calls.
 * Real implementation functions in 'GenericInstantiationManagerImpl'.
 */
#include "GenericInstantiationManagerImpl.h"

#include "Codira/Frontend/CompilerInstance.h"

using namespace Codira;

GenericInstantiationManager::GenericInstantiationManager(CompilerInstance& ci)
{
    impl = std::make_unique<GenericInstantiationManagerImpl>(ci);
}

GenericInstantiationManager::~GenericInstantiationManager()
{
}

void GenericInstantiationManager::GenericInstantiatePackage(AST::Package& pkg) const
{
    impl->GenericInstantiatePackage(pkg);
}

Ptr<AST::Decl> GenericInstantiationManager::GetInstantiatedDeclWithGenericInfo(
    const GenericInfo& genericInfo, AST::Package& pkg) const
{
    return impl->GetInstantiatedDeclWithGenericInfo(genericInfo, pkg);
}
std::unordered_set<Ptr<AST::Decl>> GenericInstantiationManager::GetInstantiatedDecls(
    const AST::Decl& genericDecl) const
{
    return impl->GetInstantiatedDecls(genericDecl);
}

void GenericInstantiationManager::ResetGenericInstantiationStage() const
{
    impl->ResetGenericInstantiationStage();
}

Generic2InsMap GenericInstantiationManager::GetAllGenericToInsDecls() const
{
    return impl->GetAllGenericToInsDecls();
}
