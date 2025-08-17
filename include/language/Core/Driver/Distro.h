//===--- Distro.h - Linux distribution detection support --------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_DRIVER_DISTRO_H
#define LANGUAGE_CORE_DRIVER_DISTRO_H

#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace driver {

/// Distro - Helper class for detecting and classifying Linux distributions.
///
/// This class encapsulates the clang Linux distribution detection mechanism
/// as well as helper functions that match the specific (versioned) results
/// into wider distribution classes.
class Distro {
public:
  enum DistroType {
    // Special value means that no detection was performed yet.
    UninitializedDistro,
    // NB: Releases of a particular Linux distro should be kept together
    // in this enum, because some tests are done by integer comparison against
    // the first and last known member in the family, e.g. IsRedHat().
    AlpineLinux,
    ArchLinux,
    DebianLenny,
    DebianSqueeze,
    DebianWheezy,
    DebianJessie,
    DebianStretch,
    DebianBuster,
    DebianBullseye,
    DebianBookworm,
    DebianTrixie,
    DebianForky,
    DebianDuke,
    Exherbo,
    RHEL5,
    RHEL6,
    RHEL7,
    Fedora,
    Gentoo,
    OpenSUSE,
    UbuntuHardy,
    UbuntuIntrepid,
    UbuntuJaunty,
    UbuntuKarmic,
    UbuntuLucid,
    UbuntuMaverick,
    UbuntuNatty,
    UbuntuOneiric,
    UbuntuPrecise,
    UbuntuQuantal,
    UbuntuRaring,
    UbuntuSaucy,
    UbuntuTrusty,
    UbuntuUtopic,
    UbuntuVivid,
    UbuntuWily,
    UbuntuXenial,
    UbuntuYakkety,
    UbuntuZesty,
    UbuntuArtful,
    UbuntuBionic,
    UbuntuCosmic,
    UbuntuDisco,
    UbuntuEoan,
    UbuntuFocal,
    UbuntuGroovy,
    UbuntuHirsute,
    UbuntuImpish,
    UbuntuJammy,
    UbuntuKinetic,
    UbuntuLunar,
    UbuntuMantic,
    UbuntuNoble,
    UbuntuOracular,
    UbuntuPlucky,
    UbuntuQuesting,
    UnknownDistro
  };

private:
  /// The distribution, possibly with specific version.
  DistroType DistroVal;

public:
  /// @name Constructors
  /// @{

  /// Default constructor leaves the distribution unknown.
  Distro() : DistroVal() {}

  /// Constructs a Distro type for specific distribution.
  Distro(DistroType D) : DistroVal(D) {}

  /// Detects the distribution using specified VFS.
  explicit Distro(toolchain::vfs::FileSystem &VFS, const toolchain::Triple &TargetOrHost);

  bool operator==(const Distro &Other) const {
    return DistroVal == Other.DistroVal;
  }

  bool operator!=(const Distro &Other) const {
    return DistroVal != Other.DistroVal;
  }

  bool operator>=(const Distro &Other) const {
    return DistroVal >= Other.DistroVal;
  }

  bool operator<=(const Distro &Other) const {
    return DistroVal <= Other.DistroVal;
  }

  /// @}
  /// @name Convenience Predicates
  /// @{

  bool IsRedhat() const {
    return DistroVal == Fedora || (DistroVal >= RHEL5 && DistroVal <= RHEL7);
  }

  bool IsOpenSUSE() const { return DistroVal == OpenSUSE; }

  bool IsDebian() const {
    return DistroVal >= DebianLenny && DistroVal <= DebianDuke;
  }

  bool IsUbuntu() const {
    return DistroVal >= UbuntuHardy && DistroVal <= UbuntuQuesting;
  }

  bool IsAlpineLinux() const { return DistroVal == AlpineLinux; }

  bool IsGentoo() const { return DistroVal == Gentoo; }

  /// @}
};

} // end namespace driver
} // end namespace language::Core

#endif
