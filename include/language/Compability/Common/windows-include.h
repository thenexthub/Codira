//===-- language/Compability/Common/windows-include.h ------------------*- C++ -*-===//
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
//
// Wrapper around windows.h that works around the name conflicts.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_COMMON_WINDOWS_INCLUDE_H_
#define LANGUAGE_COMPABILITY_COMMON_WINDOWS_INCLUDE_H_

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Target Windows 2000 and above. This is needed for newer Windows API
// functions, e.g. GetComputerNameExA()
#define _WIN32_WINNT 0x0500

#include <windows.h>

#endif // _WIN32

#endif // FORTRAN_COMMON_WINDOWS_INCLUDE_H_
