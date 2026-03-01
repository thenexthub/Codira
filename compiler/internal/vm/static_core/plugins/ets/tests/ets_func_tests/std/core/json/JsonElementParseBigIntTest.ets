/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 	 
function main(): int {
    const suite = new arktest.ArkTestsuite('json.JsonElement')

    suite.addTest('JsonElement test string to bigint with ALWAYS_PARSE_AS_BIGINT', testStringToBigIntWithMode2)
    suite.addTest('JsonElement test string to bigint with PARSE_AS_BIGINT', testStringToBigIntWithMode1)
    suite.addTest('JsonElement test string to bigint with DEFAULT', testStringToBigIntWithMode0)
    suite.addTest('JsonElement test User Parse JsonElement', testUesrParseJsonElement)
    return suite.run()
}

function testStringToBigIntWithMode2(): void {
    let str = '9007199254740991'  // Double.MAX_SAFE_INTEGER
    let opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT} as jsonx.ParseOptions;
    let elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 9007199254740991n)
    
    str = '9007199254740992'   
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 9007199254740992n)
    arktest.assertEQ(elem.tryAsDouble(), undefined)
    

    str = '9223372036854775807'  // Long.MAX_VALUE
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 9223372036854775807n)
    arktest.assertEQ(elem.tryAsLong(), undefined) 
    

    str = '9223372036854775808'  // Long.MAX_VALUE + 1
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 9223372036854775808n)
    

    str = '1000000000000000000000000'  // 10^24
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 1000000000000000000000000n)
    

    str = '-9007199254740992'  // MIN_SAFE_INTEGER
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), -9007199254740992n)
    
    str = '-9223372036854775808'  // Long.MIN_VALUE
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), -9223372036854775808n)
    
    str = '0'
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 0n)

    
    str = '-0'
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 0n)  
    
    str = '18446744073709551616'  // 2^64
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asBigInt(), 18446744073709551616n)    
}

function testStringToBigIntWithMode1(): void {
    let str = '9007199254740991'  // Double.MAX_SAFE_INTEGER
    let opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.PARSE_AS_BIGINT} as jsonx.ParseOptions;
    let elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 9007199254740991)
    arktest.assertEQ(elem.asDouble(), 9007199254740991)
    arktest.assertEQ(elem.tryAsBigInt(), 9007199254740991n)
    arktest.assertEQ(elem.asBigInt(), 9007199254740991n)
   
    str = '9007199254740992'   
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 9007199254740992)
    arktest.expectError((): void => { elem.asDouble() }, new RangeError(`'9007199254740992' is not a double`))
    arktest.assertEQ(elem.tryAsBigInt(), 9007199254740992n)
    arktest.assertEQ(elem.asBigInt(), 9007199254740992n)
        
    str = '9223372036854775808'  // Long.MAX_VALUE + 1
    elem = JSON.parseJsonElement(str, opt)
    arktest.expectError((): void => { elem.asLong() }, new jsonx.JsonTypeError(`Expected long, but element stores different value type.`))
    arktest.expectError((): void => { elem.asDouble() }, new jsonx.JsonTypeError(`This value's ValueKind is not Number.`))
    arktest.assertEQ(elem.asBigInt(), 9223372036854775808n)

    str = '1000000000000000000000000'  // 10^24
    elem = JSON.parseJsonElement(str, opt)
    arktest.expectError((): void => { elem.asLong() }, new jsonx.JsonTypeError(`Expected long, but element stores different value type.`))
    arktest.expectError((): void => { elem.asDouble() }, new jsonx.JsonTypeError(`This value's ValueKind is not Number.`))
    arktest.assertEQ(elem.asBigInt(), 1000000000000000000000000n)

    str = '-9007199254740992'  // MIN_SAFE_INTEGER   
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), -9007199254740992)
    arktest.expectError((): void => { elem.asDouble() }, new RangeError(`'-9007199254740992' is not a double`))
    arktest.assertEQ(elem.asBigInt(), -9007199254740992n)

    str = '-9223372036854775808'  // MIN_SAFE_INTEGER   
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), -9223372036854775808)
    arktest.expectError((): void => { elem.asDouble() }, new RangeError(`'-9223372036854775808' is not a double`))
    arktest.assertEQ(elem.asBigInt(), -9223372036854775808n)

    str = '0' 
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 0)
    arktest.assertEQ(elem.asDouble(), 0)
    arktest.assertEQ(elem.asBigInt(), 0n)

    str = '-0' 
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 0)
    arktest.assertEQ(elem.asDouble(), 0)
    arktest.assertEQ(elem.tryAsBigInt(), 0n)

    str = '18446744073709551616'  // 2^64  
    elem = JSON.parseJsonElement(str, opt)
    arktest.expectError((): void => { elem.asLong() }, new jsonx.JsonTypeError(`Expected long, but element stores different value type.`))
    arktest.expectError((): void => { elem.asDouble() }, new jsonx.JsonTypeError(`This value's ValueKind is not Number.`))
    arktest.assertEQ(elem.asBigInt(), 18446744073709551616n)
}

function testStringToBigIntWithMode0(): void {
    let str = '9007199254740991'  // Double.MAX_SAFE_INTEGER
    let opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.DEFAULT} as jsonx.ParseOptions;
    let elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 9007199254740991)
    arktest.assertEQ(elem.asDouble(), 9007199254740991)
    arktest.assertEQ(elem.asBigInt(), 9007199254740991n)
   
    str = '9007199254740992'   
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 9007199254740992)
    arktest.expectError((): void => { elem.asDouble() }, new RangeError(`'9007199254740992' is not a double`))
    arktest.assertEQ(elem.asBigInt(), 9007199254740992n)
        
    str = '9223372036854775808'  // Long.MAX_VALUE + 1
    arktest.expectError((): void => { JSON.parseJsonElement(str, opt) }, new FormatError('Value exceeds integer limits'))
    arktest.expectError((): void => { JSON.parseJsonElement(str, opt).asBigInt() }, new FormatError('Value exceeds integer limits'))

    
    str = '1000000000000000000000000'  // 10^24
    arktest.expectError((): void => { JSON.parseJsonElement(str, opt) }, new FormatError('Value exceeds integer limits'))

    str = '-9007199254740992'  // MIN_SAFE_INTEGER   
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), -9007199254740992)
    arktest.expectError((): void => { elem.asDouble() }, new RangeError(`'-9007199254740992' is not a double`))
    arktest.assertEQ(elem.asBigInt(), -9007199254740992n)

    str = '-9223372036854775808'  // MIN_SAFE_INTEGER   
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), -9223372036854775808)
    arktest.expectError((): void => { elem.asDouble() }, new RangeError(`'-9223372036854775808' is not a double`))
    arktest.assertEQ(elem.asBigInt(), -9223372036854775808n)

    str = '0' 
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 0)
    arktest.assertEQ(elem.asDouble(), 0)
    arktest.assertEQ(elem.asBigInt(), 0n)

    str = '-0' 
    elem = JSON.parseJsonElement(str, opt)
    arktest.assertEQ(elem.asLong(), 0)
    arktest.assertEQ(elem.asDouble(), 0)
    arktest.assertEQ(elem.tryAsBigInt(), 0n)

    str = '18446744073709551616'  // 2^64  
    arktest.expectError((): void => { JSON.parseJsonElement(str, opt) }, new FormatError('Value exceeds integer limits'))
}

class User implements jsonx.JsonElementSerializable {
    name: string = '';
    age: int = 0;
    money: bigint = 0n;

    toJSON(): jsonx.JsonElement {
        let elem = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
        elem.setElement('name', jsonx.JsonElement.createString(this.name));
        elem.setElement('age', jsonx.JsonElement.createInteger(this.age));
        elem.setElement('money', jsonx.JsonElement.createBigInt(this.money));
        return elem;
    }
}

function testUesrParseJsonElement() {
    let user = new User();
    user.name = 'John';
    user.age = 30;
    user.money=BigInt('1000000000000000000000000');
    const jsonelem = user.toJSON();
    const jsonstr = JSON.stringifyJsonElement(jsonelem)
    let opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.PARSE_AS_BIGINT} as jsonx.ParseOptions;
    let elem = JSON.parseJsonElement(jsonstr, opt)
    arktest.assertEQ(elem.getElement('age').asInteger(), 30)
    arktest.assertEQ(elem.getElement('name').asString(), 'John')
    arktest.assertEQ(elem.getElement('money').asBigInt(), 1000000000000000000000000n)
}
