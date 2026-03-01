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

#ifndef CPP_ABCKIT_H
#define CPP_ABCKIT_H

#ifdef ABCKIT_TEST_ENABLE_MOCK
#include "../../tests/mock_headers/mock_c_api.h"
#endif

#include "headers/utils.h"
#include "headers/graph.h"
#include "headers/instruction.h"
#include "headers/basic_block.h"
#include "headers/file.h"
#include "headers/value.h"
#include "headers/literal.h"
#include "headers/literal_array.h"
#include "headers/type.h"

#include "headers/core/annotation_element.h"
#include "headers/core/annotation_interface_field.h"
#include "headers/core/annotation_interface.h"
#include "headers/core/annotation.h"
#include "headers/core/class.h"
#include "headers/core/interface.h"
#include "headers/core/enum.h"
#include "headers/core/field.h"
#include "headers/core/export_descriptor.h"
#include "headers/core/function.h"
#include "headers/core/import_descriptor.h"
#include "headers/core/module.h"
#include "headers/core/namespace.h"

#include "headers/arkts/annotation_element.h"
#include "headers/arkts/annotation_interface_field.h"
#include "headers/arkts/annotation_interface.h"
#include "headers/arkts/annotation.h"
#include "headers/arkts/class.h"
#include "headers/arkts/interface.h"
#include "headers/arkts/enum.h"
#include "headers/arkts/export_descriptor.h"
#include "headers/arkts/field.h"
#include "headers/arkts/function.h"
#include "headers/arkts/import_descriptor.h"
#include "headers/arkts/module.h"
#include "headers/arkts/namespace.h"

#include "headers/js/export_descriptor.h"
#include "headers/js/import_descriptor.h"
#include "headers/js/module.h"

// implementations
#include "headers/core/annotation_interface_field_impl.h"
#include "headers/core/annotation_interface_impl.h"
#include "headers/core/annotation_element_impl.h"
#include "headers/core/annotation_impl.h"
#include "headers/core/module_impl.h"
#include "headers/core/namespace_impl.h"
#include "headers/core/class_impl.h"
#include "headers/core/interface_impl.h"
#include "headers/core/enum_impl.h"
#include "headers/core/function_impl.h"
#include "headers/core/function_param_impl.h"
#include "headers/core/field_impl.h"
#include "headers/core/export_descriptor_impl.h"
#include "headers/core/import_descriptor_impl.h"

#include "headers/arkts/annotation_interface_field_impl.h"
#include "headers/arkts/annotation_interface_impl.h"
#include "headers/arkts/annotation_element_impl.h"
#include "headers/arkts/annotation_impl.h"
#include "headers/arkts/field_impl.h"
#include "headers/arkts/function_impl.h"
#include "headers/arkts/module_impl.h"
#include "headers/arkts/class_impl.h"
#include "headers/arkts/interface_impl.h"
#include "headers/arkts/enum_impl.h"
#include "headers/arkts/export_descriptor_impl.h"
#include "headers/arkts/import_descriptor_impl.h"
#include "headers/arkts/namespace_impl.h"

#include "headers/js/export_descriptor_impl.h"
#include "headers/js/import_descriptor_impl.h"
#include "headers/js/module_impl.h"

#include "headers/dynamic_isa_impl.h"
#include "headers/static_isa_impl.h"
#include "headers/basic_block_impl.h"
#include "headers/file_impl.h"
#include "headers/graph_impl.h"
#include "headers/instruction_impl.h"
#include "headers/literal_impl.h"
#include "headers/literal_array_impl.h"
#include "headers/value_impl.h"

#include "headers/hash_specializations.h"

#endif  // CPP_ABCKIT
