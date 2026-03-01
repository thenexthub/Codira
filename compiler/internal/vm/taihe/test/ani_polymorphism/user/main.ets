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
import {BusinessError} from "@ohos.base";
import * as poly from "poly";

loadLibrary("ani_polymorphism");

function ifacePolyTest() {
    let unknownObj = poly.getAnimal(poly.AnimalTag.ANIMAL);
    let dogObj = poly.getAnimal(poly.AnimalTag.DOG);
    let catObj = poly.getAnimal(poly.AnimalTag.CAT);
    arktest.assertEQ(poly.interactAnimal(unknownObj), "Unknown Animal");
    arktest.assertEQ(poly.interactAnimal(dogObj), "Dog");
    arktest.assertEQ(poly.interactAnimal(catObj), "Cat");
}

function structPolyTest() {
    let unknownPer = poly.getPerson(poly.PersonTag.PERSON, "Mike", 18);
    let workerPer = poly.getPerson(poly.PersonTag.WORKER, "Tom", 30);
    let studentPer = poly.getPerson(poly.PersonTag.STUDENT, "jimmy", 20);
    arktest.assertEQ(poly.introduceYourself(unknownPer), "Unknown Person");
    arktest.assertEQ(poly.introduceYourself(workerPer), "Worker");
    arktest.assertEQ(poly.introduceYourself(studentPer), "Student");
}

function main() {
    const suite = new arktest.ArkTestsuite("Polymorphism Test");
    suite.addTest("ifacePolyTest", ifacePolyTest);
    suite.addTest("structPolyTest", structPolyTest);
    exit(suite.run());
}