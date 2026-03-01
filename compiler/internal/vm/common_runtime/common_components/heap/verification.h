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

#ifndef VERIFICATION_H
#define VERIFICATION_H

#include "ark_collector/ark_collector.h"

namespace common {

class WVerify {
public:
    static void VerifyAfterMark(ArkCollector &collector);
    static void VerifyAfterForward(ArkCollector &collector);
    static void VerifyAfterFix(ArkCollector &collector);
    static void EnableReadBarrierDFX(ArkCollector &collector);
    static void DisableReadBarrierDFX(ArkCollector &collector);

private:
    static void VerifyAfterMarkInternal(RegionalHeap &space);
    static void VerifyAfterForwardInternal(RegionalHeap &space);
    static void VerifyAfterFixInternal(RegionalHeap &space);
    static void EnableReadBarrierDFXInternal(RegionalHeap &space);
    static void DisableReadBarrierDFXInternal(RegionalHeap &space);
};

}  // namespace common

#endif  // VERIFICATION_H
