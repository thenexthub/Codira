//===-- windows.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_lldb_windows_h_
#define LLDB_lldb_windows_h_

#define NTDDI_VERSION NTDDI_VISTA
#undef _WIN32_WINNT // undef a previous definition to avoid warning
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#undef NOMINMAX // undef a previous definition to avoid warning
#define NOMINMAX
#include <windows.h>
#undef CreateProcess
#undef GetMessage
#undef GetUserName
#undef LoadImage
#undef Yield
#undef far
#undef near
#undef FAR
#undef NEAR
#define FAR
#define NEAR

#endif // LLDB_lldb_windows_h_
