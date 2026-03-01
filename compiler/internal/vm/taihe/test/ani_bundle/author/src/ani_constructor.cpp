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
#include "abilityInfo.ani.hpp"
#include "applicationInfo.ani.hpp"
#include "bundleInfo.ani.hpp"
#include "customizeData.ani.hpp"
#include "elementName.ani.hpp"
#include "hapModuleInfo.ani.hpp"
#include "moduleInfo.ani.hpp"
#include "ohos_app_ability_want.ani.hpp"
#include "ohos_bundle.ani.hpp"
#include "shortcutInfo.ani.hpp"
#if __has_include(<ani.h>)
#include <ani.h>
#elif __has_include(<ani/ani.h>)
#include <ani/ani.h>
#else
#error "ani.h not found. Please ensure the Ani SDK is correctly installed."
#endif
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    if (ANI_OK != customizeData::ANIRegister(env)) {
        std::cerr << "Error from customizeData::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != ohos_app_ability_want::ANIRegister(env)) {
        std::cerr << "Error from ohos_app_ability_want::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != moduleInfo::ANIRegister(env)) {
        std::cerr << "Error from moduleInfo::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != elementName::ANIRegister(env)) {
        std::cerr << "Error from elementName::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != abilityInfo::ANIRegister(env)) {
        std::cerr << "Error from abilityInfo::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != shortcutInfo::ANIRegister(env)) {
        std::cerr << "Error from shortcutInfo::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != ohos_bundle::ANIRegister(env)) {
        std::cerr << "Error from ohos_bundle::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != hapModuleInfo::ANIRegister(env)) {
        std::cerr << "Error from hapModuleInfo::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != applicationInfo::ANIRegister(env)) {
        std::cerr << "Error from applicationInfo::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != bundleInfo::ANIRegister(env)) {
        std::cerr << "Error from bundleInfo::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}