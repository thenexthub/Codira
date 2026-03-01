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
export let tsInt = 1;
export let notFound = 'Not found';
export let handler = {
    get: function (target, property) {
        if (!isNaN(property)) {
            return target.items[property];
        }
        if (typeof target[property] === 'function') {
            return target[property]();
        }
        if (target[property]) {
            return target[property];
        }
        return notFound;
    },
    set: function (target, property, value) {
        if (!isNaN(property)) {
            target.items[property] = value;
            return true;
        }
        target[property] = value;
        return true;
    }
};
export let SimpleObject = {
    int: tsInt,
    items: [],
    add: function () { return tsInt + tsInt; },
};
