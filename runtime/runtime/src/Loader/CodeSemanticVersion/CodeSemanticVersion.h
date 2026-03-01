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


#ifndef MRT_CODESEMANTICVERSION_H
#define MRT_CODESEMANTICVERSION_H

#include "Base/CString.h"

namespace MapleRuntime {
enum class VersionType {
    MAJOR = 0,
    MINOR,
    PATCH,
    PRE_RELEASE,
    BUILD_METADATA,
    VERSION_TYPE_NUMBER
};

struct SemanticVersionInfo {
    size_t major;
    size_t minor;
    size_t patch;
    CString preRelease;
    CString buildMetaData;

    SemanticVersionInfo()
    {
        major = 0;
        minor = 0;
        patch = 0;
    }
    explicit SemanticVersionInfo(CString& version);
    ~SemanticVersionInfo() {}
};

class CodeSemanticVersion {
public:
    CodeSemanticVersion();
    ~CodeSemanticVersion() {}
    const char* GetRuntimeSDKVersion();
    bool CheckPackageCompatibility(CString& packageName, CString& binaryVersion);
private:
    bool IsCompatible(CString& binaryVersion);
    bool IsSemver(CString& version);
#ifndef DISABLE_VERSION_CHECK
    bool IsCorePackage(CString& packageName);
#endif
    bool IsUnstableVersion(SemanticVersionInfo& version);

    SemanticVersionInfo runtimeSemanticVersionInfo;
    SemanticVersionInfo stableSemanticVersionInfo;
};
} // namespace MapleRuntime
#endif // MRT_CODESEMANTICVERSION_H
