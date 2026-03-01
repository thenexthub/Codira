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
print(stsVm);

const UserClass = stsVm.getClass('Lclass_method/test/UserClass;');

const createUserClassFromSts = stsVm.getFunction('Lclass_method/test/ETSGLOBAL;', 'createUserClassFromSts');
print(createUserClassFromSts);

const ChildClass = stsVm.getClass('Lclass_method/test/ChildClass;');
const createChildClassFromSts = stsVm.getFunction('Lclass_method/test/ETSGLOBAL;', 'createChildClassFromSts');

const InterfaceClass = stsVm.getClass('Lclass_method/test/InterfaceClass;');
const createInterfaceClassFromSts = stsVm.getFunction('Lclass_method/test/ETSGLOBAL;', 'createInterfaceClassFromSts');

const PrivateClass = stsVm.getClass('Lclass_method/test/PrivateClass;');
const createPrivateClassFromSts = stsVm.getFunction('Lclass_method/test/ETSGLOBAL;', 'createPrivateClassFromSts');

const ProtectedClass = stsVm.getClass('Lclass_method/test/ProtectedClass;');
const createProtectedClassFromSts = stsVm.getFunction('Lclass_method/test/ETSGLOBAL;', 'createProtectedClassFromSts');

const ChildProtectedClass = stsVm.getClass('Lclass_method/test/ChildProtectedClass;');
const createChildProtectedClassFromSts = stsVm.getFunction('Lclass_method/test/ETSGLOBAL;', 'createChildProtectedClassFromSts');

const ChildAbstractClass = stsVm.getClass('Lclass_method/test/ChildAbstractClass;');
const createChildAbstractClassFromSts = stsVm.getFunction('Lclass_method/test/ETSGLOBAL;', 'createChildAbstractClassFromSts');

module.exports = {
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
};
