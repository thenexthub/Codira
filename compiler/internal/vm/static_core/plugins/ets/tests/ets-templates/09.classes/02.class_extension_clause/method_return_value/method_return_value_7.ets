/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    In multi-layer inheritance, the parent class method is called in the subclass method method,
    and the modifier is internal. Verify that the rewrite works correctly.
---*/

class GrandParent {
    getValue(): number {
        return 100;
    }
}

class Parent extends GrandParent {
    getValue(): number {
        return super.getValue() + 100;
    }
}

class Child extends Parent {
    getValue(): number {
        return super.getValue() + 100;
    }
}

function main(): void {
    let grandParent = new GrandParent();
    let grandParentValue: number = grandParent.getValue();
    arktest.assertEQ(grandParentValue, 100);

    let parent = new Parent();
    let parentValue: number = parent.getValue();
    arktest.assertEQ(parentValue, 200);

    let child = new Child();
    let childValue: number = child.getValue();
    arktest.assertEQ(childValue, 300);
}