/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const { getTestModule } = require('conversion_types.test.abc');

const stsMod = getTestModule('conversion_types');

const globStsString = stsMod.getClass('ETSGLOBAL').globStsString;
const globStsNumber = stsMod.getClass('ETSGLOBAL').globStsNumber;
const globStsInt = stsMod.getClass('ETSGLOBAL').globStsInt;    
const globStsByte = stsMod.getClass('ETSGLOBAL').globStsByte;   
const globStsShort = stsMod.getClass('ETSGLOBAL').globStsShort;  
const globStsLong = stsMod.getClass('ETSGLOBAL').globStsLong;   
const globStsFloat = stsMod.getClass('ETSGLOBAL').globStsFloat;  
const globStsDouble = stsMod.getClass('ETSGLOBAL').globStsDouble; 
const globStsChar = stsMod.getClass('ETSGLOBAL').globStsChar; 
const globStsBoolean = stsMod.getClass('ETSGLOBAL').globStsBoolean;
const globStsBigInt = stsMod.getClass('ETSGLOBAL').globStsBigInt; 

function checkGlobStsString() {
    ASSERT_TRUE(typeof globStsString === 'string');
    ASSERT_TRUE(globStsString === 'Hello');
}

function checkGlobStsNumber() {
    ASSERT_TRUE(typeof globStsNumber === 'number');
    ASSERT_TRUE(globStsNumber === 42);
}

function checkGlobStsInt() {
    ASSERT_TRUE(globStsInt === 42);
}

function checkGlobStsByte() {
    ASSERT_TRUE(globStsByte === 126);
}

function checkGlobStsShort() {
    ASSERT_TRUE(globStsShort === 4242);
}

function checkGlobStsLong() {
    ASSERT_TRUE(globStsLong === 42);
}

function checkGlobStsFloat() {
    ASSERT_TRUE(Math.abs(globStsFloat - 42.42) < 0.1);
}

function checkGlobStsDouble() {
    ASSERT_TRUE(Math.abs(globStsDouble - 42.42) < 0.1);
}

function checkGlobStsChar() {
    ASSERT_TRUE(globStsChar === 'a');
}

function checkGlobStsBoolean() {
    ASSERT_TRUE(globStsBoolean === true);
}

function checkGlobStsBigInt() {
    let val = BigInt(42); 
    ASSERT_TRUE(typeof globStsBigInt === 'bigint');
    ASSERT_TRUE(globStsBigInt === val);
}

checkGlobStsString();
checkGlobStsNumber();
checkGlobStsInt();
checkGlobStsByte();
checkGlobStsShort();
checkGlobStsLong();
checkGlobStsFloat();
checkGlobStsDouble();
checkGlobStsChar();
checkGlobStsBoolean();
checkGlobStsBigInt();
