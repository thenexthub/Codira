//===-- ZipFile.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_ZIPFILE_H
#define LLDB_UTILITY_ZIPFILE_H

#include "lldb/lldb-private.h"

namespace lldb_private {

/// In Android API level 23 and above, bionic dynamic linker is able to load
/// .so file directly from APK or .zip file. This is a utility class to find
/// .so file offset and size from zip file.
/// https://android.googlesource.com/platform/bionic/+/master/
/// android-changes-for-ndk-developers.md#
/// opening-shared-libraries-directly-from-an-apk
class ZipFile {
public:
  static bool Find(lldb::DataBufferSP zip_data, const llvm::StringRef file_path,
                   lldb::offset_t &file_offset, lldb::offset_t &file_size);
};

} // end of namespace lldb_private

#endif // LLDB_UTILITY_ZIPFILE_H
