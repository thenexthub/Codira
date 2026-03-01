//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONDEPARCH_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONDEPARCH_H

#include "vm/core/ADT/StringSwitch.h"
#include <optional>

namespace vm::core {
namespace Hexagon {
enum class ArchEnum {
  NoArch,
  Generic,
  V5,
  V55,
  V60,
  V62,
  V65,
  V66,
  V67,
  V68,
  V69,
  V71,
  V73,
  V75,
  V79,
  V81
};

inline std::optional<Hexagon::ArchEnum> getCpu(StringRef CPU) {
  return StringSwitch<std::optional<Hexagon::ArchEnum>>(CPU)
      .Case("generic", Hexagon::ArchEnum::V5)
      .Case("hexagonv5", Hexagon::ArchEnum::V5)
      .Case("hexagonv55", Hexagon::ArchEnum::V55)
      .Case("hexagonv60", Hexagon::ArchEnum::V60)
      .Case("hexagonv62", Hexagon::ArchEnum::V62)
      .Case("hexagonv65", Hexagon::ArchEnum::V65)
      .Case("hexagonv66", Hexagon::ArchEnum::V66)
      .Case("hexagonv67", Hexagon::ArchEnum::V67)
      .Case("hexagonv67t", Hexagon::ArchEnum::V67)
      .Case("hexagonv68", Hexagon::ArchEnum::V68)
      .Case("hexagonv69", Hexagon::ArchEnum::V69)
      .Case("hexagonv71", Hexagon::ArchEnum::V71)
      .Case("hexagonv71t", Hexagon::ArchEnum::V71)
      .Case("hexagonv73", Hexagon::ArchEnum::V73)
      .Case("hexagonv75", Hexagon::ArchEnum::V75)
      .Case("hexagonv79", Hexagon::ArchEnum::V79)
      .Case("hexagonv81", Hexagon::ArchEnum::V81)
      .Default(std::nullopt);
}
} // namespace Hexagon
} // namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONDEPARCH_H
