/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
import { BusinessError } from "@ohos.base"

const BufferOverFlowErrorCodeId: number = 10200062;
const IllegalPositionErrorCodeId: number = 10200063;
const EmptyErrorCodeId: number = 10200064;
const NoElementMatchErrorCodeId: number = 10200065;
const EncodingErrorCodeId: number = 10200066;

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlDynamicSerializer API tests');

    suite.addTest('Testing XML length exceeding xml.MaxXmlLength and checking error code.',
        testXMLBoundaryValueExceedsMaxLength);
    suite.addTest('Testing mismatched start and end elements and checking error code.',
        testMismatchedStartAndEndElements);
    suite.addTest('Testing illegal position to set attributes and checking error code.',
        testIllegalPositionToSetAttributes);
    suite.addTest('Testing setting empty name for attributes and checking error code.', testSetEmptyNameForAttributes);
    suite.addTest('Testing setting unsupported encoding for XML and checking error code.', testSetUnsupportedEncoding);

    return suite.run();
}

/**
 * Common helper function to perform XML tests with expected exceptions
 * 
 * @param setup Function to set up the test scenario
 * @param errorAction Function that should trigger an error
 * @param expectedErrorCodeId Expected error code ID
 */
function performXmlTest(
    setup: (serializer: xml.XmlDynamicSerializer) => void,
    errorAction: (serializer: xml.XmlDynamicSerializer) => void,
    expectedErrorCodeId: number
): void {
    let thatSer = new xml.XmlDynamicSerializer();
    
    setup(thatSer);
    
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == expectedErrorCodeId;
        }
        return false;
    }
    
    arktest.expectThrow(() => errorAction(thatSer), exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_04800
function testXMLBoundaryValueExceedsMaxLength() {
    performXmlTest(
        (serializer) => {
            for (let i = 0; i < xml.MaxXmlLength / 25; ++i) {
                serializer.setNamespace('h', 'aaa');
                serializer.startElement('note');
                serializer.endElement();
            }
        },
        (serializer) => {
            serializer.startElement('note1');
        },
        BufferOverFlowErrorCodeId
    );
}

// Test cases ported from ArkTS 1.0: XML_04900
function testMismatchedStartAndEndElements() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.endElement();
        },
        (serializer) => {
            serializer.endElement();
        },
        NoElementMatchErrorCodeId
    );
}

// Test cases ported from ArkTS 1.0: XML_05000
function testIllegalPositionToSetAttributes() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.endElement();
        },
        (serializer) => {
            serializer.setAttributes('a', 'b');
        },
        IllegalPositionErrorCodeId
    );
}

// Test cases ported from ArkTS 1.0: XML_05100
function testSetEmptyNameForAttributes() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.endElement();
        },
        (serializer) => {
            serializer.setAttributes('', 'b');
        },
        EmptyErrorCodeId
    );
}

// Test cases ported from ArkTS 1.0: XML_05200
function testSetUnsupportedEncoding() {
    performXmlTest(
        (serializer) => {},
        (serializer) => {
            let thatSer = new xml.XmlDynamicSerializer('utf-16')
        },
        EncodingErrorCodeId
    );
}
