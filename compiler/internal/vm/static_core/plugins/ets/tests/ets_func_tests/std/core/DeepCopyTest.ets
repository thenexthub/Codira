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

function main(): int {
    const suite = new arktest.ArkTestsuite('deepcopy API tests for SDK');
    suite.addTest('Test DeepCopy Date', testdeepcopydate);
    suite.addTest('Test DeepCopy Map', testdeepcopymap);
    suite.addTest('Test DeepCopy Set', testdeepcopyset);
    suite.addTest('Test DeepCopy Array', testdeepcopyarray);
    suite.addTest('Test DeepCopy Enum', testdeepcopyenum);
    suite.addTest('Test DeepCopy Extends', testdeepcopyextends);
    suite.addTest('Test DeepCopy Error', testdeepcopyerror);
    return suite.run(); 
}

function testdeepcopydate() {
    let testDate = new Date(Date.UTC(1996, 1, 2, 3, 4, 5));
    let copiedDate = deepcopy(testDate) as Date;
    const expected = 'Fri, 02 Feb 1996 07:30:05 GMT';
    const copiedexpected = 'Fri, 02 Feb 1996 03:04:45 GMT';

    testDate.setUTCHours(7);
    testDate.setUTCMinutes(30);
    copiedDate.setUTCSeconds(45);
    arktest.assertEQ(expected, testDate.toUTCString(), `Expected: "${expected}", Actual: "${testDate.toUTCString()}"`);
    arktest.assertEQ(copiedexpected, copiedDate.toUTCString(), `Expected: "${copiedexpected}", Actual: "${copiedDate.toUTCString()}"`);
}

function testdeepcopymap() {
    let testMap1 : Map<number, string> = new Map<number, string>([[1, 'a'], [2, 'A']]);
    let testMap2 : Map<number, string> = new Map<number, string>([[1, 'b'], [2, 'B']]);
    let testMap3 : Map<number, string> = new Map<number, string>([[1, 'c'], [2, 'C']]);
    let testMap : Map<number, string> = new Map<number, string>([
        [1, 'a'], 
        [2, 'b'],
        [3, 'c']
    ]);
    let testMap4 = new Map<number, Map<number, string>>([
        [1, testMap1],
        [2, testMap2],
        [3, testMap3],
    ]);
    let copiedMap = deepcopy(testMap) as Map<number, string>;
    let copiedMap1 = deepcopy(testMap4) as Map<number, Map<number, string>>;
    const expected : Map<number, string> = new Map<number, string>([
        [1, 'A'], 
        [2, 'b'],
        [3, 'c'],
        [4, 'd']
    ]);
    const copiedexpected : Map<number, string> = new Map<number, string>([
        [1, 'a'], 
        [2, 'b'],
        [3, 'c']
    ]);
    const testMap5 : Map<number, string> = new Map<number, string>([[1, 'A'], [2, 'C']]);
    const expected1 = new Map<number, Map<number, string>>([
        [1, testMap1],
        [2, testMap2],
        [3, testMap5],
    ]);
    const testMap6 : Map<number, string> = new Map<number, string>([[1, 'c'], [2, 'C']]);
    const copiedexpected1 = new Map<number, Map<number, string>>([
        [1, testMap1],
        [2, testMap2],
        [3, testMap6],
    ]);

    testMap.set(1, 'A');
    testMap.set(4, 'd');
    testMap3.set(1, 'A');

    for (let i = 1; i <= testMap.size; i++) {
        arktest.assertEQ(expected.get(i), testMap.get(i), `Expected: "${expected.get(i)}", Actual: "${testMap.get(i)}"`);
    }

    for (let j = 1; j <= copiedMap.size; j++) {
        arktest.assertEQ(copiedexpected.get(j), copiedMap.get(j), `Expected: "${copiedexpected.get(j)}", Actual: "${copiedMap.get(j)}"`);
    }

    for (let m = 1; m <= testMap4.size; m++) {
        let temp = testMap4.get(m) as Map<number, string>;
        let temp1 = expected1.get(m) as Map<number, string>;
        for (let n = 1; n <= temp.size; n++) {
            arktest.assertEQ(temp1.get(n), temp.get(n), `Expected: "${temp1.get(n)}", Actual: "${temp.get(n)}"`);
        }
    }

    for (let p = 1; p <= copiedMap1.size; p++) {
        let temp = copiedMap1.get(p) as Map<number, string>;
        let temp1 = copiedexpected1.get(p) as Map<number, string>;
        for (let q = 1; q <= temp.size; q++) {
            arktest.assertEQ(temp1.get(q), temp.get(q), `Expected: "${temp1.get(q)}", Actual: "${temp.get(q)}"`);
        }
    }
}

class A implements Cloneable {
    public math: number;
    constructor(math: number) {
        this.math = math;
    }

    clone() : A {
        let instance = new A(99);
        instance.math = this.math;
        return instance;
    }
}

class B implements Cloneable {
    public english: number;
    constructor(english: number) {
        this.english = english;
    }

    clone() : B {
        let instance = new B(99);
        instance.english = this.english;
        return instance;
    }
}

function testdeepcopyset() {
    let testSet : Set<number> = new Set<number>([1, 2, 3, 4]);
    let copiedSet = deepcopy(testSet) as Set<number>;

    let classa = new A(110);
    let classb = new B(111);
    let testSet1 = new Set<A | B>([classa, classb]);
    let copiedSet1 = deepcopy(testSet1) as Set<A | B>;

    const expected : Set<number> = new Set<number>([1, 2, 3, 4, 5]);
    const copiedexpected : Set<number> = new Set<number>([1, 2, 3, 4]);
    const expectedmath = 120;
    const expectedenglish = 130;
    const copiedexpectedmath = 110;
    const copiedexpectedenglish = 111;

    testSet.add(5);
    classa.math = 120;
    classb.english = 130;

    for (const item of testSet1) {
        if (item instanceof A) {
            arktest.assertEQ(expectedmath, item.math, `Expected: "${expectedmath}", Actual: "${item.math}"`);
        } else if (item instanceof B) {
            arktest.assertEQ(expectedenglish, item.english, `Expected: "${expectedenglish}", Actual: "${item.english}"`);
        }
    }

    for (const item of copiedSet1) {
        if (item instanceof A) {
            arktest.assertEQ(copiedexpectedmath, item.math, `Expected: "${copiedexpectedmath}", Actual: "${item.math}"`);
        } else if (item instanceof B) {
            arktest.assertEQ(copiedexpectedenglish, item.english, `Expected: "${copiedexpectedenglish}", Actual: "${item.english}"`);
        }
    }

    if (testSet.size == expected.size) {
        for (let item of testSet) {
            if (!expected.has(item)) {
                arktest.assertEQ(expected, testSet, `Expected: "${expected}", Actual: "${testSet}"`);
            }
        }
    } else {
        arktest.assertEQ(expected, testSet, `Expected: "${expected}", Actual: "${testSet}"`);
    }

    if (copiedSet.size == copiedexpected.size) {
        for (let item of copiedSet) {
            if (!copiedexpected.has(item)) {
                arktest.assertEQ(copiedexpected, copiedSet, `Expected: "${copiedexpected}", Actual: "${copiedSet}"`);
            }
        }
    } else {
        arktest.assertEQ(copiedexpected, copiedSet, `Expected: "${copiedexpected}", Actual: "${copiedSet}"`);
    }
}

function testdeepcopyarray() {
    let testArray : number[] = [1, 2, 3, 4];
    let copiedArray = deepcopy(testArray) as Array<number>;
    const expected : number[] = [1, 2, 3, 4, 5];
    const copiedexpected : number[] = [1, 2, 3, 4];

    testArray.push(5);

    for (let i = 0; i < testArray.length; i++) {
        arktest.assertEQ(expected[i], testArray[i], `Expected: "${expected[i]}", Actual: "${testArray[i]}"`);
    }

    for (let j = 0; j < copiedArray.length; j++) {
        arktest.assertEQ(copiedexpected[j], copiedArray[j], `Expected: "${copiedexpected[j]}", Actual: "${copiedArray[j]}"`);
    }
}

enum MyColor {
    White = 0
}

function testdeepcopyenum() {
    let mycolor = MyColor.White;
    let copiedcolor = deepcopy(mycolor) as MyColor;
    const expected = MyColor.White;

    arktest.assertEQ(expected, mycolor, `Expected: "${expected}", Actual: "${mycolor}"`);
    arktest.assertEQ(expected, copiedcolor, `Expected: "${expected}", Actual: "${copiedcolor}"`);
}

class C extends A {
    public physics: number;
    constructor(math: number, physics: number) {
        super(math);
        this.math = math;
        this.physics = physics;
    }

    clone() : C {
        let instance = new C(99, 99);
        instance.math = this.math;
        instance.physics = this.physics;
        return instance;
    }
}

class D extends A {
    public physics: number;
    constructor(math: number, physics: number) {
        super(math);
        this.math = math;
        this.physics = physics;
    }

    clone() : D {
        let instance = new D(99, 99);
        instance.math = this.math;
        return instance;
    }
}

//Test that user can customize certain member variables in the copy class
function testdeepcopyextends() {
    let classc = new C(100, 100);
    let cloneclassc = classc.clone() as C;
    let deepcopyclassc = deepcopy(classc) as C;

    const expectedmath = 100;
    const expectedphysics = 111;
    const copiedexpectedmath = 100;
    const copiedexpectedphysics = 100;

    let classd = new D(100, 100);
    let cloneclassd = classd.clone() as D;
    let deepcopyclassd = deepcopy(classd) as D; 

    const expectedmath1 = 122;
    const expectedphysics1 = 100;
    const copiedexpectedmath1 = 100;
    const copiedexpectedphysics1 = 99;

    classc.physics = 111;
    classd.math = 122;

    arktest.assertEQ(expectedmath, classc.math, `Expected: "${expectedmath}", Actual: "${classc.math}"`);
    arktest.assertEQ(expectedphysics, classc.physics, `Expected: "${expectedphysics}", Actual: "${classc.physics}"`);
    arktest.assertEQ(copiedexpectedmath, cloneclassc.math, `Expected: "${copiedexpectedmath}", Actual: "${cloneclassc.math}"`);
    arktest.assertEQ(copiedexpectedphysics, cloneclassc.physics, `Expected: "${copiedexpectedphysics}", Actual: "${cloneclassc.physics}"`);
    arktest.assertEQ(copiedexpectedmath, deepcopyclassc.math, `Expected: "${copiedexpectedmath}", Actual: "${deepcopyclassc.math}"`);
    arktest.assertEQ(copiedexpectedphysics, deepcopyclassc.physics, `Expected: "${copiedexpectedphysics}", Actual: "${deepcopyclassc.physics}"`);
    
    arktest.assertEQ(expectedmath1, classd.math, `Expected: "${expectedmath1}", Actual: "${classd.math}"`);
    arktest.assertEQ(expectedphysics1, classd.physics, `Expected: "${expectedphysics1}", Actual: "${classd.physics}"`);
    arktest.assertEQ(copiedexpectedmath1, cloneclassd.math, `Expected: "${copiedexpectedmath1}", Actual: "${cloneclassd.math}"`);
    arktest.assertEQ(copiedexpectedphysics1, cloneclassd.physics, `Expected: "${copiedexpectedphysics1}", Actual: "${cloneclassd.physics}"`);
    arktest.assertEQ(copiedexpectedmath1, deepcopyclassd.math, `Expected: "${copiedexpectedmath1}", Actual: "${deepcopyclassd.math}"`);
    arktest.assertEQ(copiedexpectedphysics1, deepcopyclassd.physics, `Expected: "${copiedexpectedphysics1}", Actual: "${deepcopyclassd.physics}"`);
}

class E extends A {
    public physics: number;
    constructor(math: number, physics: number) {
        super(math);
        this.math = math;
        this.physics = physics;
    } 
}

//Tests that inherit from the parent's children do not override clone with an error
function testdeepcopyerror() {
    let a : A = new A(100);
    let copieda = deepcopy(a);

    let expected = "Error: class DeepCopyTest.E doesn't override clone";

    try {
        let b : E = new E(100, 100);
        let copyC = deepcopy(b);
    } catch (error) {
        const errorString = `${error}`;
        arktest.assertEQ(expected, errorString, `Expected: "${expected}", Actual: "${errorString}"`);
    }

    try {
        let c : A = new E(100, 100);
        let copyC = deepcopy(c);
    } catch (error) {
        const errorString = `${error}`;
        arktest.assertEQ(expected, errorString, `Expected: "${expected}", Actual: "${errorString}"`);
    }
}
