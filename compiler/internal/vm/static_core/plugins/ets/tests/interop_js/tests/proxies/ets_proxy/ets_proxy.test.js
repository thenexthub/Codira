/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const etsVm = globalThis.gtest.etsVm;

function getTestClass(name) {
	return etsVm.getClass('Lclass_proxy/test/' + name + ';');
}

function getTestFunction(name) {
	return etsVm.getFunction('Lclass_proxy/test/ETSGLOBAL;', name);
}

module.exports = {
	getTestClass,
	getTestFunction,
};
