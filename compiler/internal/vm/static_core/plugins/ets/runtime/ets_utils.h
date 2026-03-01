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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"

#include <string>
#include <map>
#include <algorithm>
#include <vector>

namespace ark::ets {

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const ETSGLOBAL_CLASS_NAME[] = "ETSGLOBAL";
// NOLINTEND(modernize-avoid-c-arrays)

static constexpr std::string_view INDEXED_INT_GET_METHOD_SIGNATURE = "I:Lstd/core/Object;";
static constexpr std::string_view INDEXED_INT_SET_METHOD_SIGNATURE = "ILstd/core/Object;:V";

bool IsEtsGlobalClassName(const std::string &descriptor);

EtsObject *GetBoxedValue(EtsCoroutine *coro, Value value, EtsType type);

Value GetUnboxedValue(EtsCoroutine *coro, EtsObject *obj);

EtsObject *GetPropertyValue(EtsCoroutine *coro, const EtsObject *etsObj, EtsField *field);
bool SetPropertyValue(EtsCoroutine *coro, EtsObject *etsObj, EtsField *field, EtsObject *valToSet);

class LambdaUtils {
public:
    PANDA_PUBLIC_API static void InvokeVoid(EtsCoroutine *coro, EtsObject *lambda);

    NO_COPY_SEMANTIC(LambdaUtils);
    NO_MOVE_SEMANTIC(LambdaUtils);

private:
    LambdaUtils() = default;
    ~LambdaUtils() = default;
};

class ManglingUtils {
public:
    PANDA_PUBLIC_API static EtsString *GetDisplayNameStringFromField(EtsField *field);
    PANDA_PUBLIC_API static EtsField *GetFieldIDByDisplayName(EtsClass *klass, const PandaString &name,
                                                              const char *sig = nullptr);

    NO_COPY_SEMANTIC(ManglingUtils);
    NO_MOVE_SEMANTIC(ManglingUtils);

private:
    ManglingUtils() = default;
    ~ManglingUtils() = default;
};

PANDA_PUBLIC_API bool GetExportedClassDescriptorsFromModule(EtsClass *etsGlobalClass,
                                                            std::vector<std::string> &outDescriptors);

class RuntimeDescriptorParser {
private:
    PandaString name;
    size_t pos;
    static const std::map<char, PandaString> primitiveNameMapping;

    PandaString ResolveRef()
    {
        PandaString res;
        while (name[pos] != ';') {
            char curSym = name[pos];
            res += (curSym != '/') ? curSym : '.';
            pos += 1;
        }
        pos += 1;
        return res;
    }

    PandaString ResolveUnion()
    {
        PandaString res = "{U";
        while (name[pos] != '}') {
            res += ResolveFixedArray();
            if (name[pos] != '}') {
                res += ',';
            }
        }
        pos++;
        res += '}';
        return res;
    }

    PandaString ResolveFixedArray()
    {
        size_t i = 0;

        while (pos + i < name.length() && name[pos + i] == '[') {
            i++;
        }

        size_t rank = i;
        pos += i;
        PandaString brackets;
        for (size_t k = 0; k < rank; ++k) {
            brackets += "[]";
        }

        ASSERT(pos < name.length());

        char typeChar = name[pos];
        if (typeChar == 'L') {
            pos++;
            return ResolveRef() + brackets;
        } else if (typeChar == '{') {
            pos++;
            if (pos < name.length() && name[pos] == 'U') {
                pos++;
                return ResolveUnion() + brackets;
            } else {
                UNREACHABLE();
            }
        } else {
            auto it = RuntimeDescriptorParser::primitiveNameMapping.find(typeChar);
            ASSERT(it != RuntimeDescriptorParser::primitiveNameMapping.end());
            PandaString primName = it->second;
            pos++;
            return primName + brackets;
        }
    }

public:
    RuntimeDescriptorParser(const PandaString &inputStr) : name(inputStr), pos(0) {}

    PandaString Resolve()
    {
        PandaString res = "";
        if (name.empty()) {
            return "";
        }
        if (!(name[0] == '{' || name[0] == '[')) {
            return name;
        }

        while (pos < name.length()) {
            res += ResolveFixedArray();
        }
        return res;
    }
};

class ClassPublicNameParser {
private:
    size_t left;
    size_t right;
    PandaString name;
    static const std::map<PandaString, char> primitiveNameMapping;

    static PandaVector<PandaString> SplitUnion(const PandaString &unionName)
    {
        size_t deep = 0;
        PandaVector<PandaString> res;
        for (size_t i = 0; i < unionName.length(); i++) {
            PandaString temp = "";
            bool start = true;
            while (i < unionName.length() && (unionName[i] != ',' || deep != 0)) {
                char sym = unionName[i];
                if (sym == '{') {
                    deep++;
                } else if (sym == '}') {
                    deep--;
                }

                temp += sym;
                i++;
                start = false;
            }
            if (!start) {
                res.push_back(temp);
            }
        }
        return res;
    }

    PandaString ResolveUnion()
    {
        PandaString res = "";
        PandaString unionContent = name.substr(left, right - left + 1);
        PandaVector<PandaString> typesArr = SplitUnion(unionContent);

        for (const PandaString &typeName : typesArr) {
            res += ClassPublicNameParser(typeName).Resolve();
        }
        return res;
    }

    PandaString ResolveFixedArray()
    {
        size_t i = 0;
        const size_t step = 2;
        while (right - i > left && name[right - i] == ']' && name[right - i - 1] == '[') {
            i += step;
        }

        right -= i;

        PandaString brackets = PandaString(i / step, '[');
        if (left <= right && name[left] == '{') {
            left += 1;
            if (left <= right && name[left] == 'U' && name[right] == '}') {
                left += 1;
                right -= 1;
                return brackets + "{U" + ResolveUnion() + "}";
            } else {
                // Not a recognized union format
                return "{}";
            }
        }

        PandaString typeName;
        if (left <= right) {
            typeName = name.substr(left, right - left + 1);
        } else {
            return brackets;
        }

        auto it = ClassPublicNameParser::primitiveNameMapping.find(typeName);
        if (it != ClassPublicNameParser::primitiveNameMapping.end()) {
            return brackets + PandaString(1, it->second);
        } else {
            PandaString refName = typeName;
            std::replace(refName.begin(), refName.end(), '.', '/');
            return brackets + 'L' + refName + ';';
        }
    }

public:
    ClassPublicNameParser(const PandaString &inputStr) : left(0), right(inputStr.length() - 1), name(inputStr) {}

    PandaString Resolve()
    {
        if (std::find(name.begin(), name.end(), '/') != name.end()) {
            return "";
        }
        return ResolveFixedArray();
    }
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H
