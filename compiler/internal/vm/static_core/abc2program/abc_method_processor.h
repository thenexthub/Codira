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

#ifndef ABC2PROGRAM_ABC_METHOD_PROCESSOR_H
#define ABC2PROGRAM_ABC_METHOD_PROCESSOR_H

#include "abc_file_entity_processor.h"
#include "libarkfile/method_data_accessor-inl.h"

namespace ark::abc2program {

class AbcMethodProcessor : public AbcFileEntityProcessor {
public:
    AbcMethodProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData);
    void GetMethodName();
    void GetMethodCode();
    void FillProgramData();
    std::string GetMethodSignature();

private:
    pandasm::Function function_;
    void GetParams(const panda_file::File::EntityId &protoId);
    void GetMetaData();
    pandasm::Type PFTypeToPandasmType(const panda_file::Type &type, panda_file::ProtoDataAccessor &pda,
                                      size_t &refIdx) const;
    void FillFunction();
    std::unique_ptr<panda_file::MethodDataAccessor> methodDataAccessor_;
    std::string methodSignature_;
};

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC_METHOD_PROCESSOR_H
