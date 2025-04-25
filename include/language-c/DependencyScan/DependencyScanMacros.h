//===-- DependencyScanMacros.h - Swift Dependency Scanning Macros -*- C -*-===//
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

#ifdef  __cplusplus
# define SWIFTSCAN_BEGIN_DECLS  extern "C" {
# define SWIFTSCAN_END_DECLS    }
#else
# define SWIFTSCAN_BEGIN_DECLS
# define SWIFTSCAN_END_DECLS
#endif

#ifndef SWIFTSCAN_PUBLIC
# ifdef _WIN32
#  ifdef libSwiftScan_EXPORTS
#    define SWIFTSCAN_PUBLIC __declspec(dllexport)
#  else
#    define SWIFTSCAN_PUBLIC __declspec(dllimport)
#  endif
# else
#  define SWIFTSCAN_PUBLIC
# endif
#endif
