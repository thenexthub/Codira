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

class User {
    name: string;
    age: number;
    email: string;

    constructor(name: string, age: number, email: string) {
        this.name = name;
        this.age = age;
        this.email = email;
    }
}

class Employee {
    id: number;
    name: string;

    constructor(id: number, name: string) {
        this.id = id;
        this.name = name;
    }
}

class Address {
    street: string;
    city: string;
    country: string;

    constructor(street: string, city: string, country: string) {
        this.street = street;
        this.city = city;
        this.country = country;
    }
}

class Company {
    name: string;
    employees: Employee[];
    address: Address;

    constructor(name: string, employees: Employee[], address: Address) {
        this.name = name;
        this.employees = employees;
        this.address = address;
    }
}

class Calculator {
    add(a: number, b: number): number {
        return a + b;
    }

    subtract(a: number, b: number): number {
        return a - b;
    }

    multiply(a: number, b: number): number {
        return a * b;
    }
}

function main() {
    // Simple object
    const user = new User("John", 30, "john@example.com");
    console.dir(user);

    // Nested object
    const company = new Company(
        "Tech Corp",
        [
            new Employee(1, "Alice"),
            new Employee(2, "Bob")
        ],
        new Address("123 Main St", "Tech City", "Techland")
    );
    console.dir(company);

    // Object with methods
    const calculator = new Calculator();
    console.dir(calculator);

    // Array with mixed types
    const mixedArray: Any[] = [
        1,
        "string",
        [1, 2, 3],
    ];
    console.dir(mixedArray);

    console.dir()
    console.dir(1);
    console.dir("str");
    console.dir("count: %d")
}
