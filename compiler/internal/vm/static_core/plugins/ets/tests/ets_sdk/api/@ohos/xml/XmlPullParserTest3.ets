/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import {xml} from "@ohos.xml"
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part3");
    suite.addTest("Testing XML isEmptyElementTag with illegal XML structure.", testXmlIsEmptyElementTag0003)
    suite.addTest("Testing XML isEmptyElementTag with valid non-empty element.", testXmlIsEmptyElementTag0002)
    suite.addTest("Testing XML isEmptyElementTag with valid empty element.", testXmlIsEmptyElementTag0001)

    suite.addTest("Testing XML ParseInfo GetText with illegal content.", testXmlParseInfoGetText0002)
    suite.addTest("Testing XML ParseInfo GetText with legal content.", testXmlParseInfoGetText0001)

    suite.addTest("Testing XML ParseInfo Prefix.", testXmlParseInfoGetPrefix0001)

    suite.addTest("Testing XML ParseInfo getNamespace with namespace is not existent.", testXmlParseInfoGetNamespace0002)
    suite.addTest("Testing XML ParseInfo getNamespace with namespace is existent.", testXmlParseInfoGetNamespace0001)
    suite.addTest("Testing XML ParseInfo isWhitespace.", testXmlIsWhitespace0001)
    suite.addTest("Testing XML ParseInfo getAttributeCount testCase0001", testXmlGetAttributeCount0001)
    suite.addTest("Testing XML ParseInfo getAttributeCount testCase0002", testXmlGetAttributeCount0002)
    suite.addTest("Testing XML ParseInfo getAttributeCount testCase0003", testXmlGetAttributeCount0003)

    return suite.run()
}

function rawStringToBuffer( str: string ) : ArrayBuffer {
    let idx = 0
    let len = str.length
    let arr = new Uint8Array( len )
    for ( idx = 0 ; idx < len ; ++idx ) {
        arr[ idx ] = str.charCodeAt(idx) & 0xFF;
    }

    return arr.buffer as ArrayBuffer
}

let testXml =
  "<?xml version='1.0' encoding=\"utf-8\"?><!DOCTYPE example [<!ENTITY exampleEntity 'some entity'>]>\
  <!-- XMLjghk --><example><!-- fghj -->\
  <startDocument><startTag><element><!-- yesdryug -->\
  <tagName>tfyugyi</tagName><!-- xtgh --><content>ydtyfugihj</content>" +
    "<!-- CDATAguhi -->" +
    "<cdata><![CDATA[fyghjk]]></cdata>" +
    "<!-- ugh -->" +
    "<?processingInstruction example='cfgvhj'?>" +
    "<!-- gvhblkm -->" +
    "<entityReference>&exampleEntity;</entityReference>" +
    "<!-- fcguhjk -->" +
    "<whitespace>     </whitespace>" +
    "</element>" +
    "<!-- ufyui -->" +
    "</startTag>" +
    "</startDocument>" +
    "<!-- ufygo -->" +
    "<endDocument/>" +
    "</example>";

function testXmlParseInfoGetNamespace0001() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note xmlns:h="http://www.w3.org">' +
            '<h:title>Happy</h:title>' +
            '</note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getNamespace() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: false, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:http://www.w3.org"));
    } catch (err) {
        const TAG = 'testXmlParseInfoGetNamespace0001';
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetNamespace0002() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note xmlns:h="http://www.w3.org">' +
            '<h:title>Happy</h:title>' +
            '</note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getNamespace() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:"));
    } catch (err) {
        const TAG = 'testXmlParseInfoGetNamespace0002';
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetPrefix0001() {
    const TAG = 'testXmlParseInfoGetPrefix0001';
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note xmlns:h="http://www.w3.org/TR/html4">' +
            '<h:title>Happy</h:title>' +
            '</note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getPrefix() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: false, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:h"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetText0001() {
    const TAG = 'testXmlParseInfoGetText0001';
    try {
        let strXml = '<?xml version="1.0" encoding="utf-8"?><note>Happy</note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getText() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:4 value:Happy"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetText0002() {
    const TAG = 'testXmlParseInfoGetText0002';
    try {
        let strXml = '<?xml version="1.0" encoding="utf-8"?><note>Ha$ppy</note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getText() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:4 value:Ha$ppy"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlIsEmptyElementTag0001() {
    const TAG = 'testXmlIsEmptyElementTag0001';
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '<title/>' +
            '</note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.isEmptyElementTag() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:true"))
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlIsEmptyElementTag0002() {
    try {
        let strXml = '<?xml version="1.0" encoding="utf-8"?><note><to>Work</to></note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.isEmptyElementTag() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:4 value:false"))
    } catch (err) {
        const TAG = 'testXmlIsEmptyElementTag0002';
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlIsEmptyElementTag0003() {
    const TAG = 'testXmlIsEmptyElementTag0003';
    try {
            let strXml = '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <to do>Work</to do>' +
            '    <todo>Play</todo>' +
            '</note>';

            let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
            let str = "";
            const func = (key: xml.EventType, value: xml.ParseInfo) => {
                str += 'key:' + key + ' value:' + value.isEmptyElementTag() + ' ';
                return true;
            }
            let options: xml.ParseOptions = {
                supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
            }
            parser.parseXml(options);
            arktest.assertTrue(false)
    } catch (err) {
        arktest.assertTrue(err instanceof BusinessError)
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertEQ((err as BusinessError).code, 401)
    }
}

function testXmlIsWhitespace0001() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '<title> </title>' +
            '</note>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.isWhitespace() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:true"));
    } catch (err) {
        const TAG = 'testXmlIsWhitespace0001';
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlGetAttributeCount0001() {
    try {
        let strXml = '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high"/>';
        
        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getAttributeCount() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:1"));
    } catch (err) {
        const TAG = 'testXmlGetAttributeCount0001';
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlGetAttributeCount0002() {
    const TAG = 'testXmlGetAttributeCount0002';
    try {
        let strXml = '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true"/>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(strXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getAttributeCount() + " ";
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:2"))
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlGetAttributeCount0003() {
    const TAG = 'testXmlGetAttributeCount0003';
    try {
        let strXml = '<note importance=high logged="true"/>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml));
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getAttributeCount() + ' ';
            return true;
        }
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: func
        }
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
            arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}
