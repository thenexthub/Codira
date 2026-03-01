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
 * This file implements parser-related diagnostics for codira-native java FFI
 */

#include "../../ParserImpl.h"
#include "JFFIParserImpl.h"

using namespace Codira;
using namespace AST;

void JFFIParserImpl::DiagJavaMirrorCannotHaveFinalizer(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_mirror_cannot_have_finalizer, node);
}

void JFFIParserImpl::DiagJavaMirrorCannotHavePrivateMember(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_mirror_cannot_have_private_member, node);
}

void JFFIParserImpl::DiagJavaMirrorCannotHaveStaticInit(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_mirror_cannot_have_static_init, node);
}

void JFFIParserImpl::DiagJavaMirrorCannotHaveConstMember(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_mirror_cannot_have_const_member, node);
}

void JFFIParserImpl::DiagJavaImplCannotBeGeneric(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_impl_cannot_be_generic, node);
}

void JFFIParserImpl::DiagJavaImplCannotBeAbstract(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_impl_cannot_be_abstract, node);
}

void JFFIParserImpl::DiagJavaImplCannotBeSealed(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_impl_cannot_be_sealed, node);
}

void JFFIParserImpl::DiagJavaMirrorCannotBeSealed(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_mirror_cannot_be_sealed, node);
}

void JFFIParserImpl::DiagJavaImplCannotHaveStaticInit(const AST::Node& node) const
{
    p.ParseDiagnoseRefactor(DiagKindRefactor::parse_java_impl_cannot_have_static_init, node);
}
