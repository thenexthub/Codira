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

const {
    UserClass,
    createUserClassFromSts,
    ChildClass,
    createChildClassFromSts,
    InterfaceClass,
    createInterfaceClassFromSts,
    PrivateClass,
    createPrivateClassFromSts,
    ProtectedClass,
    createProtectedClassFromSts,
    ChildProtectedClass,
    createChildProtectedClassFromSts,
    ChildAbstractClass,
    createChildAbstractClassFromSts,
} = require('class_method.test.abc');

function checkUserClassMethod() {
    const ETSClass = new UserClass();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreateUserClassFromSts() {
    const ETSClass = createUserClassFromSts();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkProtectedClassMethod() {
    const ETSClass = new ProtectedClass();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreateProtectedClassFromSts() {
    const ETSClass = createProtectedClassFromSts();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkPrivateClassMethod() {
    const ETSClass = new PrivateClass();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreatePrivateClassFromSts() {
    const ETSClass = createPrivateClassFromSts();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkChildProtectedClassMethod() {
    const ETSClass = new ChildProtectedClass();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreateChildProtectedClassFromSts() {
    const ETSClass = createChildProtectedClassFromSts();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkChildClassMethod() {
    const ETSClass = new ChildClass();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreateChildClassFromSts() {
    const ETSClass = createChildClassFromSts();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkChildClassOwnMethod() {
    const ETSClass = new ChildClass();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreateChildClassOwnMethodFromSts() {
    const ETSClass = createChildClassFromSts();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkAbstractClassMethod() {
    const ETSClass = new ChildAbstractClass();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreateChildAbstractClassFromSts() {
    const ETSClass = createChildAbstractClassFromSts();
    const AnotherClass = new InterfaceClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkInterfaceClassMethod() {
    const ETSClass = new InterfaceClass();
    const AnotherClass = new UserClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}

function checkCreateInterfaceClassFromSts() {
    const ETSClass = createInterfaceClassFromSts();
    const AnotherClass = new UserClass();

    try {
        ETSClass.compare(AnotherClass);
    } catch (e) {
        ASSERT_TRUE(true);
        return;
    }
    ASSERT_TRUE(false);
}


checkUserClassMethod();
checkCreateUserClassFromSts();
checkProtectedClassMethod();
checkCreateProtectedClassFromSts();
checkPrivateClassMethod();
checkCreatePrivateClassFromSts();
checkChildProtectedClassMethod();
checkCreateChildProtectedClassFromSts();
checkChildClassMethod();
checkCreateChildClassFromSts();
checkChildClassOwnMethod();
checkCreateChildClassOwnMethodFromSts();
checkAbstractClassMethod();
checkCreateChildAbstractClassFromSts();
checkInterfaceClassMethod();
checkCreateInterfaceClassFromSts();
