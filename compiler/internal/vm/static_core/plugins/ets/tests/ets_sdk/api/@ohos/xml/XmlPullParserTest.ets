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
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests");

    suite.addTest("Test create XmlPullParser with empty buffer & default Encoding", testXmlPullParserDefault)
    suite.addTest("Test XmlPullParser with TagValueCallback with supportDocType", testWithTagValueCb)
    suite.addTest("Test XmlPullParser with TagValueCallback with ignoreNamespace", testWithTagValueCbWithIgnoreNS)
    suite.addTest("Test XmlPullParser with TagValueCallback with supportDocType and ignoreNamespace", testWithTagValueCbWithSupportDocTypeAndIgnoreNS)
    suite.addTest("Test XmlPullParser with TokenValueCallback with supportDocType and ignoreNamespace", testWithTokenValueCb)
    suite.addTest("Test XmlPullParser replace xml special chars", testReplaceXmlSpecChars)
    suite.addTest("Test XmlPullParser CDATA section", testCDATA)
    suite.addTest("Test XmlPullParser isEmptyElementTag", testIsEmptyElementTag)
    suite.addTest("Test XmlPullParser getText", testGetText)
    suite.addTest("Test XmlPullParser getPrefix", testGetPrefix)
    suite.addTest("Test XmlPullParser getNamespace", testGetNamespace)
    suite.addTest("Test XmlPullParser name:value tag pair", testTagValue)
    suite.addTest("Testing XML isWhitespace with valid whitespace.", testXmlIsWhitespace0001)
    suite.addTest("Testing XmlPullParser startElement", testStartElement)

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
    return str.replace(new RegExp("[\r\n]","g"), "")
}

function testWithTagValueCb() {
    const str : ReadonlyArray<string> = Array.of<string>(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
        "<note importance=\"high\" logged=\"true\">",
        "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1\r\n}\r\n]]>",
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

    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: false, tagValueCallbackFunction: cb}

    parser.parseXml(options)

    const res = '{  :  }; {  :  note [<!ENTITY foo "baa">] }; { note :  }; ' +
        '{  : function matchwo(a,6){return 1} }; {  : Hello, World! }; {  :   }; ' +
        '{ company :  }; {  : John & Hans }; { company :  }; {  :   }; { title :  }; ' +
        '{  : Happy }; { title :  }; { title :  }; {  : Happy }; { title :  }; ' +
        '{  :   }; { todo :  }; {  : Work }; { todo :  }; {  :   }; { todo :  }; ' +
        '{  : Play }; { todo :  }; {  :   }; {  : go there }; { a :  }; { b :  }; ' +
        '{ b :  }; { a :  }; {  :   }; { table :  }; {  :   }; { tr :  }; { td :  }; ' +
        '{  : Apples }; { td :  }; {  :   }; { td :  }; {  : Bananas }; { td :  }; ' +
        '{  :   }; { tr :  }; { table :  }; { note :  }; {  :  }; ';
    arktest.assertEQ(processStr(g_testStr), processStr(res))
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

    const res = '{  :  }; { note :  }; {  : function matchwo(a,6){return 1;} }; ' +
        '{  : Hello, World! }; {  :   }; { company :  }; {  : John & Hans }; ' +
        '{ company :  }; {  :   }; { title :  }; {  : Happy }; { title :  }; ' +
        '{ title :  }; {  : Happy }; { title :  }; {  :   }; { todo :  }; {  : Work }; ' +
        '{ todo :  }; {  :   }; { todo :  }; {  : Play }; { todo :  }; {  :   }; ' +
        '{  : go there }; { a :  }; { b :  }; { b :  }; { a :  }; {  :   }; ' +
        '{ h:table :  }; {  :   }; { h:tr :  }; { h:td :  }; {  : Apples }; ' +
        '{ h:td :  }; {  :   }; { h:td :  }; {  : Bananas }; { h:td :  }; {  :   }; ' +
        '{ h:tr :  }; { h:table :  }; { note :  }; {  :  }; ';
    arktest.assertEQ(processStr(g_testStr), processStr(res))
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

    const res = '{  :  }; {  :  note [<!ENTITY foo "baa">] }; { note :  }; ' +
        '{  : function matchwo(a,6){return 1;} }; {  : Hello, World! }; {  :   }; ' +
        '{ company :  }; {  : John & Hans }; { company :  }; {  :   }; { title :  }; ' +
        '{  : Happy }; { title :  }; { title :  }; {  : Happy }; { title :  }; ' +
        '{  :   }; { todo :  }; {  : Work }; { todo :  }; {  :   }; { todo :  }; ' +
        '{  : Play }; { todo :  }; {  :   }; {  : go there }; { a :  }; { b :  }; ' +
        '{ b :  }; { a :  }; {  :   }; { h:table :  }; {  :   }; { h:tr :  }; ' +
        '{ h:td :  }; {  : Apples }; { h:td :  }; {  :   }; { h:td :  }; {  : Bananas }; ' +
        '{ h:td :  }; {  :   }; { h:tr :  }; { h:table :  }; { note :  }; {  :  }; ';

    arktest.assertEQ(processStr(g_testStr), processStr(res))
}

function eventName(type: xml.EventType): string {
    switch (type) {
        case xml.EventType.START_DOCUMENT : return "START_DOCUMENT"
        case xml.EventType.END_DOCUMENT: return "END_DOCUMENT"
        case xml.EventType.START_TAG : return "START_TAG"
        case xml.EventType.END_TAG: return "END_TAG"
        case xml.EventType.TEXT: return "TEXT"
        case xml.EventType.CDSECT: return "CDSECT"
        case xml.EventType.COMMENT: return "COMMENT"
        case xml.EventType.DOCDECL: return "DOCDECL"
        case xml.EventType.INSTRUCTION: return "INSTRUCTION"
        case xml.EventType.ENTITY_REFERENCE: return "ENTITY_REFERENCE"
        case xml.EventType.WHITESPACE: return "WHITESPACE"
        default: return "default"
    }
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

function testWithTokenValueCb() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance=\"high\" logged=\"true\">",
    "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "<!--Hello, World!--> <company>John &amp; Hans</company> <title>Happy</title>",
    "</note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
            g_testStr += "{ " + eventName(eventType) + " : " + parseInfo2Str(value) + " }; "
        return true
    }

    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: cb}

    parser.parseXml(options)

    const expected = '{ START_DOCUMENT : { depth : 0}; { columnNumber : 1};' +
        ' { lineNumber : 1}; { attributeCount : 0}; { isEmptyElementTag : false}; ' +
        '{ isWhitespace : true}; { namespace : }; { name : }; { prefix : }; { text : } }; ' +
        '{ START_TAG : { depth : 1}; { columnNumber : 77}; { lineNumber : 1}; ' +
        '{ attributeCount : 2}; { isEmptyElementTag : false}; { isWhitespace : false}; ' +
        '{ namespace : }; { name : note}; { prefix : }; { text : } }; { CDSECT : { depth : 1}; ' +
        '{ columnNumber : 4}; { lineNumber : 6}; { attributeCount : 0}; { isEmptyElementTag : false}; ' +
        '{ isWhitespace : true}; { namespace : }; { name : }; { prefix : }; ' +
        '{ text : function matchwo(a,6){return 1;}} }; { COMMENT : { depth : 1}; ' +
        '{ columnNumber : 24}; { lineNumber : 6}; { attributeCount : 0}; { isEmptyElementTag : false}; ' +
        '{ isWhitespace : true}; { namespace : }; { name : }; { prefix : }; { text : Hello, World!} }; ' +
        '{ WHITESPACE : { depth : 1}; { columnNumber : 25}; { lineNumber : 6}; { attributeCount : 0}; ' +
        '{ isEmptyElementTag : false}; { isWhitespace : true}; { namespace : }; { name : }; { prefix : }; ' +
        '{ text :  } }; { START_TAG : { depth : 2}; { columnNumber : 34}; { lineNumber : 6}; ' +
        '{ attributeCount : 0}; { isEmptyElementTag : false}; { isWhitespace : true}; { namespace : }; ' +
        '{ name : company}; { prefix : }; { text : } }; { TEXT : { depth : 2}; { columnNumber : 49}; ' +
        '{ lineNumber : 6}; { attributeCount : 0}; { isEmptyElementTag : false}; { isWhitespace : false}; ' +
        '{ namespace : }; { name : }; { prefix : }; { text : John & Hans} }; { END_TAG : { depth : 2}; ' +
        '{ columnNumber : 59}; { lineNumber : 6}; { attributeCount : 0}; { isEmptyElementTag : false}; ' +
        '{ isWhitespace : true}; { namespace : }; { name : company}; { prefix : }; { text : } }; ' +
        '{ WHITESPACE : { depth : 1}; { columnNumber : 60}; { lineNumber : 6}; { attributeCount : 0}; ' +
        '{ isEmptyElementTag : false}; { isWhitespace : true}; { namespace : }; { name : }; { prefix : }; ' +
        '{ text :  } }; { START_TAG : { depth : 2}; { columnNumber : 67}; { lineNumber : 6}; ' +
        '{ attributeCount : 0}; { isEmptyElementTag : false}; { isWhitespace : true}; { namespace : }; ' +
        '{ name : title}; { prefix : }; { text : } }; { TEXT : { depth : 2}; { columnNumber : 72}; ' +
        '{ lineNumber : 6}; { attributeCount : 0}; { isEmptyElementTag : false}; { isWhitespace : false}; ' +
        '{ namespace : }; { name : }; { prefix : }; { text : Happy} }; { END_TAG : { depth : 2}; ' +
        '{ columnNumber : 80}; { lineNumber : 6}; { attributeCount : 0}; { isEmptyElementTag : false}; ' +
        '{ isWhitespace : true}; { namespace : }; { name : title}; { prefix : }; { text : } }; ' +
        '{ END_TAG : { depth : 1}; { columnNumber : 87}; { lineNumber : 6}; { attributeCount : 0}; ' +
        '{ isEmptyElementTag : false}; { isWhitespace : true}; { namespace : }; { name : note}; ' +
        '{ prefix : }; { text : } }; { END_DOCUMENT : { depth : 0}; { columnNumber : 87}; ' +
        '{ lineNumber : 11}; { attributeCount : 0}; { isEmptyElementTag : false}; ' +
        '{ isWhitespace : true}; { namespace : }; { name : }; { prefix : }; { text : } }; ';

    arktest.assertEQ(processStr(g_testStr), processStr(expected))
}

// testParseXml013
function testReplaceXmlSpecChars() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='hi&amp;gh' logged='&lt;true'>",
    "    <title>Hello&gt;World&apos;</title>",
    "    <todo>Wo&quot;rk</todo>",
    "</note>")

    let textStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if (eventType === xml.EventType.TEXT) {
            textStr += value.getText()
        }
        return true
    }

    let attStr = ''
    let attrCb = (name: string, value: string): boolean => { 
        attStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: tokenCb, attributeValueCallbackFunction: attrCb}
    parser.parseXml(options)

    let expectedTextStr = 'Hello>World\'Wo"rk'
    let expectedAttrStr = "{ importance : hi&gh }; { logged : <true }; "

    arktest.assertEQ(attStr, expectedAttrStr)
    arktest.assertEQ(textStr, expectedTextStr)
}

// testParse012
function testCDATA() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high' logged='true'>",
    "    <title>Hello\nWorld</title>",
    "    <todo>Work\n</todo>",
    "    <mess><![CDATA[This is a \n CDATA section]]></mess>",
    "</note>")

    let textStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if ((eventType === xml.EventType.TEXT) || (eventType === xml.EventType.CDSECT)) {
            textStr += value.getText() + " "
        }
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": true, "tokenValueCallbackFunction": tokenCb}
    parser.parseXml(options)

    let expectedTextStr = 'Hello\nWorld Work\n This is a \n CDATA section '

    arktest.assertEQ(textStr, expectedTextStr)
}

// testParseXml010
function testIsEmptyElementTag() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high'>",
    "<title/>",
    "<todo>Work</todo>",
    "<todo/>",
    "</note>")

    let textStr = ''
    let oldStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if ((eventType === xml.EventType.START_TAG)) {
            textStr += "{ " + value.getName() + " : " + value.isEmptyElementTag() + " }; "
            oldStr += "key:" + eventType + " value:" + value.isEmptyElementTag() + " "
        }
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": true, "tokenValueCallbackFunction": tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{ note : false }; { title : true }; { todo : false }; { todo : true }; '

    arktest.assertEQ(textStr, expectedTextStr)

    const expectedOld = 'key:2 value:false key:2 value:true key:2 value:false key:2 value:true '
    arktest.assertEQ(expectedOld, oldStr)
}

// testParseXml009
function testGetText() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high'>",
    "<title>Happy</title>",
    "<todo>Work</todo>",
    "<todo>Play</todo>",
    "</note>")

    let textStr = ''
    let oldStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if ((eventType === xml.EventType.TEXT)) {
            textStr += "{ " + value.getName() + " : " + value.getText() + " }; "
            oldStr += "key:" + eventType + " value:" + value.getText() + " "
        }
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": true, "tokenValueCallbackFunction": tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{  : Happy }; {  : Work }; {  : Play }; '

    arktest.assertEQ(textStr, expectedTextStr)

    let expectedOld = "key:4 value:Happy key:4 value:Work key:4 value:Play "
    arktest.assertEQ(oldStr, expectedOld)
}

// testParseXml008
function testGetPrefix() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high'>",
    "<h:table xmlns:h='html4'>",
    "<h:tr><h:td>Bananas</h:td> </h:tr>",
    "</h:table>",
    "</note>")

    let textStr = ''
    let oldStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if ((eventType === xml.EventType.START_TAG) || (eventType === xml.EventType.END_TAG)) {
            textStr += "{ " + value.getName() + " : " + value.getPrefix() + " }; "
            oldStr += "key:" + eventType + " value:" + value.getPrefix() + " "
        }
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": false, "tokenValueCallbackFunction": tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{ note :  }; { table : h }; { tr : h }; { td : h }; { td : h }; { tr : h }; { table : h }; { note :  }; '

    arktest.assertEQ(textStr, expectedTextStr)

    let oldExpected = "key:2 value: key:2 value:h key:2 value:h key:2 value:h " +
    "key:3 value:h key:3 value:h key:3 value:h key:3 value: "
    arktest.assertEQ(oldExpected, oldStr)
}

// testParseXml007
function testGetNamespace() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high'>",
    "<h:table xmlns:h='html4'>",
    "<h:tr><h:td>Bananas</h:td> </h:tr>",
    "</h:table>",
    "</note>")

    let textStr = ''
    let oldStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if ((eventType === xml.EventType.START_TAG) || (eventType === xml.EventType.END_TAG)) {
            textStr += "{ " + value.getName() + " : " + value.getNamespace() + " }; "
            oldStr += "key:" + eventType + " value:" + value.getNamespace() + " "
        }
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": false, "tokenValueCallbackFunction": tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{ note :  }; { table : html4 }; { tr : html4 }; { td : html4 }; { td : html4 }; { tr : html4 }; { table : html4 }; { note :  }; '

    arktest.assertEQ(textStr, expectedTextStr)

    let oldExpected = "key:2 value: key:2 value:html4 key:2 value:html4 key:2 value:html4 " +
    "key:3 value:html4 key:3 value:html4 key:3 value:html4 key:3 value: "
    arktest.assertEQ(oldStr, oldExpected)
}

// testParseXml002
function testTagValue() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='heigh' logged='true'>",
    "    <![CDATA[function \n matchwo(a,6)]]>",
    "    <!--Hello, World!-->",
    "    <company>John &amp; Hans</company>",
    "    <title>Happy</title>",
    "    <lens>Play</lens>",
    "    <?go there?>",
    "</note>")

    let textStr = ''
    let oldTextStr = ''
    let tagCb = (name: string, value: string) => { 
        textStr += "{ " + name + " : " + value + " }; "
        oldTextStr += name + value
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tagValueCallbackFunction: tagCb}
    parser.parseXml(options)

    let expectedTextStr = '{  :  }; { note :  }; {  :      }; {  : function \n matchwo(a,6) }; ' +
        '{  :      }; {  : Hello, World! }; {  :      }; { company :  }; {  : John & Hans }; ' +
        '{ company :  }; {  :      }; { title :  }; {  : Happy }; { title :  }; {  :      }; ' +
        '{ lens :  }; {  : Play }; { lens :  }; {  :      }; {  : go there }; { note :  }; {  :  }; ';
    arktest.assertEQ(textStr, expectedTextStr)

    let oldExpectedStr = 'note    function \n matchwo(a,6)    Hello, World!    companyJohn & Hanscompany\
    titleHappytitle    lensPlaylens    go therenote'
    arktest.assertEQ(oldTextStr, oldExpectedStr)
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
            arktest.assertTrue(str.includes("key:2 value:true"))
        } catch (err) {
            const TAG = 'testXmlIsWhitespace0001';
            console.error(`${TAG} failed, error: ${err}`);
            arktest.assertTrue(false)
        }
    }

function testStartElement() {
    let thatSer = new xml.XmlDynamicSerializer();
    let max = 100000;
    let value = 10200062;
    let result = 0;
    for (let i = 0; i < max / 25; ++i) {
        thatSer.setNamespace("h", "aaa");
        thatSer.startElement("note");
        thatSer.endElement();
    }
    arktest.expectError(() => {
	    thatSer.startElement("note1");
	}, 'The length has exceeded the upper limit');
}

