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
    const suite = new arktest.ArkTestsuite('NumericType API tests');

    suite.addTest('ByteType: primitives & reference', testByteType);
    suite.addTest('ShortType: primitives & reference', testShortType);
    suite.addTest('IntType: primitives & reference', testIntType);
    suite.addTest('LongType: primitives & reference', testLongType);
    suite.addTest('FloatType: primitives & reference', testFloatType);
    suite.addTest('DoubleType: primitives & reference', testDoubleType);
    suite.addTest('Numeric assignableFrom widening', testNumericAssignableFromAnyNumeric);

    return suite.run();
}

function testByteType() {
    const val = ByteType.VAL;
    const ref = ByteType.REF;

    arktest.assertTrue(val.isPrimitive());
    arktest.assertFalse(val.isReference());
    arktest.assertTrue(ref.isPrimitive());
    arktest.assertTrue(ref.isReference());

    arktest.assertFalse(val.hasName());
    arktest.assertEQ('byte', val.getLiteral());
    arktest.assertTrue(ref.hasName());
    arktest.assertEQ('Byte', ref.getLiteral());

    arktest.assertTrue(val.equals(ByteType.VAL));
    arktest.assertTrue(ref.equals(ByteType.REF));
    arktest.assertTrue(val.equals(ref));
    arktest.assertTrue(ref.equals(val));
}

function testShortType() {
    const val = ShortType.VAL;
    const ref = ShortType.REF;

    arktest.assertTrue(val.isPrimitive());
    arktest.assertFalse(val.isReference());
    arktest.assertTrue(ref.isPrimitive());
    arktest.assertTrue(ref.isReference());

    arktest.assertFalse(val.hasName());
    arktest.assertEQ('short', val.getLiteral());
    arktest.assertTrue(ref.hasName());
    arktest.assertEQ('Short', ref.getLiteral());

    arktest.assertTrue(val.equals(ShortType.VAL));
    arktest.assertTrue(ref.equals(ShortType.REF));
    arktest.assertTrue(val.equals(ref));
}

function testIntType() {
    const val = IntType.VAL;
    const ref = IntType.REF;

    arktest.assertTrue(val.isPrimitive());
    arktest.assertFalse(val.isReference());
    arktest.assertTrue(ref.isPrimitive());
    arktest.assertTrue(ref.isReference());

    arktest.assertFalse(val.hasName());
    arktest.assertEQ('int', val.getLiteral());
    arktest.assertTrue(ref.hasName());
    arktest.assertEQ('Int', ref.getLiteral());

    arktest.assertTrue(val.equals(IntType.VAL));
    arktest.assertTrue(ref.equals(IntType.REF));
    arktest.assertTrue(val.equals(ref));
}

function testLongType() {
    const val = LongType.VAL;
    const ref = LongType.REF;

    arktest.assertTrue(val.isPrimitive());
    arktest.assertFalse(val.isReference());
    arktest.assertTrue(ref.isPrimitive());
    arktest.assertTrue(ref.isReference());

    arktest.assertFalse(val.hasName());
    arktest.assertEQ('long', val.getLiteral());
    arktest.assertTrue(ref.hasName());
    arktest.assertEQ('Long', ref.getLiteral());

    arktest.assertTrue(val.equals(LongType.VAL));
    arktest.assertTrue(ref.equals(LongType.REF));
    arktest.assertTrue(val.equals(ref));
}

function testFloatType() {
    const val = FloatType.VAL;
    const ref = FloatType.REF;

    arktest.assertTrue(val.isPrimitive());
    arktest.assertFalse(val.isReference());
    arktest.assertTrue(ref.isPrimitive());
    arktest.assertTrue(ref.isReference());

    arktest.assertFalse(val.hasName());
    arktest.assertEQ('float', val.getLiteral());
    arktest.assertTrue(ref.hasName());
    arktest.assertEQ('Float', ref.getLiteral());

    arktest.assertTrue(val.equals(FloatType.VAL));
    arktest.assertTrue(ref.equals(FloatType.REF));
    arktest.assertTrue(val.equals(ref));
}

function testDoubleType() {
    const val = DoubleType.VAL;
    const ref = DoubleType.REF;

    arktest.assertTrue(val.isPrimitive());
    arktest.assertFalse(val.isReference());
    arktest.assertTrue(ref.isPrimitive());
    arktest.assertTrue(ref.isReference());

    arktest.assertFalse(val.hasName());
    arktest.assertEQ('double', val.getLiteral());
    arktest.assertTrue(ref.hasName());
    arktest.assertEQ('Double', ref.getLiteral());

    arktest.assertTrue(val.equals(DoubleType.VAL));
    arktest.assertTrue(ref.equals(DoubleType.REF));
    arktest.assertTrue(val.equals(ref));
}

function testNumericAssignableFromAnyNumeric() {
    const nums = [
        ByteType.VAL,
        ShortType.VAL,
        IntType.VAL,
        LongType.VAL,
        FloatType.VAL,
        DoubleType.VAL,
    ];

    for (let i = 0; i < nums.length; ++i) {
        for (let j = 0; j < nums.length; ++j) {
        arktest.assertEQ(
            true,
            nums[i].assignableFrom(nums[j]),
            `expected ${nums[i].getLiteral()} ← ${nums[j].getLiteral()} to be true`
        );
        }
    }
}
