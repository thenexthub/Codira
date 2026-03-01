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

    suite.addTest('Test writing DOCTYPE declaration.', testWriteDocTypeDeclaration);
    suite.addTest('Test writing DOCTYPE declaration with URI.', testWriteDocTypeWithURI);
    suite.addTest('Test writing DOCTYPE declaration with URI after end element.', testWriteDocTypeWithURIAfterEnd);
    suite.addTest('Test writing DOCTYPE declaration with URI before start element.',
        testWriteDocTypeWithURIBeforeStart);
    suite.addTest('Test writing DOCTYPE declaration with URI between start and end elements.',
        testWriteDocTypeWithURIBetweenStartAndEnd);
    suite.addTest('Test writing empty DOCTYPE throws an error.', testWriteEmptyDocTypeThrowsError);

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

// Test cases ported from ArkTS 1.0: XML_04100
function testWriteDocTypeDeclaration() {
    performXmlTest((serializer) => {
        serializer.setDocType('root SYSTEM');
    }, '<!DOCTYPE root SYSTEM>');
}

// Test cases ported from ArkTS 1.0: XML_04200
function testWriteDocTypeWithURI() {
    performXmlTest((serializer) => {
        serializer.setDocType('root SYSTEM "http://www.test.org/test.dtd"');
    }, '<!DOCTYPE root SYSTEM "http://www.test.org/test.dtd">');
}

// Test cases ported from ArkTS 1.0: XML_04300
function testWriteDocTypeWithURIAfterEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.endElement();
        serializer.setDocType('root SYSTEM "http://www.test.org/test.dtd"');
    }, '<note/>\r\n<!DOCTYPE root SYSTEM "http://www.test.org/test.dtd">');
}

// Test cases ported from ArkTS 1.0: XML_04400
function testWriteDocTypeWithURIBeforeStart() {
    performXmlTest((serializer) => {
        serializer.setDocType('root SYSTEM "http://www.test.org/test.dtd"');
        serializer.startElement('note');
        serializer.endElement();
    }, '<!DOCTYPE root SYSTEM "http://www.test.org/test.dtd">\r\n<note/>');
}

// Test cases ported from ArkTS 1.0: XML_04500
function testWriteDocTypeWithURIBetweenStartAndEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setDocType('root SYSTEM "http://www.test.org/test.dtd"');
        serializer.endElement();
    }, '<note>\r\n  <!DOCTYPE root SYSTEM "http://www.test.org/test.dtd">\r\n</note>');
}

function testWriteEmptyDocTypeThrowsError() {
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
    arktest.expectThrow(() => { thatSer.setDocType('') }, exceptionCheck);
}
