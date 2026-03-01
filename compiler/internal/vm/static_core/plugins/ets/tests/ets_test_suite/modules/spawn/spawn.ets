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

/*---
  tags: [not-a-test]
  files: [ability.ets]
  arktsconfig: arktsconfig.json
---*/

import { Ability } from "./ability";

native function getAppAbcFiles(): string[];

function getNativeLibName(): string {
    let procManager = new StdProcess.ProcessManager();
    return procManager.getEnvironmentVar("NATIVE_LIB_NAME");
}

function getAppMainAbilityClassName(): string {
    let procManager = new StdProcess.ProcessManager();
    return procManager.getEnvironmentVar("APP_MAIN_ABILITY_CLASS");
}

function loadAndInvokeAbility(mainAbilityClassName: string): void {
    const appRuntimeLinker = new AbcRuntimeLinker(undefined, getAppAbcFiles());
    const mainAbilityClass = appRuntimeLinker.loadClass(mainAbilityClassName);
    const mainAbility = mainAbilityClass.createInstance() as Ability;
    mainAbility.invoke();
}

function main() {
    loadLibrary(getNativeLibName());

    loadAndInvokeAbility(getAppMainAbilityClassName());

    return 0;
}

function abilityExpectFailure(cb: (e: Error) => void) {
    loadLibrary(getNativeLibName())
    try {
        loadAndInvokeAbility(getAppMainAbilityClassName())
    } catch (e) {
        cb(e)
        return 0
    }
    throw new Error("Error expected, but nothing was thrown")
}

function abilityExpectVerificationError(expectedMessage: string) {
    abilityExpectFailure((e: Error) => {
        arktest.assertTrue(e instanceof LinkerVerificationError)
        arktest.assertEQ(e.message, expectedMessage);
    })
}

function mainCatchLinkerUnresolvedClassError() {
    abilityExpectFailure((e: Error) => {
        arktest.assertTrue(e instanceof LinkerVerificationError)
        console.log(e.message);
    })
}

function mainCatchLinkerBadSupertypeError() {
    abilityExpectFailure((e: Error) => {
        arktest.assertTrue(e instanceof LinkerBadSupertypeError)
        console.log(e.message);
    })
}

function mainMisatchedExtendsFinal() {
    abilityExpectVerificationError("L@mismatched/extends_final/ExtendsFinal;")
}

function mainMisatchedOverrideFinal() {
    abilityExpectVerificationError("L@app/module5/MainAbility;")
}

function mainCircularityExtends() {
    abilityExpectVerificationError("L@app/module6/MainAbility;")
}

function mainMultipleOverrideBase() {
    abilityExpectVerificationError("L@app/module/MainAbility;");
}

function mainMultipleOverrideBase1() {
    abilityExpectVerificationError("L@app/module1/MainAbility;");
}

function mainVerificationModeOnTheFly() {
    abilityExpectVerificationError("@verification_mode.bar.ETSGLOBAL::bar");
}

function mainVerificationModeAheadOfTime() {
    abilityExpectVerificationError("L@verification_mode/bar/ETSGLOBAL;");
}

function mainMultipleImplement() {
    abilityExpectVerificationError("L@app/module/MainAbility;");
}

function mainMultipleImplement1() {
    abilityExpectVerificationError("L@app/module1/MainAbility;");
}
