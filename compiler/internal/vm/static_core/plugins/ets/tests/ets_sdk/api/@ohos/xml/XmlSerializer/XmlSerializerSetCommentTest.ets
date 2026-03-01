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
const MaxBufferLength = 2048;

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlSerializer API tests');

    suite.addTest('Test writing a comment between start and end elements (short text).',
        testWriteShortCommentBetweenElements);
    suite.addTest('Test writing a comment between start and end elements (long text).',
        testWriteLongCommentBetweenElements);
    suite.addTest('Test writing a comment without start or end elements (short text).',
        testWriteShortCommentWithoutElements);
    suite.addTest('Test writing a comment without start or end elements (long text).',
        testWriteLongCommentWithoutElements);
    suite.addTest('Test writing a comment before the start element.', testWriteCommentBeforeStartElement);
    suite.addTest('Test writing an empty comment throws an error.', testWriteEmptyCommentThrowsError);

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

// Test cases ported from ArkTS 1.0: XML_03600
function testWriteShortCommentBetweenElements() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setComment('Hi!');
        serializer.endElement();
    }, '<note>\r\n  <!--Hi!-->\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_03700
function testWriteLongCommentBetweenElements() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setComment('Hello, World!');
        serializer.endElement();
    }, '<note>\r\n  <!--Hello, World!-->\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_03800
function testWriteShortCommentWithoutElements() {
    performXmlTest((serializer) => {
        serializer.setComment('Hi!');
    }, '<!--Hi!-->');
}

// Test cases ported from ArkTS 1.0: XML_03900
function testWriteLongCommentWithoutElements() {
    performXmlTest((serializer) => {
        serializer.setComment('Hello, World!');
    }, '<!--Hello, World!-->');
}

// Test cases ported from ArkTS 1.0: XML_04000
function testWriteCommentBeforeStartElement() {
    performXmlTest((serializer) => {
        serializer.setComment('Hello, World!');
        serializer.startElement('note');
        serializer.endElement();
    }, '<!--Hello, World!-->\r\n<note/>');
}

function testWriteEmptyCommentThrowsError() {
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
    arktest.expectThrow(() => { thatSer.setComment('') }, exceptionCheck);
}
