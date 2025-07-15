//===--- EnumTraits.h - Traits for densely-packed enums ---------*- C++ -*-===//
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
///
///  This file defines the EnumTraits concept, which can be used to
///  communicate information about an enum type's enumerators that currently
///  can't be recovered from the compiler.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_ENUMTRAITS_H
#define LANGUAGE_BASIC_ENUMTRAITS_H

namespace language {

/// A simple traits concept for recording the number of cases in an enum.
///
///  template <> class EnumTraits<WdigetKind> {
///    static constexpr size_t NumValues = NumWidgetKinds;
///  };
template <class E>
struct EnumTraits;

} // end namespace language

#endif
