/*
 * Copyright (c) NeXTHub Corporation. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 */

package llvm

/*
#include "llvm-c/Core.h"
#include <stdlib.h>
*/
import "C"
import (
	"runtime"
	"unsafe"
)

func (c Context) Finalize() {
	runtime.SetFinalizer(c.C, func(c C.LLVMContextRef) {
		Context{c}.Dispose()
	})
}

func (m Module) Finalize() {
	runtime.SetFinalizer(m.C, func(m C.LLVMModuleRef) {
		Module{m}.Dispose()
	})
}

func (b Builder) Finalize() {
	runtime.SetFinalizer(b.C, func(b C.LLVMBuilderRef) {
		Builder{b}.Dispose()
	})
}

var (
	emptyCStr [1]C.char
)

func CreateBinOp(b Builder, op Opcode, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildBinOp(b.C, C.LLVMOpcode(op), lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreateNot(b Builder, v Value) (rv Value) {
	rv.C = C.LLVMBuildNot(b.C, v.C, &emptyCStr[0])
	return
}

func CreateNeg(b Builder, v Value) (rv Value) {
	rv.C = C.LLVMBuildNeg(b.C, v.C, &emptyCStr[0])
	return
}

func CreateFNeg(b Builder, v Value) (rv Value) {
	rv.C = C.LLVMBuildFNeg(b.C, v.C, &emptyCStr[0])
	return
}

func CreateShl(b Builder, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildShl(b.C, lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreateLShr(b Builder, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildLShr(b.C, lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreateAShr(b Builder, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildAShr(b.C, lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreateAnd(b Builder, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildAnd(b.C, lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreateXor(b Builder, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildXor(b.C, lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreatePHI(b Builder, t Type) (v Value) {
	v.C = C.LLVMBuildPhi(b.C, t.C, &emptyCStr[0])
	return
}

func CreateSelect(b Builder, ifv, thenv, elsev Value) (v Value) {
	v.C = C.LLVMBuildSelect(b.C, ifv.C, thenv.C, elsev.C, &emptyCStr[0])
	return
}

func CreateICmp(b Builder, op IntPredicate, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildICmp(b.C, C.LLVMIntPredicate(op), lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreateFCmp(b Builder, op FloatPredicate, lhs, rhs Value) (v Value) {
	v.C = C.LLVMBuildFCmp(b.C, C.LLVMRealPredicate(op), lhs.C, rhs.C, &emptyCStr[0])
	return
}

func CreateBitCast(b Builder, v Value, t Type) (r Value) {
	r.C = C.LLVMBuildBitCast(b.C, v.C, t.C, &emptyCStr[0])
	return
}

func CreateIntCast(b Builder, v Value, t Type) (r Value) {
	r.C = C.LLVMBuildIntCast(b.C, v.C, t.C, &emptyCStr[0])
	return
}

func CreateTrunc(b Builder, v Value, t Type) (r Value) {
	r.C = C.LLVMBuildTrunc(b.C, v.C, t.C, &emptyCStr[0])
	return
}

func CreateZExt(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildZExt(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreateSExt(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildSExt(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreateFPTrunc(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildFPTrunc(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreateFPExt(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildFPExt(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreatePtrToInt(b Builder, v Value, t Type) (r Value) {
	r.C = C.LLVMBuildPtrToInt(b.C, v.C, t.C, &emptyCStr[0])
	return
}

func CreateIntToPtr(b Builder, v Value, t Type) (r Value) {
	r.C = C.LLVMBuildIntToPtr(b.C, v.C, t.C, &emptyCStr[0])
	return
}

func CreatePointerCast(b Builder, v Value, t Type) (r Value) {
	r.C = C.LLVMBuildPointerCast(b.C, v.C, t.C, &emptyCStr[0])
	return
}

func CreateFPToUI(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildFPToUI(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreateFPToSI(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildFPToSI(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreateUIToFP(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildUIToFP(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreateSIToFP(b Builder, val Value, t Type) (v Value) {
	v.C = C.LLVMBuildSIToFP(b.C, val.C, t.C, &emptyCStr[0])
	return
}

func CreateAlloca(b Builder, t Type) (v Value) {
	v.C = C.LLVMBuildAlloca(b.C, t.C, &emptyCStr[0])
	return
}

func CreateArrayAlloca(b Builder, t Type, n Value) (v Value) {
	v.C = C.LLVMBuildArrayAlloca(b.C, t.C, n.C, &emptyCStr[0])
	return
}

func CreateGEP(b Builder, t Type, p Value, indices []Value) (v Value) {
	ptr, nvals := llvmValueRefs(indices)
	v.C = C.LLVMBuildGEP2(b.C, t.C, p.C, ptr, nvals, &emptyCStr[0])
	return
}

func CreateInBoundsGEP(b Builder, t Type, p Value, indices []Value) (v Value) {
	ptr, nvals := llvmValueRefs(indices)
	v.C = C.LLVMBuildInBoundsGEP2(b.C, t.C, p.C, ptr, nvals, &emptyCStr[0])
	return
}

func CreateStructGEP(b Builder, t Type, p Value, i int) (v Value) {
	v.C = C.LLVMBuildStructGEP2(b.C, t.C, p.C, C.unsigned(i), &emptyCStr[0])
	return
}

func CreateExtractValue(b Builder, agg Value, i int) (v Value) {
	v.C = C.LLVMBuildExtractValue(b.C, agg.C, C.unsigned(i), &emptyCStr[0])
	return
}

func CreateLoad(b Builder, t Type, p Value) (v Value) {
	v.C = C.LLVMBuildLoad2(b.C, t.C, p.C, &emptyCStr[0])
	return
}

func CreateCall(b Builder, t Type, fn Value, args []Value) (v Value) {
	ptr, nvals := llvmValueRefs(args)
	v.C = C.LLVMBuildCall2(b.C, t.C, fn.C, ptr, nvals, &emptyCStr[0])
	return
}

func CreateGlobalStringPtr(b Builder, str string) (v Value) {
	cstr := C.CString(str)
	defer C.free(unsafe.Pointer(cstr))
	v.C = C.LLVMBuildGlobalStringPtr(b.C, cstr, &emptyCStr[0])
	return
}
