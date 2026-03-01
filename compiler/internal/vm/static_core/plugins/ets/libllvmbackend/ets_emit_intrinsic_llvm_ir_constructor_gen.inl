/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
// Must be lowered earlier in IrBuilder, impossible to meet
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_GET_LENGTH:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_IS_EMPTY:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_NAN:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_NAN:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_FINITE:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_FINITE:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS_INT:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS_LONG:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS_FLOAT:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MOD:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_RUNTIME_IS_SAME_REFERENCE:
    UNREACHABLE();
