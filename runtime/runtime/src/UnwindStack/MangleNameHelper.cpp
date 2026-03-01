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


#include "MangleNameHelper.h"

#include <cctype>

#include "Utils/Demangler.h"

namespace MapleRuntime {
CString MangleNameHelper::RemoveGenericTypeName(const MapleRuntime::CString& rawName)
{
    MapleRuntime::CString name;
    int preBracketNum = 0;
    for (int i = 0; i < rawName.Length(); ++i) {
        if (preBracketNum == 0) {
            if (rawName[i] == '<') {
                ++preBracketNum;
            }
            name.Append(CString(rawName[i]));
        } else {
            if (rawName[i] == '<') {
                ++preBracketNum;
            } else if (rawName[i] == '>' && --preBracketNum == 0) {
                name.Append("...>");
            }
        }
    }
    return name;
}

void MangleNameHelper::Demangle()
{
    std::function<MapleRuntime::CString(const MapleRuntime::CString&)> genericTypefilter;
    if (stackTraceFormat == StackTraceFormatFlag::ALL) {
        genericTypefilter = [](const MapleRuntime::CString& str) { return str; };
    } else {
        genericTypefilter = [](const MapleRuntime::CString& str) { return RemoveGenericTypeName(str); };
    }
    const char POSTFIX_WITHOUT_TI[] = "$withoutTI";
    if (mangleName.EndsWith(POSTFIX_WITHOUT_TI)) {
        mangleName.Truncate(mangleName.Length() - strlen(POSTFIX_WITHOUT_TI));
    }
    auto demangler = CreateNativeDemangler(mangleName.Str(), "::", genericTypefilter);
    auto functionDemangleInfo = demangler.Demangle();
    methodName = functionDemangleInfo.GetFullName(demangler.ScopeResolution());
    className = functionDemangleInfo.GetPkgName();
    demangleName = className + (className.Length() > 0 ? "." : "") + methodName;
}

CString MangleNameHelper::GetSimpleClassName() const
{
    auto demangler = CreateNativeDemangler(mangleName.Str());
    return demangler.DemangleClassType();
}

bool MangleNameHelper::IsNeedFilt() const
{
    if (mangleName.IsEmpty()) {
        return false;
    }

    for (auto filtMangleName : filtMangleNameSet) {
        if (mangleName.Find(filtMangleName.Str()) == 0) {
            return true;
        }
    }
    return false;
}
} // namespace MapleRuntime
