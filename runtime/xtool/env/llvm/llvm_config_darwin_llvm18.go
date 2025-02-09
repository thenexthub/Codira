//go:build !byollvm && darwin && !amd64 && !llvm14 && !llvm15 && !llvm16 && !llvm17

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

package llvm

const ldLLVMConfigBin = "/opt/homebrew/opt/llvm@18/bin/llvm-config"
