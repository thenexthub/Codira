//===--- SimpleDisplay.h - Simple Type Display ------------------*- C++ -*-===//
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
//  This file defines the main customization point, simple_display, for
//  displaying values of a given type
//  encoding of (static) type information for use as a simple replacement
//  for run-time type information.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_BASIC_SIMPLE_DISPLAY_H
#define LANGUAGE_BASIC_SIMPLE_DISPLAY_H

#include "toolchain/ADT/TinyPtrVector.h"
#include "toolchain/Support/raw_ostream.h"
#include <tuple>
#include <type_traits>
#include <utility>

namespace language {
  template<typename T>
  struct HasTrivialDisplay {
    static const bool value = false;
  };

#define HAS_TRIVIAL_DISPLAY(Type)     \
  template<>                          \
  struct HasTrivialDisplay<Type> {    \
    static const bool value = true;   \
  }

  HAS_TRIVIAL_DISPLAY(unsigned char);
  HAS_TRIVIAL_DISPLAY(signed char);
  HAS_TRIVIAL_DISPLAY(char);
  HAS_TRIVIAL_DISPLAY(short);
  HAS_TRIVIAL_DISPLAY(unsigned short);
  HAS_TRIVIAL_DISPLAY(int);
  HAS_TRIVIAL_DISPLAY(unsigned int);
  HAS_TRIVIAL_DISPLAY(long);
  HAS_TRIVIAL_DISPLAY(unsigned long);
  HAS_TRIVIAL_DISPLAY(long long);
  HAS_TRIVIAL_DISPLAY(unsigned long long);
  HAS_TRIVIAL_DISPLAY(float);
  HAS_TRIVIAL_DISPLAY(double);
  HAS_TRIVIAL_DISPLAY(bool);
  HAS_TRIVIAL_DISPLAY(std::string);

#undef HAS_TRIVIAL_DISPLAY

  template<typename T>
  typename std::enable_if<HasTrivialDisplay<T>::value>::type
  simple_display(toolchain::raw_ostream &out, const T &value) {
    out << value;
  }

  template<unsigned I, typename ...Types,
  typename std::enable_if<I == sizeof...(Types)>::type* = nullptr>
  void simple_display_tuple(toolchain::raw_ostream &out,
                            const std::tuple<Types...> &value);

  template<unsigned I, typename ...Types,
           typename std::enable_if<I < sizeof...(Types)>::type* = nullptr>
  void simple_display_tuple(toolchain::raw_ostream &out,
                            const std::tuple<Types...> &value) {
    // Start or separator.
    if (I == 0) out << "(";
    else out << ", ";

    // Current element.
    simple_display(out, std::get<I>(value));

    // Print the remaining elements.
    simple_display_tuple<I+1>(out, value);
  }

  template<unsigned I, typename ...Types,
           typename std::enable_if<I == sizeof...(Types)>::type*>
  void simple_display_tuple(toolchain::raw_ostream &out,
                            const std::tuple<Types...> &value) {
    // Last element.
    out << ")";
  }

  template<typename ...Types>
  void simple_display(toolchain::raw_ostream &out,
                      const std::tuple<Types...> &value) {
    simple_display_tuple<0>(out, value);
  }

  template<typename T1, typename T2>
  void simple_display(toolchain::raw_ostream &out,
                      const std::pair<T1, T2> &value) {
    out << "(";
    simple_display(out, value.first);
    out << ", ";
    simple_display(out, value.second);
    out << ")";
  }

  template<typename T>
  void simple_display(toolchain::raw_ostream &out,
                      const toolchain::TinyPtrVector<T> &vector) {
    out << "{";
    bool first = true;
    for (const T &value : vector) {
      if (first) first = false;
      else out << ", ";
      
      simple_display(out, value);
    }
    out << "}";
  }
  
  template<typename T>
  void simple_display(toolchain::raw_ostream &out,
                      const toolchain::ArrayRef<T> &array) {
    out << "{";
    bool first = true;
    for (const T &value : array) {
      if (first) first = false;
      else out << ", ";
      
      simple_display(out, value);
    }
    out << "}";
  }

  template<typename T>
  void simple_display(toolchain::raw_ostream &out,
                      const toolchain::SmallVectorImpl<T> &vec) {
    out << "{";
    bool first = true;
    for (const T &value : vec) {
      if (first) first = false;
      else out << ", ";

      simple_display(out, value);
    }
    out << "}";
  }

  template<typename T>
  void simple_display(toolchain::raw_ostream &out,
                      const std::vector<T> &vec) {
    out << "{";
    bool first = true;
    for (const T &value : vec) {
      if (first) first = false;
      else out << ", ";

      simple_display(out, value);
    }
    out << "}";
  }

  template<typename T, typename U>
  void simple_display(toolchain::raw_ostream &out,
                      const toolchain::PointerUnion<T, U> &ptrUnion) {
    if (const auto t = ptrUnion.template dyn_cast<T>())
      simple_display(out, t);
    else
      simple_display(out, ptrUnion.template get<U>());
  }
}

#endif // LANGUAGE_BASIC_SIMPLE_DISPLAY_H
