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

#ifndef _CShims_h
#define _CShims_h

#include "_CShimsTargetConditionals.h"
#include "_CStdlib.h"
#include "CFUniCharBitmapData.inc.h"
#include "CFUniCharBitmapData.h"
#include "string_shims.h"
#include "bplist_shims.h"
#include "io_shims.h"
#include "platform_shims.h"
#include "filemanager_shims.h"
#include "uuid.h"

#if FOUNDATION_FRAMEWORK && !TARGET_OS_EXCLAVEKIT
#include "sandbox_shims.h"
#endif

#endif /* _CShims_h */
