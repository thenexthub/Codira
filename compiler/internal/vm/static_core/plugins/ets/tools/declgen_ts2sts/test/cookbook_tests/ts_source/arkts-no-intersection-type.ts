/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

export type A = string & number;

export interface Person {
    name: string;
    age: number;
}

export interface Employee {
    employeeId: string;
    department: string;
}

export type PersonEmployee = Person & Employee;

class Logger {
    log(message: string) {
      console.log(message);
    }
  }

class Validator {
    validate(data: any) {
      return typeof data === 'number';
    }
}

export type LoggerValidator = Logger & Validator;

type Status = 'active' | 'inactive';
type User = {
  id: number;
  name: string;
};

export type ActiveUser = User & { status: 'active' };
