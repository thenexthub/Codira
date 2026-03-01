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
 * This file declares normal type and exception type maping struct template.
 */

#ifndef CODIRA_CHIR_EXCEPTION_TYPEMAPPING_H
#define CODIRA_CHIR_EXCEPTION_TYPEMAPPING_H

#include "Codira/CHIR/Expression/Terminator.h"

namespace Codira::CHIR {
template <typename T> struct CHIRNodeMap {
};

// Defined CHIR type mapping register macro.
#define DEFINE_CHIR_TYPE_MAPPING(TYPE)                                                                                 \
    template <> struct CHIRNodeMap<TYPE> {                                                                             \
        using Normal = TYPE;                                                                                           \
        using Exception = TYPE##WithException;                                                                         \
    }

DEFINE_CHIR_TYPE_MAPPING(CHIR::Apply);
DEFINE_CHIR_TYPE_MAPPING(CHIR::Invoke);
DEFINE_CHIR_TYPE_MAPPING(CHIR::InvokeStatic);
DEFINE_CHIR_TYPE_MAPPING(CHIR::TypeCast);
DEFINE_CHIR_TYPE_MAPPING(CHIR::Allocate);
DEFINE_CHIR_TYPE_MAPPING(CHIR::Spawn);
DEFINE_CHIR_TYPE_MAPPING(CHIR::Intrinsic);
DEFINE_CHIR_TYPE_MAPPING(CHIR::RawArrayAllocate);

template <> struct CHIRNodeMap<UnaryExpression> {
    using Normal = CHIR::UnaryExpression;
    using Exception = CHIR::IntOpWithException;
};
template <> struct CHIRNodeMap<BinaryExpression> {
    using Normal = CHIR::BinaryExpression;
    using Exception = CHIR::IntOpWithException;
};

template <typename T> using CHIRNodeNormalT = typename CHIRNodeMap<T>::Normal;
template <typename T> using CHIRNodeExceptionT = typename CHIRNodeMap<T>::Exception;
} // namespace Codira::CHIR
#endif
