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

#ifndef CODIRA_CHIR_TRANSFORMATION_NO_SIDE_EFFECT_MARKER_H
#define CODIRA_CHIR_TRANSFORMATION_NO_SIDE_EFFECT_MARKER_H

#include "Codira/CHIR/Expression/Expression.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
class NoSideEffectMarker {
public:
    static void RunOnPackage(const Ptr<const Package>& package, bool isDebug);

    static void RunOnFunc(const Ptr<Value>& value, bool isDebug);

private:
    static bool CheckPackage(const std::string& packageName);

    static bool CheckNoSideEffectList(const Func& func);
};
}  // namespace CHIR

#endif
