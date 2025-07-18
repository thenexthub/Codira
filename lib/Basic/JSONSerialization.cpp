//===--- JSONSerialization.cpp - JSON serialization support ---------------===//
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

#include "language/Basic/Assertions.h"
#include "language/Basic/JSONSerialization.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/Format.h"

using namespace language::json;
using namespace language;

unsigned Output::beginArray() {
  StateStack.push_back(ArrayFirstValue);
  Stream << '[';
  return 0;
}

bool Output::preflightElement(unsigned, void *&) {
  if (StateStack.back() != ArrayFirstValue) {
    assert(StateStack.back() == ArrayOtherValue && "We must be in a sequence!");
    Stream << ',';
  }
  if (PrettyPrint) {
    Stream << '\n';
    indent();
  }
  return true;
}

void Output::postflightElement(void*) {
  if (StateStack.back() == ArrayFirstValue) {
    StateStack.pop_back();
    StateStack.push_back(ArrayOtherValue);
  }
}

void Output::endArray() {
  bool HadContent = StateStack.back() != ArrayFirstValue;
  StateStack.pop_back();
  if (PrettyPrint && HadContent) {
    Stream << '\n';
    indent();
  }
  Stream << ']';
}

bool Output::canElideEmptyArray() {
  if (StateStack.size() < 2)
    return true;
  if (StateStack.back() != ObjectFirstKey)
    return true;
  State checkedState = StateStack[StateStack.size() - 2];
  return (checkedState != ArrayFirstValue && checkedState != ArrayOtherValue);
}

void Output::beginObject() {
  StateStack.push_back(ObjectFirstKey);
  Stream << "{";
}

void Output::endObject() {
  bool HadContent = StateStack.back() != ObjectFirstKey;
  StateStack.pop_back();
  if (PrettyPrint && HadContent) {
    Stream << '\n';
    indent();
  }
  Stream << "}";
}

bool Output::preflightKey(toolchain::StringRef Key, bool Required,
                          bool SameAsDefault, bool &UseDefault, void *&) {
  UseDefault = false;
  if (Required || !SameAsDefault) {
    if (StateStack.back() != ObjectFirstKey) {
      assert(StateStack.back() == ObjectOtherKey && "We must be in an object!");
      Stream << ',';
    }
    if (PrettyPrint) {
      Stream << '\n';
      indent();
    }
    Stream << '"' << Key << "\":";
    if (PrettyPrint)
      Stream << ' ';
    return true;
  }
  return false;
}

void Output::postflightKey(void*) {
  if (StateStack.back() == ObjectFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(ObjectOtherKey);
  }
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(const char *Str, bool Match) {
  if (Match && !EnumerationMatchFound) {
    toolchain::StringRef StrRef(Str);
    scalarString(StrRef, true);
    EnumerationMatchFound = true;
  }
  return false;
}

void Output::endEnumScalar() {
  if (!EnumerationMatchFound)
    toolchain_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  Stream << '[';
  if (PrettyPrint)
    Stream << ' ';
  NeedBitValueComma = false;
  DoClear = false;
  return true;
}

bool Output::bitSetMatch(const char *Str, bool Matches) {
  if (Matches) {
    if (NeedBitValueComma) {
      Stream << ',';
      if (PrettyPrint)
        Stream << ' ';
    }
    toolchain::StringRef StrRef(Str);
    scalarString(StrRef, true);
    NeedBitValueComma = true;
  }
  return false;
}

void Output::endBitSetScalar() {
  if (PrettyPrint)
    Stream << ' ';
  Stream << ']';
}

void Output::scalarString(toolchain::StringRef &S, bool MustQuote) {
  if (MustQuote) {
    Stream << '"';
    for (unsigned char c : S) {
      // According to the JSON standard, the following characters must be
      // escaped:
      //   - Quotation mark (U+0022)
      //   - Reverse solidus (U+005C)
      //   - Control characters (U+0000 to U+001F)
      // We need to check for these and escape them if present.
      //
      // Since these are represented by a single byte in UTF8 (and will not be
      // present in any multi-byte UTF8 representations), we can just switch on
      // the value of the current byte.
      //
      // Any other bytes present in the string should therefore be emitted
      // as-is, without any escaping.
      switch (c) {
      // First, check for characters for which JSON has custom escape sequences.
      case '"':
        Stream << '\\' << '"';
        break;
      case '\\':
        Stream << '\\' << '\\';
        break;
      case '/':
        Stream << '\\' << '/';
        break;
      case '\b':
        Stream << '\\' << 'b';
        break;
      case '\f':
        Stream << '\\' << 'f';
        break;
      case '\n':
        Stream << '\\' << 'n';
        break;
      case '\r':
        Stream << '\\' << 'r';
        break;
      case '\t':
        Stream << '\\' << 't';
        break;
      default:
        // Otherwise, check to see if the current byte is a control character.
        if (c <= '\x1F') {
          // Since we have a control character, we need to escape it using
          // JSON's only valid escape sequence: \uxxxx (where x is a hex digit).

          // The upper two digits for control characters are always 00.
          Stream << "\\u00";

          // Convert the current character into hexadecimal digits.
          Stream << toolchain::hexdigit((c >> 4) & 0xF);
          Stream << toolchain::hexdigit((c >> 0) & 0xF);
        } else {
          // This isn't a control character, so we don't need to escape it.
          // As a result, emit it directly; if it's part of a multi-byte UTF8
          // representation, all bytes will be emitted in this fashion.
          Stream << c;
        }
        break;
      }
    }
    Stream << '"';
  }
  else
    Stream << S;
}

void Output::null() {
  Stream << "null";
}

void Output::indent() {
  Stream.indent(StateStack.size() * 2);
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

toolchain::StringRef ScalarReferenceTraits<bool>::stringRef(const bool &Val) {
  return (Val ? "true" : "false");
}

toolchain::StringRef
ScalarReferenceTraits<toolchain::StringRef>::stringRef(const toolchain::StringRef &Val) {
  return Val;
}

toolchain::StringRef
ScalarReferenceTraits<std::string>::stringRef(const std::string &Val) {
  return Val;
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, toolchain::raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val,
                                    toolchain::raw_ostream &Out) {
  Out << Val;
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val,
                                    toolchain::raw_ostream &Out) {
  Out << Val;
}

#if defined(_MSC_VER)
void ScalarTraits<unsigned long>::output(const unsigned long &Val,
                                         toolchain::raw_ostream &Out) {
  Out << Val;
}
#endif

void ScalarTraits<uint64_t>::output(const uint64_t &Val,
                                    toolchain::raw_ostream &Out) {
  Out << Val;
}

void ScalarTraits<int8_t>::output(const int8_t &Val, toolchain::raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

void ScalarTraits<int16_t>::output(const int16_t &Val, toolchain::raw_ostream &Out) {
  Out << Val;
}

void ScalarTraits<int32_t>::output(const int32_t &Val, toolchain::raw_ostream &Out) {
  Out << Val;
}

void ScalarTraits<int64_t>::output(const int64_t &Val, toolchain::raw_ostream &Out) {
  Out << Val;
}

void ScalarTraits<double>::output(const double &Val, toolchain::raw_ostream &Out) {
  Out << toolchain::format("%g", Val);
}

void ScalarTraits<float>::output(const float &Val, toolchain::raw_ostream &Out) {
  Out << toolchain::format("%g", Val);
}
