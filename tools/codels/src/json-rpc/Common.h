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

#ifndef LSPSERVER_COMMON_H
#define LSPSERVER_COMMON_H

#include "URI.h"
#include "Codira/Basic/Position.h"

namespace ark {
struct URIForFile {
    std::string file = "";
    bool operator==(const URIForFile &rhs) const
    {
        return this->file == rhs.file;
    }
    bool operator!=(const URIForFile &rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const URIForFile &rhs) const
    {
        return this->file < rhs.file;
    }
};

struct Range {
    Codira::Position start = {0, 0, 0};
    Codira::Position end = {0, 0, 0};

    bool operator==(const Range &rhs) const
    {
        return std::tie(this->start, this->end) == std::tie(rhs.start, rhs.end);
    }

    bool operator!=(const Range &rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(const Range &rhs) const
    {
        return std::tie(this->start, this->end) < std::tie(rhs.start, rhs.end);
    }
};

struct Location {
    // The text document's URI.
    URIForFile uri;
    Range range;
    bool operator==(const Location &rhs) const
    {
        return this->uri == rhs.uri && this->range == rhs.range;
    }

    bool operator!=(const Location &rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(const Location &rhs) const
    {
        return std::tie(this->uri, this->range) < std::tie(rhs.uri, rhs.range);
    }
};
};
#endif // LSPSERVER_COMMON_H
