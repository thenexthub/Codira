//===--- CompatibilityOverride.cpp - Back-deploying compatibility fixes ---s-===//
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
// Support back-deploying compatibility fixes for newer apps running on older runtimes.
//
//===----------------------------------------------------------------------===//

#include "CompatibilityOverride.h"

#ifdef LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT

#include "../runtime/ImageInspection.h"
#include "language/Runtime/Once.h"
#include <assert.h>
#include <atomic>
#include <mach-o/getsect.h>
#include <type_traits>

using namespace language;

/// The definition of the contents of the override section.
///
/// The runtime looks in the main executable (not any libraries!) for a
/// __language54_hooks section and uses the hooks defined therein. This struct
/// defines the layout of that section. These hooks allow extending
/// runtime functionality when running apps built with a more recent
/// compiler. If additional hooks are needed, they may be added at the
/// end, but once ABI stability hits, existing ones must not be removed
/// or rearranged. The version number at the beginning can be used to
/// indicate the presence of added functions. Until we do so, the
/// version must be set to 0.
struct OverrideSection {
  uintptr_t version;
  
#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  Override_ ## name name;
#include "CompatibilityOverrideIncludePath.h"
};

static_assert(std::is_pod<OverrideSection>::value,
              "OverrideSection has a set layout and must be POD.");

// We only support mach-o for overrides, so the implementation of lookupSection
// can be mach-o specific.
#if __POINTER_WIDTH__ == 64
using mach_header_platform = mach_header_64;
#else
using mach_header_platform = mach_header;
#endif

extern "C" mach_header_platform *_NSGetMachExecuteHeader();
static void *lookupSection(const char *segment, const char *section,
                           size_t *outSize) {
  unsigned long size;
  auto *executableHeader = _NSGetMachExecuteHeader();
  uint8_t *data = getsectiondata(executableHeader, segment, section, &size);
  if (outSize != nullptr && data != nullptr)
    *outSize = size;
  return static_cast<void *>(data);
}

static OverrideSection *getOverrideSectionPtr() {
  static OverrideSection *OverrideSectionPtr;
  static language_once_t Predicate;
  language_once(&Predicate, [](void *) {
    size_t Size;
    OverrideSectionPtr = static_cast<OverrideSection *>(
        lookupSection("__DATA", COMPATIBILITY_OVERRIDE_SECTION_NAME, &Size));
    if (Size < sizeof(OverrideSection))
      OverrideSectionPtr = nullptr;
  }, nullptr);
  
  return OverrideSectionPtr;
}

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  Override_ ## name language::getOverride_ ## name() {                 \
    auto *Section = getOverrideSectionPtr();                        \
    if (Section == nullptr)                                         \
      return nullptr;                                               \
    return Section->name;                                           \
  }

#define OVERRIDE_NORETURN(name, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  Override_ ## name language::getOverride_ ## name() {                 \
    auto *Section = getOverrideSectionPtr();                        \
    if (Section == nullptr)                                         \
      nullptr;                                               \
    Section->name;                                           \
  }

#include "CompatibilityOverrideIncludePath.h"

#endif // #ifdef LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT
