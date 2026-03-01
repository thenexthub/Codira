/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

'use strict';
Object.defineProperty(exports, '__esModule', { value: true });
const foo1 = require('./foo.abc');
function test() {
	const arr = new Array();
	arr.push(new foo1.FooClass('zero'));
	arr.push(new foo1.FooClass('one'));
	arr.push(new foo1.FooClass('two'));
	print('test: ' + (0, foo1.FooFunction)(arr));
	const arr2 = (0, foo1.BarFunction)();
	print('test: check instanceof Array: ' + (arr2 instanceof Array));
	print('test: ' + arr2.at(0).name);
	print('test: ' + arr2.at(1).name);
	print('test: ' + arr2.at(2).name);
}
test();
