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

#include<string>
#include <map>

class SingleInstance {
public:
    std::string pathIn = "";
    std::string pathOut = "";
    std::string workPath = "";
    std::string pathPwd = "";
    std::string messagePath = "";
    std::string testFolder = "";
    std::string pathBuildScript = "";
    std::string caseProPath = "";
    std::string binaryPath = "";
    bool useDB = false;
    std::map<std::string, std::string> replaceMap = {};
    static SingleInstance* GetInstance()
    {
        if (m_pInstance == nullptr) {
            m_pInstance = new SingleInstance();
        }
        return m_pInstance;
    }
private:
    SingleInstance() {};
    static SingleInstance *m_pInstance;
    class CGarbo {
    public:
        ~CGarbo()
        {
            if (SingleInstance::m_pInstance) {
                delete SingleInstance::m_pInstance;
            }
        }
    };
    static CGarbo Garbo;
};
