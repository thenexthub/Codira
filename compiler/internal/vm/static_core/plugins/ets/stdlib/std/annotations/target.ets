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

package std.annotations;

export enum AnnotationTargets {
    // Top-Level Declarations
    CLASS,
    ENUMERATION,
    FUNCTION,
    FUNCTION_WITH_RECEIVER,
    INTERFACE,
    NAMESPACE,
    TYPE_ALIAS,
    VARIABLE,

    // Class Members
    CLASS_FIELD,
    CLASS_METHOD,
    CLASS_GETTER,
    CLASS_SETTER,

    // Interface Members
    INTERFACE_PROPERTY,
    INTERFACE_METHOD,
    INTERFACE_GETTER,
    INTERFACE_SETTER,

    // Other targets
    LAMBDA,
    PARAMETER,
    STRUCT,
    TYPE
};

export @interface Target {
    targets: AnnotationTargets[]
};