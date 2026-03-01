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

let etsVm = globalThis.gtest.etsVm;
let etsGlobal = etsVm.getClass('Lets_tuple/ETSGLOBAL;');
let tuple0Object = etsGlobal.tuple0Object;
let tuple1Object = etsGlobal.tuple1Object;
let tuple3Object = etsGlobal.tuple3Object;
let tuple8Object = etsGlobal.tuple8Object;
let tuple16Object = etsGlobal.tuple16Object;

const SET_TUPLE_OUT_OF_RANGE_MESSAGE = `Tuple index out of bounds`;

function testTuple0() {
    // ets: export let tuple0Object: [] = [];

    // test tuple get
    ASSERT_TRUE(tuple0Object[0] === undefined); // shuld be undefined

    // test tuple set
    // this will throw an error in ETS
    let errorCaught = false;
    try {
        tuple0Object[0] = 0x55aa; // This should not change the length
    } catch (e) {
        ASSERT_TRUE(e instanceof Error);
        ASSERT_TRUE(e.message.includes(SET_TUPLE_OUT_OF_RANGE_MESSAGE));
        errorCaught = true;
    }

    ASSERT_TRUE(errorCaught);
}

function testTuple1() {
    // ets: export let tuple1Object: [number] = [0x55aa];

    // test tuple get
    ASSERT_TRUE(tuple1Object[0] === 0x55aa);

    // test tuple set number
    tuple1Object[0] = 0x7c00;
    ASSERT_TRUE(tuple1Object[0] === 0x7c00);

    // test tuple set string
    tuple1Object[0] = 'world'; // This should not change the length
    ASSERT_TRUE(tuple1Object[0] === 'world');

    // test tuple get out of range
    ASSERT_TRUE(tuple0Object[1] === undefined); // should be undefined

    // test tuple set out of range
    let errorCaught = false;
    try {
        tuple1Object[1] = 0x1234; // This should not change the length
    } catch (e) {
        ASSERT_TRUE(e instanceof Error);
        ASSERT_TRUE(e.message.includes(SET_TUPLE_OUT_OF_RANGE_MESSAGE));
        errorCaught = true;
    }

    ASSERT_TRUE(errorCaught);
}

function testTuple3() {
    // ets export let tuple3Object: [number, number, string] = [0xbabe, 0xcafe, `hello`];

    // test tuple get
    ASSERT_TRUE(tuple3Object[0] === 0xbabe);
    ASSERT_TRUE(tuple3Object[1] === 0xcafe);
    ASSERT_TRUE(tuple3Object[2] === `hello`);

    // test tuple set
    tuple3Object[0] = 0x55aa;
    tuple3Object[1] = 0x7c00;
    tuple3Object[2] = `world`;
    ASSERT_TRUE(tuple3Object[0] === 0x55aa);
    ASSERT_TRUE(tuple3Object[1] === 0x7c00);
    ASSERT_TRUE(tuple3Object[2] === `world`);

    // test tuple get out of range
    ASSERT_TRUE(tuple3Object[3] === undefined); // should be undefined

    // test tuple set out of range
    let errorCaught = false;
    try {
        tuple3Object[3] = 0x1234; // This should not change the length
    } catch (e) {
        ASSERT_TRUE(e instanceof Error);
        ASSERT_TRUE(e.message.includes(SET_TUPLE_OUT_OF_RANGE_MESSAGE));
        errorCaught = true;
    }
    ASSERT_TRUE(errorCaught);
}

function testTuple8() {
    // ets: export let tuple8Object: [number, number, string, number, number, string, number, number] = [1, 2, `three`, 4, 5, `six`, 7, 8];

    // test tuple get
    ASSERT_TRUE(tuple8Object[0] === 1);
    ASSERT_TRUE(tuple8Object[1] === 2);
    ASSERT_TRUE(tuple8Object[2] === `three`);
    ASSERT_TRUE(tuple8Object[3] === 4);
    ASSERT_TRUE(tuple8Object[4] === 5);
    ASSERT_TRUE(tuple8Object[5] === `six`);
    ASSERT_TRUE(tuple8Object[6] === 7);
    ASSERT_TRUE(tuple8Object[7] === 8);

    // test tuple set
    tuple8Object[0] = 10;
    tuple8Object[1] = 20;
    tuple8Object[2] = `thirty`;
    tuple8Object[3] = 40;
    tuple8Object[4] = 50;
    tuple8Object[5] = `sixty`;
    tuple8Object[6] = 70;
    tuple8Object[7] = 80;
    ASSERT_TRUE(tuple8Object[0] === 10);
    ASSERT_TRUE(tuple8Object[1] === 20);
    ASSERT_TRUE(tuple8Object[2] === `thirty`);
    ASSERT_TRUE(tuple8Object[3] === 40);
    ASSERT_TRUE(tuple8Object[4] === 50);
    ASSERT_TRUE(tuple8Object[5] === `sixty`);
    ASSERT_TRUE(tuple8Object[6] === 70);
    ASSERT_TRUE(tuple8Object[7] === 80);

    // test tuple get out of range
    ASSERT_TRUE(tuple8Object[8] === undefined); // should be undefined

    // test tuple set out of range
    let errorCaught = false;
    try {
        tuple8Object[8] = 0x1234; // This should not change the length
    } catch (e) {
        ASSERT_TRUE(e instanceof Error);
        ASSERT_TRUE(e.message.includes(SET_TUPLE_OUT_OF_RANGE_MESSAGE));
        errorCaught = true;
    }
    ASSERT_TRUE(errorCaught);
}

function testTuple16() {
    // ets:
    // export let tuple16Object: [number, number, string, number, number, string, number, number, number, number, string, number, number, string, number, number] = [
    //     1, 2, `three`, 4, 5, `six`, 7, 8,
    //     9, 10, `eleven`, 12, 13, `fourteen`, 15, 16
    // ];

    // test tuple get
    ASSERT_TRUE(tuple16Object[0] === 1);
    ASSERT_TRUE(tuple16Object[1] === 2);
    ASSERT_TRUE(tuple16Object[2] === `three`);
    ASSERT_TRUE(tuple16Object[3] === 4);
    ASSERT_TRUE(tuple16Object[4] === 5);
    ASSERT_TRUE(tuple16Object[5] === `six`);
    ASSERT_TRUE(tuple16Object[6] === 7);
    ASSERT_TRUE(tuple16Object[7] === 8);
    ASSERT_TRUE(tuple16Object[8] === 9);
    ASSERT_TRUE(tuple16Object[9] === 10);
    ASSERT_TRUE(tuple16Object[10] === `eleven`);
    ASSERT_TRUE(tuple16Object[11] === 12);
    ASSERT_TRUE(tuple16Object[12] === 13);
    ASSERT_TRUE(tuple16Object[13] === `fourteen`);
    ASSERT_TRUE(tuple16Object[14] === 15);
    ASSERT_TRUE(tuple16Object[15] === 16);

    // test tuple set
    tuple16Object[0] = 100;
    tuple16Object[1] = 200;
    tuple16Object[2] = `three hundred`;
    tuple16Object[3] = 400;
    tuple16Object[4] = 500;
    tuple16Object[5] = `six hundred`;
    tuple16Object[6] = 700;
    tuple16Object[7] = 800;
    tuple16Object[8] = 900;
    tuple16Object[9] = 1000;
    tuple16Object[10] = `eleven hundred`;
    tuple16Object[11] = 1200;
    tuple16Object[12] = 1300;
    tuple16Object[13] = `fourteen hundred`;
    tuple16Object[14] = 1500;
    tuple16Object[15] = 1600;
    ASSERT_TRUE(tuple16Object[0] === 100);
    ASSERT_TRUE(tuple16Object[1] === 200);
    ASSERT_TRUE(tuple16Object[2] === `three hundred`);
    ASSERT_TRUE(tuple16Object[3] === 400);
    ASSERT_TRUE(tuple16Object[4] === 500);
    ASSERT_TRUE(tuple16Object[5] === `six hundred`);
    ASSERT_TRUE(tuple16Object[6] === 700);
    ASSERT_TRUE(tuple16Object[7] === 800);
    ASSERT_TRUE(tuple16Object[8] === 900);
    ASSERT_TRUE(tuple16Object[9] === 1000);
    ASSERT_TRUE(tuple16Object[10] === `eleven hundred`);
    ASSERT_TRUE(tuple16Object[11] === 1200);
    ASSERT_TRUE(tuple16Object[12] === 1300);
    ASSERT_TRUE(tuple16Object[13] === `fourteen hundred`);
    ASSERT_TRUE(tuple16Object[14] === 1500);
    ASSERT_TRUE(tuple16Object[15] === 1600);

    // test tuple get out of range
    ASSERT_TRUE(tuple16Object[16] === undefined); // should be undefined

    // test tuple set out of range
    let errorCaught = false;
    try {
        tuple16Object[16] = 0x1234; // This should not change the length
    } catch (e) {
        ASSERT_TRUE(e instanceof Error);
        ASSERT_TRUE(e.message.includes(SET_TUPLE_OUT_OF_RANGE_MESSAGE));
        errorCaught = true;
    }
    ASSERT_TRUE(errorCaught);
}

function testGetTheSameObject() {
    let newTuple3Object = etsGlobal.tuple3Object;
    ASSERT_TRUE(tuple3Object === newTuple3Object);
}

function main() {
    testTuple0();
    testTuple1();
    testTuple3();
    testTuple8();
    testTuple16();

    testGetTheSameObject();
}

main();
