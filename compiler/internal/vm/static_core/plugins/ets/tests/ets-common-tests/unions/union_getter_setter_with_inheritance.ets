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

class TestClass {
    public prop1_: String = '123';
    get prop1():String {
        return this.prop1_;
    }
    set prop1(i:String) {
        this.prop1_ = i
    }
    public prop2_:Int = 123;
    get prop2():Int {
        return this.prop2_;
    }
    set prop2(i:Int) {
        this.prop2_ = i
    }
    public prop3_: Byte = 12;
    get prop3():Byte {
        return this.prop3_;
    }
    set prop3(i:Byte) {
        this.prop3_ = i
    }
    public prop4_: Long = 456;
    get prop4():Long {
        return this.prop4_;
    }
    set prop4(i:Long) {
        this.prop4_ = i
    }
    public prop5_: Short = 13;
    get prop5():Short {
        return this.prop5_;
    }
    set prop5(i:Short) {
        this.prop5_ = i
    }
}

class TestClass1 {
    public prop1_: String = '345';
    get prop1():String {
        return this.prop1_;
    }
    set prop1(i:String) {
        this.prop1_ = i
    }
    public prop2_: Int = 321;
    get prop2():Int {
        return this.prop2_;
    }
    set prop2(i:Int) {
        this.prop2_ = i
    }
    public prop3_: Byte = 14;
    get prop3():Byte {
        return this.prop3_;
    }
    set prop3(i:Byte) {
        this.prop3_ = i
    }
    public prop4_: Long = 654;
    get prop4():Long {
        return this.prop4_;
    }
    set prop4(i:Long) {
        this.prop4_ = i
    }
    public prop5_: Short = 15;
    get prop5():Short {
        return this.prop5_;
    }
    set prop5(i:Short) {
        this.prop5_ = i
    }

}

class TestClass2 extends TestClass {
}

class TestClass3 extends TestClass1 {
}

function test(a: TestClass2 | TestClass3) {
    arktest.assertEQ(a.prop1, '123', 'expected \'123\', but got ' + a.prop1);
    a.prop1 = '234'
    arktest.assertEQ(a.prop1, '234', 'expected \'234\', but got ' + a.prop1);
    arktest.assertEQ(a.prop2, 123, 'expected 123, but got ' + a.prop2);
    a.prop2 = 234
    arktest.assertEQ(a.prop2, 234, 'expected 234, but got ' + a.prop2);
    arktest.assertEQ(a.prop3, 12, 'expected 12, but got ' + a.prop3);
    a.prop3 = 13
    arktest.assertEQ(a.prop3, 13, 'expected 13, but got ' + a.prop3);
    arktest.assertEQ(a.prop4, 456, 'expected 456, but got ' + a.prop4);
    a.prop4 = 567
    arktest.assertEQ(a.prop4, 567, 'expected 567, but got ' + a.prop4);
    arktest.assertEQ(a.prop5, 13, 'expected 13, but got ' + a.prop5);
    a.prop5 = 14
    arktest.assertEQ(a.prop5, 14, 'expected 14, but got ' + a.prop5);
    return 0;
}

function main() {
    let a = new TestClass2()
    return test(a)
}
