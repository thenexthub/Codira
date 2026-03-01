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

// Matrix type definitions remain as types
type MatrixRow = [string, string, string];
type Matrix = MatrixRow[];

// Basic user class
class User {
    id: number;
    name: string;
    age: number;

    constructor(id: number, name: string, age: number) {
        this.id = id;
        this.name = name;
        this.age = age;
    }
}

// Product class
class Product {
    id: number;
    name: string;
    price: number;
    stock: number;

    constructor(id: number, name: string, price: number, stock: number) {
        this.id = id;
        this.name = name;
        this.price = price;
        this.stock = stock;
    }
}

// Mixed data class
class MixedData {
    id: number;
    value: string | number | boolean;

    constructor(id: number, value: string | number | boolean) {
        this.id = id;
        this.value = value;
    }
}

// Nested data classes
class Contact {
    email: string;

    constructor(email: string) {
        this.email = email;
    }
}

class Info {
    name: string;
    contact: Contact;

    constructor(name: string, contact: Contact) {
        this.name = name;
        this.contact = contact;
    }
}

class NestedData {
    id: number;
    info: Info;

    constructor(id: number, info: Info) {
        this.id = id;
        this.info = info;
    }
}

// Data display utility class
class DataDisplay {
    static displayUsers(): void {
        const users : Array<Any> = [
            new User(1, "John", 30),
            new User(2, "Alice", 25),
            new User(3, "Bob", 35)
        ];
        console.table(users);
    }

    static displayMatrix(): void {
        const matrix: Matrix = [
            ["A1", "B1", "C1"],
            ["A2", "B2", "C2"],
            ["A3", "B3", "C3"]
        ];
        console.table(matrix as Object as Array<Any>);
    }

    static displayProducts(): void {
        const products : Array<Any> = [
            new Product(1, "Phone", 699, 50),
            new Product(2, "Laptop", 999, 30),
            new Product(3, "Tablet", 399, 75)
        ];
        console.table(products);
    }

    static displayMixedData(): void {
        const mixedData : Array<Any> = [
            new MixedData(1, "string"),
            new MixedData(2, 42),
            new MixedData(3, true)
        ];
        console.table(mixedData);
    }
}

interface A {
    a: number[],
    b: number,
    c: B,
}
interface B {
    e: number,
}

function main() {
    // Display simple array of objects
    DataDisplay.displayUsers();

    // Display array of arrays
    DataDisplay.displayMatrix();

    // Display array with specific columns
    DataDisplay.displayProducts();

    // Display mixed data types
    DataDisplay.displayMixedData();

    console.table(42)
    console.table("str")
    console.table("count %d")

    let a: A = {
        a: [1, 2, 3, 4, 5], b: 5,
        c: {
        e: 5
        }
    }
    console.table(a);
        
    let number1: number = 1;
    let str = "str";
    console.table(number1);
    console.table(str);
    console.table('count: %d');
    console.table();
    console.table([1, 2, 3]);
}
