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
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part7");

    suite.addTest("Testing XML PullParser with attributeValueCallbackFunction as function.", testXmlPullParserParseOptions0007)
    suite.addTest("Testing XML PullParser with attributeValueCallbackFunction as undefined.", testXmlPullParserParseOptions0010)
    suite.addTest("Test XmlPullParser with AttributeValueCallback", testWithAttributeValueCb)
    suite.addTest("Test XmlPullParser with AttributeValueCallback with supportDocType and ignoreNamespace", testWithAttributeValueCbWithSupportDocTypeAndIgnoreNS)
    suite.addTest("Test XmlPullParser getAttribute", testGetAttributeValue)
    suite.addTest("Test XmlPullParser getAttributeCount", testGetAttributeCount)
    suite.addTest("Testing XML getAttributeCount with illegal attribute.",testXmlGetAttributeCount0003)
    suite.addTest("Testing XML getAttributeCount with multiple valid attributes.", testXmlGetAttributeCount0002)
    suite.addTest("Testing XML getAttributeCount with one valid attribute.", testXmlGetAttributeCount0001)
    
    return suite.run()
}

const testXml =
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

function processStr(str: string): string {
    return str.replace(new RegExp("[\r\n\s]","g"), "")
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

function testWithAttributeValueCb() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "    <![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "    <!--Hello, World!-->    <company>John &amp; Hans</company>    <title>Happy</title>",
    "    <title>Happy</title>    <todo>Work</todo>    <todo>Play</todo>    <?go there?>",
    "    <a><b/></a>    <h:table xmlns:h=\"http://www.w3.org/TR/html4/\">        <h:tr>",
    "            <h:td>Apples</h:td>            <h:td>Bananas</h:td>        </h:tr>",
    "    </h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {supportDoctype: false, ignoreNameSpace: true, attributeValueCallbackFunction: cb}

    parser.parseXml(options)

    const expected = "{ importance : high }; { logged : true }; { xmln:h : http://www.w3.org/TR/html4/ }; "
    arktest.assertEQ(processStr(g_testStr), processStr(expected))
}

function testWithAttributeValueCbWithSupportDocTypeAndIgnoreNS() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "    <![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "    <!--Hello, World!-->    <company>John &amp; Hans</company>    <title>Happy</title>",
    "    <title>Happy</title>    <todo>Work</todo>    <todo>Play</todo>    <?go there?>",
    "    <a><b/></a>    <h:table xmlns:h=\"http://www.w3.org/TR/html4/\">        <h:tr>",
    "            <h:td>Apples</h:td>            <h:td>Bananas</h:td>        </h:tr>",
    "    </h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, attributeValueCallbackFunction: cb}

    parser.parseXml(options)

    const expected = "{ importance : high }; { logged : true }; { xmln:h : http://www.w3.org/TR/html4/ }; "
    arktest.assertEQ(processStr(g_testStr), processStr(expected));
}

// testParseXml011
function testGetAttributeCount() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high' logged='true'>",
    "    <title>Happy</title>",
    "    <todo>Work</todo>",
    "    <todo>Play</todo>",
    "</note>")

    let textStr = ''
    let oldStr = ""
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if (eventType === xml.EventType.START_TAG) {
            textStr += "{ " + value.getName() + " : " + value.getAttributeCount() + " }; "
            oldStr += "key:" + eventType + " value:" + value.getAttributeCount() + " "
        }
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{ note : 2 }; { title : 0 }; { todo : 0 }; { todo : 0 }; '

    arktest.assertEQ(textStr, expectedTextStr)

    let expectedOld = "key:2 value:2 key:2 value:0 key:2 value:0 key:2 value:0 "
    arktest.assertEQ(oldStr, expectedOld)
}

//testParseXml001
function testGetAttributeValue() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='heigh' logged='true'>",
    "    <!--Hello, World!-->",
    "    <company>John &amp; Hans</company>",
    "    <lens>Work</lens>",
    "    <h:table xmlns:h=\"http://www.w3.org/TR/html4/\">",
    "         <h:tr>",
    "               <h:td>Apples</h:td>",
    "               </h:tr>",
    "    </h:table>",
    "</note>")

    let textStr = ''
    let attrCb = (name: string, value: string) => { 
        textStr += "{ " + name + " : " + value + " }; "
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, attributeValueCallbackFunction: attrCb}
    parser.parseXml(options)

    let expectedTextStr = '{ importance : heigh }; { logged : true }; { xmlns:h : http://www.w3.org/TR/html4/ }; '

    arktest.assertEQ(textStr, expectedTextStr)
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

function testXmlPullParserParseOptions0007() {
    const TAG = 'testXmlPullParserParseOptions0007';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high">' +
            '    <title>Happy</title>' +
            '</note>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            attributeValueCallbackFunction: (name: string, value: string) => true,
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0010() {
    const TAG = 'testXmlPullParserParseOptions0010';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high">' +
            '    <title>Happy</title>' +
            '</note>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            attributeValueCallbackFunction: undefined,
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

