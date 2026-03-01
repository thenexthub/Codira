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

#ifndef CPP_ABCKIT_HASH_SPECIALIZATIONS_H
#define CPP_ABCKIT_HASH_SPECIALIZATIONS_H

#include "config.h"

#include "basic_block.h"
#include "instruction.h"
#include "literal.h"
#include "literal_array.h"

#include "core/annotation_element.h"
#include "core/annotation_interface_field.h"
#include "core/annotation_interface.h"
#include "core/annotation.h"
#include "core/class.h"
#include "core/export_descriptor.h"
#include "core/function.h"
#include "core/import_descriptor.h"
#include "core/module.h"
#include "core/namespace.h"

#include "arkts/annotation_element.h"
#include "arkts/annotation_interface_field.h"
#include "arkts/annotation_interface.h"
#include "arkts/annotation.h"
#include "arkts/class.h"
#include "arkts/export_descriptor.h"
#include "arkts/function.h"
#include "arkts/import_descriptor.h"
#include "arkts/module.h"
#include "arkts/namespace.h"

// NOLINTNEXTLINE(cert-dcl58-cpp)
namespace std {

template <>
struct hash<abckit::BasicBlock> : public abckit::DefaultHash<abckit::BasicBlock> {
};

template <>
struct hash<abckit::Instruction> : public abckit::DefaultHash<abckit::Instruction> {
};

template <>
struct hash<abckit::Literal> : public abckit::DefaultHash<abckit::Literal> {
};

template <>
struct hash<abckit::LiteralArray> : public abckit::DefaultHash<abckit::LiteralArray> {
};

template <>
struct hash<abckit::core::AnnotationElement> : public abckit::DefaultHash<abckit::core::AnnotationElement> {
};

template <>
struct hash<abckit::core::AnnotationInterfaceField>
    : public abckit::DefaultHash<abckit::core::AnnotationInterfaceField> {
};

template <>
struct hash<abckit::core::AnnotationInterface> : public abckit::DefaultHash<abckit::core::AnnotationInterface> {
};

template <>
struct hash<abckit::core::Annotation> : public abckit::DefaultHash<abckit::core::Annotation> {
};

template <>
struct hash<abckit::core::Class> : public abckit::DefaultHash<abckit::core::Class> {
};

template <>
struct hash<abckit::core::ExportDescriptor> : public abckit::DefaultHash<abckit::core::ExportDescriptor> {
};

template <>
struct hash<abckit::core::Function> : public abckit::DefaultHash<abckit::core::Function> {
};

template <>
struct hash<abckit::core::ImportDescriptor> : public abckit::DefaultHash<abckit::core::ImportDescriptor> {
};

template <>
struct hash<abckit::core::Module> : public abckit::DefaultHash<abckit::core::Module> {
};

template <>
struct hash<abckit::core::Namespace> : public abckit::DefaultHash<abckit::core::Namespace> {
};

template <>
struct hash<abckit::arkts::AnnotationElement> : public abckit::DefaultHash<abckit::arkts::AnnotationElement> {
};

template <>
struct hash<abckit::arkts::AnnotationInterfaceField>
    : public abckit::DefaultHash<abckit::arkts::AnnotationInterfaceField> {
};

template <>
struct hash<abckit::arkts::AnnotationInterface> : public abckit::DefaultHash<abckit::arkts::AnnotationInterface> {
};

template <>
struct hash<abckit::arkts::Annotation> : public abckit::DefaultHash<abckit::arkts::Annotation> {
};

template <>
struct hash<abckit::arkts::Class> : public abckit::DefaultHash<abckit::arkts::Class> {
};

template <>
struct hash<abckit::arkts::ExportDescriptor> : public abckit::DefaultHash<abckit::arkts::ExportDescriptor> {
};

template <>
struct hash<abckit::arkts::Function> : public abckit::DefaultHash<abckit::arkts::Function> {
};

template <>
struct hash<abckit::arkts::ImportDescriptor> : public abckit::DefaultHash<abckit::arkts::ImportDescriptor> {
};

template <>
struct hash<abckit::arkts::Module> : public abckit::DefaultHash<abckit::arkts::Module> {
};

template <>
struct hash<abckit::arkts::Namespace> : public abckit::DefaultHash<abckit::arkts::Namespace> {
};

}  // namespace std

#endif  // CPP_ABCKIT_HASH_SPECIALIZATIONS_H
