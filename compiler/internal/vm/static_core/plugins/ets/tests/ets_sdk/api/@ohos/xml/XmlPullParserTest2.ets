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

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part2");
    suite.addTest("Testing XML PullParser with attributeValueCallbackFunction as function throw error.", testXmlPullParserParseOptions0010)
    suite.addTest("Testing XML PullParser with tokenValueCallbackFunction as undefined.", testXmlPullParserParseOptions0009)
    suite.addTest("Testing XML PullParser with tokenValueCallbackFunction as function.", testXmlPullParserParseOptions0008)
    suite.addTest("Testing XML PullParser with tagValueCallbackFunction as function.", testXmlPullParserParseOptions0006)
    suite.addTest("Testing XML PullParser with tagValueCallbackFunction as undefined.", testXmlPullParserParseOptions0005)
    suite.addTest("Testing XML PullParser with ignoreNameSpace set to false.", testXmlPullParserParseOptions0004)
    suite.addTest("Testing XML PullParser with ignoreNameSpace set to true.", testXmlPullParserParseOptions0003)
    suite.addTest("Testing the XML PullParser with supportDoctype set to false.", testXmlPullParserParseOptions0002)
    suite.addTest("Testing the XML PullParser with supportDoctype set to true.", testXmlPullParserParseOptions0001)

    return suite.run()
}

function testXmlPullParserDefault(){
    let buffer = new ArrayBuffer(0)
    let parser = new xml.XmlPullParser(buffer)
    arktest.assertNE(parser, null)
    parser.parseXml({} as xml.ParseOptions)
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

function processStr(str: string): string {
    return str.replace(new RegExp("[\r\n\s]","g"), "")
}

function testWithTagValueCb() {
    const str : ReadonlyArray<string> = Array.of<string>(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
        "<note importance=\"high\" logged=\"true\">",
        "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1\r\n}\r\n]]>",
        "<!--Hello, World!--> <company>John &amp Hans</company> <title>Happy</title>",
        "<title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?>",
        "<a><b/></a> <h:table xmlns:h=\"http://www.w3.org/TR/html4/\"> <h:tr>",
        "<h:td>Apples</h:td> <h:td>Bananas</h:td> </h:tr>",
        "</h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")
    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: false, tagValueCallbackFunction: cb}

    parser.parseXml(options)

    const res : ReadonlyArray<string> = Array.of<string>(
        "{  : <?xml verion=\"1.0\" encoding=\"utf-8\"?> }",
        "{  : <?go there?> }",
        "{  : <!--Hello, World!--> }",
        "{  : <!ENTITY foo \"baa\"> }",
        "{  : <!DOCTYPE note [<!ENTITY foo \"baa\">]> }",
        "{  : <![CDATA[function matchwo(a,6){return 1}]]> }",
        "{ company : John &amp Han }",
        "{ title : Happy }",
        "{ title : Happy }",
        "{ todo : Work }",
        "{ todo : Play }",
        "{ b :  }",
        "{ a : <b/> }",
        "{ td : Apple }",
        "{ td : Banana }",
        "{ tr : <h:td>Apple</h:td> <h:td>Banana</h:td>  }",
        "{ table :  <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr> }",
        "{ note : <![CDATA[function matchwo(a,6){return 1}]]><!--Hello, World!--> <company>John &amp Han</company> \
<title>Happy</title><title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?><a><b/></a> \
<h:table xmln:h=\"http://www.w3.org/TR/html4/\"> <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr></h:table> }",
        "{  :  }"
)
    let result = res.join("; ") + "; "
    arktest.assertEQ(processStr(g_testStr), processStr(result))
}

function testWithTagValueCbWithIgnoreNS() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "<!--Hello, World!--> <company>John &amp; Hans</company> <title>Happy</title>",
    "<title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?>",
    "<a><b/></a> <h:table xmlns:h=\"http://www.w3.org/TR/html4/\"> <h:tr>",
    "<h:td>Apples</h:td> <h:td>Bananas</h:td> </h:tr>",
    "</h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")
    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {supportDoctype: false, ignoreNameSpace: true, tagValueCallbackFunction: cb}

    parser.parseXml(options)

    const res : ReadonlyArray<string> = Array.of<string>(
        "{  : <?xml verion=\"1.0\" encoding=\"utf-8\"?> }",
        "{  : <?go there?> }",
        "{  : <!--Hello, World!--> }",
        "{  : <!ENTITY foo \"baa\"> }",
        "{  : <![CDATA[function matchwo(a,6){return 1;}]]> }",
        "{ company : John &amp; Han }",
        "{ title : Happy }",
        "{ title : Happy }",
        "{ todo : Work }",
        "{ todo : Play }",
        "{ b :  }",
        "{ a : <b/> }",
        "{ h:td : Apple }",
        "{ h:td : Banana }",
        "{ h:tr : <h:td>Apple</h:td> <h:td>Banana</h:td>  }",
        "{ h:table :  <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr> }",
        "{ note : <![CDATA[function matchwo(a,6){return 1;}]]><!--Hello, World!--> <company>John &amp; Han</company> <title>Happy</title><title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?><a><b/></a> <h:table xmln:h=\"http://www.w3.org/TR/html4/\"> <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr></h:table> }",
        "{  :  }")
    let result = res.join("; ") + "; "
    arktest.assertEQ(processStr(g_testStr), processStr(result))
}

function testWithTagValueCbWithSupportDocTypeAndIgnoreNS() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "<!--Hello, World!--> <company>John &amp; Hans</company> <title>Happy</title>",
    "<title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?>",
    "<a><b/></a> <h:table xmlns:h=\"http://www.w3.org/TR/html4/\"> <h:tr>",
    "<h:td>Apples</h:td> <h:td>Bananas</h:td> </h:tr>",
    "</h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tagValueCallbackFunction: cb}

    parser.parseXml(options)

    const res : ReadonlyArray<string> = Array.of<string>(
        "{  : <?xml verion=\"1.0\" encoding=\"utf-8\"?> }",
        "{  : <?go there?> }",
        "{  : <!--Hello, World!--> }",
        "{  : <!ENTITY foo \"baa\"> }",
        "{  : <!DOCTYPE note [<!ENTITY foo \"baa\">]> }",
        "{  : <![CDATA[function matchwo(a,6){return 1;}]]> }",
        "{ company : John &amp; Hans }", 
        "{ title : Happy }",
        "{ title : Happy }",
        "{ todo : Work }",
        "{ todo : Play }",
        "{ b :  }",
        "{ a : <b/> }",
        "{ h:td : Apple }",
        "{ h:td : Banana }",
        "{ h:tr : <h:td>Apple</h:td> <h:td>Banana</h:td>  }",
        "{ h:table :  <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr> }",
        "{ note : <![CDATA[function matchwo(a,6){return 1;}]]><!--Hello, World!--> <company>John &amp; Han</company> \
<title>Happy</title><title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?><a><b/></a> \
<h:table xmln:h=\"http://www.w3.org/TR/html4/\"> <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr></h:table> }",
        "{  :  }")
    let result = res.join("; ") + "; "
    arktest.assertEQ(processStr(g_testStr), processStr(result))
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

    const expected = "{ xmlns:h : http://www.w3.org/TR/html4/ }; { importance : high }; { logged : true }; "
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

    const expected = "{ xmlns:h : http://www.w3.org/TR/html4/ }; { importance : high }; { logged : true }; "
    arktest.assertEQ(processStr(g_testStr), processStr(expected));
}

function parseInfo2Str(info: xml.ParseInfo): string {
    let sb = new StringBuilder("")
    sb.append("{ depth : " + info.getDepth() + "}; ")
    sb.append("{ columnNumber : " + info.getColumnNumber()  + "}; ")
    sb.append("{ lineNumber : " + info.getLineNumber()  + "}; ")
    sb.append("{ attributeCount : " + info.getAttributeCount()  + "}; ")
    sb.append("{ isEmptyElementTag : " + info.isEmptyElementTag()  + "}; ")
    sb.append("{ isWhitespace : " + info.isWhitespace()  + "}; ")
    sb.append("{ namespace : " + info.getNamespace()  + "}; ")
    sb.append("{ name : " + info.getName()  + "}; ")
    sb.append("{ prefix : " + info.getPrefix()  + "}; ")
    sb.append("{ text : " + info.getText()  + "}")
    return sb.toString()
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

function testXmlPullParserParseOptions0001() {
    const TAG = 'testXmlPullParserParseOptions0001';
    try {
        let testXml =
          '<?xml version="1.0" encoding="utf-8"?>' +
            '<!DOCTYPE note>' +
            '<note>' +
            '    <title>Happy</title>' +
            '</note>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
          supportDoctype: true,
          ignoreNameSpace: true,
          tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0002() {
    const TAG = 'testXmlPullParserParseOptions0002';
    try {
        let testXml =
          '<?xml version="1.0" encoding="utf-8"?>' +
            '<note>' +
            '    <title>Happy</title>' +
            '</note>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
          supportDoctype: false,
          ignoreNameSpace: false,
          tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0003() {
    const TAG = 'testXmlPullParserParseOptions0003';
    try {
        let testXml =
          '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<h:root xmlns:h="http://www.w3.org/TR/html4/">' +
            '   <h:b/>' +
            '</h:root>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
          supportDoctype: false,
          ignoreNameSpace: true,
          tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0004() {
    const TAG = 'testXmlPullParserParseOptions0004';
    try {
        let testXml =
          '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<h:root xmlns:h="http://www.w3.org/TR/html4/">' +
            '   <h:b/>' +
            '</h:root>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
          supportDoctype: true,
          ignoreNameSpace: false,
          tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0005() {
    const TAG = 'testXmlPullParserParseOptions0005';
    try {
        let testXml =
          '<?xml version="1.0" encoding="utf-8" ?>' +
            '<note>' +
            '    <title>Happy</title>' +
            '</note>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
          supportDoctype: true,
          ignoreNameSpace: false,
          tagValueCallbackFunction: undefined
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0006() {
    const TAG = 'testXmlPullParserParseOptions0006';
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
          tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0008() {
    const TAG = 'testXmlPullParserParseOptions0008';
    try {
        let testXml =
          '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Happy</title>' +
            '    <todo>Work</todo>' +
            '    <todo>Play</todo>' +
            '</note>';
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
          supportDoctype: true,
          ignoreNameSpace: false,
          tagValueCallbackFunction: (name: string, value: string) => true,
          tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo) => true,
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0009() {
    const TAG = 'testXmlPullParserParseOptions0009';
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
          tokenValueCallbackFunction: undefined,
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserParseOptions0010() {
    const str : ReadonlyArray<string> = Array.of<string>(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
        "<note importance='hi&amp;gh' logged='&lt;true'>",
        " <title>Hello&gt;World&apos;</title>",
        " <todo>Wo&quot;rk</todo>",
        "</note>");
    
    let textStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if (eventType === xml.EventType.TEXT) {
            textStr += value.getText()
        }
        return true
    }

    let attrCb = (name: string, value: string): boolean => {
        throw new Error("123")
        return true 
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": true,  "tokenValueCallbackFunction":tokenCb, "attributeValueCallbackFunction": attrCb}
    arktest.expectError(() => {
		parser.parseXml(options)
	}, '123');
}

