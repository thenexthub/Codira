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

#include "StructuralRuleGFMT01.h"
#include "Codira/Basic/Match.h"
#include "Codira/Utils/FileUtil.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

const int UTF8_FIRST_BYTE = 1;
const int UFT8_SECOND_BYTE = 2;
const int UFT8_THIRD_BYTE = 3;
const int UTF8_FOURTH_BYTE = 4;

bool StructuralRuleGFMT01::CheckUTF8Text(unsigned char *start, unsigned char *end) const
{
    bool isUTF8 = true;
    while (start < end) {
        if (*start < 0x80) { // (10000000) ASCII range
            ++start;
        } else if (*start < (0xC0)) { // (11000000) Invalid UTF8 range
            isUTF8 = false;
            break;
        } else if (*start < (0xE0)) { // (11100000) Two byte UTF8 range
            if (start >= end - UTF8_FIRST_BYTE) {
                break;
            }

            if ((start[UTF8_FIRST_BYTE] & (0xC0)) != 0x80) {
                isUTF8 = false;
                break;
            }

            start += UFT8_SECOND_BYTE;
        } else if (*start < (0xF0)) { // (11110000) Three byte UTF8 range
            if (start >= end - UFT8_SECOND_BYTE) {
                break;
            }

            if ((start[UTF8_FIRST_BYTE] & (0xC0)) != 0x80 || (start[UFT8_SECOND_BYTE] & (0xC0)) != 0x80) {
                isUTF8 = false;
                break;
            }

            start += UFT8_THIRD_BYTE;
        } else if (*start < (0xF8)) { // (11111000) Four byte UTF8 range
            if (start >= end - UFT8_THIRD_BYTE) {
                break;
            }

            if ((start[UTF8_FIRST_BYTE] & (0xC0)) != 0x80 || (start[UFT8_SECOND_BYTE] & (0xC0)) != 0x80 ||
                (start[UFT8_THIRD_BYTE] & (0xC0)) != 0x80) {
                isUTF8 = false;
                break;
            }

            start += UTF8_FOURTH_BYTE;
        } else {
            isUTF8 = false;
            break;
        }
    }

    return isUTF8;
}

bool StructuralRuleGFMT01::CheckUTF8File(const std::string &filePath)
{
    if (IsEmptyFile(filePath)) {
        return true;
    }
    std::ifstream in(filePath);
    if (in) {
        std::vector<unsigned char> buffer;
        unsigned char data;
        while (in >> data) {
            buffer.push_back(data);
        }
        return CheckUTF8Text(buffer.data(), buffer.data() + buffer.size());
    }
    return false;
}

void StructuralRuleGFMT01::GetFileFromNode(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    match (*node)([this](const Package &package) {
        for (auto &it : package.files) {
            this->filepath = it->filePath;
            this->pos = it->begin;
            if (!CheckUTF8File(this->filepath.c_str())) {
                Diagnose(pos, pos, CodeCheckDiagKind::G_FMT_01_encoding_UTF_8_information, "");
            }
        }
    });
}

void StructuralRuleGFMT01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    GetFileFromNode(node);
}
bool StructuralRuleGFMT01::IsEmptyFile(const std::string &path) const
{
    // If file doesn't exist, it's empty.
    if (!FileUtil::FileExist(path)) {
        return true;
    }
    size_t sz = FileUtil::GetFileSize(path);
    return sz == 0;
}
} // namespace Codira::CodeCheck
