/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
import { uri } from "@ohos.uri";

let uriObj: uri.URI;

function SetSchemeTest() {
    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "https:"
    arktest.assertEQ(uriObj.toString(), 'https://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "3https:"
    arktest.assertEQ(uriObj.toString(), 'ht2tp://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "asffdsa"
    arktest.assertEQ(uriObj.toString(), 'asffdsa://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "%asffdsa"
    arktest.assertEQ(uriObj.toString(), 'ht2tp://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = ""
    arktest.assertEQ(uriObj.toString(), 'http://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://user@www.1hw.1com:77?query#qwer");
    uriObj.scheme = "testuri006";
    arktest.assertEQ(uriObj.scheme, 'testuri006');
}

function SetUserInfoTest() {
    uriObj = new uri.URI("http://user@www.hw.com:77/path/path23?query#qwer");
    uriObj.userInfo = "testuri007"
    arktest.assertEQ(uriObj.userInfo, 'testuri007');
    arktest.assertEQ(uriObj.toString(), 'http://testuri007@www.hw.com:77/path/path23?query#qwer');
    // CC-OFFNXT(G.OTH.05) test logic
    uriObj = new uri.URI("http://www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = "testuri008"
    arktest.assertEQ(uriObj.userInfo, 'testuri008');
    arktest.assertEQ(uriObj.toString(), 'http://testuri008@www.test.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = "sji<+s大"
    arktest.assertEQ(uriObj.userInfo, 'sji<+s大');
    arktest.assertEQ(uriObj.toString(), 'http://sji%3C%2Bs%E5%A4%A7@www.test.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = "sji<+s大"
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = ""
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.toString(), 'http://www.test.com:77/path/path23?query#qwer');  // CC-OFF(G.OTH.05) test logic
}

function SetPathTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.path = ""
    arktest.assertEQ(uriObj.path, null);
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.path = "pathtest/pp"
    arktest.assertEQ(uriObj.path, 'pathtest/pp');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/pp?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.path = "pathtest/p大p"
    arktest.assertEQ(uriObj.path, 'pathtest/p大p');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/p%E5%A4%A7p?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77?query#qwer");
    uriObj.path = "/testuri015/p大p/"
    arktest.assertEQ(uriObj.path, '/testuri015/p大p/');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri015/p%E5%A4%A7p/?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77?query#qwer");
    uriObj.path = "/testuri016/p大p/"
    arktest.assertEQ(uriObj.ssp, '//userinfo@www.test.com:77/testuri016/p大p/?query');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri016/p%E5%A4%A7p/?query#qwer');

    uriObj = new uri.URI("http:userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.path = "/testuri017/p大p/"
    arktest.assertEQ(uriObj.path, '/testuri017/p大p/');
    arktest.assertEQ(uriObj.toString(), 'http:/testuri017/p%E5%A4%A7p/#qwer');
}

function SetQueryTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = "/testuri018/p大p/=sasf"
    arktest.assertEQ(uriObj.query, '/testuri018/p大p/=sasf');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?%2Ftesturi018%2Fp%E5%A4%A7p%2F%3Dsasf#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = "key=value&key1=value1"
    arktest.assertEQ(uriObj.query, 'key=value&key1=value1');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?key%3Dvalue%26key1%3Dvalue1#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = '';
    arktest.assertEQ(uriObj.query, null);
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = 'kkey=sal';
    arktest.assertEQ(uriObj.query, "kkey=sal");
    arktest.assertEQ(uriObj.ssp, "//userinfo@www.test.com:77/testuri017/p大p/?kkey=sal");
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey%3Dsal#qwer');

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = 'kkey=sal&jj=oi';
    arktest.assertEQ(uriObj.query, "kkey=sal&jj=oi");
    arktest.assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?kkey=sal&jj=oi");
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey%3Dsal%26jj%3Doi#qwer');

}

function SetFragmentTest() {
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.fragment = 'kkey=sal&jj=oi';
    arktest.assertEQ(uriObj.fragment, "kkey=sal&jj=oi");
    arktest.assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?query");
    let res = 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#kkey%3Dsal%26jj%3Doi';
    arktest.assertEQ(uriObj.toString(), res);

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.fragment = '';
    arktest.assertEQ(uriObj.fragment, null);
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query');

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.fragment = '#jia片段';
    arktest.assertEQ(uriObj.fragment, "#jia片段");
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#%23jia%E7%89%87%E6%AE%B5');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.fragment = '#jia片段f';
    arktest.assertEQ(uriObj.fragment, "#jia片段f");
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#%23jia%E7%89%87%E6%AE%B5f');

    uriObj = new uri.URI("http:userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.fragment = 'jia片段f';
    arktest.assertEQ(uriObj.fragment, "jia片段f");
    arktest.assertEQ(uriObj.toString(), 'http:userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#jia%E7%89%87%E6%AE%B5f');

}

function SetAuthorityTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = 'www.test11.cn:88';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, "www.test11.cn");
    arktest.assertEQ(uriObj.port, "88");
    arktest.assertEQ(uriObj.toString(), 'http://www.test11.cn:88/testuri017/p%E5%A4%A7p/?query#asd');  // CC-OFF(G.OTH.05) test logic

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = 'uusc@www.testuc.com:741';
    arktest.assertEQ(uriObj.userInfo, "uusc");
    arktest.assertEQ(uriObj.host, "www.testuc.com");
    arktest.assertEQ(uriObj.port, "741");
    arktest.assertEQ(uriObj.authority, "uusc@www.testuc.com:741");
    arktest.assertEQ(uriObj.toString(), 'http://uusc@www.testuc.com:741/testuri017/p%E5%A4%A7p/?query#asd');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = '';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.port, "-1");
    arktest.assertEQ(uriObj.authority, null);
    arktest.assertEQ(uriObj.ssp, '/testuri017/p大p/?query');
    arktest.assertEQ(uriObj.toString(), 'http:/testuri017/p%E5%A4%A7p/?query#asd');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = 'usc@';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.port, "-1");
    arktest.assertEQ(uriObj.authority, "usc@");
    arktest.assertEQ(uriObj.ssp, '//usc@/testuri017/p大p/?query');
    arktest.assertEQ(uriObj.toString(), 'http://usc@/testuri017/p%E5%A4%A7p/?query#asd');
}

function SetSspTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.ssp = 'usc@asjf:99/path1/ss?key=1';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.port, "-1");
    arktest.assertEQ(uriObj.authority, null);
    arktest.assertEQ(uriObj.path, null);
    arktest.assertEQ(uriObj.ssp, 'usc@asjf:99/path1/ss?key=1');
    arktest.assertEQ(uriObj.toString(), 'http:usc@asjf:99/path1/ss?key=1#asd');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.ssp = '/usc@asjf:99/path1/ss?key=1';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.path, "/usc@asjf:99/path1/ss");
    arktest.assertEQ(uriObj.authority, null);
    arktest.assertEQ(uriObj.ssp, '/usc@asjf:99/path1/ss?key=1');
    arktest.assertEQ(uriObj.toString(), 'http:/usc@asjf:99/path1/ss?key=1#asd');

    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.ssp = '//usc@www.test321.cn:996?key=1';
    arktest.assertEQ(uriObj.userInfo, "usc");
    arktest.assertEQ(uriObj.host, "www.test321.cn");
    arktest.assertEQ(uriObj.path, null);
    arktest.assertEQ(uriObj.authority, 'usc@www.test321.cn:996');
    arktest.assertEQ(uriObj.ssp, '//usc@www.test321.cn:996?key=1');
    arktest.assertEQ(uriObj.toString(), 'safdcas://usc@www.test321.cn:996?key=1#asd');

    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asdasf");
    uriObj.ssp = '//www.test321.cn';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, "www.test321.cn");
    arktest.assertEQ(uriObj.path, null);
    arktest.assertEQ(uriObj.authority, 'www.test321.cn');
    arktest.assertEQ(uriObj.ssp, '//www.test321.cn');
    arktest.assertEQ(uriObj.toString(), 'safdcas://www.test321.cn#asdasf');


}

function SetEncodedUserInfo() {
    uriObj = new uri.URI("http://user@www.hw.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = "testuri007"
    arktest.assertEQ(uriObj.encodedUserInfo, 'testuri007');
    arktest.assertEQ(uriObj.toString(), 'http://testuri007@www.hw.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://www.test.com:77/path/path23?query#qwer");  // CC-OFF(G.OTH.05) test logic
    uriObj.encodedUserInfo = "testuri008"
    arktest.assertEQ(uriObj.encodedUserInfo, 'testuri008');
    arktest.assertEQ(uriObj.toString(), 'http://testuri008@www.test.com:77/path/path23?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = "sji<+s大"
    arktest.assertEQ(uriObj.encodedUserInfo, 'userinfo');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/path/path23?query#qwer');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = "sji<+s大"
    arktest.assertEQ(uriObj.encodedUserInfo, null);
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/path/path23?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = ""
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.toString(), 'http://www.test.com:77/path/path23?query#qwer');  // CC-OFF(G.OTH.05) test logic
}

function SetEncodedPathTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedPath = ""
    arktest.assertEQ(uriObj.encodedPath, null);
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedPath = "pathtest/pp"
    arktest.assertEQ(uriObj.encodedPath, 'pathtest/pp');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/pp?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedPath = "pathtest/p%E5%A4%A7p"
    arktest.assertEQ(uriObj.encodedPath, 'pathtest/p%E5%A4%A7p');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/p%E5%A4%A7p?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77?query#qwer");
    uriObj.encodedPath = "/testuri015/p大p/"
    arktest.assertEQ(uriObj.encodedPath, '/testuri015/p大p/');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri015/p%E5%A4%A7p/?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77");
    uriObj.encodedPath = "testuri016/p%E5%A4%A7/"
    arktest.assertEQ(uriObj.encodedPath, 'testuri016/p%E5%A4%A7/');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri016/p%E5%A4%A7/');
}

function SetEncodedQuery() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = "/testuri018/p大p/=sasf"
    arktest.assertEQ(uriObj.encodedQuery, '/testuri018/p大p/=sasf');
    let res = 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?/testuri018/p%E5%A4%A7p/=sasf#qwer';
    arktest.assertEQ(uriObj.toString(), res);
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = "key=value&key1=value1"
    arktest.assertEQ(uriObj.encodedQuery, 'key=value&key1=value1');
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?key=value&key1=value1#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = '';
    arktest.assertEQ(uriObj.encodedQuery, null);
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = 'kkey=sal';
    arktest.assertEQ(uriObj.encodedQuery, "kkey=sal");
    arktest.assertEQ(uriObj.ssp, "//userinfo@www.test.com:77/testuri017/p大p/?kkey=sal");
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey=sal#qwer');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = 'kkey=sal&jj=oi';
    arktest.assertEQ(uriObj.encodedQuery, "kkey=sal&jj=oi");
    arktest.assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?kkey=sal&jj=oi");
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey=sal&jj=oi#qwer');
}

function SetEncodedFragment() {
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedFragment = 'kkey=sal&jj=oi';
    arktest.assertEQ(uriObj.encodedFragment, "kkey=sal&jj=oi");
    arktest.assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?query");
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#kkey=sal&jj=oi');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedFragment = '';
    arktest.assertEQ(uriObj.encodedFragment, null);
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedFragment = '#jia片段';
    arktest.assertEQ(uriObj.encodedFragment, "qwer");
    arktest.assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.encodedFragment = 'jia片段f';
    arktest.assertEQ(uriObj.encodedFragment, "jia片段f");
    arktest.assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#jia%E7%89%87%E6%AE%B5f');
    uriObj = new uri.URI("http:userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.encodedFragment = 'jia片段f';
    arktest.assertEQ(uriObj.encodedFragment, "jia片段f");
    arktest.assertEQ(uriObj.toString(), 'http:userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#jia%E7%89%87%E6%AE%B5f');
}

function SetEncodedAuthority() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = 'www.test11.cn:88';
    arktest.assertEQ(uriObj.encodedUserInfo, null);
    arktest.assertEQ(uriObj.host, "www.test11.cn");
    arktest.assertEQ(uriObj.port, "88");
    arktest.assertEQ(uriObj.toString(), 'http://www.test11.cn:88/testuri017/p%E5%A4%A7p/?query#asd');  // CC-OFF(G.OTH.05) test logic
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = 'uusc@www.testuc.com:741';
    arktest.assertEQ(uriObj.encodedUserInfo, "uusc");
    arktest.assertEQ(uriObj.host, "www.testuc.com");
    arktest.assertEQ(uriObj.port, "741");
    arktest.assertEQ(uriObj.encodedAuthority, "uusc@www.testuc.com:741");
    arktest.assertEQ(uriObj.toString(), 'http://uusc@www.testuc.com:741/testuri017/p%E5%A4%A7p/?query#asd');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = '';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.port, "-1");
    arktest.assertEQ(uriObj.encodedAuthority, null);
    arktest.assertEQ(uriObj.encodedSSP, '/testuri017/p大p/?query');
    arktest.assertEQ(uriObj.toString(), 'http:/testuri017/p%E5%A4%A7p/?query#asd');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = 'usc@';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.port, "-1");
    arktest.assertEQ(uriObj.encodedAuthority, "usc@");
    arktest.assertEQ(uriObj.encodedSSP, '//usc@/testuri017/p大p/?query');
    arktest.assertEQ(uriObj.toString(), 'http://usc@/testuri017/p%E5%A4%A7p/?query#asd');
}

function SetEncodedSsp() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedSSP = 'usc@asjf:99/path1/ss?key=1';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.port, "-1");
    arktest.assertEQ(uriObj.authority, null);
    arktest.assertEQ(uriObj.path, null);
    arktest.assertEQ(uriObj.encodedSSP, 'usc@asjf:99/path1/ss?key=1');
    arktest.assertEQ(uriObj.toString(), 'http:usc@asjf:99/path1/ss?key=1#asd');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedSSP = '/usc@asjf:99/path1/ss?key=1';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, null);
    arktest.assertEQ(uriObj.path, "/usc@asjf:99/path1/ss");
    arktest.assertEQ(uriObj.authority, null);
    arktest.assertEQ(uriObj.encodedSSP, '/usc@asjf:99/path1/ss?key=1');
    arktest.assertEQ(uriObj.toString(), 'http:/usc@asjf:99/path1/ss?key=1#asd');
    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedSSP = '//usc@www.test321.cn:996?key=1';
    arktest.assertEQ(uriObj.userInfo, "usc");
    arktest.assertEQ(uriObj.host, "www.test321.cn");
    arktest.assertEQ(uriObj.path, null);
    arktest.assertEQ(uriObj.encodedAuthority, 'usc@www.test321.cn:996');
    arktest.assertEQ(uriObj.encodedSSP, '//usc@www.test321.cn:996?key=1');
    arktest.assertEQ(uriObj.toString(), 'safdcas://usc@www.test321.cn:996?key=1#asd');
    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asdasf");
    uriObj.encodedSSP = '//www.test321.cn';
    arktest.assertEQ(uriObj.userInfo, null);
    arktest.assertEQ(uriObj.host, "www.test321.cn");
    arktest.assertEQ(uriObj.path, null);
    arktest.assertEQ(uriObj.authority, 'www.test321.cn');
    arktest.assertEQ(uriObj.encodedSSP, '//www.test321.cn');
    arktest.assertEQ(uriObj.toString(), 'safdcas://www.test321.cn#asdasf');
}

function main(): int {
    let suite = new arktest.ArkTestsuite("URI UT tests");

    suite.addTest("SetSchemeTest", SetSchemeTest);
    suite.addTest("SetUserInfoTest", SetUserInfoTest);
    suite.addTest("SetPathTest", SetPathTest);
    suite.addTest("SetQueryTest", SetQueryTest);
    suite.addTest("SetFragmentTest", SetFragmentTest);
    suite.addTest("SetAuthorityTest", SetAuthorityTest);
    suite.addTest("SetSspTest", SetSspTest);
    suite.addTest("SetEncodedUserInfo", SetEncodedUserInfo);
    suite.addTest("SetEncodedPathTest", SetEncodedPathTest);
    suite.addTest("SetEncodedQuery", SetEncodedQuery);
    suite.addTest("SetEncodedFragment", SetEncodedFragment);
    suite.addTest("SetEncodedAuthority", SetEncodedAuthority);
    suite.addTest("SetEncodedSsp", SetEncodedSsp);
    //passed
    return suite.run();
}
