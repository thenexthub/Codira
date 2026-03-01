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


#include "CodiraDemangle.h"
#ifndef BUILD_LIB_CODIRA_DEMANGLE
#define BUILD_LIB_CODIRA_DEMANGLE
#endif
#include "Demangler.h"
#include "StdString.h"

namespace Codira {
using DemangleMetaData = DemangleInfo<StdString>;

std::string DemangleData::GetPkgName() const { return pkgName; }

std::string DemangleData::GetFullName() const { return fullName; }

bool DemangleData::IsFunctionLike() const { return isFunctionLike; }

bool DemangleData::IsValid() const { return isValid; }

void DemangleData::SetPrivateDeclaration(bool isPrivate) { isPrivateDeclaration = isPrivate; }

bool DemangleData::IsPrivateDeclaration() const { return isPrivateDeclaration; }

DemangleData Demangle(const std::string& mangled, const std::string& scopeRes)
{
    auto demangler = Demangler<StdString>(mangled.c_str(), scopeRes);
    auto di = demangler.Demangle();
    auto dd = DemangleData{ di.GetPkgName(), di.GetFullName(demangler.ScopeResolution()), di.IsFunctionLike(),
        di.IsValid() };
    dd.SetPrivateDeclaration(di.isPrivateDeclaration);
    return dd;
}

DemangleData Demangle(const std::string& mangled) { return Demangle(mangled, "::"); }

DemangleData Demangle(const std::string& mangled, const std::string& scopeRes,
    const std::vector<std::string>& genericVec)
{
    auto demangler = Demangler<StdString>(mangled.c_str(), scopeRes);
    demangler.setGenericVec(genericVec);
    auto di = demangler.Demangle();
    auto dd = DemangleData{ di.GetPkgName(), di.GetFullName(demangler.ScopeResolution()), di.IsFunctionLike(),
        di.IsValid() };
    dd.SetPrivateDeclaration(di.isPrivateDeclaration);
    return dd;
}

DemangleData Demangle(const std::string& mangled, const std::vector<std::string>& genericVec)
{
    return Demangle(mangled, "::", genericVec);
}

DemangleData DemangleType(const std::string& mangled, const std::string& scopeRes)
{
    auto demangler = Demangler<StdString>(mangled, scopeRes);
    auto di = demangler.Demangle(true);
    auto dd = DemangleData{ di.GetPkgName(), di.GetFullName(demangler.ScopeResolution()), false,
        di.IsValid()};
    dd.SetPrivateDeclaration(di.isPrivateDeclaration);
    return dd;
}

DemangleData DemangleType(const std::string& mangled) { return DemangleType(mangled, "::"); }
} // namespace Codira
