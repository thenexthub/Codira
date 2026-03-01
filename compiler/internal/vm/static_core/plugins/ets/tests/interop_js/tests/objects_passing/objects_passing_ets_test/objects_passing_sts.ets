/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

package objects_passing_ets_test


interface TestUserType {
    name: string;
    age: number;
    showInfo(): string;
}

interface TestObjectType {
    name: string;

}

interface TestInnerType {
    id: number;

}

interface TestOuterType {
    id: number;
    TestInnerType:TestInnerType

}



function getClassUserName(obj:TestUserType){
    return obj.name;

}

function getClassUserAge(obj:TestUserType){
    return obj.age;

}

function getClassUserInfo(obj:TestUserType){
    return obj.showInfo();

}

function getObjectName(obj:TestObjectType){
    return obj.name
}

function getOuterObject(obj:TestOuterType){
    return obj.TestInnerType.id
}




function GCODESRuntimeCleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
}