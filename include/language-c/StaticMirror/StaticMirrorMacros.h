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
# define SWIFTSTATICMIRROR_BEGIN_DECLS  extern "C" {
# define SWIFTSTATICMIRROR_END_DECLS    }
#else
# define SWIFTSTATICMIRROR_BEGIN_DECLS
# define SWIFTSTATICMIRROR_END_DECLS
#endif

#ifndef SWIFTSTATICMIRROR_PUBLIC
# ifdef _WIN32
#  ifdef libStaticMirror_EXPORTS
#    define SWIFTSTATICMIRROR_PUBLIC __declspec(dllexport)
#  else
#    define SWIFTSTATICMIRROR_PUBLIC __declspec(dllimport)
#  endif
# else
#  define SWIFTSTATICMIRROR_PUBLIC
# endif
#endif
