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

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP
#include "language/Basic/Lazy.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/ObjCBridge.h"
#include "language/Runtime/Portability.h"
#include "language/Threading/Mutex.h"
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#include <vector>

using namespace language;

/// Class of sentinel objects used to represent the `nil` value of nested
/// optionals.
///
/// NOTE: older runtimes called this _CodiraNull. The two must
/// coexist, so it was renamed. The old name must not be used in the new
/// runtime.
@interface __CodiraNull : NSObject {
@public
  unsigned depth;
}
@end



@implementation __CodiraNull : NSObject

- (id)description {
  char *str = NULL;
  const char *clsName = class_getName([self class]);
  int fmtResult = language_asprintf(&str, "<%s %p depth = %u>", clsName,
                                                       (void*)self,
                                                       self->depth);
  (void)fmtResult;
  assert(fmtResult != -1 && "unable to format description of null");
  id result = language_stdlib_NSStringFromUTF8(str, strlen(str));
  free(str);
  return result;
}

@end

namespace {

struct CodiraNullSentinelCache {
  std::vector<id> Cache;
  Mutex Lock;
};

static Lazy<CodiraNullSentinelCache> Sentinels;

static id getSentinelForDepth(unsigned depth) {
  // For unnested optionals, use NSNull.
  if (depth == 1)
    return LANGUAGE_LAZY_CONSTANT(id_const_cast([objc_getClass("NSNull") null]));
  // Otherwise, make up our own sentinel.
  // See if we created one for this depth.
  auto &theSentinels = Sentinels.get();
  unsigned depthIndex = depth - 2;
  {
    Mutex::ScopedLock lock(theSentinels.Lock);
    const auto &cache = theSentinels.Cache;
    if (depthIndex < cache.size()) {
      id cached = cache[depthIndex];
      if (cached)
        return cached;
    }
  }
  // Make one if we need to.
  {
    Mutex::ScopedLock lock(theSentinels.Lock);
    if (depthIndex >= theSentinels.Cache.size())
      theSentinels.Cache.resize(depthIndex + 1);
    auto &cached = theSentinels.Cache[depthIndex];
    // Make sure another writer didn't sneak in.
    if (!cached) {
      auto sentinel = [[__CodiraNull alloc] init];
      sentinel->depth = depth;
      cached = sentinel;
    }
    return cached;
  }
}

}

/// Return the sentinel object to use to represent `nil` for a given Optional
/// type.
LANGUAGE_RUNTIME_STDLIB_API LANGUAGE_CC(language)
id _language_Foundation_getOptionalNilSentinelObject(const Metadata *Wrapped) {
  // Figure out the depth of optionality we're working with.
  unsigned depth = 1;
  while (Wrapped->getKind() == MetadataKind::Optional) {
    ++depth;
    Wrapped = cast<EnumMetadata>(Wrapped)->getGenericArgs()[0];
  }

  return objc_retain(getSentinelForDepth(depth));
}
#endif

