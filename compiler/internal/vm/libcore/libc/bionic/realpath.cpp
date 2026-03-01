/*
 * Copyright (C) 2019 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "private/FdPath.h"
#include "private/ScopedFd.h"

// This function needs a 4KiB (PATH_MAX) buffer.
// The alternative is to heap allocate and then trim, but that's 2x the code.
// (Remember that readlink(2) won't tell you the needed size, so the multi-pass
// algorithm isn't even an option unless you want to just guess, in which case
// you're back needing to trim again.)
#pragma clang diagnostic ignored "-Wframe-larger-than="

char* realpath(const char* path, char* result) {
  // Weird special case.
  if (!path) {
    errno = EINVAL;
    return nullptr;
  }

  // Get an O_PATH fd, and...
  ScopedFd fd(open(path, O_PATH | O_CLOEXEC));
  if (fd.get() == -1) return nullptr;

  // (...remember the device/inode that we're talking about and...)
  struct stat sb_before;
  if (fstat(fd.get(), &sb_before) == -1) return nullptr;

  // ...ask the kernel to do the hard work for us.
  FdPath fd_path(fd.get());
  char dst[PATH_MAX];
  ssize_t l = readlink(fd_path.c_str(), dst, sizeof(dst) - 1);
  if (l == -1) return nullptr;
  dst[l] = '\0';

  // What if the file was removed in the meantime? readlink(2) will have
  // returned "/a/b/c (deleted)", and we want to return ENOENT instead.
  struct stat sb_after;
  if (stat(dst, &sb_after) == -1 ||
      sb_before.st_dev != sb_after.st_dev ||
      sb_before.st_ino != sb_after.st_ino) {
    errno = ENOENT;
    return nullptr;
  }

  return result ? strcpy(result, dst) : strdup(dst);
}
