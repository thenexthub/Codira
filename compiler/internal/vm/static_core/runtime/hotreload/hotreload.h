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
#ifndef PANDA_RUNTIME_HOTRELOAD_HOTRELOAD_H_
#define PANDA_RUNTIME_HOTRELOAD_HOTRELOAD_H_

#include "libarkbase/macros.h"
#include "libarkfile/file.h"
#include "runtime/include/class.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/rendezvous.h"

namespace ark::hotreload {

struct ClassContainment {
    const panda_file::File *pf;
    panda_file::File::EntityId classId;
    const std::string className_;
    Class *loadedClass;
    Class *tmpClass;
    uint32_t fChanges;
};

enum class Error {
    NONE,
    INTERNAL,
    CLASS_ALREADY_LOADED,
    CLASS_NOT_FOUND,
    EXTERNAL_CLASS_READ,
    INVALID_INPUT_ARG,
    UNSUPPORTED_CHANGES,
    NO_DEBUGGER_ATTACHED,
    CLASS_UNMODIFIABLE,
    WRONG_CLASS_DESCRIPTOR,
    INVALID_CLASS_FORMAT,
    CIRCULAR_CLASS,
    METHOD_ADDED,
    METHOD_DELETED,
    METHOD_SIGN,
    CLASS_MODIFIERS,
    HIERARCHY_CHANGED,
    FIELD_CHANGED
};

enum ChangesFlags : uint32_t {
    F_NO_STRUCT_CHANGES = 0x0000U,
    F_INHERITANCE = 0x0001U,
    F_METHOD_SIGN = 0x0002U,
    F_METHOD_DELETED = 0x0004U,
    F_INTERFACES = 0x0008U,
    F_FIELDS_TYPE = 0x0010U,
    F_FIELDS_AMOUNT = 0x0020U,
    F_ACCESS_FLAGS = 0x0040U,
    F_METHOD_ADDED = 0x0080U
};

enum class Type { STRUCTURAL, NORMAL, INVALID };

class ArkHotreloadBase {
public:
    /*
     * There is no API for adding classes for hotreload
     * 'cause its signature is language-dependent
     * this API should be declared in superclass
     */
    PANDA_PUBLIC_API Error ProcessHotreload();

    PANDA_PUBLIC_API const panda_file::File *ReadAndOwnPandaFileFromFile(const char *location);
    PANDA_PUBLIC_API const panda_file::File *ReadAndOwnPandaFileFromMemory(const void *buffer, size_t size);

    NO_COPY_SEMANTIC(ArkHotreloadBase);
    NO_MOVE_SEMANTIC(ArkHotreloadBase);

protected:
    explicit ArkHotreloadBase(ManagedThread *mthread, panda_file::SourceLang lang);
    virtual ~ArkHotreloadBase();

    virtual Error LangSpecificValidateClasses() = 0;
    virtual void LangSpecificHotreloadPart() = 0;

    Error ValidateClassesHotreloadPossibility();
    Error ValidateClassForHotreload(const ClassContainment &hCls);
    Type RecognizeHotreloadType(ClassContainment *hCls);

    Type InheritanceChangesCheck(ClassContainment *hCls);
    Type FlagsChangesCheck(ClassContainment *hCls);
    Type FieldChangesCheck(ClassContainment *hCls);
    Type MethodChangesCheck(ClassContainment *hCls);

    void ReloadClassNormal(const ClassContainment *hCls);
    void UpdateVtablesInRuntimeClasses(ClassLinker *classLinker);
    void AddLoadedPandaFilesToRuntime(ClassLinker *classLinker);
    void AddObsoleteClassesToRuntime(ClassLinker *classLinker);

    using FieldIdTable = PandaUnorderedMap<panda_file::File::EntityId, panda_file::File::EntityId>;

    panda_file::SourceLang lang_;            // NOLINT(misc-non-private-member-variables-in-classes)
    ManagedThread *thread_;                  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaVector<ClassContainment> classes_;  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaVector<std::unique_ptr<const panda_file::File>>
        pandaFiles_;                                         // NOLINT(misc-non-private-member-variables-in-classes)
    PandaUnorderedMap<Method *, Method *> methodsTable_;     // NOLINT(misc-non-private-member-variables-in-classes)
    PandaUnorderedMap<Class *, FieldIdTable> fieldsTables_;  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaUnorderedSet<Class *> reloadedClasses_;             // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::hotreload

#endif  // PANDA_RUNTIME_HOTRELOAD_HOTRELOAD_H_
