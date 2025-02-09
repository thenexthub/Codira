//go:build byollvm

/*
 * Copyright (c) NeXTHub Corporation. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 */

// The word "byollvm" stands for "Bring Your Own LLVM" and is a build tag used
// to indicate that the package is being built with a custom LLVM installation.

package llvm

// ldLLVMConfigBin is the path to the llvm-config binary. It shoud be set via
// -ldflags when building the package.
var ldLLVMConfigBin string
