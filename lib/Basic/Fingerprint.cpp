//===--- Fingerprint.cpp - A stable identity for compiler data --*- C++ -*-===//
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
#include "language/Basic/Fingerprint.h"
#include "language/Basic/STLExtras.h"
#include "toolchain/Support/Format.h"
#include "toolchain/Support/raw_ostream.h"

#include <inttypes.h>
#include <sstream>

using namespace language;

toolchain::raw_ostream &toolchain::operator<<(toolchain::raw_ostream &OS,
                                    const Fingerprint &FP) {
  return OS << FP.getRawValue();
}

void language::simple_display(toolchain::raw_ostream &out, const Fingerprint &fp) {
  out << fp.getRawValue();
}

std::optional<Fingerprint> Fingerprint::fromString(toolchain::StringRef value) {
  assert(value.size() == Fingerprint::DIGEST_LENGTH &&
         "Only supports 32-byte hash values!");
  auto fp = Fingerprint::ZERO();
  {
    std::istringstream s(value.drop_back(Fingerprint::DIGEST_LENGTH/2).str());
    s >> std::hex >> fp.core.first;
  }
  {
    std::istringstream s(value.drop_front(Fingerprint::DIGEST_LENGTH/2).str());
    s >> std::hex >> fp.core.second;
  }
  // If the input string is not valid hex, the conversion above can fail.
  if (value != fp.getRawValue())
    return std::nullopt;

  return fp;
}

toolchain::SmallString<Fingerprint::DIGEST_LENGTH> Fingerprint::getRawValue() const {
  toolchain::SmallString<Fingerprint::DIGEST_LENGTH> Str;
  toolchain::raw_svector_ostream Res(Str);
  Res << toolchain::format_hex_no_prefix(core.first, 16);
  Res << toolchain::format_hex_no_prefix(core.second, 16);
  return Str;
}
