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

#include "CodeSemanticVersion.h"
#include "ExceptionManager.h"
#include "Common/Runtime.h"
#include "os/Path.h"

namespace MapleRuntime {
const char* g_stableVersion = "0.59.6";
CodeSemanticVersion::CodeSemanticVersion()
{
    CString runtimeVersion(GetRuntimeSDKVersion());
    runtimeSemanticVersionInfo = SemanticVersionInfo(runtimeVersion);
    CString stableVersion(g_stableVersion);
    stableSemanticVersionInfo = SemanticVersionInfo(stableVersion);
}

const char* CodeSemanticVersion::GetRuntimeSDKVersion()
{
#ifdef CODE_SDK_VERSION
    return CODE_SDK_VERSION;
#else
    return nullptr;
#endif
}

bool CodeSemanticVersion::CheckPackageCompatibility(CString& packageName, CString& binaryVersion)
{
    if (IsCompatible(binaryVersion)) {
        return true;
    }
#ifndef DISABLE_VERSION_CHECK
    if (IsCorePackage(packageName)) {
        // The exception cannot be thrown when the core package is incompatible.
        LOG(RTLOG_FATAL,
            "executable cangjie file %s version %s is not compatible with deployed cangjie runtime version %s",
            packageName.Str(), binaryVersion.Str(), GetRuntimeSDKVersion());
    } else {
        LOG(RTLOG_ERROR,
            "executable cangjie file %s version %s is not compatible with deployed cangjie runtime version %s",
            packageName.Str(), binaryVersion.Str(), GetRuntimeSDKVersion());
    }
#endif
    return false;
}

bool CodeSemanticVersion::IsCompatible(CString& binaryVersion)
{
    SemanticVersionInfo binarySemanticVersionInfo(binaryVersion);
    // Unstable versions are compatible only when the versions are the same.
    if (IsUnstableVersion(binarySemanticVersionInfo) || IsUnstableVersion(runtimeSemanticVersionInfo)) {
        return binarySemanticVersionInfo.major == runtimeSemanticVersionInfo.major &&
            binarySemanticVersionInfo.minor == runtimeSemanticVersionInfo.minor &&
            binarySemanticVersionInfo.patch == runtimeSemanticVersionInfo.patch;
    }
    // Released versions are compatible when the major version is the same.
    if (binarySemanticVersionInfo.major > 1 || runtimeSemanticVersionInfo.major > 1) {
        return binarySemanticVersionInfo.major == runtimeSemanticVersionInfo.major;
    }
    return true;
}

#ifndef DISABLE_VERSION_CHECK
bool CodeSemanticVersion::IsCorePackage(CString& packageName)
{
    CString baseName = Os::Path::GetBaseName(packageName.Str());
    return baseName.Find("cangjie-std-core") != -1;
}
#endif

SemanticVersionInfo::SemanticVersionInfo(CString& version)
{
    if (version.IsEmpty()) {
        return;
    }
    CString coreVersion;
    int dashPos = version.Find('-');
    int plusPos = version.Find('+');
    int endPos = static_cast<int>(version.Length() - 1);
    if (dashPos == endPos || plusPos == endPos) {
        LOG(RTLOG_ERROR, "The version %s is incorrect.", version.Str());
        return;
    }
    if (dashPos >= 0 && plusPos >= 0) {
        if (dashPos > plusPos) {
            LOG(RTLOG_ERROR, "The version %s is incorrect.", version.Str());
            return;
        }
        coreVersion = version.SubStr(0, dashPos);
        preRelease = version.SubStr(dashPos + 1, plusPos - dashPos - 1);
        buildMetaData = version.SubStr(plusPos + 1);
    } else if (dashPos < 0 && plusPos < 0) {
        coreVersion = version;
    } else if (dashPos < 0) {
        coreVersion = version.SubStr(0, plusPos);
        buildMetaData = version.SubStr(plusPos + 1);
    } else {
        coreVersion = version.SubStr(0, dashPos);
        preRelease = version.SubStr(dashPos + 1);
    }
    auto tokens = CString::Split(coreVersion, '.');
    // 3: A normal version number MUST consists of three parts, MAJOR.MINOR.PATCH.
    if (tokens.size() != 3) {
        LOG(RTLOG_ERROR, "The version %s is incorrect.", version.Str());
        return;
    }
    major = CString::ParsePosNumFromEnv(tokens[static_cast<size_t>(VersionType::MAJOR)]);
    minor = CString::ParsePosNumFromEnv(tokens[static_cast<size_t>(VersionType::MINOR)]);
    patch = CString::ParsePosNumFromEnv(tokens[static_cast<size_t>(VersionType::PATCH)]);
}

bool CodeSemanticVersion::IsUnstableVersion(SemanticVersionInfo& version)
{
    if (version.major > stableSemanticVersionInfo.major) {
        return false;
    }
    if (version.minor > stableSemanticVersionInfo.minor) {
        return false;
    }
    if (version.minor == stableSemanticVersionInfo.minor &&
        version.patch >= stableSemanticVersionInfo.patch) {
        return false;
    }
    return true;
}
} // namespace MapleRuntime
