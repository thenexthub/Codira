//===----------------------------------------------------------------------===//
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
///
/// \file
/// This file implements \c OutputFile class methods.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Support/VirtualOutputFile.h"
#include "vm/core/Support/VirtualOutputError.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Support/raw_ostream_proxy.h"

using namespace vm::core;
using namespace vm::core::vfs;

char OutputFileImpl::ID = 0;
char NullOutputFileImpl::ID = 0;

void OutputFileImpl::anchor() {}
void NullOutputFileImpl::anchor() {}

class OutputFile::TrackedProxy : public raw_pwrite_stream_proxy {
public:
  void resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    assert(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = this;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  if (OpenProxy)
    return make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  return std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  if (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  if (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  return E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  if (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  if (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  return E;
}

void OutputFile::destroy() {
  if (!Impl)
    return;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  assert(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  if (LLVM_UNLIKELY(!DiscardHandler))
    toolchain::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  else if (E)
    DiscardHandler(std::move(E));
}
