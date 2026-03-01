/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_CHA_H
#define PANDA_CHA_H

#include "runtime/include/class.h"

namespace ark {

class ClassHierarchyAnalysis final {
public:
    void Update(Class *klass);
    void AddDependency(Method *callee, Method *caller);

private:
    void InvalidateMethod(Method *method, PandaSet<Method *> *dependentMethods);
    void InvalidateMethods(const PandaSet<Method *> &methods);
    void UpdateMethod(Method *method);
    bool HasSingleImplementation(Method *method);
    void SetHasSingleImplementation(Method *method, bool singleImplementation);
    auto &GetLock()
    {
        return lock_;
    }

private:
    PandaMap<Method *, PandaSet<Method *>> dependencyMap_;
    os::memory::Mutex lock_;
};

}  // namespace ark

#endif  // PANDA_CHA_H
