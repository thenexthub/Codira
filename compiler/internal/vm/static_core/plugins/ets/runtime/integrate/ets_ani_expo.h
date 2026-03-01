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
#ifndef PLUGINS_ETS_RUNTIME_INTEGRATE_ETS_ANI_EXPO
#define PLUGINS_ETS_RUNTIME_INTEGRATE_ETS_ANI_EXPO
// NOLINTBEGIN

#ifdef __cplusplus
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#else
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#endif
#include <string>
#include <vector>
#include <ani.h>

#ifndef PANDA_TARGET_WINDOWS
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDA_ETS_ANI_EXPO_PUBLIC_API __attribute__((visibility("default")))
#else
#define PANDA_ETS_ANI_EXPO_PUBLIC_API __declspec(dllexport)
#endif

namespace ark::ets {
class PANDA_ETS_ANI_EXPO_PUBLIC_API ETSAni {
public:
    static constexpr std::string_view AOT_FILE_OPTION_PREFIX = "--ext:--aot-file=";
    static constexpr std::string_view INTEROP_OPTION_PREFIX = "--ext:interop";
    static constexpr std::string_view ENABLE_AN_OPTION = "--ext:--enable-an";
    static constexpr std::string_view ETSSTDLIB_ABC = "/system/framework/etsstdlib_bootabc.abc";
    static void Prefork(ani_env *env, void *napienv);
    static void Postfork(ani_env *env, const std::vector<ani_option> &options, bool postZygoteFork = true);

private:
    static void TryLoadAotFileForBoot();
    static void LoadAotFileForApp(std::string const &aotFileName);
    /**
     * @brief Pre create exclusive worker for taskpool execution engine
     * @return true if success, false if failed
     */
    static bool PreCreateExclusiveWorkerForTaskpool();

    /**
     * @brief Destroy exclusive worker for taskpool execution engine in preFork
     * @return true if success, false if failed
     */
    static bool DestroyExclusiveWorkerForTaskpoolIfExists();

    /**
     * @brief Process taskpool worker in preFork and postFork
     * @param preFork true if preFork, false if postFork
     */
    static void ProcessTaskpoolWorker(bool preFork);
};
}  // namespace ark::ets
#endif  // !PLUGINS_ETS_RUNTIME_INTEGRATE_ETS_ANI_EXPO
