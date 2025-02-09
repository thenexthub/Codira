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

package typesutil

import (
	"go/constant"
	"go/types"
	"unsafe"

	"github.com/goplus/gogen"
)

// An OperandMode specifies the (addressing) mode of an operand.
type OperandMode byte

const (
	Invalid  OperandMode = iota // operand is invalid
	NoValue                     // operand represents no value (result of a function call w/o result)
	Builtin                     // operand is a built-in function
	TypExpr                     // operand is a type
	Constant                    // operand is a constant; the operand's typ is a Basic type
	Variable                    // operand is an addressable variable
	MapIndex                    // operand is a map index expression (acts like a variable on lhs, commaok on rhs of an assignment)
	Value                       // operand is a computed value
	CommaOK                     // like value, but operand may be used in a comma,ok expression
	CommaErr                    // like commaok, but second value is error, not boolean
	CgoFunc                     // operand is a cgo function
)

// TypeAndValue reports the type and value (for constants)
// of the corresponding expression.
type TypeAndValue struct {
	mode  OperandMode
	Type  types.Type
	Value constant.Value
}

func NewTypeAndValueForType(typ types.Type) (ret types.TypeAndValue) {
	switch t := typ.(type) {
	case *gogen.TypeType:
		typ = t.Type()
	}
	ret.Type = typ
	(*TypeAndValue)(unsafe.Pointer(&ret)).mode = TypExpr
	return
}

func NewTypeAndValueForValue(typ types.Type, val constant.Value, mode OperandMode) (ret types.TypeAndValue) {
	switch t := typ.(type) {
	case *gogen.TypeType:
		typ = t.Type()
	}
	if val != nil {
		mode = Constant
	}
	ret.Type = typ
	ret.Value = val
	(*TypeAndValue)(unsafe.Pointer(&ret)).mode = mode
	return
}

func NewTypeAndValueForCallResult(typ types.Type, val constant.Value) (ret types.TypeAndValue) {
	var mode OperandMode
	if typ == nil {
		ret.Type = &types.Tuple{}
		mode = NoValue
	} else {
		ret.Type = typ
		if val != nil {
			ret.Value = val
			mode = Constant
		} else {
			mode = Value
		}
	}
	(*TypeAndValue)(unsafe.Pointer(&ret)).mode = mode
	return
}

func NewTypeAndValueForObject(obj types.Object) (ret types.TypeAndValue) {
	var mode OperandMode
	var val constant.Value
	switch v := obj.(type) {
	case *types.Const:
		mode = Constant
		val = v.Val()
	case *types.TypeName:
		mode = TypExpr
	case *types.Var:
		mode = Variable
	case *types.Func:
		mode = Value
	case *types.Builtin:
		mode = Builtin
	case *types.Nil:
		mode = Value
	}
	ret.Type = obj.Type()
	ret.Value = val
	(*TypeAndValue)(unsafe.Pointer(&ret)).mode = mode
	return
}
