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

class A {
    public static newA()
    {
        return new A();
    }

    public stringMethod(a0: int, a1: int): String
    {
        if (a0 > a1) {
			return "hello";
		} else {
			return "nihao";
		}
    }

    public stringMethodVoidParam(): String
    {
        return "hello world";
    }

    public stringMethodMultipleParam(a0: int, a1: boolean, a2: float, a3: int, a4: float): String
    {
        if (a1) {
            return "china";
        } else if (a0 > a3) {
            return "england";
        } else if (a2 > a4) {
            return "japan";
        } else {
            return "korea";
        }
    }

    public nestedMethod(data1: int, data2: int): String
    {
        let method = (value1: int, value2: int): String => {
            if (value1 + value2 > 10) {
				return "hello";
			} else {
				return "nihao";
			}
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: int): String
    {
        if (a0 > 100) {
            return "hello";
        } else {
            return this.recursionMethod((a0 + 10).toShort());
        }
    }
}

final class B extends A {

    public subStringMethodVoidParam(): String
    {
        return "world";
    }
}

abstract class C {
    public abstract func(a: int, b: int): String;
}

class D extends C {
    public override func(a: int, b: int): String {
        if (a > b) {
            return "hello";
        } else {
            return "nihao";
        }
    }
}

class E extends D {
    public override func(a: int, b: int): String {
        if (a + b > 10) {
            return "hello world";
        } else {
            return "hello";
        }
    }
}