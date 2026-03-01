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

    suite.addTest('Test writing simple CDATA.', testWriteSimpleCDATA);
    suite.addTest('Test writing CDATA with URI.', testWriteCDATAWithURI);
    suite.addTest('Test writing CDATA with URI after end element.', testWriteCDATAWithURIAfterEnd);
    suite.addTest('Test writing CDATA with URI before start element.', testWriteCDATAWithURIBeforeStart);
    suite.addTest('Test writing CDATA with URI between start and end elements.',
        testWriteCDATAWithURIBetweenStartAndEnd);
    suite.addTest('Test writing CDATA containing ]]> between start and end elements.',
        testWriteCDATAWithSpecialCharacters);
    suite.addTest('Test writing nested CDATA between start and end elements.', testWriteNestedCDATA);
    suite.addTest('Test writing CDATA with function syntax between start and end elements.',
        testWriteCDATAWithFunctionSyntax1);
    suite.addTest('Test writing CDATA with logical operators between start and end elements.',
        testWriteCDATAWithFunctionSyntax2);
    suite.addTest('Test writing CDATA with equality check between start and end elements.',
        testWriteCDATAWithFunctionSyntax3);
    suite.addTest('Test writing empty CDATA throws an error.', testWriteEmptyCDATAThrowsError);

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

// Test cases ported from ArkTS 1.0: XML_04600
function testWriteSimpleCDATA() {
    performXmlTest((serializer) => {
        serializer.setCDATA('root SYSTEM');
    }, '<![CDATA[root SYSTEM]]>');
}

// Test cases ported from ArkTS 1.0: XML_04700
function testWriteCDATAWithURI() {
    performXmlTest((serializer) => {
        serializer.setCDATA('root SYSTEM "http://www.test.org/test.dtd"');
    }, '<![CDATA[root SYSTEM "http://www.test.org/test.dtd"]]>');
}

// Test cases ported from ArkTS 1.0: XML_04800
function testWriteCDATAWithURIAfterEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.endElement();
        serializer.setCDATA('root SYSTEM "http://www.test.org/test.dtd"');
    }, '<note/>\r\n<![CDATA[root SYSTEM "http://www.test.org/test.dtd"]]>');
}

// Test cases ported from ArkTS 1.0: XML_04900
function testWriteCDATAWithURIBeforeStart() {
    performXmlTest((serializer) => {
        serializer.setCDATA('root SYSTEM "http://www.test.org/test.dtd"');
        serializer.startElement('note');
        serializer.endElement();
    }, '<![CDATA[root SYSTEM "http://www.test.org/test.dtd"]]>\r\n<note/>');
}

// Test cases ported from ArkTS 1.0: XML_05000
function testWriteCDATAWithURIBetweenStartAndEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setCDATA('root SYSTEM "http://www.test.org/test.dtd"');
        serializer.endElement();
    }, '<note>\r\n  <![CDATA[root SYSTEM "http://www.test.org/test.dtd"]]>\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_11900
function testWriteCDATAWithSpecialCharacters() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setCDATA(']]>');
        serializer.endElement();
    }, '<note>\r\n  <![CDATA[]]]]><![CDATA[>]]>\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_12000
function testWriteNestedCDATA() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setCDATA('<![CDATA[]]>');
        serializer.endElement();
    }, '<note>\r\n  <![CDATA[<![CDATA[]]]]><![CDATA[>]]>\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_12100
function testWriteCDATAWithFunctionSyntax1() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setCDATA('fun() {if (a < b) return;}');
        serializer.endElement();
    }, '<note>\r\n  <![CDATA[fun() {if (a < b) return;}]]>\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_12200
function testWriteCDATAWithFunctionSyntax2() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setCDATA('fun() {if (a && b) return;}');
        serializer.endElement();
    }, '<note>\r\n  <![CDATA[fun() {if (a && b) return;}]]>\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_12300
function testWriteCDATAWithFunctionSyntax3() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setCDATA('fun() {if (a == \'xml\') return;}');
        serializer.endElement();
    }, '<note>\r\n  <![CDATA[fun() {if (a == \'xml\') return;}]]>\r\n</note>');
}

function testWriteEmptyCDATAThrowsError() {
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
    arktest.expectThrow(() => { thatSer.setCDATA('') }, exceptionCheck);
}
