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

package setter.test;

type Union = number | boolean;

class UnionSetter {
   private _value: Union = 0;

   set value(arg: Union) {
      this._value = arg;
   }

   get value(): Union {
      return this._value;
   }
}

interface ISetter {
   _value: string;

   set value(arg: string);

   get value(): string;
}

class InterfaceSetter implements ISetter {
   _value: string = "";

   set value(arg: string) {
      this._value = arg;
   }

   get value(): string {
      return this._value;
   }
}

class BaseClass {
   private _value: string = "";

   set value(arg: string) {
      this._value = arg;
   }

   get value(): string {
      return this._value;
   }
}

class SubsetRefSet extends BaseClass { }

class ValueSetter {
   protected _value = 'string';
}

class SubsetValueSet extends ValueSetter {
   constructor() {
      super();
   }

   set value(arg: string) {
      this._value = arg;
   }

   get value(): string {
      return this._value;
   }
}

class SetterAnyType<T> {
   private _value: T;

   set value(arg: T) {
      this._value = arg;
   }

   get value(): T {
      return this._value;
   }
}

type TupleT = [number, string];

class TupleSet {
   private _value: TupleT = [0, ""];

   public set value(arg: TupleT) {
      this._value = arg;
   }

   public get value(): TupleT {
      return this._value;
   }
}

abstract class AbstractSetter {
   protected _value: string = "";

   abstract set value(arg: string);

   abstract get value(): string;
}

class AbstractClass extends AbstractSetter {
   override set value(arg: string) {
      this._value = arg
   }

   override get value(): string {
      return this._value;
   }
}

function create_abstratc_class(): AbstractClass {
   return new AbstractClass();
}

function create_base_class(): BaseClass {
   return new BaseClass();
}

function create_union_class(): UnionSetter {
   return new UnionSetter();
}

function create_interface_class(): InterfaceSetter {
   return new InterfaceSetter();
}

function create_subset_ref_set_class(): SubsetRefSet {
   return new SubsetRefSet();
}

function create_subset_value_set_class(): SubsetValueSet {
   return new SubsetValueSet()
}

function create_setter_any_type_class(): SetterAnyType<string> {
   return new SetterAnyType<string>()
}

function create_tuple_set_class(): TupleSet {
   return new TupleSet()
}
