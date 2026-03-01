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

#ifndef LSPSERVER_DOCCACHE_H
#define LSPSERVER_DOCCACHE_H

#include <string>
#include <map>
#include <mutex>
#include <cstdint>
#include "../json-rpc/Protocol.h"
namespace ark {
class DocCache {
public:
    struct Doc {
        std::string contents = "";
        std::int64_t version = -1;
        bool needReParser = false;
        bool isInitCompiled = false;
    };

    std::int64_t AddDoc(const std::string &file, int64_t version, std::string contents);

    void AddDocWhenInitCompile(const std::string &file);

    void RemoveDoc(const std::string &file);

    Doc GetDoc(const std::string &file);

    bool UpdateDoc(const std::string &file, std::int64_t version, bool needReParser,
                   const std::vector<TextDocumentContentChangeEvent> &contentChanges);

    void UpdateDocNeedReparse(const std::string &file, int64_t version, bool needReParser);

private:
    mutable std::mutex mutex;
    std::map<std::string, Doc> Docs;
};
} // namespace ark

#endif // LSPSERVER_DOCCACHE_H
