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

package generic.test;

type Union = string | number;

const ets_string = 'ets_string_test';

class UnionClass<T extends Union> {
   private _value: T;

   constructor(value: T) {
      this._value = value;
   }

   set(arg: T): void {
      this._value = arg;
   }

   get(): T {
      return this._value;
   }
}

function create_union_object_from_ets(): UnionClass<string> {
   return new UnionClass<string>(ets_string)
}

interface GInterface<T> {
   value: T
}

class GIClass<T> implements GInterface<T> {
   public value: T;

   constructor(value: T) {
      this.value = value;
   }

   set(arg: T): void {
      this.value = arg;
   }

   get(): T {
      return this.value;
   }
}

function create_interface_object_from_ets(): GIClass<string> {
   return new GIClass<string>(ets_string)
}

abstract class GAbstract<T> {
   private _value: T

   constructor(value: T) {
      this._value = value;
   }

   set(arg: T): void {
      this._value = arg;
   }

   get(): T {
      return this._value;
   }
}

class AbstractClass<T> extends GAbstract<T> {
   constructor(value: T) {
      super(value);
   }
}


function create_abstract_object_from_ets(): AbstractClass<string> {
   return new AbstractClass<string>(ets_string)
}

class GClass<T> {
   public content: T;

   constructor(content: T) {
      this.content = content;
   }

   public get(): T {
      return this.content;
   }
}

function create_generic_object_from_ets(): GClass<string> {
   return new GClass<string>(ets_string)
}

function generic_function<T>(arg: T): T {
   return arg;
}

function tuple_declared_type<T, U>(items: [T, U]): [T, U] {
   return items;
}

interface Data {
   data: string
};

function generic_subset_ref<T extends Data>(items: T): T {
   return items;
}

type TGenericFn<T> = () => T;

// const explicitly_declared_type: TGenericFn<string> = () => {
//    return 'test';
// };
