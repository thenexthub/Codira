//===--- SwiftNativeNSXXXBaseARC.mm - Runtime stubs that require ARC ------===//
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
#include "language/Runtime/Config.h"

#if SWIFT_OBJC_INTEROP

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#include <objc/NSObject.h>
#include <objc/runtime.h>
#include <objc/objc.h>

// The following two routines need to be implemented in ARC because
// decomposedStringWithCanonicalMapping returns its result autoreleased. And we
// want ARC to insert 'objc_retainAutoreleasedReturnValue' and the necessary
// markers for the hand-off to facilitate the remove from autorelease pool
// optimization such that the object is not handed into the current autorelease
// pool which might be scoped such that repeatedly placing objects into it
// results in unbounded memory growth.

// On i386 the remove from autorelease pool optimization is foiled by the
// decomposedStringWithCanonicalMapping implementation. Instead, we use a local
// autorelease pool to prevent leaking of the temporary object into the callers
// autorelease pool.
//
//
// FIXME: Right now we force an autoreleasepool here on x86_64 where we really
// do not want to do so. The reason why is that without the autoreleasepool (or
// really something like a defer), we tail call
// objc_retainAutoreleasedReturnValue which blocks the hand shake. Evidently
// this is something that we do not want to do. See:
// b79ff50f1bca97ecfd053372f5f6dc9d017398bc. Until that is resolved, just create
// an autoreleasepool here on x86_64. On arm/arm64 we do not have such an issue
// since we use an assembly marker instead.
#if defined(__i386__) || defined(__x86_64__)
#define AUTORELEASEPOOL @autoreleasepool
#else
// On other platforms we rely on the remove from autorelease pool optimization.
#define AUTORELEASEPOOL
#endif

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_API
size_t swift_stdlib_NSStringHashValue(NSString *str,
                                      bool isASCII) {
  AUTORELEASEPOOL {
    return isASCII ? str.hash : str.decomposedStringWithCanonicalMapping.hash;
  }
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_API
size_t
swift_stdlib_NSStringHashValuePointer(void *opaque, bool isASCII) {
  NSString __unsafe_unretained *str =
      (__bridge NSString __unsafe_unretained *)opaque;
  AUTORELEASEPOOL {
    return isASCII ? str.hash : str.decomposedStringWithCanonicalMapping.hash;
  }
}

#else

extern char ignore_pedantic_warning;

#endif
