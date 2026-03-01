//===- Layout.h -----------------------------------------------------------===//
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

// Convenience macros for obtaining offsets of members in structs.
//
// Usage:
//
//   #define FOR_EACH_FOO_FIELD(DO) \
//     DO(Ptr, bar)                 \
//     DO(uint32_t, baz)            \
//   CREATE_LAYOUT_CLASS(Foo, FOR_EACH_FOO_FIELD)
//   #undef FOR_EACH_FOO_FIELD
//
// This will generate
//
//   struct FooLayout {
//     uint32_t barOffset;
//     uint32_t bazOffset;
//     uint32_t totalSize;
//
//     FooLayout(size_t wordSize) {
//       if (wordSize == 8)
//         init<uint64_t>();
//       else {
//         assert(wordSize == 4);
//         init<uint32_t>();
//       }
//     }
//
//   private:
//     template <class Ptr> void init() {
//       FOR_EACH_FIELD(_INIT_OFFSET);
//       barOffset = offsetof(Layout<Ptr>, bar);
//       bazOffset = offsetof(Layout<Ptr>, baz);
//       totalSize = sizeof(Layout<Ptr>);
//     }
//     template <class Ptr> struct Layout {
//       Ptr bar;
//       uint32_t baz;
//     };
//   };

#define _OFFSET_FOR_FIELD(_, name) uint32_t name##Offset;
#define _INIT_OFFSET(type, name) name##Offset = offsetof(Layout<Ptr>, name);
#define _LAYOUT_ENTRY(type, name) type name;

#define CREATE_LAYOUT_CLASS(className, FOR_EACH_FIELD)                         \
  struct className##Layout {                                                   \
    FOR_EACH_FIELD(_OFFSET_FOR_FIELD)                                          \
    uint32_t totalSize;                                                        \
                                                                               \
    className##Layout(size_t wordSize) {                                       \
      if (wordSize == 8)                                                       \
        init<uint64_t>();                                                      \
      else {                                                                   \
        assert(wordSize == 4);                                                 \
        init<uint32_t>();                                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
  private:                                                                     \
    template <class Ptr> void init() {                                         \
      FOR_EACH_FIELD(_INIT_OFFSET);                                            \
      totalSize = sizeof(Layout<Ptr>);                                         \
    }                                                                          \
    template <class Ptr> struct Layout {                                       \
      FOR_EACH_FIELD(_LAYOUT_ENTRY)                                            \
    };                                                                         \
  }
