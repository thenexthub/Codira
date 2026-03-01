//===-- PluginLoader.cpp - Implement -load command line option ------------===//
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
// This file implements the -load <plugin> command line option handler.
//
//===----------------------------------------------------------------------===//

#define DONT_GET_PLUGIN_LOADER_OPTION
#include "vm/core/Support/PluginLoader.h"
#include "vm/core/Support/DynamicLibrary.h"
#include "vm/core/Support/Mutex.h"
#include "vm/core/Support/raw_ostream.h"
#include <vector>
using namespace vm::core;

namespace {

struct Plugins {
  sys::SmartMutex<true> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  static Plugins P;
  return P;
}

} // anonymous namespace

void PluginLoader::operator=(const std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  std::string Error;
  if (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } else {
    P.List.push_back(Filename);
  }
}

unsigned PluginLoader::getNumPlugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  return P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned num) {
  auto &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  assert(num < P.List.size() && "Asking for an out of bounds plugin");
  return P.List[num];
}
