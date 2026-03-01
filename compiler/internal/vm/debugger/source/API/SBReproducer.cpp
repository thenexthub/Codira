//===-- SBReproducer.cpp --------------------------------------------------===//
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

#include "lldb/API/SBReproducer.h"
#include "lldb/Utility/Instrumentation.h"

using namespace lldb;
using namespace lldb_private;

SBReplayOptions::SBReplayOptions() = default;

SBReplayOptions::SBReplayOptions(const SBReplayOptions &rhs) = default;

SBReplayOptions::~SBReplayOptions() = default;

SBReplayOptions &SBReplayOptions::operator=(const SBReplayOptions &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs)
  return *this;
}

void SBReplayOptions::SetVerify(bool verify) {
  LLDB_INSTRUMENT_VA(this, verify);
}

bool SBReplayOptions::GetVerify() const {
  LLDB_INSTRUMENT_VA(this);
  return false;
}

void SBReplayOptions::SetCheckVersion(bool check) {
  LLDB_INSTRUMENT_VA(this, check);
}

bool SBReplayOptions::GetCheckVersion() const {
  LLDB_INSTRUMENT_VA(this);
  return false;
}

const char *SBReproducer::Capture() {
  LLDB_INSTRUMENT()
  return "Reproducer capture has been removed";
}

const char *SBReproducer::Capture(const char *path) {
  LLDB_INSTRUMENT_VA(path)
  return "Reproducer capture has been removed";
}

const char *SBReproducer::PassiveReplay(const char *path) {
  LLDB_INSTRUMENT_VA(path)
  return "Reproducer replay has been removed";
}

const char *SBReproducer::Replay(const char *path) {
  LLDB_INSTRUMENT_VA(path)
  return "Reproducer replay has been removed";
}

const char *SBReproducer::Replay(const char *path, bool skip_version_check) {
  LLDB_INSTRUMENT_VA(path, skip_version_check)
  return "Reproducer replay has been removed";
}

const char *SBReproducer::Replay(const char *path,
                                 const SBReplayOptions &options) {
  LLDB_INSTRUMENT_VA(path, options)
  return "Reproducer replay has been removed";
}

const char *SBReproducer::Finalize(const char *path) {
  LLDB_INSTRUMENT_VA(path)
  return "Reproducer finalize has been removed";
}

bool SBReproducer::Generate() {
  LLDB_INSTRUMENT()
  return false;
}

bool SBReproducer::SetAutoGenerate(bool b) {
  LLDB_INSTRUMENT_VA(b)
  return false;
}

const char *SBReproducer::GetPath() {
  LLDB_INSTRUMENT()
  return "Reproducer GetPath has been removed";
}

void SBReproducer::SetWorkingDirectory(const char *path) {
  LLDB_INSTRUMENT_VA(path)
}
