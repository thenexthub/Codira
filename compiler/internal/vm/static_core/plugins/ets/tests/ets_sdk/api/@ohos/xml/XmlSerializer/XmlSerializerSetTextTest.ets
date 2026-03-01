/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import { xml } from "@ohos.xml"
import { BusinessError } from "@ohos.base"

const TypeErrorCodeId: int = 401;
const MaxBufferLength = 1024;

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlSerializer API tests');

    suite.addTest("Test writing text 'Happy1' with attributes.", testWriteTextHappy1);
    suite.addTest("Test writing text containing '>'.", testWriteTextWithGreaterThan);
    suite.addTest("Test writing text containing '<'.", testWriteTextWithLessThan);
    suite.addTest("Test writing text containing '&'.", testWriteTextWithAmpersand);
    suite.addTest("Test writing text containing single quote.", testWriteTextWithSingleQuote);
    suite.addTest("Test writing text containing double quote.", testWriteTextWithDoubleQuote);
    suite.addTest("Test writing empty text throws an error.", testWriteEmptyTextThrowsError);

    return suite.run();
}

/**
 * Common function to perform XML serialization tests
 * @param serializerOperation Function to perform operations on the serializer
 * @param expectedResult Expected XML string result
 */
function performXmlTest(serializerOperation: (serializer: xml.XmlSerializer) => void, expectedResult: string): void {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let serializer = new xml.XmlSerializer(arrayBuffer);
    
    serializerOperation(serializer);
    
    let view = new Uint8Array(arrayBuffer);
    let result = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        result += String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(result, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_02100
function testWriteTextHappy1() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'high');
        serializer.setText('Happy1');
        serializer.endElement();
    }, '<note importance="high">Happy1</note>');
}

// Test cases ported from ArkTS 1.0: XML_09400
function testWriteTextWithGreaterThan() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'high');
        serializer.setText('Hello>World');
        serializer.endElement();
    }, '<note importance="high">Hello&gt;World</note>');
}

// Test cases ported from ArkTS 1.0: XML_09500
function testWriteTextWithLessThan() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'high');
        serializer.setText('Hello<World');
        serializer.endElement();
    }, '<note importance="high">Hello&lt;World</note>');
}

// Test cases ported from ArkTS 1.0: XML_09600
function testWriteTextWithAmpersand() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'high');
        serializer.setText('Hello&World');
        serializer.endElement();
    }, '<note importance="high">Hello&amp;World</note>');
}

// Test cases ported from ArkTS 1.0: XML_09700
function testWriteTextWithSingleQuote() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'high');
        serializer.setText("Hello'World");
        serializer.endElement();
    }, '<note importance="high">Hello&apos;World</note>');
}

// Test cases ported from ArkTS 1.0: XML_09800
function testWriteTextWithDoubleQuote() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'high');
        serializer.setText('Hello"World');
        serializer.endElement();
    }, '<note importance="high">Hello&quot;World</note>');
}

function testWriteEmptyTextThrowsError() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement('note');
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "Parameter error. Parameter cannot be empty";
        }
        return false;
    }
    arktest.expectThrow(() => { thatSer.setText('') }, exceptionCheck);
}
