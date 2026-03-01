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

package std.core;

/**
 * @class Represents a boxed object value
 */
export final class Box<T> {
    private value: T|null;

    public constructor() {
        this.value = null;
    }

    public constructor(value: T) {
        this.value = value;
    }

    /**
     * Construct a new Box instance with provided initial value
     *
     * @param value: TT the initial value
     */
    public static box<TT>(value: TT): Box<TT> {
        return new Box<TT>(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns T - value in the box
     */
    public get(): T {
        return this.value as T;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: T): T {
        return (this.value = value) as T;
    }
}

export final class BooleanBox {
    private value: boolean;

    public constructor() {
        this.value = false;
    }

    public constructor(value: boolean) {
        this.value = value;
    }

    /**
     * Construct a new BooleanBox instance with provided initial value
     *
     * @param value: boolean the initial value
     */
    public static box(value: boolean): BooleanBox {
        return new BooleanBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns boolean - value in the box
     */
    public get(): boolean {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: boolean): boolean {
        return (this.value = value);
    }
}

export final class ByteBox {
    private value: byte;

    public constructor() {
        this.value = 0;
    }

    public constructor(value: byte) {
        this.value = value;
    }

    /**
     * Construct a new ByteBox instance with provided initial value
     *
     * @param value: byte the initial value
     */
    public static box(value: byte): ByteBox {
        return new ByteBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns byte - value in the box
     */
    public get(): byte {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: byte): byte {
        return (this.value = value);
    }
}

export final class CharBox {
    private value: char;

    public constructor() {
        this.value = c'\0';
    }

    public constructor(value: char) {
        this.value = value;
    }

    /**
     * Construct a new CharBox instance with provided initial value
     *
     * @param value: char the initial value
     */
    public static box(value: char): CharBox {
        return new CharBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns char - value in the box
     */
    public get(): char {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: char): char {
        return (this.value = value);
    }
}

export final class ShortBox {
    private value: short;

    public constructor() {
        this.value = 0;
    }

    public constructor(value: short) {
        this.value = value;
    }

    /**
     * Construct a new ShortBox instance with provided initial value
     *
     * @param value: short the initial value
     */
    public static box(value: short): ShortBox {
        return new ShortBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns short - value in the box
     */
    public get(): short {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: short): short {
        return (this.value = value);
    }
}

export final class IntBox {
    private value: int;

    public constructor() {
        this.value = 0;
    }

    public constructor(value: int) {
        this.value = value;
    }

    /**
     * Construct a new IntBox instance with provided initial value
     *
     * @param value: int the initial value
     */
    public static box(value: int): IntBox {
        return new IntBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns int - value in the box
     */
    public get(): int {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: int): int {
        return (this.value = value);
    }
}

export final class LongBox {
    private value: long;

    public constructor() {
        this.value = 0;
    }

    public constructor(value: long) {
        this.value = value;
    }

    /**
     * Construct a new LongBox instance with provided initial value
     *
     * @param value: long the initial value
     */
    public static box(value: long): LongBox {
        return new LongBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns long - value in the box
     */
    public get(): long {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: long): long {
        return (this.value = value);
    }
}

export final class FloatBox {
    private value: float;

    public constructor() {
        this.value = 0.0f;
    }

    public constructor(value: float) {
        this.value = value;
    }

    /**
     * Construct a new FloatBox instance with provided initial value
     *
     * @param value: float the initial value
     */
    public static box(value: float): FloatBox {
        return new FloatBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns float - value in the box
     */
    public get(): float {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: float): float {
        return (this.value = value);
    }
}

export final class DoubleBox {
    private value: double;

    public constructor() {
        this.value = 0.0;
    }

    public constructor(value: double) {
        this.value = value;
    }

    /**
     * Construct a new DoubleBox instance with provided initial value
     *
     * @param value: double the initial value
     */
    public static box(value: double): DoubleBox {
        return new DoubleBox(value);
    }

    /**
     * Returns the value contained in the box
     *
     * @returns double - value in the box
     */
    public get(): double {
        return this.value;
    }

    /**
     * Sets the value contained in the box
     */
    public set(value: double): double {
        return (this.value = value);
    }
}
