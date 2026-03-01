/* 
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


if (!globalThis.$262) {
    globalThis.$262 = {
        global: globalThis,
        evalScript(code) {
            try {
                global.evalScript(code);
                return { type: 'normal', value: undefined };
            } catch (e) {
                return { type: 'throw', value: e };
            }
        },
        gc() {
            global.gc();
        },
        getGlobal(name) {
            return global[name];
        },
        setGlobal(name, value) {
            global[name] = value;
        },
        agent: (function () {
            function thrower() {
                throw new Test262Error('agent.* not yet supported.');
            };
            return {
                start: thrower,
                broadcast: thrower,
                getReport: thrower,
                sleep: thrower,
                monotonicNow: thrower,
            };
        })(),
    };
}

$262.IsHTMLDDA = function () { };
$262.destroy = function () { };
$262.getGlobal = function (name) {
    return this.global[name];
};
$262.setGlobal = function (name, value) {
    this.global[name] = value;
};
$262.source = $SOURCE;
