/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


/*---
desc: All annotation targets
tags: [not-a-test]

---*/


export @Target([AnnotationTargets.ENUMERATION]) @interface EnumAnnotation {}
export @Target([AnnotationTargets.FUNCTION]) @interface AFunction {}
export @Target([AnnotationTargets.FUNCTION_WITH_RECEIVER]) @interface ReceiverFunction {}
export @Target([AnnotationTargets.NAMESPACE]) @interface NamespaceAnnotation {}
export @Retention("SOURCE") @Target([AnnotationTargets.TYPE_ALIAS]) @interface Alias {}
export @Target([AnnotationTargets.VARIABLE]) @interface VariableAnnotation {}

export @Target([AnnotationTargets.STRUCT]) @interface StructAnnotation {}
export @Target([AnnotationTargets.CLASS]) @interface ClassAnnotation {}
export @Target([AnnotationTargets.CLASS_FIELD]) @interface ClassField {}
export @Target([AnnotationTargets.CLASS_METHOD]) @interface ClassMethod {}
export @Target([AnnotationTargets.CLASS_GETTER]) @interface ClassGetter {}
export @Target([AnnotationTargets.CLASS_SETTER]) @interface ClassSetter {}

export @Target([AnnotationTargets.INTERFACE]) @interface InterfaceAnnotation {}
export @Target([AnnotationTargets.INTERFACE_METHOD]) @interface InterfaceMethod {}
export @Target([AnnotationTargets.INTERFACE_PROPERTY]) @interface Property {}
export @Target([AnnotationTargets.INTERFACE_GETTER]) @interface InterfaceGetter {}
export @Target([AnnotationTargets.INTERFACE_SETTER]) @interface InterfaceSetter {}

export @Retention("SOURCE") @Target([AnnotationTargets.LAMBDA]) @interface Lambda {}
export @Target([AnnotationTargets.PARAMETER]) @interface CallParameter {}
export @Target([AnnotationTargets.TYPE]) @interface AType {}

export @Target([AnnotationTargets.CLASS, AnnotationTargets.INTERFACE,
        AnnotationTargets.STRUCT]) @interface InterfaceOrClassOrStruct {}

export @Target([AnnotationTargets.INTERFACE_PROPERTY,
        AnnotationTargets.CLASS_FIELD]) @interface PropertyOrField {}

export @Target([AnnotationTargets.FUNCTION,
        AnnotationTargets.CLASS_METHOD, AnnotationTargets.INTERFACE_METHOD,
        AnnotationTargets.FUNCTION_WITH_RECEIVER]) @interface MethodOrFunction {}
