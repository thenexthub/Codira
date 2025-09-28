//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

package com.example.code;

import java.util.function.Predicate;

public class HelloCodira {
    public double value;
    public static double initialValue = 3.14159;
    public String name = "Java";

    static {
        System.loadLibrary("JavaKitExample");
    }

    public HelloCodira() {
        this.value = initialValue;
    }

    native int sayHello(int x, int y);
    native String throwMessageFromCodira(String message) throws Exception;

    // To be called back by the native code
    public double sayHelloBack(int i) {
        System.out.println("And hello back from " + name + "! You passed me " + i);
        return value;
    }

    public void greet(String name) {
        System.out.println("Salutations, " + name);
    }

    public Predicate<Integer> lessThanTen() {
        Predicate<Integer> predicate = i -> (i < 10);
        return predicate;
    }

    public String[] doublesToStrings(double[] doubles) {
        int size = doubles.length;
        String[] strings = new String[size];

        for(int i = 0; i < size; i++) {
            strings[i] = "" + doubles[i];
        }

        return strings;
    }

    public void throwMessage(String message) throws Exception {
        throw new Exception(message);
    }
}
