//===-- CFCString.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCSTRING_H
#define LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCSTRING_H

#include <iosfwd>

#include "CFCReleaser.h"

class CFCString : public CFCReleaser<CFStringRef> {
public:
  // Constructors and Destructors
  CFCString(CFStringRef cf_str = NULL);
  CFCString(const char *s, CFStringEncoding encoding = kCFStringEncodingUTF8);
  CFCString(const CFCString &rhs);
  CFCString &operator=(const CFCString &rhs);
  ~CFCString() override;

  const char *GetFileSystemRepresentation(std::string &str);
  CFStringRef SetFileSystemRepresentation(const char *path);
  CFStringRef SetFileSystemRepresentationFromCFType(CFTypeRef cf_type);
  CFStringRef SetFileSystemRepresentationAndExpandTilde(const char *path);
  const char *UTF8(std::string &str);
  CFIndex GetLength() const;
  static const char *UTF8(CFStringRef cf_str, std::string &str);
  static const char *FileSystemRepresentation(CFStringRef cf_str,
                                              std::string &str);
  static const char *ExpandTildeInPath(const char *path,
                                       std::string &expanded_path);
};

#endif // LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCSTRING_H
