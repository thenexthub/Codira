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


#ifndef MRT_FIELDINFO_H
#define MRT_FIELDINFO_H

#include "Common/TypeDef.h"
#include "Common/Dataref.h"

namespace MapleRuntime {
class TypeInfo;
class ATTR_PACKED(4) InstanceFieldInfo {
public:
    const char* GetName(TypeInfo* declaringTypeInfo) const;
    U32 GetModifier() const;
    TypeInfo* GetFieldType(TypeInfo* declaringTypeInfo);

    void* GetValue(TypeInfo* declaringTi, ObjRef obj);
    void SetValue(TypeInfo* declaringTi, ObjRef instanceObj, ObjRef newValue);
    void* GetAnnotations(TypeInfo* arrayTi);
private:
    inline U32 GetOffset(TypeInfo* declaringTypeInfo) const;
    I32 modifier;
    U32 fieldIdx;
    Uptr annotationMethod;
};

class ATTR_PACKED(4) StaticFieldInfo {
public:
    const char* GetName() { return fieldName.GetDataRef(); }
    U32 GetModifier() { return modifier; }
    TypeInfo* GetFieldType() { return fieldTypeInfo; }
    void* GetValue();
    void SetValue(ObjRef newValue);
    void* GetAnnotations(TypeInfo* arrayTi);

private:
    void* GetStructValue(ObjRef obj);
    DataRefOffset64<char> fieldName;
    U32 modifier;
    U32 __attribute__((unused)) slot;
    TypeInfo* fieldTypeInfo;
    Uptr addr;
    U64 annotationMethod;
};
} // namespace MapleRuntime
#endif // MRT_FIELDINFO_H
