//===- ExtractAPI/ExtractAPIActionBase.h -----------------------------*- C++
//-*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
/// This file defines the ExtractAPIActionBase class.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_EXTRACTAPI_ACTION_BASE_H
#define LANGUAGE_CORE_EXTRACTAPI_ACTION_BASE_H

#include "language/Core/ExtractAPI/API.h"
#include "language/Core/ExtractAPI/APIIgnoresList.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {

/// Base class to be used by front end actions to generate ExtarctAPI info
///
/// Deriving from this class equips an action with all the necessary tools to
/// generate ExractAPI information in form of symbol-graphs
class ExtractAPIActionBase {
protected:
  /// A representation of the APIs this action extracts.
  std::unique_ptr<extractapi::APISet> API;

  /// A stream to the main output file of this action.
  std::unique_ptr<toolchain::raw_pwrite_stream> OS;

  /// The product this action is extracting API information for.
  std::string ProductName;

  /// The synthesized input buffer that contains all the provided input header
  /// files.
  std::unique_ptr<toolchain::MemoryBuffer> Buffer;

  /// The list of symbols to ignore during serialization
  extractapi::APIIgnoresList IgnoresList;

  /// Implements EndSourceFileAction for Symbol-Graph generation
  ///
  /// Use the serializer to generate output symbol graph files from
  /// the information gathered during the execution of Action.
  void ImplEndSourceFileAction(CompilerInstance &CI);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_EXTRACTAPI_ACTION_BASE_H
