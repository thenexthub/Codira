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

import AbilityConstant from "@ohos.app.ability.AbilityConstant";
import hilog from "@ohos.hilog";
import UIAbility from "@ohos.app.ability.UIAbility";
import Want from "@ohos.app.ability.Want";
import window from "@ohos.window";
import { InitEtsRuntime } from "./EtsRuntime";

export default class ${BENCH_NAME}_Ability extends UIAbility {
    onCreate(want: Want, launchParam: AbilityConstant.LaunchParam) {
        AppStorage.SetOrCreate<string>("runtimeMode", "$BENCH_MODE");
        hilog.info(0, "VMB", "%{public}s", "ABILITY ON CREATE");
        InitEtsRuntime("$BENCH_FILE_SO", "$BENCH_MODE");
        hilog.info(0, "VMB", "%{public}s", "ETS RUNTIME INIT");
    }

    onWindowStageCreate(windowStage: window.WindowStage) {
        hilog.info(0, "VMB", "%{public}s", "WINDOW CREATE");
        windowStage.loadContent("pages/Index", (err, data) => {
            if (err.code) {
                hilog.info(0, "VMB", "ERROR LOADING PAGE: %{public}s",
                           JSON.stringify(err) ?? "");
            } else {
                hilog.info(0, "VMB", "%{public}s", "PAGE LOADED");
            }
        });
        const launcher = globalThis.Panda.getClass("L$BENCH_NAME/VmbLauncher;");
        launcher.main();
        hilog.info(0, "VMB", "%{public}s", "VMB MAIN FINISHED");
    }

    onDestroy() {
        hilog.info(0, "VMB", "%{public}s", "ABILITY ON DESTROY");
    }
}