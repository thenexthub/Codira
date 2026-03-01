//===-- CFBundle.cpp --------------------------------------------*- C++ -*-===//
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
//
//  Created by Greg Clayton on 1/16/08.
//
//===----------------------------------------------------------------------===//

#include "CFBundle.h"
#include "CFString.h"

// CFBundle constructor
CFBundle::CFBundle(const char *path)
    : CFReleaser<CFBundleRef>(), m_bundle_url() {
  if (path && path[0])
    SetPath(path);
}

// CFBundle copy constructor
CFBundle::CFBundle(const CFBundle &rhs) = default;

// CFBundle copy constructor
CFBundle &CFBundle::operator=(const CFBundle &rhs) {
  if (this != &rhs)
    *this = rhs;
  return *this;
}

// Destructor
CFBundle::~CFBundle() = default;

// Set the path for a bundle by supplying a
bool CFBundle::SetPath(const char *path) {
  CFAllocatorRef alloc = kCFAllocatorDefault;
  // Release our old bundle and ULR
  reset(); // This class is a CFReleaser<CFBundleRef>
  m_bundle_url.reset();
  // Make a CFStringRef from the supplied path
  CFString cf_path;
  cf_path.SetFileSystemRepresentation(path);
  if (cf_path.get()) {
    // Make our Bundle URL
    m_bundle_url.reset(::CFURLCreateWithFileSystemPath(
        alloc, cf_path.get(), kCFURLPOSIXPathStyle, true));
    if (m_bundle_url.get()) {
      reset(::CFBundleCreate(alloc, m_bundle_url.get()));
    }
  }
  return get() != NULL;
}

CFStringRef CFBundle::GetIdentifier() const {
  CFBundleRef bundle = get();
  if (bundle != NULL)
    return ::CFBundleGetIdentifier(bundle);
  return NULL;
}

CFURLRef CFBundle::CopyExecutableURL() const {
  CFBundleRef bundle = get();
  if (bundle != NULL)
    return CFBundleCopyExecutableURL(bundle);
  return NULL;
}
