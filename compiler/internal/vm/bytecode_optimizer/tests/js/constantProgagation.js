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

function sccpFoldGreater()
{
    let a = 2;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a > b);
    a = intMaxVal;
    print(a > b);

    a = true;
    b = false;
    print(a > b);

    a = 2.1;
    b = 2.2;
    print(a > b);
}

function sccpFoldGreaterNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a > b);
    b = false;
    print(a > b);

    a = 1.1;
    b = 1;
    print(a > b);

    b = true;
    print(a > b);
    b = intMaxVal;
    print(a > b);

    a = true;
    print(a > b);
    b = 1.1;
    print(a > b);
    b = 1;
    print(a > b);
}

function sccpFoldGreaterEq()
{
    let a = 2;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a >= b);
    a = intMaxVal;
    print(a >= b);
    a = 1;
    print(a >= b);

    a = true;
    b = false;
    print(a >= b);
    b = true;
    print(a >= b);

    a = 2.1;
    b = 2.2;
    print(a >= b);
    b = 2.1;
    print(a >= b);
}

function sccpFoldGreaterEqNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a >= b);
    b = false;
    print(a >= b);

    a = 1.1;
    b = 1;
    print(a >= b);
    b = true;
    print(a >= b);
    b = intMaxVal;
    print(a >= b);

    a = true;
    print(a >= b);
    b = 1.1;
    print(a >= b);
    b = 1;
    print(a >= b);
}

function sccpFoldLess()
{
    let a = 2;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a < b);
    a = intMaxVal;
    print(a < b);

    a = true;
    b = false;
    print(a < b);

    a = 2.1;
    b = 2.2;
    print(a < b);
}

function sccpFoldLessNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a < b);
    b = false;
    print(a < b);

    a = 1.1;
    b = 1;
    print(a < b);
    b = true;
    print(a < b);
    b = intMaxVal;
    print(a < b);

    a = true;
    print(a < b);
    b = 1.1;
    print(a < b);
    b = 1;
    print(a < b);
}

function sccpFoldLessEq()
{
    let a = 2;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a <= b);
    a = intMaxVal;
    print(a <= b);
    a = 1;
    print(a <= b);

    a = false;
    b = true;
    print(a <= b);
    b = false;
    print(a <= b);

    a = 3.1;
    b = 3.2;
    print(a <= b);
    b = 3.1;
    print(a <= b);
}

function sccpFoldLessEqNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a <= b);
    b = false;
    print(a <= b);

    a = 1.1;
    b = 1;
    print(a <= b);
    b = true;
    print(a <= b);
    b = intMaxVal;
    print(a <= b);

    a = true;
    print(a <= b);
    b = 1.1;
    print(a <= b);
    b = 1;
    print(a < b);
}

function sccpFoldEq()
{
    let a = 1;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a == b);
    print(a == intMaxVal);

    a = true;
    b = true;
    print(a == b);

    a = 3.2;
    b = 3.3;
    print(a == b);
}

function sccpFoldEqNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a == b);
    b = false;
    print(a == b);

    a = 1.1;
    b = 1;
    print(a == b);
    b = true;
    print(a == b);
    b = intMaxVal;
    print(a == b);

    a = true;
    print(a == b);
    b = 1.1;
    print(a == b);
    b = 1;
    print(a == b);
}

function sccpFoldStrictEq()
{
    let a = 1;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a === b);
    print(a === intMaxVal);

    a = true;
    b = true;
    print(a === b);

    a = 3.2;
    b = 3.3;
    print(a === b);
}

function sccpFoldStrictEqNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a === b);
    b = false;
    print(a === b);

    a = 1.1;
    b = 1;
    print(a === b);
    b = true;
    print(a === b);
    b = intMaxVal;
    print(a === b);

    a = true;
    print(a === b);
    b = 1.1;
    print(a === b);
    b = 1;
    print(a === b);
}

function sccpFoldNotEq()
{
    let a = 1;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a != b);
    print(a != intMaxVal);

    a = true;
    b = true;
    print(a != b);

    a = 3.2;
    b = 3.3;
    print(a != b);
}

function sccpFoldNotEqNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a != b);
    b = false;
    print(a != b);

    a = 1.1;
    b = 1;
    print(a != b);
    b = true;
    print(a != b);
    b = intMaxVal;
    print(a != b);

    a = true;
    print(a != b);
    b = 1.1;
    print(a != b);
    b = 1;
    print(a != b);
}

function sccpFoldStrictNotEq()
{
    let a = 1;
    let b = 1;
    let intMaxVal = 2147483647;
    print(a !== b);
    print(a !== intMaxVal);

    a = true;
    b = true;
    print(a !== b);

    a = 3.2;
    b = 3.3;
    print(a !== b);

}

function sccpFoldStrictNotEqNoEffect()
{
    let a = 2;
    let b = 1.1;
    let intMaxVal = 2147483647;
    print(a !== b);
    b = false;
    print(a !== b);

    a = 1.1;
    b = 1;
    print(a !== b);
    b = true;
    print(a !== b);
    b = intMaxVal;
    print(a !== b);

    a = true;
    print(a !== b);
    b = 1.1;
    print(a !== b);
    b = 1;
    print(a !== b);
}

function sccpNoEffectNanWithInt() {
    let a = NaN;
    let intVal = 1;
    let intMaxVal = 2147483647;
    let intMinVal = -2147483648;
    print(a === intVal);
    print(a != intVal);
    print(a !== intVal);
    print(a > intVal);
    print(a >= intVal);
    print(b >= intVal);
    print(b > intVal);

    // Check NAN with int min.
    print(a === intMinVal);
    print(a !== intMinVal);
    print(a > intMinVal);
    print(a >= intMinVal);
    print(a < intMinVal);
    print(a <= intMinVal);
    print(a == intMinVal);
    print(a != intMinVal);

    // Check NAN with int maxVal.
    print(a === intMaxVal);
    print(a !== intMaxVal);
    print(a > intMaxVal);
    print(a >= intMaxVal);
    print(a < intMaxVal);
    print(a <= intMaxVal);
    print(a == intMaxVal);
    print(a != intMaxVal);
}

function sccpNoEffectNanWithBoolAndDouble() {
    let a = NaN;
    let doubleVal = 1.1;
    let trueVal = true;
    let falseVal = false;

    // Check NAN with double val.
    print(a === doubleVal);
    print(a !== doubleVal);
    print(a > doubleVal);
    print(a >= doubleVal);
    print(a < doubleVal);
    print(a <= doubleVal);
    print(a == doubleVal);
    print(a != doubleVal);

    // Check NAN with true.
    print(a === trueVal);
    print(a !== trueVal);
    print(a > trueVal);
    print(a >= trueVal);
    print(a < trueVal);
    print(a <= trueVal);
    print(a == trueVal);
    print(a != trueVal);

    // Check NAN with false.
    print(a === falseVal);
    print(a !== falseVal);
    print(a > falseVal);
    print(a >= falseVal);
    print(a < falseVal);
    print(a <= falseVal);
    print(a == falseVal);
    print(a != falseVal);
}

function sccpNoEffectInfinityWithInt() {
    let a = Infinity;
    let intVal = 1;
    let intMaxVal = 2147483647;
    let intMinVal = -2147483648;

    print(a === intVal);
    print(a != intVal);
    print(a !== intVal);
    print(a > intVal);
    print(a >= intVal);
    print(b >= intVal);
    print(b > intVal);

    // Check Infinity with int min.
    print(a === intMinVal);
    print(a !== intMinVal);
    print(a > intMinVal);
    print(a >= intMinVal);
    print(a < intMinVal);
    print(a <= intMinVal);
    print(a == intMinVal);
    print(a != intMinVal);

    // Check Infinity with int maxVal.
    print(a === intMaxVal);
    print(a !== intMaxVal);
    print(a > intMaxVal);
    print(a >= intMaxVal);
    print(a < intMaxVal);
    print(a <= intMaxVal);
    print(a == intMaxVal);
    print(a != intMaxVal);
}

function sccpNoEffectInfinityWithBoolAndDouble() {
    let doubleVal = 1.1;
    let trueVal = true;
    let falseVal = false;

    // Check Infinity with double val.
    print(a === doubleVal);
    print(a !== doubleVal);
    print(a > doubleVal);
    print(a >= doubleVal);
    print(a < doubleVal);
    print(a <= doubleVal);
    print(a == doubleVal);
    print(a != doubleVal);

    // Check Infinity with true.
    print(a === trueVal);
    print(a !== trueVal);
    print(a > trueVal);
    print(a >= trueVal);
    print(a < trueVal);
    print(a <= trueVal);
    print(a == trueVal);
    print(a != trueVal);

    // Check Infinity with false.
    print(a === falseVal);
    print(a !== falseVal);
    print(a > falseVal);
    print(a >= falseVal);
    print(a < falseVal);
    print(a <= falseVal);
    print(a == falseVal);
    print(a != falseVal);
}

function sccpNoEffectArithmetic() {
    let a = 1;
    let b = 2;
    print(a * b);
    print(a + b);
    print(a - b);
    print(a / b);
    print(a >> b);
    print(a << b);
    print(a >>> b);

    a = 1.2;
    b = 2.2;
    print(a * b);
    print(a + b);
    print(a - b);
    print(a / b);
    print(a >> b);
    print(a << b);
    print(a >>> b);
}

function sccpIfCheck() {
    if (true) {
        print(true);
    }

    if (false) {
        print(false);
    }
    let a = 1;
    let b = 2;
    if (a < b) {
        print(true);
    }

    if (a > b) {
        print(true);
    }
}

function sccpPhi(a) {
    let b = a;
    if (a) {
        b = true;
    } else {
        b = true;
    }
    print(b);
}

function sccpPhi2()
{
    let a = 1;
    if (true) {
        a = 2;
    } else {
        a = 1;
    }
    print(a);
}

function sccpPhiNoEffect(a) {
    let b = a;
    if (a){
        b = true;
    } else {
        b = false;
    }
    print(b);
}
