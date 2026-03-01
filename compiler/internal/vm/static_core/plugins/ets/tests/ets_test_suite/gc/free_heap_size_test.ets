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

function AllocMemoryAndReturnFreeHeapSize(): long {
    const ALLOC_SIZE: int = 10000;
    let mem: FixedArray<long> = new FixedArray<long>(ALLOC_SIZE);
    return GC.getFreeHeapSize();
}

function main(): void {
    // Free allocations done during runtime creation (i.e. application RuntimeLinker)
    GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
    let free_size_in_start = GC.getFreeHeapSize();
    let free_size_after_new = AllocMemoryAndReturnFreeHeapSize();
    GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
    let free_size_after_gc = GC.getFreeHeapSize();

    arktest.assertLT(0, free_size_in_start);
    arktest.assertLT(free_size_after_new, free_size_in_start, "After allocation free memory value should decrease");
    arktest.assertLT(free_size_after_new, free_size_after_gc, "After in place gc free memory value should increase");
    arktest.assertLE(free_size_after_gc, free_size_in_start);
}
