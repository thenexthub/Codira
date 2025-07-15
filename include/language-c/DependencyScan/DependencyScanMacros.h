//===-- DependencyScanMacros.h - Codira Dependency Scanning Macros -*- C -*-===//
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
# define LANGUAGESCAN_BEGIN_DECLS  extern "C" {
# define LANGUAGESCAN_END_DECLS    }
#else
# define LANGUAGESCAN_BEGIN_DECLS
# define LANGUAGESCAN_END_DECLS
#endif

#ifndef LANGUAGESCAN_PUBLIC
# ifdef _WIN32
#  ifdef libCodiraScan_EXPORTS
#    define LANGUAGESCAN_PUBLIC __declspec(dllexport)
#  else
#    define LANGUAGESCAN_PUBLIC __declspec(dllimport)
#  endif
# else
#  define LANGUAGESCAN_PUBLIC
# endif
#endif
