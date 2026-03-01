/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file declares some condition check helper macros.
 */

#ifndef CODIRA_UTILS_CHECKUTILS_H
#define CODIRA_UTILS_CHECKUTILS_H

#include <cassert>
#include <cstdlib>
#include <string>

namespace {
    inline const char* to_cstring(const char* s)
    {
        return s;
    }
    
    inline const char* to_cstring(const std::string& s)
    {
        return s.c_str();
    }
}

#ifdef CMAKE_ENABLE_ASSERT
#define CODEC_ASSERT(f)                                                                                                  \
    {                                                                                                                  \
        if (!(f)) {                                                                                                    \
            abort();                                                                                                   \
        }                                                                                                              \
    }
#define CODEC_ASSERT_WITH_MSG(f, msg)                                                                                    \
    {                                                                                                                  \
        if (!(f)) {                                                                                                    \
            fprintf(stderr, "CODEC_ASSERT failed at %s:%d: %s\n", __FILE__, __LINE__,                                    \
                    to_cstring(msg));                                                                                  \
            abort();                                                                                                   \
        }                                                                                                              \
    }
#define CODEC_ABORT() abort()
#define CODEC_ABORT_WITH_MSG(msg)                                                                                        \
    {                                                                                                                  \
        fprintf(stderr, "CODEC_ABORT at %s:%d: %s\n", __FILE__, __LINE__, to_cstring(msg));                              \
        abort();                                                                                                       \
    }
#else
#ifdef NDEBUG
#define CODEC_ASSERT(f) static_cast<void>(f)
#define CODEC_ASSERT_WITH_MSG(f, msg) (static_cast<void>(f), static_cast<void>(msg))
#define CODEC_ABORT()
#define CODEC_ABORT_WITH_MSG(msg) static_cast<void>(msg)
#else
#define CODEC_ASSERT(f) assert(f)
#define CODEC_ASSERT_WITH_MSG(f, msg)                                                                                    \
    {                                                                                                                  \
        if (!(f)) {                                                                                                    \
            fprintf(stderr, "CODEC_ASSERT failed at %s:%d: %s\n", __FILE__, __LINE__,                                    \
                    to_cstring(msg));                                                                                  \
            assert(f);                                                                                                 \
        }                                                                                                              \
    }
#define CODEC_ABORT() abort()
#define CODEC_ABORT_WITH_MSG(msg)                                                                                        \
    {                                                                                                                  \
        fprintf(stderr, "CODEC_ABORT at %s:%d: %s\n", __FILE__, __LINE__, to_cstring(msg));                              \
        abort();                                                                                                       \
    }
#endif
#endif

#define CODEC_NULLPTR_CHECK(p) CODEC_ASSERT((p) != nullptr)

#endif
