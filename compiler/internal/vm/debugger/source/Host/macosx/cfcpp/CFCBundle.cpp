//===-- CFCBundle.cpp -----------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "CFCBundle.h"
#include "CFCString.h"

// CFCBundle constructor
CFCBundle::CFCBundle(const char *path) : CFCReleaser<CFBundleRef>() {
  if (path && path[0])
    SetPath(path);
}

CFCBundle::CFCBundle(CFURLRef url)
    : CFCReleaser<CFBundleRef>(url ? CFBundleCreate(NULL, url) : NULL) {}

// Destructor
CFCBundle::~CFCBundle() = default;

// Set the path for a bundle by supplying a
bool CFCBundle::SetPath(const char *path) {
  CFAllocatorRef alloc = kCFAllocatorDefault;
  // Release our old bundle and URL
  reset();

  // Make a CFStringRef from the supplied path
  CFCString cf_path;
  cf_path.SetFileSystemRepresentation(path);
  if (cf_path.get()) {
    // Make our Bundle URL
    CFCReleaser<CFURLRef> bundle_url(::CFURLCreateWithFileSystemPath(
        alloc, cf_path.get(), kCFURLPOSIXPathStyle, true));
    if (bundle_url.get())
      reset(::CFBundleCreate(alloc, bundle_url.get()));
  }
  return get() != NULL;
}

bool CFCBundle::GetPath(char *dst, size_t dst_len) {
  CFBundleRef bundle = get();
  if (bundle) {
    CFCReleaser<CFURLRef> bundle_url(CFBundleCopyBundleURL(bundle));
    if (bundle_url.get()) {
      Boolean resolveAgainstBase = 0;
      return ::CFURLGetFileSystemRepresentation(bundle_url.get(),
                                                resolveAgainstBase,
                                                (UInt8 *)dst, dst_len) != 0;
    }
  }
  return false;
}

CFStringRef CFCBundle::GetIdentifier() const {
  CFBundleRef bundle = get();
  if (bundle != NULL)
    return ::CFBundleGetIdentifier(bundle);
  return NULL;
}

CFTypeRef CFCBundle::GetValueForInfoDictionaryKey(CFStringRef key) const {
  CFBundleRef bundle = get();
  if (bundle != NULL)
    return ::CFBundleGetValueForInfoDictionaryKey(bundle, key);
  return NULL;
}

CFURLRef CFCBundle::CopyExecutableURL() const {
  CFBundleRef bundle = get();
  if (bundle != NULL)
    return CFBundleCopyExecutableURL(bundle);
  return NULL;
}
