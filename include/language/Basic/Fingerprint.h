//===--- Fingerprint.h - A stable identity for compiler data ----*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_FINGERPRINT_H
#define LANGUAGE_BASIC_FINGERPRINT_H

#include "language/Basic/StableHasher.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>

#include <string>

namespace toolchain {
namespace yaml {
class IO;
}
} // namespace toolchain

namespace language {

/// A \c Fingerprint represents a stable summary of a given piece of data
/// in the compiler.
///
/// A \c Fingerprint value is subject to the following invariants:
/// 1) For two values \c x and \c y of type T, if \c T::operator==(x, y) is
///    \c true, then the Fingerprint of \c x and the Fingerprint of \c y must be
///    equal.
/// 2) For two values \c x and \c y of type T, the chance of a collision in
///    fingerprints is a rare occurrence - especially if \c y is a minor
///    perturbation of \c x.
/// 3) The \c Fingerprint value is required to be stable *across compilation
///    sessions*.
///
/// Property 3) is the most onerous. It implies that data like addresses, file
/// paths, and other ephemeral compiler state *may not* be used as inputs to the
/// fingerprint generation function.
///
/// \c Fingerprint values are currently used in two places by the compiler's
/// dependency tracking subsystem. They are used at the level of files to detect
/// when tokens (outside of the body of a function or an iterable decl context)
/// have been perturbed. Additionally, they are used at the level of individual
/// iterable decl contexts to detect when the tokens in their bodies have
/// changed. This makes them a coarse - yet safe - overapproximation for when a
/// decl has changed semantically.
class Fingerprint final {
public:
  /// The size (in bytes) of the raw value of all fingerprints.
  constexpr static size_t DIGEST_LENGTH = 32;

  using Core = std::pair<uint64_t, uint64_t>;
private:
  Core core;

  friend struct StableHasher::Combiner<language::Fingerprint>;

public:
  /// Creates a fingerprint value from a pair of 64-bit integers.
  explicit Fingerprint(Fingerprint::Core value) : core(value) {}

  /// Creates a fingerprint value from the given input string that is known to
  /// be a 32-byte hash value, i.e. that represent a valid 32-bit hex integer.
  ///
  /// Strings that violate this invariant will return a null optional.
  static std::optional<Fingerprint> fromString(toolchain::StringRef value);

  /// Creates a fingerprint value by consuming the given \c StableHasher.
  explicit Fingerprint(StableHasher &&stableHasher)
      : core{std::move(stableHasher).finalize()} {}

public:
  /// Retrieve the raw underlying bytes of this fingerprint.
  toolchain::SmallString<Fingerprint::DIGEST_LENGTH> getRawValue() const;

public:
  friend bool operator==(const Fingerprint &lhs, const Fingerprint &rhs) {
    return lhs.core == rhs.core;
  }

  friend bool operator!=(const Fingerprint &lhs, const Fingerprint &rhs) {
    return lhs.core != rhs.core;
  }

  friend toolchain::hash_code hash_value(const Fingerprint &fp) {
    return toolchain::hash_value(fp.core);
  }

public:
  bool operator<(const Fingerprint &other) const {
    return core < other.core;
  }

public:
  /// The fingerprint value consisting of 32 bytes of zeroes.
  ///
  /// This fingerprint is a perfectly fine value for a hash, but it is
  /// completely arbitrary.
  static Fingerprint ZERO() {
    return Fingerprint(Fingerprint::Core{0, 0});
  }

private:
  /// toolchain::yaml would like us to be default constructible, but \c Fingerprint
  /// would prefer to enforce its internal invariants.
  ///
  /// Very well, LLVM. A default value you shall have.
  friend class toolchain::yaml::IO;
  Fingerprint() : core{Fingerprint::Core{0, 0}} {}
};

void simple_display(toolchain::raw_ostream &out, const Fingerprint &fp);
} // namespace language

namespace language {

template <> struct StableHasher::Combiner<Fingerprint> {
  static void combine(StableHasher &hasher, const Fingerprint &Val) {
    // Our underlying buffer is already byte-swapped. Combine the
    // raw bytes from the core by hand.
    uint8_t buffer[8];
    memcpy(buffer, &Val.core.first, sizeof(buffer));
    hasher.combine<sizeof(buffer)>(buffer);
    memcpy(buffer, &Val.core.second, sizeof(buffer));
    hasher.combine<sizeof(buffer)>(buffer);
  }
};

} // namespace language

namespace toolchain {
class raw_ostream;
raw_ostream &operator<<(raw_ostream &OS, const language::Fingerprint &fp);
} // namespace toolchain

#endif // LANGUAGE_BASIC_FINGERPRINT_H
