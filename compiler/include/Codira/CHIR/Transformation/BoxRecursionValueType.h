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

#ifndef CODIRA_CHIR_TRANSFORMATION_BOX_RECURSION_VALUE_TYPE_H
#define CODIRA_CHIR_TRANSFORMATION_BOX_RECURSION_VALUE_TYPE_H

#include "Codira/CHIR/CHIRBuilder.h"

namespace Codira::CHIR {
/**
 * CHIR Normal Pass: add box and unbox between of several certain expressions, such as GetElementRef, tuple.
 */
class BoxRecursionValueType {
public:
    /**
     * @brief constructor for pass to add box and unbox expressions.
     * @param pkg input package.
     * @param builder CHIR builder for generating IR.
     */
    BoxRecursionValueType(Package& pkg, CHIRBuilder& builder);

    /**
     * @brief main process to add box and unbox expressions.
     */
    void CreateBoxTypeForRecursionValueType();

private:
    void CreateBoxTypeForRecursionEnum();
    void CreateBoxTypeForRecursionStruct();
    void InsertBoxAndUnboxExprForRecursionValueType();

    Package& pkg;
    CHIRBuilder& builder;
};
}
#endif
