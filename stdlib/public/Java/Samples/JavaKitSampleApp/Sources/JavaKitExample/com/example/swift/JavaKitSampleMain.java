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

package com.example.swift;

/**
 * This sample shows off a {@link HelloSwift} type which is partially implemented in Swift.
 * For the Swift implementation refer to
 */
public class JavaKitSampleMain {
    public static void main(String[] args) {
        int result = new HelloSubclass("Swift").sayHello(17, 25);
        System.out.println("sayHello(17, 25) = " + result);
    }
}
