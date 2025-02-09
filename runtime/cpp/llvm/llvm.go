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

// -----------------------------------------------------------------------------

const (
	LLGoPackage = "link: $(llvm-config --libs) $(llvm-config --ldflags); -lLLVM"
)

// StringView represents a C++ std::string_view object.
type StringView = string

// -----------------------------------------------------------------------------
