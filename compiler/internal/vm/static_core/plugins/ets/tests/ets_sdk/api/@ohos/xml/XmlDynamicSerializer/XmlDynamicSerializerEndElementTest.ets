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

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlSerializer API tests');
    suite.addTest('Testing end tag without attributes', testEndElementWithoutAttributes);
    suite.addTest('Testing end tag with single attribute', testEndElementWithSingleAttribute);
    suite.addTest('Testing nested end tags', testNestedEndElements);

    return suite.run();
}

/**
 * Common function to perform XML dynamic serialization tests
 * @param serializerOperation Function to perform operations on the serializer
 * @param expectedResult Expected XML string result
 */
function performXmlTest(serializerOperation: (serializer: xml.XmlDynamicSerializer) => void, expectedResult: string): void {
    let thatSer = new xml.XmlDynamicSerializer();
    
    serializerOperation(thatSer);
    
    let arrayBuffer = thatSer.getOutput();
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

function testEndElementWithoutAttributes() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.endElement();
    }, '<note/>');
}

function testEndElementWithSingleAttribute() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'high');
        serializer.endElement();
    }, '<note importance="high"/>');
}

function testNestedEndElements() {
    performXmlTest((serializer) => {
        serializer.startElement('note1');
        serializer.startElement('note2');
        serializer.endElement();
        serializer.endElement();
    }, '<note1>\r\n  <note2/>\r\n</note1>');
}
