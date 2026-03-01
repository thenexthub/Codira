/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import hilog from '@ohos.hilog';

export function test_load_hilog() {
    let module = ESValue.load('@ohos.hilog');
    if (module.isUndefined()) {
        throw new Error('module not found');
    } else {
        let defaultModule = module.getProperty('default');
        arktest.assertEQ(defaultModule.isUndefined(), false);
        let info = defaultModule.getProperty('info');
        arktest.assertEQ(info.isFunction(), true);
        let val = info.invoke(0, 'tag', 'test');
        arktest.assertEQ(val.toBoolean(), true);
    }
}

export function test_call_hilog_function() {
    let val = hilog.info(0, 'tag', 'test2');
    arktest.assertEQ(val, true);
}