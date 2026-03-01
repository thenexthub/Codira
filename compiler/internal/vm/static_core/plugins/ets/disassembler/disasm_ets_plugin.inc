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

#ifndef DISASM_ETS_PLUGIN_INCLUDED
#define DISASM_ETS_PLUGIN_INCLUDED

void GeteTSMetadata();
void GetETSMetadata(const std::map<std::string, pandasm::Function> &functionTable);
void GetETSMetadata(pandasm::Record *record, const panda_file::File::EntityId &recordId);
void SetETSAttributes(pandasm::Record *record, const panda_file::File::EntityId &recordId) const;
void GetETSMetadata(pandasm::Function *method, const panda_file::File::EntityId &methodId);
void SetETSAttributes(pandasm::Function *method, const panda_file::File::EntityId &methodId) const;
void GetETSMetadata(pandasm::Field *field, const panda_file::File::EntityId &fieldId);
void SetETSAttributes(pandasm::Field *field, const panda_file::File::EntityId &fieldId) const;
AnnotationList GetETSAnnotation(const panda_file::File::EntityId &id, const std::string &type = "");
void GetETSAnnotationIteration(const panda_file::File::EntityId &annotationId, const std::string &type,
                               panda_file::AnnotationDataAccessor &annotationAccessor, AnnotationList &annList,
                               const std::string &className);
void GetETSAnnotationIteration(const panda_file::ArrayValue &values, AnnotationList &annList);
#endif
