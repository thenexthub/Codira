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

#ifndef DEBUG_STEP_FLAGS_H
#define DEBUG_STEP_FLAGS_H

#include <mutex>

class DebugStepFlags {
public:
    static DebugStepFlags& Get();
    void SetDyn2StatInto(bool value);
    void SetDyn2StatOut(bool value);
    void SetDyn2StatOver(bool value);
    void SetStat2DynInto(bool value);
    void SetStat2DynOut(bool value);
    void SetStat2DynOver(bool value);

    bool GetDyn2StatInto();
    bool GetDyn2StatOut();
    bool GetDyn2StatOver();
    bool GetStat2DynInto();
    bool GetStat2DynOut();
    bool GetStat2DynOver();

private:
    bool dyn2statInto_ = false;
    bool dyn2statOut_  = false;
    bool dyn2statOver_ = false;

    bool stat2dynInto_ = false;
    bool stat2dynOut_  = false;
    bool stat2dynOver_ = false;

    std::mutex mutex_;
};

#endif // DEBUG_STEP_FLAGS_H