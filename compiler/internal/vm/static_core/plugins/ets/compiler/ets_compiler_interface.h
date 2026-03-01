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
#ifndef PANDA_PLUGINS_ETS_COMPILER_ETS_COMPILER_INTERFACE_H
#define PANDA_PLUGINS_ETS_COMPILER_ETS_COMPILER_INTERFACE_H

virtual ClassPtr GetEscompatArrayClass() const
{
    return nullptr;
}

virtual FieldPtr GetEscompatArrayBuffer([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

virtual FieldPtr GetEscompatArrayActualLength([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

virtual FieldPtr GetEscompatTypedArrayBuffer([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

virtual FieldPtr GetEscompatTypedArrayByteOffset([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

virtual FieldPtr GetEscompatUnsignedTypedArrayByteOffsetInt([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

virtual FieldPtr GetEscompatTypedArrayLengthInt([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

virtual ClassPtr GetEscompatArrayBufferClass() const
{
    return nullptr;
}

virtual FieldPtr GetEscompatArrayBufferDataAddress([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

virtual FieldPtr GetEscompatArrayBufferManagedData([[maybe_unused]] ClassPtr klass) const
{
    return nullptr;
}

#endif  // PANDA_PLUGINS_ETS_COMPILER_ETS_COMPILER_INTERFACE_H
