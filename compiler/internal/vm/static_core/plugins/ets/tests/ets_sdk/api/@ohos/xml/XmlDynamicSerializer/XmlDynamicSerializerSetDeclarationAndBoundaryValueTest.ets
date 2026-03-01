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

    suite.addTest('Testing XML declaration with utf-8 encoding.', testXMLDeclarationWithUtf8Encoding);
    suite.addTest('Testing XML serialization with zero length.', testXMLSerializationWithZeroLength);

    return suite.run();
}

// Common function to perform XML serialization tests
function performXmlTest(serializerSetup: (serializer: xml.XmlDynamicSerializer) => void, 
                        encoding?: string, 
                        expectedResult: string = ''): void {
    let thatSer: xml.XmlDynamicSerializer;
    if (encoding == undefined) {
        thatSer = new xml.XmlDynamicSerializer();
    } else {
        thatSer = new xml.XmlDynamicSerializer(encoding);
    }
    serializerSetup(thatSer);
    let arrayBuffer = thatSer.getOutput();
    let view1 = '';
    let view = new Uint8Array(arrayBuffer);
    // Iterate through the appropriate length (either the expected result length or the full view)
    const iterLength = expectedResult ? expectedResult.length : view.length;
    for (let i = 0; i < iterLength; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_00100
function testXMLDeclarationWithUtf8Encoding() {
    performXmlTest(
        (serializer) => serializer.setDeclaration(),
        'utf-8',
        '<?xml version="1.0" encoding="utf-8"?>'
    );
}

// Test cases ported from ArkTS 1.0: XML_04600
function testXMLSerializationWithZeroLength() {
    performXmlTest(
        (serializer) => { /* No operations needed */ },
        undefined,
        ''
    );
}
