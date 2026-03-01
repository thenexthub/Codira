/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { xml } from "@ohos.xml"

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlDynamicSerializer API tests');

    suite.addTest('Testing writes the CDATA.', testSetCdataContent);
    suite.addTest('Testing writes the CDATA with URI.', testSetCdataWithUri);
    suite.addTest('Testing writes the CDATA with URI before start element.', testSetCdataBeforeStartElement);
    suite.addTest('Testing writes the CDATA with URI between start element and end element.',
        testSetCdataBetweenStartAndEnd);

    return suite.run();
}

/**
 * Common function to perform XML serialization tests
 * @param operationsCallback Function to perform specific XML operations on the serializer
 * @param expectedResult Expected XML output string
 */
function performXmlTest(operationsCallback: (serializer: xml.XmlDynamicSerializer) => void, 
                        expectedResult: string): void {
    let thatSer = new xml.XmlDynamicSerializer();
    operationsCallback(thatSer);
    let arrayBuffer = thatSer.getOutput();
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_03600
function testSetCdataContent() {
    performXmlTest(
        (serializer) => {
            serializer.setCdata('root SYSTEM');
        },
        '<![CDATA[root SYSTEM]]>'
    );
}

// Test cases ported from ArkTS 1.0: XML_03700
function testSetCdataWithUri() {
    performXmlTest(
        (serializer) => {
            serializer.setCdata('root SYSTEM "http://www.test.org/test.dtd"');
        }, 
        '<![CDATA[root SYSTEM "http://www.test.org/test.dtd"]]>'
    );
}

// Test cases ported from ArkTS 1.0: XML_03800
function testSetCdataBeforeStartElement() {
    performXmlTest(
        (serializer) => {
            serializer.setCdata('root SYSTEM "http://www.test.org/test.dtd"');
            serializer.startElement('note');
            serializer.endElement();
        },
        '<![CDATA[root SYSTEM "http://www.test.org/test.dtd"]]>\r\n<note/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_03900
function testSetCdataBetweenStartAndEnd() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setCdata('root SYSTEM "http://www.test.org/test.dtd"');
            serializer.endElement();
        },
        '<note>\r\n  <![CDATA[root SYSTEM "http://www.test.org/test.dtd"]]>\r\n</note>'
    );
}
