/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function testModifyErrorCode() {
    let err = new Error();
    arktest.assertEQ(err.code, 0);

    err.code = 10;
    arktest.assertEQ(err.code, 10);
}

function testConstructor01() {
    let message = 'message'
    let cause = 'cause'
    try {
        throw new Error();
    } catch (err) {
        arktest.assertEQ(err.code, 0);
        arktest.assertEQ(err.message, '');
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error(message);
    } catch (err) {
        arktest.assertEQ(err.code, 0);
        arktest.assertEQ(err.message, message);
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error(message, {cause: cause} as ErrorOptions);
    } catch (err) {
        arktest.assertEQ(err.code, 0);
        arktest.assertEQ(err.message, message);
        arktest.assertEQ(err.cause, cause);
    }
}

function testConstructor02() {
    let code: int = 10
    let message = 'message'
    let cause = 'cause'
    try {
        throw new Error(code);
    } catch (err) {
        arktest.assertEQ(err.code, code);
        arktest.assertEQ(err.message, '');
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error(code, message);
    } catch (err) {
        arktest.assertEQ(err.code, code);
        arktest.assertEQ(err.message, message);
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error(code, message, {cause: cause} as ErrorOptions);
    } catch (err) {
        arktest.assertEQ(err.code, code);
        arktest.assertEQ(err.message, message);
        arktest.assertEQ(err.cause, cause);
    }
}

function testConstructor03() {
    let message = 'message'
    let cause = 'cause'
    try {
        throw new Error('name', undefined, undefined);
    } catch (err) {
        arktest.assertEQ(err.code, 0);
        arktest.assertEQ(err.message, '');
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error('name', message, undefined);
    } catch (err) {
        arktest.assertEQ(err.code, 0);
        arktest.assertEQ(err.message, message);
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error('name', undefined, {cause: cause} as ErrorOptions);
    } catch (err) {
        arktest.assertEQ(err.code, 0);
        arktest.assertEQ(err.message, '');
        arktest.assertEQ(err.cause, cause);
    }

    try {
        throw new Error('name', message, {cause: cause} as ErrorOptions);
    } catch (err) {
        arktest.assertEQ(err.code, 0);
        arktest.assertEQ(err.message, message);
        arktest.assertEQ(err.cause, cause);
    }
}

function testConstructor04() {
    let code: int = 10
    let message = 'message'
    let cause = 'cause'
    try {
        throw new Error('name', code, undefined, undefined);
    } catch (err) {
        arktest.assertEQ(err.code, code);
        arktest.assertEQ(err.message, '');
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error('name', code, message, undefined);
    } catch (err) {
        arktest.assertEQ(err.code, code);
        arktest.assertEQ(err.message, message);
        arktest.assertTrue(err.cause === undefined);
    }

    try {
        throw new Error('name', code, undefined, {cause: cause} as ErrorOptions);
    } catch (err) {
        arktest.assertEQ(err.code, code);
        arktest.assertEQ(err.message, '');
        arktest.assertEQ(err.cause, cause);
    }

    try {
        throw new Error('name', code, message, {cause: cause} as ErrorOptions);
    } catch (err) {
        arktest.assertEQ(err.code, code);
        arktest.assertEQ(err.message, message);
        arktest.assertEQ(err.cause, cause);
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Error code tests');
    suite.addTest('Test modify error code', testModifyErrorCode);
    suite.addTest('Test constructor01', testConstructor01);
    suite.addTest('Test constructor02', testConstructor02);
    suite.addTest('Test constructor03', testConstructor03);
    suite.addTest('Test constructor04', testConstructor04);
    return suite.run();
}
