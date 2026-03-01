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


#ifndef MRT_MANGLE_NAME_H
#define MRT_MANGLE_NAME_H

#include <set>

#include "Base/CString.h"
#include "Base/Macros.h"

namespace MapleRuntime {
enum class StackTraceFormatFlag : int8_t {
    SIMPLE = 0,
    DEFAULT,
    ALL
};

class MangleNameHelper {
public:
    explicit MangleNameHelper(CString name, StackTraceFormatFlag format = StackTraceFormatFlag::DEFAULT)
        : mangleName(name), demangleName(""), packName(""), className(""), methodName(""), stackTraceFormat(format) {}
    MangleNameHelper() = delete;
    ~MangleNameHelper() = default;

    // Return true if mangleName is in the filtMangleNameSet.
    bool IsNeedFilt() const;

    CString GetPackName() const { return packName; }

    CString GetClassName() const { return className; }

    CString GetMethodName() const { return methodName; }

    CString GetMangleName() const { return mangleName; }

    CString GetDemangleName() const { return demangleName; }

    CString GetPackClassName() const
    {
        if (packName != "" && className != "") {
            return packName + "." + className;
        } else {
            return packName + className;
        }
    }

    CString GetSimpleClassName() const;

    static CString RemoveGenericTypeName(const MapleRuntime::CString& rawName);

    void Demangle();

private:
    CString mangleName;
    // demangle name
    CString demangleName;
    // demangle packName.
    CString packName;
    // demangle className.
    CString className;
    // demangle methodName.
    CString methodName;
    StackTraceFormatFlag stackTraceFormat;

    // Filter out the basic library functions when the exception stacktrace is dumped
    std::set<CString> filtMangleNameSet = {
#ifdef __APPLE__
        "_user.main",
        "_code_entry$",
        "__CNat",
        "_rt$",
#else
        "user.main",
        "code_entry$",
        "_CNat",
        "rt$",
#endif
    };

    DISABLE_CLASS_COPY_AND_ASSIGN(MangleNameHelper);
};
} // namespace MapleRuntime
#endif // MRT_MANGLE_NAME_H
