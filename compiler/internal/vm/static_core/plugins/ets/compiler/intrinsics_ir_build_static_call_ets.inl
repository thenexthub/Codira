/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_RUNTIME_IS_SAME_REFERENCE: {
    Builder()->BuildStdRuntimeEquals(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_NAN:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_NAN: {
    Builder()->BuildIsNanIntrinsic(bcInst_, ACC_READ);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_FINITE:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_FINITE: {
    /* Inlining makes sense for 64-bits archs as for 32-bits archs
     * the implementation has to be made much more complicated to
     * deal with 64-bits data (double type), so, for for 32-bit archs
     * (e.g. ARM32) it is just better to use c++ implementation */
    if (Is64BitsArch(GetGraph()->GetArch())) {
        Builder()->BuildIsFiniteIntrinsic(bcInst_, ACC_READ);
    } else {
        BuildDefaultStaticIntrinsic(intrinsicId);
    }
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS_INT:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS_LONG:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS_FLOAT:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS: {
    Builder()->BuildAbsIntrinsic(bcInst_, ACC_READ);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SQRT: {
    Builder()->BuildSqrtIntrinsic(bcInst_, ACC_READ);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SIGNBIT: {
    Builder()->BuildSignbitIntrinsic(bcInst_, ACC_READ);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_MATH_MAX: {
    Builder()->template BuildBinaryOperationIntrinsic<Opcode::Max>(bcInst_, ACC_READ);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_MATH_MIN: {
    Builder()->template BuildBinaryOperationIntrinsic<Opcode::Min>(bcInst_, ACC_READ);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MOD: {
    Builder()->template BuildBinaryOperationIntrinsic<Opcode::Mod>(bcInst_, ACC_READ);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_IS_UPPER_CASE: {
    Builder()->BuildCharIsUpperCaseIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_UPPER_CASE: {
    Builder()->BuildCharToUpperCaseIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_IS_LOWER_CASE: {
    Builder()->BuildCharIsLowerCaseIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_LOWER_CASE: {
    Builder()->BuildCharToLowerCaseIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT8:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT16:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT32:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT64:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_FLOAT32:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_FLOAT64:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_NUMBER:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_BOOLEAN: {
    Builder()->BuildUnsafeStoreIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT8:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT16:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT32:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT64:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_BOOLEAN:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_FLOAT32:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_FLOAT64:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_NUMBER: {
    Builder()->BuildUnsafeLoadIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_GET_STRING_SIZE_IN_BYTES:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_STRING:
case RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_STRING: {
    if (Builder()->IsInBootContext()) {
        BuildDefaultStaticIntrinsic(intrinsicId);
    }
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_GET_BUFFER: {
    Builder()->BuildEscompatArrayGetBufferIntrinsic(bcInst_, ACC_READ);
    break;
}
