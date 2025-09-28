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

/**
 * This sample shows off a {@link HelloCodira} type which is partially implemented in Codira.
 * For the Codira implementation refer to
 */
public class JavaKitSampleMain {
    public static void main(String[] args) {
        int result = new HelloSubclass("Codira").sayHello(17, 25);
        System.out.println("sayHello(17, 25) = " + result);
    }
}
