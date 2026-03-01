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

class TestModule {
	constructor(name) {
		this.descriptorPrefix = 'L' + name.replaceAll('.', '/') + '/';
	}

	getClass(name) {
		return etsVm.getClass(this.descriptorPrefix + name + ';');
	}
	getFunction(name) {
		return etsVm.getFunction(this.descriptorPrefix + 'ETSGLOBAL;', name);
	}

	static descriptorPrefix;
}

function getTestModule(name) {
	return new TestModule(name);
}

module.exports = {
	etsVm,
	getTestModule,
};
