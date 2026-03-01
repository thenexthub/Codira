/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

function NonMovableTest(): void {
    arktest.assertEQ(GC.getObjectSpaceType("spaceTypeTest"), GC.ObjectSpaceType.NON_MOVABLE, "Must be non movable space");
}

function AllocateAndMoveTest(): void {
    let obj: Object = new Object();
    arktest.assertEQ(GC.getObjectSpaceType(obj), GC.ObjectSpaceType.YOUNG, "Must be young space");

    let gc_id = GC.startGC(GC.Cause.YOUNG);
    GC.waitForFinishGC(gc_id);

    arktest.assertEQ(GC.getObjectSpaceType(obj), GC.ObjectSpaceType.TENURED, "Must be tenured space");
}

function HumongousObjectTest(): void {
    let h_obj: FixedArray<long> = new FixedArray<long>(1024 * 1024);
    arktest.assertEQ(GC.getObjectSpaceType(h_obj), GC.ObjectSpaceType.HUMONGOUS, "Must be humongous space");
}

/**
 * Tests for getObjectSpaceType builtin.
 */
function main(): int {
    try {
        AllocateAndMoveTest();
        NonMovableTest();
        HumongousObjectTest();
    }
    catch (e) {
        arktest.assertTrue(false, "Space allocation test must not throw errors");
    }
    return 0;
}
