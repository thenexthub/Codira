//===--- FileSystemProvider.h - ---------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_SUPPORT_FILESYSTEMPROVIDER_H
#define TOOLCHAIN_SOURCEKIT_SUPPORT_FILESYSTEMPROVIDER_H

#include "SourceKit/Core/LangSupport.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/VirtualFileSystem.h"

namespace SourceKit {

/// Allows clients of SourceKit to specify custom toolchain::vfs::FileSystems to be
/// used while serving a request.
///
/// Requests to SourceKit select FileSystemProviders by specifying
/// 'key.vfs.name', and pass arguments to the FileSystemProviders by
/// specifying 'key.vfs.args'. SourceKit then passes the given arguments to the
/// selected FileSystemProvider, and uses the resulting toolchain::vfs::FileSystem
/// while serving the request.
///
/// The following requests currently support custom FileSystemProviders (other
/// requests respond with an invalid request error if you try):
/// - source.request.editor.open:
///     Associates the given custom filesystem with this editor file, so that
///     all subsequent requests on the file use it, unless the subsequent
///     request specifies its own filesystem.
/// - source.request.cursorinfo:
///     Uses the given custom filesystem. If none is specified, uses the
///     filesystem associated with the file.
/// - source.request.codecomplete:
///     Uses the given custom filesystem.
/// - source.request.codecomplete.open:
///     Associates the given custom filesystem with this completion session, so
///     that all subsequent requests in the session use it.
/// - source.request.codecomplete.update:
///     Uses the filesystem associated with the completion session.
class FileSystemProvider {
public:
  virtual ~FileSystemProvider() = default;

  /// Returns a toolchain::vfs::FileSystem to be used while serving a request, or
  /// nullptr on failure.
  /// \param options arguments passed into the request under 'key.vfs.options'.
  /// \param [out] error filled with an error message on failure.
  virtual toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
  getFileSystem(OptionsDictionary &options, std::string &error) = 0;
};

} // namespace SourceKit

#endif
