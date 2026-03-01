/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_PANDA_FILE_ITEMS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_PANDA_FILE_ITEMS_H_

#include <string_view>

namespace ark::ets::panda_file_items {

// clang-format off
namespace class_descriptors {

// Base classes
static constexpr std::string_view ERROR                                = "Lescompat/Error;";
static constexpr std::string_view BIG_INT                              = "Lstd/core/BigInt;";
static constexpr std::string_view ASYNC                                = "Lets/coroutine/Async;";
static constexpr std::string_view OBJECT                               = "Lstd/core/Object;";
static constexpr std::string_view PROMISE                              = "Lstd/core/Promise;";
static constexpr std::string_view NULL_VALUE                           = "Lstd/core/Null;";
static constexpr std::string_view STRING                               = "Lstd/core/String;";
static constexpr std::string_view LINE_STRING                          = "Lstd/core/LineString;";
static constexpr std::string_view SLICED_STRING                        = "Lstd/core/SlicedString;";
static constexpr std::string_view TREE_STRING                          = "Lstd/core/TreeString;";
static constexpr std::string_view BASE_WEAK_REF                        = "Lstd/core/BaseWeakRef;";
static constexpr std::string_view FINALIZABLE_WEAK_REF                 = "Lstd/core/FinalizableWeakRef;";
static constexpr std::string_view TYPE                                 = "Lstd/core/Type;";
static constexpr std::string_view STRING_BUILDER                       = "Lstd/core/StringBuilder;";

// Proxy
static constexpr std::string_view PROXY                                = "Lstd/core/reflect/Proxy;";

// Runtime classes
static constexpr std::string_view CLASS                                = "Lstd/core/Class;";

// Box classes
static constexpr std::string_view BOX_INT                              = "Lstd/core/Int;";
static constexpr std::string_view BOX_LONG                             = "Lstd/core/Long;";

// Arrays of base classes
static constexpr std::string_view CLASS_ARRAY                          = "[Lstd/core/Class;";
static constexpr std::string_view STRING_ARRAY                         = "[Lstd/core/String;";

// Functional interfaces
static constexpr std::string_view FUNCTION0                            = "Lstd/core/Function0;";
static constexpr std::string_view FUNCTION1                            = "Lstd/core/Function1;";
static constexpr std::string_view FUNCTION2                            = "Lstd/core/Function2;";
static constexpr std::string_view FUNCTION3                            = "Lstd/core/Function3;";
static constexpr std::string_view FUNCTION4                            = "Lstd/core/Function4;";
static constexpr std::string_view FUNCTION5                            = "Lstd/core/Function5;";
static constexpr std::string_view FUNCTION6                            = "Lstd/core/Function6;";
static constexpr std::string_view FUNCTION7                            = "Lstd/core/Function7;";
static constexpr std::string_view FUNCTION8                            = "Lstd/core/Function8;";
static constexpr std::string_view FUNCTION9                            = "Lstd/core/Function9;";
static constexpr std::string_view FUNCTION10                           = "Lstd/core/Function10;";
static constexpr std::string_view FUNCTION11                           = "Lstd/core/Function11;";
static constexpr std::string_view FUNCTION12                           = "Lstd/core/Function12;";
static constexpr std::string_view FUNCTION13                           = "Lstd/core/Function13;";
static constexpr std::string_view FUNCTION14                           = "Lstd/core/Function14;";
static constexpr std::string_view FUNCTION15                           = "Lstd/core/Function15;";
static constexpr std::string_view FUNCTION16                           = "Lstd/core/Function16;";
static constexpr std::string_view FUNCTIONN                            = "Lstd/core/FunctionN;";

// varargs Functional interfaces
static constexpr std::string_view FUNCTIONR0                            = "Lstd/core/FunctionR0;";
static constexpr std::string_view FUNCTIONR1                            = "Lstd/core/FunctionR1;";
static constexpr std::string_view FUNCTIONR2                            = "Lstd/core/FunctionR2;";
static constexpr std::string_view FUNCTIONR3                            = "Lstd/core/FunctionR3;";
static constexpr std::string_view FUNCTIONR4                            = "Lstd/core/FunctionR4;";
static constexpr std::string_view FUNCTIONR5                            = "Lstd/core/FunctionR5;";
static constexpr std::string_view FUNCTIONR6                            = "Lstd/core/FunctionR6;";
static constexpr std::string_view FUNCTIONR7                            = "Lstd/core/FunctionR7;";
static constexpr std::string_view FUNCTIONR8                            = "Lstd/core/FunctionR8;";
static constexpr std::string_view FUNCTIONR9                            = "Lstd/core/FunctionR9;";
static constexpr std::string_view FUNCTIONR10                           = "Lstd/core/FunctionR10;";
static constexpr std::string_view FUNCTIONR11                           = "Lstd/core/FunctionR11;";
static constexpr std::string_view FUNCTIONR12                           = "Lstd/core/FunctionR12;";
static constexpr std::string_view FUNCTIONR13                           = "Lstd/core/FunctionR13;";
static constexpr std::string_view FUNCTIONR14                           = "Lstd/core/FunctionR14;";
static constexpr std::string_view FUNCTIONR15                           = "Lstd/core/FunctionR15;";
static constexpr std::string_view FUNCTIONR16                           = "Lstd/core/FunctionR16;";

// Tuple classes
static constexpr std::string_view TUPLE                                = "Lstd/core/Tuple;";
static constexpr std::string_view TUPLE0                               = "Lstd/core/Tuple0;";
static constexpr std::string_view TUPLE1                               = "Lstd/core/Tuple1;";
static constexpr std::string_view TUPLE2                               = "Lstd/core/Tuple2;";
static constexpr std::string_view TUPLE3                               = "Lstd/core/Tuple3;";
static constexpr std::string_view TUPLE4                               = "Lstd/core/Tuple4;";
static constexpr std::string_view TUPLE5                               = "Lstd/core/Tuple5;";
static constexpr std::string_view TUPLE6                               = "Lstd/core/Tuple6;";
static constexpr std::string_view TUPLE7                               = "Lstd/core/Tuple7;";
static constexpr std::string_view TUPLE8                               = "Lstd/core/Tuple8;";
static constexpr std::string_view TUPLE9                               = "Lstd/core/Tuple9;";
static constexpr std::string_view TUPLE10                              = "Lstd/core/Tuple10;";
static constexpr std::string_view TUPLE11                              = "Lstd/core/Tuple11;";
static constexpr std::string_view TUPLE12                              = "Lstd/core/Tuple12;";
static constexpr std::string_view TUPLE13                              = "Lstd/core/Tuple13;";
static constexpr std::string_view TUPLE14                              = "Lstd/core/Tuple14;";
static constexpr std::string_view TUPLE15                              = "Lstd/core/Tuple15;";
static constexpr std::string_view TUPLE16                              = "Lstd/core/Tuple16;";
static constexpr std::string_view TUPLEN                               = "Lstd/core/TupleN;";
// Base type for all enums
static constexpr std::string_view BASE_ENUM                            = "Lstd/core/BaseEnum;";

// core-defined error classes
static constexpr std::string_view ABC_FILE_NOT_FOUND_ERROR             = "Lstd/core/AbcFileNotFoundError;";
static constexpr std::string_view ARITHMETIC_ERROR                     = "Lstd/core/ArithmeticError;";
static constexpr std::string_view ARRAY_INDEX_OUT_OF_BOUNDS_ERROR      = "Lstd/core/ArrayIndexOutOfBoundsError;";
static constexpr std::string_view ARRAY_STORE_ERROR                    = "Lstd/core/ArrayStoreError;";
static constexpr std::string_view CLASS_CAST_ERROR                     = "Lstd/core/ClassCastError;";
static constexpr std::string_view COROUTINES_LIMIT_EXCEED_ERROR        = "Lstd/core/CoroutinesLimitExceedError;";
static constexpr std::string_view ILLEGAL_LOCK_STATE_ERROR             = "Lstd/core/IllegalLockStateError;";
static constexpr std::string_view EXCEPTION_IN_INITIALIZER_ERROR       = "Lstd/core/ExceptionInInitializerError;";
static constexpr std::string_view FILE_NOT_FOUND_ERROR                 = "Lstd/core/FileNotFoundError;";
static constexpr std::string_view ILLEGAL_ACCESS_ERROR                 = "Lstd/core/IllegalAccessError;";
// remove or make an Error
static constexpr std::string_view ILLEGAL_ARGUMENT_ERROR               = "Lstd/core/IllegalArgumentError;";
static constexpr std::string_view ILLEGAL_MONITOR_STATE_ERROR          = "Lstd/core/IllegalMonitorStateError;";
// remove or make an Error
static constexpr std::string_view ILLEGAL_STATE_ERROR                  = "Lstd/core/IllegalStateError;";
static constexpr std::string_view INDEX_OUT_OF_BOUNDS_ERROR            = "Lstd/core/IndexOutOfBoundsError;";
static constexpr std::string_view INSTANTIATION_ERROR                  = "Lstd/core/InstantiationError;";
// has no class defined
static constexpr std::string_view IO_ERROR                             = "Lstd/core/IOError;";
static constexpr std::string_view NEGATIVE_ARRAY_SIZE_ERROR            = "Lstd/core/NegativeArraySizeError;";
static constexpr std::string_view LINKER_ABSTRACT_METHOD_ERROR         = "Lstd/core/LinkerAbstractMethodError;";
static constexpr std::string_view LINKER_TYPE_CIRCULARITY_ERROR        = "Lstd/core/LinkerTypeCircularityError;";
static constexpr std::string_view LINKER_CLASS_NOT_FOUND_ERROR         = "Lstd/core/LinkerClassNotFoundError;";
static constexpr std::string_view LINKER_BAD_SUPERTYPE_ERROR           = "Lstd/core/LinkerBadSupertypeError;";
static constexpr std::string_view LINKER_UNRESOLVED_CLASS_ERROR        = "Lstd/core/LinkerUnresolvedClassError;";
static constexpr std::string_view LINKER_UNRESOLVED_FIELD_ERROR        = "Lstd/core/LinkerUnresolvedFieldError;";
static constexpr std::string_view LINKER_UNRESOLVED_METHOD_ERROR       = "Lstd/core/LinkerUnresolvedMethodError;";
static constexpr std::string_view LINKER_METHOD_CONFLICT_ERROR         = "Lstd/core/LinkerMethodConflictError;";
static constexpr std::string_view LINKER_VERIFICATION_ERROR            = "Lstd/core/LinkerVerificationError;";
static constexpr std::string_view NULL_POINTER_ERROR                   = "Lstd/core/NullPointerError;";
static constexpr std::string_view OUT_OF_MEMORY_ERROR                  = "Lstd/core/OutOfMemoryError;";
static constexpr std::string_view RANGE_ERROR                          = "Lstd/core/RangeError;";
static constexpr std::string_view SYNTAX_ERROR                         = "Lstd/core/SyntaxError;";
static constexpr std::string_view REFERENCE_ERROR                      = "Lescompat/ReferenceError;";
static constexpr std::string_view URI_ERROR                            = "Lescompat/URIError;";
static constexpr std::string_view TYPE_ERROR                           = "Lescompat/TypeError;";
// remove or make an Error
static constexpr std::string_view RUNTIME_ERROR                        = "Lstd/core/RuntimeError;";
static constexpr std::string_view STACK_OVERFLOW_ERROR                 = "Lstd/core/StackOverflowError;";
static constexpr std::string_view STRING_INDEX_OUT_OF_BOUNDS_ERROR     = "Lstd/core/StringIndexOutOfBoundsError;";
// remove or make an Error
static constexpr std::string_view UNSUPPORTED_OPERATION_ERROR          = "Lstd/core/UnsupportedOperationError;";
static constexpr std::string_view FORMAT_ERROR                         = "Lescompat/FormatError;";

// coroutines
static constexpr std::string_view INVALID_COROUTINE_OPERATION_ERROR    = "Lstd/core/InvalidCoroutineOperationError;";

// stdlib Exception classes
static constexpr std::string_view ARGUMENT_OUT_OF_RANGE_ERROR          = "Lstd/core/ArgumentOutOfRangeError;";

// stdlib Error classes
static constexpr std::string_view ERROR_OPTIONS                        = "Lescompat/ErrorOptions;";
static constexpr std::string_view ERROR_OPTIONS_IMPL                   = "Lescompat/ErrorOptionsImpl;";

static constexpr std::string_view DOUBLE_TO_STRING_CACHE_ELEMENT       = "Lstd/core/DoubleToStringCacheElement;";
static constexpr std::string_view FLOAT_TO_STRING_CACHE_ELEMENT        = "Lstd/core/FloatToStringCacheElement;";
static constexpr std::string_view LONG_TO_STRING_CACHE_ELEMENT         = "Lstd/core/LongToStringCacheElement;";

// interop
static constexpr std::string_view NO_INTEROP_CONTEXT_ERROR             = "Lstd/interop/js/NoInteropContextError;";

// interop/js
static constexpr std::string_view JS_RUNTIME                           = "Lstd/interop/js/JSRuntime;";
static constexpr std::string_view JS_VALUE                             = "Lstd/interop/js/JSValue;";
static constexpr std::string_view ES_ERROR                             = "Lstd/interop/js/ESError;";

// Interop function class for invoking dynamic functions
static constexpr std::string_view INTEROP_DYNAMIC_FUNCTION             = "Lstd/interop/js/DynamicFunction;";

static constexpr std::string_view ARRAY                                = "Lstd/core/Array;";
static constexpr std::string_view ARRAY_AS_LIST_INT                    = "Lstd/containers/containers/ArrayAsListInt;";

// ANI annotation classes
static constexpr std::string_view ANI_UNSAFE_QUICK                     = "Lstd/annotations/ani/unsafe/Quick;";
static constexpr std::string_view ANI_UNSAFE_DIRECT                    = "Lstd/annotations/ani/unsafe/Direct;";

// Module annotation class
static constexpr std::string_view ANNOTATION_MODULE                    = "Lets/annotation/Module;";
static constexpr std::string_view ANNOTATION_MODULE_EXPORTED           = "exported";

// Interface object literal annotation class
static constexpr std::string_view INTERFACE_OBJ_LITERAL                = "Lstd/annotations/InterfaceObjectLiteral;";

// escompat
static constexpr std::string_view DATE                                 = "Lstd/core/Date;";
static constexpr std::string_view ARRAY_ENTRIES_ITERATOR_T             = "Lstd/core/ArrayEntriesIterator_T;";
static constexpr std::string_view ITERATOR_RESULT                      = "Lstd/core/IteratorResult;";
static constexpr std::string_view ARRAY_KEYS_ITERATOR                  = "Lstd/core/ArrayKeysIterator;";
static constexpr std::string_view ARRAY_VALUES_ITERATOR_T              = "Lstd/core/ArrayValuesIterator_T;";
static constexpr std::string_view MAP                                  = "Lstd/core/Map;";
static constexpr std::string_view MAPITERATOR                          = "Lstd/core/MapIteratorImpl;";
static constexpr std::string_view SETITERATOR                          = "Lstd/core/SetIteratorImpl;";
static constexpr std::string_view EMPTYMAPITERATOR                     = "Lstd/core/EmptyMapIteratorImpl;";
static constexpr std::string_view SET                                  = "Lstd/core/Set;";
static constexpr std::string_view RECORD                               = "Lstd/core/Record;";

// Json Annotations
static constexpr std::string_view JSON_STRINGIFY_IGNORE                = "Lstd/core/JSONStringifyIgnore;";
static constexpr std::string_view JSON_PARSE_IGNORE                    = "Lstd/core/JSONParseIgnore;";
static constexpr std::string_view JSON_RENAME                          = "Lstd/core/JSONRename;";

// Annotation for optional parameters
static constexpr std::string_view OPTIONAL_PARAMETERS_ANNOTATION       =
    "Lstd/annotations/functions/OptionalParametersAnnotation;";

// Annotation for function reference
static constexpr std::string_view ANNOTATION_FUNCTIONAL_REFERENCE      =
    "Lets/annotation/FunctionalReference;";

}  // namespace class_descriptors

namespace classes {

static constexpr std::string_view STRING_BUILDER                       = "std.core.StringBuilder";

}  // namespace classes

namespace methods {

static constexpr std::string_view STRING_CONCAT                        = "std.core.String::concat";
static constexpr std::string_view STRING_GET_LENGTH                    = "std.core.String::getLength";
static constexpr std::string_view STRING_BUILDER_APPEND                = "std.core.StringBuilder::append";
static constexpr std::string_view STRING_BUILDER_TO_STRING             = "std.core.StringBuilder::toString";

}  // namespace methods

namespace getters {

static constexpr std::string_view STRING_GET_LENGTH                    = "std.core.String::%%get-length";
static constexpr std::string_view STRING_BUILDER_GET_STRING_LENGTH     = "std.core.StringBuilder::%%get-stringLength";

}  // namespace getters

namespace fields {

static constexpr std::string_view ACTUAL_LENGTH                        = "actualLength";
static constexpr std::string_view BOOLEAN_FALSE                        = "FALSE";
static constexpr std::string_view BOOLEAN_TRUE                         = "TRUE";
static constexpr std::string_view BUF                                  = "buf";
static constexpr std::string_view BUFFER                               = "buffer";
static constexpr std::string_view BYTE_OFFSET                          = "byteOffset";
static constexpr std::string_view BYTE_OFFSET_INT                      = "byteOffsetInt";
static constexpr std::string_view COMPRESS                             = "compress";
static constexpr std::string_view DATA                                 = "data";
static constexpr std::string_view DATA_ADDRESS                         = "dataAddress";
static constexpr std::string_view INDEX                                = "index";
static constexpr std::string_view LENGTH                               = "length";
static constexpr std::string_view LENGTH_INT                           = "lengthInt";
static constexpr std::string_view VALUE                                = "value";

}  // namespace fields

namespace signatures {

static constexpr std::string_view RET_VOID                             = "()V";
static constexpr std::string_view RET_INT                              = "()I";
static constexpr std::string_view CHAR_ARRAY_RET_VOID                  = "([C)V";
static constexpr std::string_view STRING_RET_VOID                      = "(Lstd/core/String;)V";
static constexpr std::string_view STRING_ARRAY_RET_STRING              = "([Lstd/core/String;)Lstd/core/String;";
static constexpr std::string_view RET_STRING                           = "()Lstd/core/String;";

}  // namespace signatures

static constexpr std::string_view CCTOR = "<cctor>";
static constexpr std::string_view CTOR  = "<ctor>";
static constexpr std::string_view DOT_CCTOR = ".cctor";
static constexpr std::string_view DOT_CTOR  = ".ctor";
// clang-format on

}  // namespace ark::ets::panda_file_items

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_PANDA_FILE_ITEMS_H_
