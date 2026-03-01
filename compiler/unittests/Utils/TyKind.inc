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

DEFINE_TY(AST::ArrayTy, AST::TypeKind::TYPE_ARRAY);
DEFINE_TY(AST::PointerTy, AST::TypeKind::TYPE_POINTER);
DEFINE_TY(AST::CStringTy, AST::TypeKind::TYPE_CSTRING);
DEFINE_TY(AST::TupleTy, AST::TypeKind::TYPE_TUPLE);
DEFINE_TY(AST::FuncTy, AST::TypeKind::TYPE_FUNC);
DEFINE_TY(AST::UnionTy, AST::TypeKind::TYPE_UNION);
DEFINE_TY(AST::IntersectionTy, AST::TypeKind::TYPE_INTERSECTION);
DEFINE_TY(AST::InterfaceTy, AST::TypeKind::TYPE_INTERFACE);
DEFINE_TY(AST::ClassTy, AST::TypeKind::TYPE_CLASS);
DEFINE_TY(AST::EnumTy, AST::TypeKind::TYPE_ENUM);
DEFINE_TY(AST::StructTy, AST::TypeKind::TYPE_STRUCT);
DEFINE_TY(AST::TypeAliasTy, AST::TypeKind::TYPE);
DEFINE_TY(AST::GenericsTy, AST::TypeKind::TYPE_GENERICS);
