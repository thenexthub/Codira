//===--- DefineDiagnosticMacros.h - Shared Diagnostic Macros ----*- C++ -*-===//
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
//
//  This file defines macros shared across diagnostic definition files.
//
//===----------------------------------------------------------------------===//

// Define macros
#ifdef DEFINE_DIAGNOSTIC_MACROS

#if !(defined(DIAG) || (defined(GROUPED_ERROR) && defined(GROUPED_WARNING) &&  \
                        defined(NOTE) && defined(REMARK)))
#error Must define either DIAG or the set {GROUPED_ERROR,GROUPED_WARNING,NOTE,REMARK}
#endif

#ifndef GROUPED_ERROR
#define GROUPED_ERROR(ID, Group, Options, Text, Signature)                     \
  DIAG(ERROR, ID, Group, Options, Text, Signature)
#endif

#ifndef ERROR
#define ERROR(ID, Options, Text, Signature)                                    \
  GROUPED_ERROR(ID, no_group, Options, Text, Signature)
#endif

#ifndef GROUPED_WARNING
#define GROUPED_WARNING(ID, Group, Options, Text, Signature)                   \
  DIAG(WARNING, ID, Group, Options, Text, Signature)
#endif

#ifndef WARNING
#define WARNING(ID, Options, Text, Signature)                                  \
  GROUPED_WARNING(ID, no_group, Options, Text, Signature)
#endif

#ifndef NOTE
#define NOTE(ID, Options, Text, Signature)                                     \
  DIAG(NOTE, ID, no_group, Options, Text, Signature)
#endif

#ifndef REMARK
#define REMARK(ID, Options, Text, Signature)                                   \
  DIAG(REMARK, ID, no_group, Options, Text, Signature)
#endif

#ifndef FIXIT
#define FIXIT(ID, Text, Signature)
#endif

#undef DEFINE_DIAGNOSTIC_MACROS
#endif

// Undefine macros
#ifdef UNDEFINE_DIAGNOSTIC_MACROS

#ifndef DIAG_NO_UNDEF

#if defined(DIAG)
#undef DIAG
#endif

#undef REMARK
#undef NOTE
#undef WARNING
#undef GROUPED_WARNING
#undef ERROR
#undef GROUPED_ERROR
#undef FIXIT

#endif

#undef UNDEFINE_DIAGNOSTIC_MACROS
#endif
