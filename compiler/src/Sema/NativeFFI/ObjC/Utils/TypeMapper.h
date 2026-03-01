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
 * This file declares type mappings Codira <-> Objective-C helper
 */

#ifndef CODIRA_SEMA_OBJ_C_UTILS_TYPE_MAPPER_H
#define CODIRA_SEMA_OBJ_C_UTILS_TYPE_MAPPER_H

#include "Codira/AST/Node.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/Utils/SafePointer.h"
#include "InteropLibBridge.h"

namespace Codira::Interop::ObjC {
class TypeMapper {
public:
    explicit TypeMapper(InteropLibBridge& bridge, TypeManager& typeManager)
        : bridge(bridge), typeManager(typeManager)
    {
    }
    
    std::string Code2ObjCForObjC(const AST::Ty& from) const;
    Ptr<AST::Ty> Code2CType(Ptr<AST::Ty> codety) const;
    static bool IsObjCCompatible(const AST::Ty& ty);
    static bool IsObjCMirror(const AST::Decl& decl);
    static bool IsObjCMirrorSubtype(const AST::Decl& decl);
    static bool IsObjCImpl(const AST::Decl& decl);
    static bool IsValidObjCMirror(const AST::Ty& ty);
    static bool IsObjCMirrorSubtype(const AST::Ty& ty);
    static bool IsObjCImpl(const AST::Ty& ty);
    static bool IsObjCMirror(const AST::Ty& ty);
    static bool IsObjCPointer(const AST::Decl& decl);
    static bool IsObjCPointer(const AST::Ty& ty);

private:
    InteropLibBridge& bridge;
    TypeManager& typeManager;
};
} // namespace Codira::Interop::ObjC

#endif // CODIRA_SEMA_OBJ_C_UTILS_TYPE_MAPPER_H
