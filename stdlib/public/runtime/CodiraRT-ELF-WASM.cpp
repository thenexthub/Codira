//===--- CodiraRT-ELF-WASM.cpp ---------------------------------------------===//
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

#include "ImageInspectionCommon.h"
#include "language/shims/MetadataSections.h"
#include "language/Runtime/Backtrace.h"
#include "language/Runtime/Config.h"

#include <cstddef>
#include <new>

#if defined(__ELF__)
extern "C" const char __dso_handle[];
#elif defined(__wasm__)
// NOTE: Multi images in a single process is not yet
// stabilized in WebAssembly toolchain outside of Emscripten.
static constexpr const void *__dso_handle = nullptr;
#endif

#if LANGUAGE_ENABLE_BACKTRACING
// Drag in a symbol from the backtracer, to force the static linker to include
// the code.
static const void *__backtraceRef __attribute__((used, retain))
  = (const void *)language::runtime::backtrace::_language_backtrace_isThunkFunction;
#endif

// Create empty sections to ensure that the start/stop symbols are synthesized
// by the linker.  Otherwise, we may end up with undefined symbol references as
// the linker table section was never constructed.
#if defined(__ELF__)
# define DECLARE_EMPTY_METADATA_SECTION(name, attrs) __asm__("\t.section " #name ",\"" attrs "\"\n");
#elif defined(__wasm__)
# define DECLARE_EMPTY_METADATA_SECTION(name, attrs) __asm__("\t.section " #name ",\"R\",@\n");
#endif

#define BOUNDS_VISIBILITY __attribute__((__visibility__("hidden"), \
                                         __aligned__(1)))

#define DECLARE_BOUNDS(name)                            \
  BOUNDS_VISIBILITY extern const char __start_##name;   \
  BOUNDS_VISIBILITY extern const char __stop_##name;

#define DECLARE_LANGUAGE_SECTION(name)             \
  DECLARE_EMPTY_METADATA_SECTION(name, "aR")    \
  DECLARE_BOUNDS(name)

// These may or may not be present, depending on compiler switches; it's
// worth calling them out as a result.
#define DECLARE_LANGUAGE_REFLECTION_SECTION(name)  \
  DECLARE_LANGUAGE_SECTION(name)

extern "C" {
DECLARE_LANGUAGE_SECTION(language5_protocols)
DECLARE_LANGUAGE_SECTION(language5_protocol_conformances)
DECLARE_LANGUAGE_SECTION(language5_type_metadata)

DECLARE_LANGUAGE_REFLECTION_SECTION(language5_fieldmd)
DECLARE_LANGUAGE_REFLECTION_SECTION(language5_builtin)
DECLARE_LANGUAGE_REFLECTION_SECTION(language5_assocty)
DECLARE_LANGUAGE_REFLECTION_SECTION(language5_capture)
DECLARE_LANGUAGE_REFLECTION_SECTION(language5_reflstr)
DECLARE_LANGUAGE_REFLECTION_SECTION(language5_typeref)
DECLARE_LANGUAGE_REFLECTION_SECTION(language5_mpenum)

DECLARE_LANGUAGE_SECTION(language5_replace)
DECLARE_LANGUAGE_SECTION(language5_replac2)
DECLARE_LANGUAGE_SECTION(language5_accessible_functions)
DECLARE_LANGUAGE_SECTION(language5_runtime_attributes)

DECLARE_LANGUAGE_SECTION(language5_tests)
}

#undef DECLARE_LANGUAGE_SECTION

namespace {
static language::MetadataSections sections{};
}

LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_BEGIN
__attribute__((__constructor__))
static void language_image_constructor() {
#define LANGUAGE_SECTION_RANGE(name)                                              \
  { reinterpret_cast<uintptr_t>(&__start_##name),                              \
    static_cast<uintptr_t>(&__stop_##name - &__start_##name) }

  ::new (&sections) language::MetadataSections {
      language::CurrentSectionMetadataVersion,
      { __dso_handle },

      nullptr,
      nullptr,

      LANGUAGE_SECTION_RANGE(language5_protocols),
      LANGUAGE_SECTION_RANGE(language5_protocol_conformances),
      LANGUAGE_SECTION_RANGE(language5_type_metadata),

      LANGUAGE_SECTION_RANGE(language5_typeref),
      LANGUAGE_SECTION_RANGE(language5_reflstr),
      LANGUAGE_SECTION_RANGE(language5_fieldmd),
      LANGUAGE_SECTION_RANGE(language5_assocty),
      LANGUAGE_SECTION_RANGE(language5_replace),
      LANGUAGE_SECTION_RANGE(language5_replac2),
      LANGUAGE_SECTION_RANGE(language5_builtin),
      LANGUAGE_SECTION_RANGE(language5_capture),
      LANGUAGE_SECTION_RANGE(language5_mpenum),
      LANGUAGE_SECTION_RANGE(language5_accessible_functions),
      LANGUAGE_SECTION_RANGE(language5_runtime_attributes),
      LANGUAGE_SECTION_RANGE(language5_tests),
  };

#undef LANGUAGE_SECTION_RANGE

  language_addNewDSOImage(&sections);
}
LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_END
