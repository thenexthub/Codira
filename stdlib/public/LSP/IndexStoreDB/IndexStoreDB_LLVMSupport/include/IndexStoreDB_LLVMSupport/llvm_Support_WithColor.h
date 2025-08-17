//===- WithColor.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_SUPPORT_WITHCOLOR_H
#define LLVM_SUPPORT_WITHCOLOR_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_CommandLine.h>

namespace toolchain {

extern cl::OptionCategory ColorCategory;

class raw_ostream;

// Symbolic names for various syntax elements.
enum class HighlightColor {
  Address,
  String,
  Tag,
  Attribute,
  Enumerator,
  Macro,
  Error,
  Warning,
  Note,
  Remark
};

/// An RAII object that temporarily switches an output stream to a specific
/// color.
class WithColor {
  raw_ostream &OS;
  bool DisableColors;

public:
  /// To be used like this: WithColor(OS, HighlightColor::String) << "text";
  /// @param OS The output stream
  /// @param S Symbolic name for syntax element to color
  /// @param DisableColors Whether to ignore color changes regardless of -color
  /// and support in OS
  WithColor(raw_ostream &OS, HighlightColor S, bool DisableColors = false);
  /// To be used like this: WithColor(OS, raw_ostream::Black) << "text";
  /// @param OS The output stream
  /// @param Color ANSI color to use, the special SAVEDCOLOR can be used to
  /// change only the bold attribute, and keep colors untouched
  /// @param Bold Bold/brighter text, default false
  /// @param BG If true, change the background, default: change foreground
  /// @param DisableColors Whether to ignore color changes regardless of -color
  /// and support in OS
  WithColor(raw_ostream &OS,
            raw_ostream::Colors Color = raw_ostream::SAVEDCOLOR,
            bool Bold = false, bool BG = false, bool DisableColors = false)
      : OS(OS), DisableColors(DisableColors) {
    changeColor(Color, Bold, BG);
  }
  ~WithColor();

  raw_ostream &get() { return OS; }
  operator raw_ostream &() { return OS; }
  template <typename T> WithColor &operator<<(T &O) {
    OS << O;
    return *this;
  }
  template <typename T> WithColor &operator<<(const T &O) {
    OS << O;
    return *this;
  }

  /// Convenience method for printing "error: " to stderr.
  static raw_ostream &error();
  /// Convenience method for printing "warning: " to stderr.
  static raw_ostream &warning();
  /// Convenience method for printing "note: " to stderr.
  static raw_ostream &note();
  /// Convenience method for printing "remark: " to stderr.
  static raw_ostream &remark();

  /// Convenience method for printing "error: " to the given stream.
  static raw_ostream &error(raw_ostream &OS, StringRef Prefix = "",
                            bool DisableColors = false);
  /// Convenience method for printing "warning: " to the given stream.
  static raw_ostream &warning(raw_ostream &OS, StringRef Prefix = "",
                              bool DisableColors = false);
  /// Convenience method for printing "note: " to the given stream.
  static raw_ostream &note(raw_ostream &OS, StringRef Prefix = "",
                           bool DisableColors = false);
  /// Convenience method for printing "remark: " to the given stream.
  static raw_ostream &remark(raw_ostream &OS, StringRef Prefix = "",
                             bool DisableColors = false);

  /// Determine whether colors are displayed.
  bool colorsEnabled();

  /// Change the color of text that will be output from this point forward.
  /// @param Color ANSI color to use, the special SAVEDCOLOR can be used to
  /// change only the bold attribute, and keep colors untouched
  /// @param Bold Bold/brighter text, default false
  /// @param BG If true, change the background, default: change foreground
  WithColor &changeColor(raw_ostream::Colors Color, bool Bold = false,
                         bool BG = false);

  /// Reset the colors to terminal defaults. Call this when you are done
  /// outputting colored text, or before program exit.
  WithColor &resetColor();
};

} // end namespace toolchain

#endif // LLVM_LIB_DEBUGINFO_WITHCOLOR_H
