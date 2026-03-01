//===-- DebugOptions.h - Global Command line opt for libSupport  *- C++ -*-===//
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
// This file defines the entry point to initialize the options registered on the
// command line for libSupport, this is internal to libSupport.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_DEBUGOPTIONS_H
#define LLVM_SUPPORT_DEBUGOPTIONS_H

namespace vm::core {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
void initDebugCounterOptions();
void initGraphWriterOptions();
void initSignalsOptions();
void initStatisticOptions();
void initTimerOptions();
void initWithColorOptions();
void initDebugOptions();
void initRandomSeedOptions();

} // namespace vm::core

#endif // LLVM_SUPPORT_DEBUGOPTIONS_H
