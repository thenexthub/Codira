//=--- CommonBugCategories.cpp - Provides common issue categories -*- C++ -*-=//
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

#include "language/Core/StaticAnalyzer/Core/BugReporter/CommonBugCategories.h"

// Common strings used for the "category" of many static analyzer issues.
namespace language::Core {
namespace ento {
namespace categories {

const char *const AppleAPIMisuse = "API Misuse (Apple)";
const char *const CoreFoundationObjectiveC = "Core Foundation/Objective-C";
const char *const LogicError = "Logic error";
const char *const MemoryRefCount =
    "Memory (Core Foundation/Objective-C/OSObject)";
const char *const MemoryError = "Memory error";
const char *const UnixAPI = "Unix API";
const char *const CXXObjectLifecycle = "C++ object lifecycle";
const char *const CXXMoveSemantics = "C++ move semantics";
const char *const SecurityError = "Security error";
const char *const UnusedCode = "Unused code";
const char *const TaintedData = "Tainted data used";
} // namespace categories
} // namespace ento
} // namespace language::Core
