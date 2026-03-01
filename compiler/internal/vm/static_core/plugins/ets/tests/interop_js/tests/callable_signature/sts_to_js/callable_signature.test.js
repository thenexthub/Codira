
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

const stsVm = globalThis.gtest.etsVm;

const jsInt = 1;
const jsString = 'Test';

const InvokeClass = stsVm.getClass('Lcallable_signature/test/InvokeClass;');
const InstantiateClass = stsVm.getClass('Lcallable_signature/test/InstantiateClass;');

const invokeMethodFromSts = stsVm.getFunction('Lcallable_signature/test/ETSGLOBAL;', 'invokeMethodFromSts');
const instantiateMethodFromSts = stsVm.getFunction('Lcallable_signature/test/ETSGLOBAL;', 'instantiateMethodFromSts');

module.exports = {
    jsInt,
    jsString,
    invokeMethodFromSts,
    instantiateMethodFromSts,
    InvokeClass,
    InstantiateClass,
};
