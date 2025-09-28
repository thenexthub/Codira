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

package org.code.codekit.core;

class JNICodiraInstanceCleanup implements CodiraInstanceCleanup {
    private final Runnable destroyFunction;
    private final Runnable markAsDestroyed;

    public JNICodiraInstanceCleanup(Runnable destroyFunction, Runnable markAsDestroyed) {
        this.destroyFunction = destroyFunction;
        this.markAsDestroyed = markAsDestroyed;
    }

    @Override
    public void run() {
        markAsDestroyed.run();
        destroyFunction.run();
    }
}
