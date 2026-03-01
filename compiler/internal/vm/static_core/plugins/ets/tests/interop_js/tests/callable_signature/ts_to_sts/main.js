'use strict';
/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
let callableProto = {
    _call: function (...arg) {
        let args = [];
        for (let _i = 0; _i < arg.length; _i++) {
            args[_i] = arg[_i];
        }
        return args;
    }
};
function createCallable() {
    let fn = function (...arg) {
        let args = [];
        for (let _i = 0; _i < arg.length; _i++) {
            args[_i] = arg[_i];
        }
        return fn._call.apply(fn, args);
    };
    Object.setPrototypeOf(fn, callableProto);
    return new Proxy(fn, {
        apply: function (target, thisArg, args) { return target._call.apply(target, args); }
    });
}
export let callableInstance = createCallable();
