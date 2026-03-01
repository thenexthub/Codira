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

package interface_method.test;

const ets_number = 1;
const ets_string = 'string';

interface AnyTypeMethod<T> {
  get(a: T): T
}

class AnyTypeMethodClass<T> implements AnyTypeMethod<T> {
  get(a: T): T {
    return a;
  }
}

function create_interface_class_any_type_method(): AnyTypeMethodClass<int> {
  return new AnyTypeMethodClass<int>();
}

type Union = string | number;

interface UnionMethod {
  get(a: Union): Union
}

class UnionTypeMethodClass implements UnionMethod {
  get(a: Union): Union {
    return a;
  }
}

function create_interface_class_union_type_method(): UnionTypeMethodClass {
  return new UnionTypeMethodClass();
}

// NOTE: issue (1835) fix Literal type
// interface LiteralWithValue {
//   get(arg: 1 | 'string'): 1 | 'string';
// }

// class LiteralValueMethodClass implements LiteralWithValue {
//   get(arg: 1 | 'string'): 1 | 'string' {
//       return arg;
//   }
// }

// function create_interface_literal_value_class_from_ets(): LiteralValueMethodClass {
//   return new LiteralValueMethodClass();
// }

interface TupleT {
  get(arg: [int, string]): [int, string];
}


class TupleTypeMethodClass implements TupleT {
  get(arg: [int, string]): [int, string]{
    return arg;
  }
}

function create_interface_class_tuple_type_method(): TupleTypeMethodClass {
  return new TupleTypeMethodClass();
}


interface ArgInterface {
  get(): int
}

class SubsetByRefClass implements ArgInterface {
  get(): int {
    return ets_number;
  }
}

function subset_by_ref_interface(obj: ArgInterface): int {
  return obj.get()
}

function call_subset_by_ref_interface_from_ets(): int {
  const subsetByRefClass = new SubsetByRefClass();
  return subset_by_ref_interface(subsetByRefClass);
}

class UserClass {
  public value = 1;
}

interface SubsetByValue {
  get(): UserClass
}

class SubsetByValueClass implements SubsetByValue {
  get(): UserClass {
    return new UserClass();
  }
}

function create_subset_by_value_class_from_ets(): SubsetByValueClass {
  return new SubsetByValueClass()
}

// NOTE (issues 17769) fix optional method in interface
// interface OptionalMethod {
//   getNum?(): int;
//   getStr(): string;
// }

// class WithOptionalMethodClass implements OptionalMethod {
//  getNum(): int {
//   return ets_number;
//  }
//   getStr(): string {
//       return ets_string;
//   }
// }

// class WithoutOptionalMethodClass implements OptionalMethod {
//   getStr(): string {
//       return ets_string;
//   }
// }

// function create_class_with_optional_method(): WithOptionalMethodClass {
//   return new WithOptionalMethodClass();
// }

// function create_class_without_optional_method(): WithoutOptionalMethodClass {
//   return new WithoutOptionalMethodClass();
// }

// interface OptionalArgResult {
//   with: WithOptionalMethodClass,
//   without?: WithoutOptionalMethodClass
// }

// function optional_arg(arg: WithOptionalMethodClass, optional?: WithoutOptionalMethodClass): OptionalArgResult {
//   if (optional) {
//       return { with: arg, without: optional }
//   }

//   return { with: arg }
// }

// type ArrayArg = FixedArray<WithOptionalMethodClass | WithoutOptionalMethodClass>;

// function optional_arg_array(...arg: ArrayArg): OptionalArgResult {
//   const withOptional = arg[0] as WithOptionalMethodClass;
//   const withoutOptional = arg[1] as WithoutOptionalMethodClass;

//   if (withoutOptional) {
//       return { with: withOptional, without: withoutOptional };
//   }

//   return { with: withOptional };
// }
