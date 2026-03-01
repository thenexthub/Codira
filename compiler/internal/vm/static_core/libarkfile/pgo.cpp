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

#include "libarkfile/pgo.h"
#include "libarkbase/utils/logger.h"

namespace ark::panda_file::pgo {

/* static */
std::string ProfileOptimizer::GetNameInfo(const std::unique_ptr<BaseItem> &item)
{
    std::string identity;
    if (item->GetName() == CLASS_ITEM) {
        identity = static_cast<ClassItem *>(item.get())->GetNameItem()->GetData();
        ASSERT(identity.find('L') == 0);                         // the first character must be 'L'
        ASSERT(identity.find(";\0") == identity.length() - 2U);  // must end with ";\0"
        identity = identity.substr(1, identity.length() - 3U);   // remove 'L' and ";\0"
        std::replace(identity.begin(), identity.end(), '/', '.');
    } else if (item->GetName() == STRING_ITEM) {
        identity = static_cast<StringItem *>(item.get())->GetData();
        ASSERT(identity.find('\0') == identity.length() - 1);  // must end with '\0'
        identity.pop_back();                                   // remove '\0'
    } else {
        UNREACHABLE();
    }
    return identity;
}

void ProfileOptimizer::MarkProfileItem(std::unique_ptr<BaseItem> &item, bool setPgo) const
{
    auto inc = static_cast<uint32_t>(setPgo);
    if (item->GetName() == CLASS_ITEM) {
        item->SetPGORank(PGO_CLASS_DEFAULT_COUNT + inc);
    } else if (item->GetName() == STRING_ITEM) {
        item->SetPGORank(PGO_STRING_DEFAULT_COUNT + inc);
    } else if (item->GetName() == CODE_ITEM) {
        item->SetPGORank(PGO_CODE_DEFAULT_COUNT + inc);
    } else {
        UNREACHABLE();
    }
}

bool ProfileOptimizer::ParseProfileData()
{
    std::ifstream file;
    file.open(profileFilePath_, std::ios::in);
    if (!file.is_open()) {
        LOG(ERROR, PANDAFILE) << "failed to open pgo files: " << profileFilePath_;
        return false;
    }
    std::string strLine;
    while (std::getline(file, strLine)) {
        if (strLine.empty()) {
            continue;
        }
        auto commaPos = strLine.find(':');
        if (commaPos == std::string::npos) {
            continue;
        }
        auto itemType = strLine.substr(0, commaPos);
        auto str = strLine.substr(commaPos + 1);
        profileData_.emplace_back(itemType, str);
    }

    return true;
}

static bool Cmp(const std::unique_ptr<BaseItem> &item1, const std::unique_ptr<BaseItem> &item2)
{
    if (item1->GetPGORank() == item2->GetPGORank()) {
        return item1->GetOriginalRank() < item2->GetOriginalRank();
    }
    if ((item1->GetName() != CODE_ITEM && item2->GetName() != CODE_ITEM) ||
        (item1->GetName() == CODE_ITEM && item2->GetName() == CODE_ITEM)) {
        return item1->GetPGORank() > item2->GetPGORank();
    }
    // code items will depends on the layout of string and literal item, so put it on the end
    return item1->GetName() != CODE_ITEM;
}

void ProfileOptimizer::ProfileGuidedRelayout(std::list<std::unique_ptr<BaseItem>> &items)
{
    ParseProfileData();
    uint32_t originalRank = 0;
    for (auto &item : items) {
        item->SetOriginalRank(originalRank++);
        if (!item->NeedsEmit()) {
            continue;
        }
        auto typeName = item->GetName();
        if (typeName != CLASS_ITEM && typeName != STRING_ITEM && typeName != CODE_ITEM) {
            continue;
        }

        MarkProfileItem(item, false);

        auto finder = [&item](const std::pair<std::string, std::string> &p) {
            if (p.first != item->GetName()) {
                return false;
            }
            if (item->GetName() != CODE_ITEM) {
                return p.second == GetNameInfo(item);
            }
            // CodeItem can be shared between multiple methods, so we need to check all these methods
            auto methodNames = static_cast<CodeItem *>(item.get())->GetMethodNames();
            return std::find(methodNames.begin(), methodNames.end(), p.second) != methodNames.end();
        };
        if (std::find_if(profileData_.begin(), profileData_.end(), finder) != profileData_.end()) {
            MarkProfileItem(item, true);
        }
    }

    items.sort(Cmp);
}

}  // namespace ark::panda_file::pgo
