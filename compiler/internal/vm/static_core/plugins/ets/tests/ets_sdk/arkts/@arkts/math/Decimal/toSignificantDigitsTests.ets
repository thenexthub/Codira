/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0(the "License");
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

import { Decimal } from "@arkts.math.Decimal";

function main(): int {
    const suite = new arktest.ArkTestsuite("Decimal toSignificantDigits API tests");
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig001);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig002);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig003);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig004);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig005);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig006);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig007);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig008);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig009);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig010);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig011);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig012);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig013);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig014);
    suite.addTest("Decimal toSignificantDigits with specific config", testToSignificantDigitsWithSpecificConfig015);
    suite.addTest("Decimal toSignificantDigits with toExpNeg and toExpPos is 0", testToSignificantDigitsWithToExpNegAndToExpPosIsZero001);
    suite.addTest("Decimal toSignificantDigits with toExpNeg and toExpPos is 0", testToSignificantDigitsWithToExpNegAndToExpPosIsZero002);
    suite.addTest("Decimal toSignificantDigits with toExpNeg and toExpPos is 0", testToSignificantDigitsWithToExpNegAndToExpPosIsZero003);
    suite.addTest("Decimal toSignificantDigits with toExpNeg and toExpPos is 0", testToSignificantDigitsWithToExpNegAndToExpPosIsZero004);
    return suite.run();
}

function testToSignificantDigitsWithSpecificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(0).toSignificantDigits().valueOf(), '0');
    arktest.assertEQ(new Decimal(0.5).toSignificantDigits().valueOf(), '0.5');
    arktest.assertEQ(new Decimal(1).toSignificantDigits().valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).toSignificantDigits().valueOf(), '-1');
    arktest.assertEQ(new Decimal(Infinity).toSignificantDigits().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).toSignificantDigits().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(NaN).toSignificantDigits().valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('0').toSignificantDigits(1).valueOf(), '0');
    arktest.assertEQ(new Decimal('-0').toSignificantDigits(1).valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').toSignificantDigits(1, 0).valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').toSignificantDigits(1, 3).valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').toSignificantDigits(1, 6).valueOf(), '-0');
    arktest.assertEQ(new Decimal('0').toSignificantDigits(10).valueOf(), '0');
    arktest.assertEQ(new Decimal('-0').toSignificantDigits(20).valueOf(), '-0');
    arktest.assertEQ(new Decimal('12.345').toSignificantDigits().valueOf(), '12.345');
    arktest.assertEQ(new Decimal('123456789.12345678912346789').toSignificantDigits().valueOf(), '123456789.12345678912');
}

function testToSignificantDigitsWithSpecificConfig002() {
    Decimal.precision = 5;
    Decimal.rounding = 0;
    arktest.assertEQ(new Decimal('123456789.12345678912346789').toSignificantDigits().valueOf(), '123460000');

    Decimal.rounding = 1;
    arktest.assertEQ(new Decimal('123456789.12345678912346789').toSignificantDigits().valueOf(), '123450000');

    Decimal.rounding = 2;
    arktest.assertEQ(new Decimal('123456789.12345678912346789').toSignificantDigits().valueOf(), '123460000');

    Decimal.rounding = 3;
    arktest.assertEQ(new Decimal('123456789.12345678912346789').toSignificantDigits().valueOf(), '123450000');

    Decimal.rounding = 4;
    arktest.assertEQ(new Decimal('123456789.12345678912346789').toSignificantDigits().valueOf(), '123460000');

    Decimal.rounding = 5;
    arktest.assertEQ(new Decimal('123455000').toSignificantDigits().valueOf(), '123450000');

    Decimal.rounding = 6;
    arktest.assertEQ(new Decimal('123455000').toSignificantDigits().valueOf(), '123460000');

    Decimal.rounding = 7;
    arktest.assertEQ(new Decimal('123455000').toSignificantDigits().valueOf(), '123460000');

    Decimal.rounding = 8;
    arktest.assertEQ(new Decimal('123455000').toSignificantDigits().valueOf(), '123450000');
}

function testToSignificantDigitsWithSpecificConfig003() {
    arktest.assertEQ(new Decimal('4937809340236234102130.947044664011').toSignificantDigits(35, 0).valueOf(), '4937809340236234102130.947044664011');
    arktest.assertEQ(new Decimal('337528093391.493107').toSignificantDigits(13, 6).valueOf(), '337528093391.5');
    arktest.assertEQ(new Decimal('7725982105101004219161075340794084739.77152358557661226998117872661100').toSignificantDigits(52, 3).valueOf(), '7725982105101004219161075340794084739.771523585576612');
    arktest.assertEQ(new Decimal('52215017456426378512883312970.4861221').toSignificantDigits(36, 2).valueOf(), '52215017456426378512883312970.4861221');
    arktest.assertEQ(new Decimal('4985349771216.54991391072733').toSignificantDigits(21, 4).valueOf(), '4985349771216.54991391');
    arktest.assertEQ(new Decimal('1101895947.92762954984827601473036438720818296').toSignificantDigits(15, 4).valueOf(), '1101895947.92763');
    arktest.assertEQ(new Decimal('20026846.9372').toSignificantDigits(8, 6).valueOf(), '20026847');
    arktest.assertEQ(new Decimal('8204561821674316833266.047893028353').toSignificantDigits(21, 2).valueOf(), '8204561821674316833270');
    arktest.assertEQ(new Decimal('7419720696218.21961188299387526').toSignificantDigits(13, 1).valueOf(), '7419720696218');
    arktest.assertEQ(new Decimal('634438520.50126453').toSignificantDigits(3, 0).valueOf(), '635000000');
    arktest.assertEQ(new Decimal('28392130715407968.96').toSignificantDigits(18, 5).valueOf(), '28392130715407969');
    arktest.assertEQ(new Decimal('36765276128959576253780217524271972127.99').toSignificantDigits(38, 2).valueOf(), '36765276128959576253780217524271972128');
    arktest.assertEQ(new Decimal('88429990502356839.49260500060049004694585658780656').toSignificantDigits(43, 4).valueOf(), '88429990502356839.49260500060049004694585659');
    arktest.assertEQ(new Decimal('2768479.2081810557738981722324989595').toSignificantDigits(7, 1).valueOf(), '2768479');
    arktest.assertEQ(new Decimal('85601959550632180897338814.9').toSignificantDigits(6, 3).valueOf(), '85601900000000000000000000');
    arktest.assertEQ(new Decimal('3911951424308151086107907934806062.45495666964').toSignificantDigits(33, 5).valueOf(), '3911951424308151086107907934806060');
    arktest.assertEQ(new Decimal('892162270787217755968010370478903185093.0').toSignificantDigits(42, 6).valueOf(), '892162270787217755968010370478903185093');
    arktest.assertEQ(new Decimal('8018330257774248398702693271920270.09311146148062547723636185680287073').toSignificantDigits(33, 1).valueOf(), '8018330257774248398702693271920270');
    arktest.assertEQ(new Decimal('88092477731945.1182807043').toSignificantDigits(24, 4).valueOf(), '88092477731945.1182807043');
    arktest.assertEQ(new Decimal('9252541030951.90125310097078423727241610890757545760').toSignificantDigits(53, 3).valueOf(), '9252541030951.9012531009707842372724161089075754576');
    arktest.assertEQ(new Decimal('90993285126.5471295684197756642787492493375').toSignificantDigits(26, 6).valueOf(), '90993285126.547129568419776');
    arktest.assertEQ(new Decimal('315137924718839945152625512241078.59135072469615165060341097372').toSignificantDigits(54, 1).valueOf(), '315137924718839945152625512241078.591350724696151650603');
    arktest.assertEQ(new Decimal('822866202506325637.740891965300').toSignificantDigits(19, 3).valueOf(), '822866202506325637.7');
    arktest.assertEQ(new Decimal('78407276142689114391801017.43776').toSignificantDigits(18, 1).valueOf(), '78407276142689114300000000');
    arktest.assertEQ(new Decimal('13199803085352251536787920.6016623911545535514918245').toSignificantDigits(26, 2).valueOf(), '13199803085352251536787921');
    arktest.assertEQ(new Decimal('708.66442').toSignificantDigits(11, 0).valueOf(), '708.66442');
}

function testToSignificantDigitsWithSpecificConfig004() {
    arktest.assertEQ(new Decimal('96426394035875521556520436906.967758064780068220269263413347').toSignificantDigits(12, 1).valueOf(), '96426394035800000000000000000');
    arktest.assertEQ(new Decimal('297.1386227802795046531729004026975').toSignificantDigits(31, 3).valueOf(), '297.1386227802795046531729004026');
    arktest.assertEQ(new Decimal('98291.739').toSignificantDigits(9, 2).valueOf(), '98291.739');
    arktest.assertEQ(new Decimal('93.078130').toSignificantDigits(2, 5).valueOf(), '93');
    arktest.assertEQ(new Decimal('0.00099999999999').toSignificantDigits(3, 0).valueOf(), '0.001');
    arktest.assertEQ(new Decimal('0.00099999999999').toSignificantDigits(3, 1).valueOf(), '0.000999');
    arktest.assertEQ(new Decimal('56281970459.86927925182').toSignificantDigits(3, 6).valueOf(), '56300000000');
    arktest.assertEQ(new Decimal('565914070913.993334452356').toSignificantDigits(3, 1).valueOf(), '565000000000');
    arktest.assertEQ(new Decimal('3979389667144941900427982.77071').toSignificantDigits(16, 3).valueOf(), '3979389667144941000000000');
    arktest.assertEQ(new Decimal('52352220229587840556187937111046743.458356312735390953445935426160537204').toSignificantDigits(42, 4).valueOf(), '52352220229587840556187937111046743.4583563');
    arktest.assertEQ(new Decimal('36271.39812071417513332613').toSignificantDigits(24, 1).valueOf(), '36271.3981207141751333261');
    arktest.assertEQ(new Decimal('4739543704210297904547136327.7628335972128075226361512').toSignificantDigits(15, 6).valueOf(), '4739543704210300000000000000');
    arktest.assertEQ(new Decimal('37661669615568080247052723032589226.77024153').toSignificantDigits(36, 5).valueOf(), '37661669615568080247052723032589226.8');
    arktest.assertEQ(new Decimal('9645371928851642074344527881477634345.4914341').toSignificantDigits(34, 4).valueOf(), '9645371928851642074344527881477634000');
    arktest.assertEQ(new Decimal('698149777646325814282220973.088797026232').toSignificantDigits(15, 2).valueOf(), '698149777646326000000000000');
    arktest.assertEQ(new Decimal('18498919087448435591833489904295.729717528').toSignificantDigits(26, 5).valueOf(), '18498919087448435591833490000000');
    arktest.assertEQ(new Decimal('325969900850571568339999253781743124504.0').toSignificantDigits(38, 5).valueOf(), '325969900850571568339999253781743124500');
    arktest.assertEQ(new Decimal('40693051631.1251783538001747467807789871').toSignificantDigits(5, 4).valueOf(), '40693000000');
    arktest.assertEQ(new Decimal('810710395413475993.394678').toSignificantDigits(18, 3).valueOf(), '810710395413475993');
    arktest.assertEQ(new Decimal('86568781977428682.38222702203211').toSignificantDigits(30, 5).valueOf(), '86568781977428682.3822270220321');
    arktest.assertEQ(new Decimal('427897046936740724.911837668090826906087').toSignificantDigits(41, 3).valueOf(), '427897046936740724.911837668090826906087');
    arktest.assertEQ(new Decimal('8248561437515605126456247769501697.764296656999').toSignificantDigits(37, 4).valueOf(), '8248561437515605126456247769501697.764');
    arktest.assertEQ(new Decimal('5727898467119496952445688320883475.404402704196931184674675825').toSignificantDigits(64, 4).valueOf(), '5727898467119496952445688320883475.404402704196931184674675825');
    arktest.assertEQ(new Decimal('8752958499919994791191031106105979.47729').toSignificantDigits(19, 5).valueOf(), '8752958499919994791000000000000000');
    arktest.assertEQ(new Decimal('362154165751.58220211264627597398064').toSignificantDigits(28, 5).valueOf(), '362154165751.582202112646276');
    arktest.assertEQ(new Decimal('11914671819298396166152780469749.700178398572262303378530442966993').toSignificantDigits(2, 1).valueOf(), '11000000000000000000000000000000');
    arktest.assertEQ(new Decimal('84694.019523885').toSignificantDigits(16, 1).valueOf(), '84694.019523885');
    arktest.assertEQ(new Decimal('46198004778598591438177002960.807941976894036').toSignificantDigits(39, 2).valueOf(), '46198004778598591438177002960.8079419769');
    arktest.assertEQ(new Decimal('944540023268478392955679.235197603187056475911190458898708904').toSignificantDigits(45, 5).valueOf(), '944540023268478392955679.235197603187056475911');
    arktest.assertEQ(new Decimal('6384.0532734075270642890').toSignificantDigits(8, 0).valueOf(), '6384.0533');
}

function testToSignificantDigitsWithSpecificConfig005() {
    arktest.assertEQ(new Decimal('86.5258596344463026952440233603406633365294').toSignificantDigits(34, 0).valueOf(), '86.52585963444630269524402336034067');
    arktest.assertEQ(new Decimal('211698906765339555008927202275920.017666588018743809326').toSignificantDigits(21, 6).valueOf(), '211698906765339555009000000000000');
    arktest.assertEQ(new Decimal('93695.521776317122397800801736652801').toSignificantDigits(11, 3).valueOf(), '93695.521776');
    arktest.assertEQ(new Decimal('20.73571174').toSignificantDigits(2, 0).valueOf(), '21');
    arktest.assertEQ(new Decimal('44651390355733081311803973.6306343').toSignificantDigits(29, 2).valueOf(), '44651390355733081311803973.631');
    arktest.assertEQ(new Decimal('19628275338135891273639233078.3194337412').toSignificantDigits(26, 1).valueOf(), '19628275338135891273639233000');
    arktest.assertEQ(new Decimal('715174732399634482855366624736839961.1789741127950654699759400963516').toSignificantDigits(48, 0).valueOf(), '715174732399634482855366624736839961.178974112796');
    arktest.assertEQ(new Decimal('4010185.1941656790873904791').toSignificantDigits(16, 1).valueOf(), '4010185.194165679');
    arktest.assertEQ(new Decimal('199763544.2746').toSignificantDigits(10, 5).valueOf(), '199763544.3');
    arktest.assertEQ(new Decimal('792604593148129965009152274.7').toSignificantDigits(11, 5).valueOf(), '792604593150000000000000000');
    arktest.assertEQ(new Decimal('95649091688615551968376.399825').toSignificantDigits(13, 6).valueOf(), '95649091688620000000000');
    arktest.assertEQ(new Decimal('913366499416274482076806.54925361292970753237933').toSignificantDigits(2, 4).valueOf(), '910000000000000000000000');
    arktest.assertEQ(new Decimal('5713.7481900').toSignificantDigits(13, 6).valueOf(), '5713.74819');
    arktest.assertEQ(new Decimal('167006.16906099738293329453555919087').toSignificantDigits(4, 2).valueOf(), '167100');
    arktest.assertEQ(new Decimal('91.814948920333682589621761564879978').toSignificantDigits(30, 6).valueOf(), '91.8149489203336825896217615649');
    arktest.assertEQ(new Decimal('385450160421689186188707104582074317519.5011').toSignificantDigits(40, 0).valueOf(), '385450160421689186188707104582074317519.6');
    arktest.assertEQ(new Decimal('980624231078939283428476194025.3425674466530668543518761186154209').toSignificantDigits(4, 0).valueOf(), '980700000000000000000000000000');
    arktest.assertEQ(new Decimal('338479692666105014616253331245.01389376828879865108105798429640384').toSignificantDigits(21, 1).valueOf(), '338479692666105014616000000000');
    arktest.assertEQ(new Decimal('174223066779502215997051823.30021471115775').toSignificantDigits(19, 5).valueOf(), '174223066779502216000000000');
    arktest.assertEQ(new Decimal('321274436690589321065314360761529598.3926958740287183203559360243861156').toSignificantDigits(35, 0).valueOf(), '321274436690589321065314360761529600');
    arktest.assertEQ(new Decimal('965258557685921663672915807775689926.372616852406174796393011112241').toSignificantDigits(5, 5).valueOf(), '965260000000000000000000000000000000');
    arktest.assertEQ(new Decimal('20495856361024494227552150059704.12825253784').toSignificantDigits(41, 0).valueOf(), '20495856361024494227552150059704.128252538');
    arktest.assertEQ(new Decimal('347539608767858771651414903881.816764439318282980413').toSignificantDigits(47, 6).valueOf(), '347539608767858771651414903881.81676443931828298');
    arktest.assertEQ(new Decimal('319471356791473556138054433838044.90552027814').toSignificantDigits(16, 1).valueOf(), '319471356791473500000000000000000');
    arktest.assertEQ(new Decimal('3758411337.659079355464558774017310777636076246').toSignificantDigits(6, 1).valueOf(), '3758410000');
    arktest.assertEQ(new Decimal('986791401418080896.725674945574053721614668858858').toSignificantDigits(51, 4).valueOf(), '986791401418080896.725674945574053721614668858858');
    arktest.assertEQ(new Decimal('172864222463390.0005522631729828976762398559').toSignificantDigits(5, 5).valueOf(), '172860000000000');
    arktest.assertEQ(new Decimal('56.049').toSignificantDigits(6, 6).valueOf(), '56.049');
}

function testToSignificantDigitsWithSpecificConfig006() {
    arktest.assertEQ(new Decimal('8343815925404573683077429.4758959133829073879054267509464514').toSignificantDigits(21, 4).valueOf(), '8343815925404573683080000');
    arktest.assertEQ(new Decimal('5454663865.006413025890988464396059128886823').toSignificantDigits(38, 6).valueOf(), '5454663865.0064130258909884643960591289');
    arktest.assertEQ(new Decimal('9581036221000167548404063617040830845674.8563402624162914896694713537').toSignificantDigits(59, 1).valueOf(), '9581036221000167548404063617040830845674.8563402624162914896');
    arktest.assertEQ(new Decimal('410956568424.9910595380985324').toSignificantDigits(1, 0).valueOf(), '500000000000');
    arktest.assertEQ(new Decimal('32006369376256480193142996925472582.0').toSignificantDigits(10, 0).valueOf(), '32006369380000000000000000000000000');
    arktest.assertEQ(new Decimal('5222032172895746.902766092011').toSignificantDigits(21, 4).valueOf(), '5222032172895746.90277');
    arktest.assertEQ(new Decimal('8439793217442.92137711335321').toSignificantDigits(15, 4).valueOf(), '8439793217442.92');
    arktest.assertEQ(new Decimal('9.8660650336174887708269980').toSignificantDigits(28, 4).valueOf(), '9.866065033617488770826998');
    arktest.assertEQ(new Decimal('44049352.22669152978966908885542592854964186877').toSignificantDigits(45, 0).valueOf(), '44049352.2266915297896690888554259285496418688');
    arktest.assertEQ(new Decimal('26847260395161412327091304326394.7663271691497120655790427').toSignificantDigits(29, 3).valueOf(), '26847260395161412327091304326000');
    arktest.assertEQ(new Decimal('1085137384726.66574913879922622187703130425247').toSignificantDigits(2, 3).valueOf(), '1000000000000');
    arktest.assertEQ(new Decimal('9.9456').toSignificantDigits(4, 6).valueOf(), '9.946');
    arktest.assertEQ(new Decimal('428952193056.953').toSignificantDigits(15, 6).valueOf(), '428952193056.953');
    arktest.assertEQ(new Decimal('1941911780790529313259513.70860257943008396576247829795083').toSignificantDigits(33, 5).valueOf(), '1941911780790529313259513.70860258');
    arktest.assertEQ(new Decimal('23968438750336.405474280759813318106').toSignificantDigits(29, 0).valueOf(), '23968438750336.405474280759814');
    arktest.assertEQ(new Decimal('315154230499048332963529384927.93303628365695160150210696280635993802').toSignificantDigits(15, 5).valueOf(), '315154230499048000000000000000');
    arktest.assertEQ(new Decimal('6480935501.55496974285182162603356273156890').toSignificantDigits(2, 4).valueOf(), '6500000000');
    arktest.assertEQ(new Decimal('42302765724017563090210326618736869270.98392129213256644255722').toSignificantDigits(39, 2).valueOf(), '42302765724017563090210326618736869271');
    arktest.assertEQ(new Decimal('608747404736337255085100901273291565.318094268584914528314761117').toSignificantDigits(36, 3).valueOf(), '608747404736337255085100901273291565');
    arktest.assertEQ(new Decimal('3154176290800715548619630621.0355772810773559541').toSignificantDigits(13, 4).valueOf(), '3154176290801000000000000000');
    arktest.assertEQ(new Decimal('38865242750002932570887590890877550733.5505553313425194996432914268372269').toSignificantDigits(27, 1).valueOf(), '38865242750002932570887590800000000000');
    arktest.assertEQ(new Decimal('59457914554626139.52065942543632055255656').toSignificantDigits(19, 5).valueOf(), '59457914554626139.52');
    arktest.assertEQ(new Decimal('8728384988174071448.69274').toSignificantDigits(21, 4).valueOf(), '8728384988174071448.69');
    arktest.assertEQ(new Decimal('8209486790494710645762862448749.80842396605900775935').toSignificantDigits(54, 1).valueOf(), '8209486790494710645762862448749.80842396605900775935');
    arktest.assertEQ(new Decimal('963188883728.740694814570142059').toSignificantDigits(29, 3).valueOf(), '963188883728.74069481457014205');
    arktest.assertEQ(new Decimal('93249710101545874917863173.03705388925').toSignificantDigits(16, 3).valueOf(), '93249710101545870000000000');
    arktest.assertEQ(new Decimal('88179259072684818766023076456.58895232320350').toSignificantDigits(39, 4).valueOf(), '88179259072684818766023076456.5889523232');
    arktest.assertEQ(new Decimal('776981797289252251.093401546863758214465286704664334').toSignificantDigits(39, 4).valueOf(), '776981797289252251.093401546863758214465');
    arktest.assertEQ(new Decimal('188670879464255056850261739507365918265.9293874736873279').toSignificantDigits(28, 1).valueOf(), '188670879464255056850261739500000000000');
    arktest.assertEQ(new Decimal('790372524561301386433116293301042112.0').toSignificantDigits(25, 2).valueOf(), '790372524561301386433116300000000000');
    arktest.assertEQ(new Decimal('117626347290152284.584779').toSignificantDigits(7, 3).valueOf(), '117626300000000000');
}

function testToSignificantDigitsWithSpecificConfig007() {
    arktest.assertEQ(new Decimal('353396834933281759072773.805789609148').toSignificantDigits(37, 4).valueOf(), '353396834933281759072773.805789609148');
    arktest.assertEQ(new Decimal('4274522263032712850.0041729873').toSignificantDigits(14, 4).valueOf(), '4274522263032700000');
    arktest.assertEQ(new Decimal('8345606845226.45566624').toSignificantDigits(3, 6).valueOf(), '8350000000000');
    arktest.assertEQ(new Decimal('7139338928.10029575').toSignificantDigits(19, 2).valueOf(), '7139338928.10029575');
    arktest.assertEQ(new Decimal('1.289937959964596899900').toSignificantDigits(12, 3).valueOf(), '1.28993795996');
    arktest.assertEQ(new Decimal('581091710823047241756858512.713357759428').toSignificantDigits(14, 0).valueOf(), '581091710823050000000000000');
    arktest.assertEQ(new Decimal('4952915437691054280969583218972.8616946954635715417980').toSignificantDigits(25, 1).valueOf(), '4952915437691054280969583000000');
    arktest.assertEQ(new Decimal('9705894787058704299917761166078683.9910480951422').toSignificantDigits(11, 5).valueOf(), '9705894787100000000000000000000000');
    arktest.assertEQ(new Decimal('3709152457254657158536681.732705728776998763593525103159733').toSignificantDigits(52, 3).valueOf(), '3709152457254657158536681.732705728776998763593525103');
    arktest.assertEQ(new Decimal('186530566473536251263.858877958474287539').toSignificantDigits(30, 4).valueOf(), '186530566473536251263.858877958');
    arktest.assertEQ(new Decimal('18185426229191232807040619835808.46825483359797368754').toSignificantDigits(52, 3).valueOf(), '18185426229191232807040619835808.46825483359797368754');
    arktest.assertEQ(new Decimal('41.51330387617312622186860688055631484').toSignificantDigits(24, 4).valueOf(), '41.5133038761731262218686');
    arktest.assertEQ(new Decimal('892734285316609348922285835347841194677.279150490649933902110687168010').toSignificantDigits(7, 4).valueOf(), '892734300000000000000000000000000000000');
    arktest.assertEQ(new Decimal('8867741117136144692773028671.48789537068725372709865944763').toSignificantDigits(10, 4).valueOf(), '8867741117000000000000000000');
    arktest.assertEQ(new Decimal('58066.611929661352058113440').toSignificantDigits(18, 0).valueOf(), '58066.6119296613521');
    arktest.assertEQ(new Decimal('610095820904.311952319394878656597629841829693065').toSignificantDigits(22, 3).valueOf(), '610095820904.3119523193');
    arktest.assertEQ(new Decimal('45147764430505826.03979928037074287837692102337').toSignificantDigits(45, 2).valueOf(), '45147764430505826.0397992803707428783769210234');
    arktest.assertEQ(new Decimal('2365934669507425116.685560049269016191181112056065').toSignificantDigits(37, 4).valueOf(), '2365934669507425116.685560049269016191');
    arktest.assertEQ(new Decimal('76328567052192004276.3286634056867957577510810610864').toSignificantDigits(49, 5).valueOf(), '76328567052192004276.32866340568679575775108106109');
    arktest.assertEQ(new Decimal('9341865065715748.7103892265345035977095284').toSignificantDigits(39, 3).valueOf(), '9341865065715748.71038922653450359770952');
    arktest.assertEQ(new Decimal('1893828187345809.95025475161610431769040580234726').toSignificantDigits(51, 5).valueOf(), '1893828187345809.95025475161610431769040580234726');
    arktest.assertEQ(new Decimal('675418012725769678669671724576651465.457619554107058395110952042747864102').toSignificantDigits(36, 5).valueOf(), '675418012725769678669671724576651465');
    arktest.assertEQ(new Decimal('722723049408340056285000637093.1564846486282233386998').toSignificantDigits(31, 2).valueOf(), '722723049408340056285000637093.2');
    arktest.assertEQ(new Decimal('8039307818523262169309113965.2379556730465735379').toSignificantDigits(43, 1).valueOf(), '8039307818523262169309113965.237955673046573');
    arktest.assertEQ(new Decimal('620699795.7687720027642576108299448').toSignificantDigits(18, 6).valueOf(), '620699795.768772003');
    arktest.assertEQ(new Decimal('677976409911497719121490267737187.9281869113').toSignificantDigits(16, 1).valueOf(), '677976409911497700000000000000000');
    arktest.assertEQ(new Decimal('9913200270263779985846761891158202865.88').toSignificantDigits(14, 2).valueOf(), '9913200270263800000000000000000000000');
    arktest.assertEQ(new Decimal('95447550983654256.94').toSignificantDigits(14, 5).valueOf(), '95447550983654000');
    arktest.assertEQ(new Decimal('9727301256933984906097986873657.0').toSignificantDigits(28, 0).valueOf(), '9727301256933984906097986874000');
}

function testToSignificantDigitsWithSpecificConfig008() {
    arktest.assertEQ(new Decimal('83144797009396341772640576982431626.06487716198510170943250663066').toSignificantDigits(3, 5).valueOf(), '83100000000000000000000000000000000');
    arktest.assertEQ(new Decimal('76723773.0442321793176553237').toSignificantDigits(19, 6).valueOf(), '76723773.04423217932');
    arktest.assertEQ(new Decimal('8442290748483192.541766601132013892502484734171240').toSignificantDigits(2, 5).valueOf(), '8400000000000000');
    arktest.assertEQ(new Decimal('80224755820119141339232053676618497.04561623225397344999').toSignificantDigits(31, 3).valueOf(), '80224755820119141339232053676610000');
    arktest.assertEQ(new Decimal('655778655223372.965').toSignificantDigits(17, 0).valueOf(), '655778655223372.97');
    arktest.assertEQ(new Decimal('20184630477822715391390984001199.1866444533896426202887666335307182015').toSignificantDigits(71, 6).valueOf(), '20184630477822715391390984001199.1866444533896426202887666335307182015');
    arktest.assertEQ(new Decimal('41930826996134221.1974945198118251722677898312235505').toSignificantDigits(16, 6).valueOf(), '41930826996134220');
    arktest.assertEQ(new Decimal('9401169208627113178099.432092017583166044').toSignificantDigits(17, 6).valueOf(), '9401169208627113200000');
    arktest.assertEQ(new Decimal('46844285165612.4277871838').toSignificantDigits(8, 5).valueOf(), '46844285000000');
    arktest.assertEQ(new Decimal('732653734623947749013990.2554245257179').toSignificantDigits(27, 3).valueOf(), '732653734623947749013990.255');
    arktest.assertEQ(new Decimal('817615525922580553380.874869911815006035161427956123678666').toSignificantDigits(8, 5).valueOf(), '817615530000000000000');
    arktest.assertEQ(new Decimal('648881720909905928.428439970149048878644930752628').toSignificantDigits(4, 2).valueOf(), '648900000000000000');
    arktest.assertEQ(new Decimal('85228991367.46145277847248857183769451804173').toSignificantDigits(17, 1).valueOf(), '85228991367.461452');
    arktest.assertEQ(new Decimal('239393776.69622644151181623925655').toSignificantDigits(23, 4).valueOf(), '239393776.69622644151182');
    arktest.assertEQ(new Decimal('33546748243104239469877559937.363355786582993').toSignificantDigits(44, 0).valueOf(), '33546748243104239469877559937.363355786582993');
    arktest.assertEQ(new Decimal('3823084385271016278125.11').toSignificantDigits(1, 5).valueOf(), '4000000000000000000000');
    arktest.assertEQ(new Decimal('77046259820455751626.602680800134253741613198881').toSignificantDigits(36, 6).valueOf(), '77046259820455751626.6026808001342537');
    arktest.assertEQ(new Decimal('26687373854107297015683.74432725585956728677271691574').toSignificantDigits(26, 3).valueOf(), '26687373854107297015683.744');
    arktest.assertEQ(new Decimal('976672549609984585227222778792212901570.55874088').toSignificantDigits(28, 1).valueOf(), '976672549609984585227222778700000000000');
    arktest.assertEQ(new Decimal('2.561614266920093636561053').toSignificantDigits(23, 2).valueOf(), '2.5616142669200936365611');
    arktest.assertEQ(new Decimal('94492716203969.2225869573684291964191137943094514534843').toSignificantDigits(48, 0).valueOf(), '94492716203969.2225869573684291964191137943094515');
    arktest.assertEQ(new Decimal('990770621178724769771371129.99176486560800414').toSignificantDigits(23, 3).valueOf(), '990770621178724769771370000');
    arktest.assertEQ(new Decimal('691412410360950750128020.48447221650237091009047').toSignificantDigits(17, 0).valueOf(), '691412410360950760000000');
    arktest.assertEQ(new Decimal('29040659564825201725590.2424704967395').toSignificantDigits(38, 2).valueOf(), '29040659564825201725590.2424704967395');
    arktest.assertEQ(new Decimal('89559092584017288521728982.5833198608864').toSignificantDigits(27, 1).valueOf(), '89559092584017288521728982.5');
    arktest.assertEQ(new Decimal('1811614362251924.405121306157849629').toSignificantDigits(27, 6).valueOf(), '1811614362251924.40512130616');
    arktest.assertEQ(new Decimal('4633501474602.054640995909183').toSignificantDigits(10, 0).valueOf(), '4633501475000');
    arktest.assertEQ(new Decimal('678705341261.75297583718425453684090101').toSignificantDigits(39, 6).valueOf(), '678705341261.75297583718425453684090101');
    arktest.assertEQ(new Decimal('564078844253167778.8161248928271829563591373580664298').toSignificantDigits(52, 0).valueOf(), '564078844253167778.8161248928271829563591373580664298');
    arktest.assertEQ(new Decimal('88322850583909.74160162410286643').toSignificantDigits(25, 1).valueOf(), '88322850583909.7416016241');
    arktest.assertEQ(new Decimal('3846366946174062127758449605.702275809555523634154040950898280612811').toSignificantDigits(38, 6).valueOf(), '3846366946174062127758449605.7022758096');
    arktest.assertEQ(new Decimal('89658733356696254546423515335.28349').toSignificantDigits(9, 2).valueOf(), '89658733400000000000000000000');
    arktest.assertEQ(new Decimal('4761467.861150141326149685649922793395').toSignificantDigits(10, 2).valueOf(), '4761467.862');
    arktest.assertEQ(new Decimal('179.9261669').toSignificantDigits(8, 4).valueOf(), '179.92617');
}

function testToSignificantDigitsWithSpecificConfig009() {
    arktest.assertEQ(new Decimal('75069004857851352687270011001106689339.08735').toSignificantDigits(46, 5).valueOf(), '75069004857851352687270011001106689339.08735');
    arktest.assertEQ(new Decimal('3361644609630676688091028947986494293.04539245514308267535094798418063548394').toSignificantDigits(58, 2).valueOf(), '3361644609630676688091028947986494293.045392455143082675351');
    arktest.assertEQ(new Decimal('6.794242173558561804595').toSignificantDigits(20, 4).valueOf(), '6.7942421735585618046');
    arktest.assertEQ(new Decimal('51657502980255968504333490637.0966289278031817448038040396600018487').toSignificantDigits(55, 5).valueOf(), '51657502980255968504333490637.09662892780318174480380404');
    arktest.assertEQ(new Decimal('11855.226308528').toSignificantDigits(16, 6).valueOf(), '11855.226308528');
    arktest.assertEQ(new Decimal('61276347667662.405824737').toSignificantDigits(5, 0).valueOf(), '61277000000000');
    arktest.assertEQ(new Decimal('4174609636.0').toSignificantDigits(9, 6).valueOf(), '4174609640');
    arktest.assertEQ(new Decimal('687936219719625022378.9502057605855').toSignificantDigits(25, 3).valueOf(), '687936219719625022378.9502');
    arktest.assertEQ(new Decimal('193757131563911904.52682306122527060962641882465').toSignificantDigits(14, 6).valueOf(), '193757131563910000');
    arktest.assertEQ(new Decimal('387.45508661743').toSignificantDigits(9, 6).valueOf(), '387.455087');
    arktest.assertEQ(new Decimal('557618201766183005041293682.3722668213410766424816487716234273').toSignificantDigits(19, 6).valueOf(), '557618201766183005000000000');
    arktest.assertEQ(new Decimal('21128122508600497.05488760').toSignificantDigits(22, 6).valueOf(), '21128122508600497.05489');
    arktest.assertEQ(new Decimal('630252144005109.53366072283861381792').toSignificantDigits(32, 4).valueOf(), '630252144005109.53366072283861382');
    arktest.assertEQ(new Decimal('963826907622.364662995275692188').toSignificantDigits(24, 2).valueOf(), '963826907622.364662995276');
    arktest.assertEQ(new Decimal('4212653046051913081783119467271.3764751143959150212855631').toSignificantDigits(54, 3).valueOf(), '4212653046051913081783119467271.37647511439591502128556');
    arktest.assertEQ(new Decimal('2014467765.30542656127').toSignificantDigits(22, 4).valueOf(), '2014467765.30542656127');
    arktest.assertEQ(new Decimal('1479326666751842221164864465643932.73817033951340680').toSignificantDigits(6, 1).valueOf(), '1479320000000000000000000000000000');
    arktest.assertEQ(new Decimal('6944862581642049969018412438205282752.72886131507563067413320187385623099').toSignificantDigits(43, 2).valueOf(), '6944862581642049969018412438205282752.728862');
    arktest.assertEQ(new Decimal('9838375.645').toSignificantDigits(9, 1).valueOf(), '9838375.64');
    arktest.assertEQ(new Decimal('73392897409456861290131541129654.925607506690362687103367844755771').toSignificantDigits(45, 4).valueOf(), '73392897409456861290131541129654.9256075066904');
    arktest.assertEQ(new Decimal('89764275311096607184088076035578686331.70483764463153360153147271493').toSignificantDigits(7, 1).valueOf(), '89764270000000000000000000000000000000');
    arktest.assertEQ(new Decimal('615391331037547517357702.9541121335086812').toSignificantDigits(37, 5).valueOf(), '615391331037547517357702.9541121335087');
    arktest.assertEQ(new Decimal('2617216902482291199973663.484710966125').toSignificantDigits(37, 4).valueOf(), '2617216902482291199973663.484710966125');
    arktest.assertEQ(new Decimal('17129271618518517092155339589985.34795326188').toSignificantDigits(28, 3).valueOf(), '17129271618518517092155339580000');
    arktest.assertEQ(new Decimal('606535160521504794060646691380816193844.7').toSignificantDigits(18, 5).valueOf(), '606535160521504794000000000000000000000');
    arktest.assertEQ(new Decimal('34177380606532061225426.5490340').toSignificantDigits(20, 2).valueOf(), '34177380606532061226000');
    arktest.assertEQ(new Decimal('537578.9271229227').toSignificantDigits(12, 0).valueOf(), '537578.927123');
    arktest.assertEQ(new Decimal('5008643590729908333990609905.47558686510363696').toSignificantDigits(25, 5).valueOf(), '5008643590729908333990610000');
    arktest.assertEQ(new Decimal('4895936188848031794116383643496168477.0801543').toSignificantDigits(15, 1).valueOf(), '4895936188848030000000000000000000000');
    arktest.assertEQ(new Decimal('5683079691081606113599129167429526013511.486528758557663781739723321875379906235').toSignificantDigits(51, 3).valueOf(), '5683079691081606113599129167429526013511.48652875855');
    arktest.assertEQ(new Decimal('8629922951113.7511448487597458660248521234380600977').toSignificantDigits(41, 4).valueOf(), '8629922951113.7511448487597458660248521234');
    arktest.assertEQ(new Decimal('90278102650946226014809046003862793.7835979911').toSignificantDigits(24, 3).valueOf(), '90278102650946226014809000000000000');
}

function testToSignificantDigitsWithSpecificConfig010() {
    arktest.assertEQ(new Decimal('4821625627675322025353636064.57175523338580806993894351876').toSignificantDigits(33, 4).valueOf(), '4821625627675322025353636064.57176');
    arktest.assertEQ(new Decimal('201458719700489100373654957.8255064910003360595196823588160').toSignificantDigits(51, 3).valueOf(), '201458719700489100373654957.825506491000336059519682');
    arktest.assertEQ(new Decimal('53212790802213594493332.8650736614584969129110').toSignificantDigits(23, 3).valueOf(), '53212790802213594493332');
    arktest.assertEQ(new Decimal('8177197143472082210310164503390974.89434').toSignificantDigits(26, 3).valueOf(), '8177197143472082210310164500000000');
    arktest.assertEQ(new Decimal('6977664457920874.47627073206781').toSignificantDigits(12, 2).valueOf(), '6977664457930000');
    arktest.assertEQ(new Decimal('48200644415566530.4984835390397628883909840').toSignificantDigits(23, 0).valueOf(), '48200644415566530.498484');
    arktest.assertEQ(new Decimal('4192746732983495210446560254097740.47028995').toSignificantDigits(1, 1).valueOf(), '4000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('755.02320914910644').toSignificantDigits(9, 6).valueOf(), '755.023209');
    arktest.assertEQ(new Decimal('66963023876901420169077780295.01885877196').toSignificantDigits(6, 3).valueOf(), '66963000000000000000000000000');
    arktest.assertEQ(new Decimal('505291668854707.709062685721555135520850266111469308027').toSignificantDigits(23, 4).valueOf(), '505291668854707.70906269');
    arktest.assertEQ(new Decimal('664850.61801224118433274591608772154387').toSignificantDigits(12, 0).valueOf(), '664850.618013');
    arktest.assertEQ(new Decimal('785103038952630143044641569204316756.637970547918746160528221834671492').toSignificantDigits(68, 6).valueOf(), '785103038952630143044641569204316756.63797054791874616052822183467149');
    arktest.assertEQ(new Decimal('4146164592920910235341149.58704019746296185479').toSignificantDigits(29, 3).valueOf(), '4146164592920910235341149.587');
    arktest.assertEQ(new Decimal('579742066579367045736.9170048432302875302855793').toSignificantDigits(45, 4).valueOf(), '579742066579367045736.917004843230287530285579');
    arktest.assertEQ(new Decimal('92817.84445').toSignificantDigits(13, 4).valueOf(), '92817.84445');
    arktest.assertEQ(new Decimal('772179455775.22422834897889').toSignificantDigits(23, 1).valueOf(), '772179455775.22422834897');
    arktest.assertEQ(new Decimal('4670558124138345.3777942422216231').toSignificantDigits(14, 3).valueOf(), '4670558124138300');
    arktest.assertEQ(new Decimal('11653701002.177901693320687576204027037389422679').toSignificantDigits(24, 6).valueOf(), '11653701002.1779016933207');
    arktest.assertEQ(new Decimal('661971346000462043332006546474696174.7167702709539').toSignificantDigits(25, 4).valueOf(), '661971346000462043332006500000000000');
    arktest.assertEQ(new Decimal('7951.2507067915133').toSignificantDigits(12, 0).valueOf(), '7951.2507068');
    arktest.assertEQ(new Decimal('83825.915697306948969320604369447056').toSignificantDigits(31, 1).valueOf(), '83825.91569730694896932060436944');
    arktest.assertEQ(new Decimal('179989.23863077943963').toSignificantDigits(21, 2).valueOf(), '179989.23863077943963');
    arktest.assertEQ(new Decimal('333610464960022048972485697598948476.32749272770013219331').toSignificantDigits(46, 2).valueOf(), '333610464960022048972485697598948476.3274927278');
    arktest.assertEQ(new Decimal('567620976076010425435148100001.0364396667132672575976').toSignificantDigits(14, 5).valueOf(), '567620976076010000000000000000');
    arktest.assertEQ(new Decimal('25166561739091850523119786031.356417774102').toSignificantDigits(13, 1).valueOf(), '25166561739090000000000000000');
    arktest.assertEQ(new Decimal('35769804358466980414449956124427509.31366843647801642057').toSignificantDigits(35, 5).valueOf(), '35769804358466980414449956124427509');
    arktest.assertEQ(new Decimal('5623994624926506484134510384338582.7213130433076592758171270720102084').toSignificantDigits(4, 2).valueOf(), '5624000000000000000000000000000000');
    arktest.assertEQ(new Decimal('52471579263868615271801405152771.014318284628313').toSignificantDigits(16, 3).valueOf(), '52471579263868610000000000000000');
    arktest.assertEQ(new Decimal('754585116996.9768').toSignificantDigits(1, 0).valueOf(), '800000000000');
    arktest.assertEQ(new Decimal('6845902808062306446526711601148487105964.760230279341124548333865130108451155441').toSignificantDigits(9, 2).valueOf(), '6845902810000000000000000000000000000000');
    arktest.assertEQ(new Decimal('25329931835642796295008712007359.774').toSignificantDigits(7, 6).valueOf(), '25329930000000000000000000000000');
    arktest.assertEQ(new Decimal('838205133360935.0450208').toSignificantDigits(17, 1).valueOf(), '838205133360935.04');
    arktest.assertEQ(new Decimal('964483.4958501605537391').toSignificantDigits(2, 4).valueOf(), '960000');
}

function testToSignificantDigitsWithSpecificConfig011() {
    arktest.assertEQ(new Decimal('6985940160812637750037427078390489093.59724194766104006296648646911097283').toSignificantDigits(44, 4).valueOf(), '6985940160812637750037427078390489093.5972419');
    arktest.assertEQ(new Decimal('85912832521353633962171.622844738617471351787937').toSignificantDigits(18, 1).valueOf(), '85912832521353633900000');
    arktest.assertEQ(new Decimal('22537.044131504334747686356').toSignificantDigits(7, 4).valueOf(), '22537.04');
    arktest.assertEQ(new Decimal('64820082671929139423989159247959991.2743924454939081683').toSignificantDigits(52, 2).valueOf(), '64820082671929139423989159247959991.27439244549390817');
    arktest.assertEQ(new Decimal('81309794072838871318854500310152.09295684998521').toSignificantDigits(35, 4).valueOf(), '81309794072838871318854500310152.093');
    arktest.assertEQ(new Decimal('2983.146').toSignificantDigits(8, 0).valueOf(), '2983.146');
    arktest.assertEQ(new Decimal('7649851971073922227632383137.12367829277753284010508541882606785617').toSignificantDigits(18, 1).valueOf(), '7649851971073922220000000000');
    arktest.assertEQ(new Decimal('4413319948694474085791.65287472187114939173359969').toSignificantDigits(29, 1).valueOf(), '4413319948694474085791.6528747');
    arktest.assertEQ(new Decimal('9169434008670526360155.858673447171266307242005').toSignificantDigits(28, 4).valueOf(), '9169434008670526360155.858673');
    arktest.assertEQ(new Decimal('87281481541792866.31680021820096251746992578412').toSignificantDigits(31, 3).valueOf(), '87281481541792866.31680021820096');
    arktest.assertEQ(new Decimal('1262882505899773546530078940105109255.1340263465677131821878853130').toSignificantDigits(21, 1).valueOf(), '1262882505899773546530000000000000000');
    arktest.assertEQ(new Decimal('90957735119480552000.12757925661213812317307').toSignificantDigits(33, 3).valueOf(), '90957735119480552000.1275792566121');
    arktest.assertEQ(new Decimal('95094204140327.3868000696069952245821304362528').toSignificantDigits(26, 6).valueOf(), '95094204140327.386800069607');
    arktest.assertEQ(new Decimal('782.8315804028036627332091761646906').toSignificantDigits(16, 4).valueOf(), '782.8315804028037');
    arktest.assertEQ(new Decimal('4070074897815719013854404930086.0517196').toSignificantDigits(25, 2).valueOf(), '4070074897815719013854405000000');
    arktest.assertEQ(new Decimal('7535527006575666259327801.3118771661605538067531702602254').toSignificantDigits(24, 5).valueOf(), '7535527006575666259327800');
    arktest.assertEQ(new Decimal('992026987647463.516668').toSignificantDigits(3, 0).valueOf(), '993000000000000');
    arktest.assertEQ(new Decimal('4698757980923794411324653119136506649753.90585339652148316962906302356153227').toSignificantDigits(53, 1).valueOf(), '4698757980923794411324653119136506649753.9058533965214');
    arktest.assertEQ(new Decimal('338765415.62299907506363504130425028815639273842').toSignificantDigits(42, 6).valueOf(), '338765415.622999075063635041304250288156393');
    arktest.assertEQ(new Decimal('443272.8').toSignificantDigits(3, 0).valueOf(), '444000');
    arktest.assertEQ(new Decimal('92863547832.25594').toSignificantDigits(13, 0).valueOf(), '92863547832.26');
    arktest.assertEQ(new Decimal('227484121340486.54251108296').toSignificantDigits(21, 0).valueOf(), '227484121340486.542512');
    arktest.assertEQ(new Decimal('7034.0').toSignificantDigits(7, 2).valueOf(), '7034');
    arktest.assertEQ(new Decimal('84677601708231089.8035199547').toSignificantDigits(9, 2).valueOf(), '84677601800000000');
    arktest.assertEQ(new Decimal('903992663388421818078667246.7494838726767325648').toSignificantDigits(16, 3).valueOf(), '903992663388421800000000000');
    arktest.assertEQ(new Decimal('9440656634642185509393801198086015.4').toSignificantDigits(6, 1).valueOf(), '9440650000000000000000000000000000');
    arktest.assertEQ(new Decimal('916417601649865583796921805.163932264102471440203390988').toSignificantDigits(13, 1).valueOf(), '916417601649800000000000000');
    arktest.assertEQ(new Decimal('3546786323316815404869.163629407284618604').toSignificantDigits(17, 2).valueOf(), '3546786323316815500000');
    arktest.assertEQ(new Decimal('747408692550834866360911935.89904452751652821978027173825483').toSignificantDigits(58, 3).valueOf(), '747408692550834866360911935.8990445275165282197802717382548');
    arktest.assertEQ(new Decimal('29804424518.256135774').toSignificantDigits(23, 2).valueOf(), '29804424518.256135774');
}

function testToSignificantDigitsWithSpecificConfig012() {
    arktest.assertEQ(new Decimal('15732791881955773293163440652451.44291076191144').toSignificantDigits(31, 1).valueOf(), '15732791881955773293163440652450');
    arktest.assertEQ(new Decimal('82293484047103499038.697542264214713777186786620705401').toSignificantDigits(48, 0).valueOf(), '82293484047103499038.6975422642147137771867866208');
    arktest.assertEQ(new Decimal('5288356.17').toSignificantDigits(11, 2).valueOf(), '5288356.17');
    arktest.assertEQ(new Decimal('75977708959803656203251104.81675609237952293524970938678').toSignificantDigits(52, 4).valueOf(), '75977708959803656203251104.81675609237952293524970939');
    arktest.assertEQ(new Decimal('1958555193502587409635649733733.584').toSignificantDigits(13, 2).valueOf(), '1958555193503000000000000000000');
    arktest.assertEQ(new Decimal('61097901998.6487577').toSignificantDigits(21, 3).valueOf(), '61097901998.6487577');
    arktest.assertEQ(new Decimal('4966635612645365.095866507224630866586307217888782761159').toSignificantDigits(30, 6).valueOf(), '4966635612645365.09586650722463');
    arktest.assertEQ(new Decimal('449349314276161751863885318023458.255606199257691366208268573').toSignificantDigits(61, 6).valueOf(), '449349314276161751863885318023458.255606199257691366208268573');
    arktest.assertEQ(new Decimal('3313802055037244406355.14412694590539847622267').toSignificantDigits(38, 5).valueOf(), '3313802055037244406355.1441269459053985');
    arktest.assertEQ(new Decimal('77476038634229841225203.05729431960467859218').toSignificantDigits(35, 6).valueOf(), '77476038634229841225203.057294319605');
    arktest.assertEQ(new Decimal('532731523075.4101466861580822959').toSignificantDigits(10, 1).valueOf(), '532731523000');
    arktest.assertEQ(new Decimal('279332110982355758954949172.19150963').toSignificantDigits(3, 5).valueOf(), '279000000000000000000000000');
    arktest.assertEQ(new Decimal('76199428363070721667903.826669566165506057243720918746').toSignificantDigits(32, 6).valueOf(), '76199428363070721667903.826669566');
    arktest.assertEQ(new Decimal('297631.47066887').toSignificantDigits(4, 3).valueOf(), '297600');
    arktest.assertEQ(new Decimal('8248110729241783273113.562111307100337407').toSignificantDigits(40, 2).valueOf(), '8248110729241783273113.562111307100337407');
    arktest.assertEQ(new Decimal('1236453638891742153022.8348728811').toSignificantDigits(1, 4).valueOf(), '1000000000000000000000');
    arktest.assertEQ(new Decimal('535327351.8008').toSignificantDigits(5, 2).valueOf(), '535330000');
    arktest.assertEQ(new Decimal('4308.58715419601937041522').toSignificantDigits(24, 5).valueOf(), '4308.58715419601937041522');
    arktest.assertEQ(new Decimal('13500654611668894713386183570622548.34240861579').toSignificantDigits(40, 5).valueOf(), '13500654611668894713386183570622548.34241');
    arktest.assertEQ(new Decimal('995.71558219931730116083231270171862650').toSignificantDigits(25, 3).valueOf(), '995.7155821993173011608323');
    arktest.assertEQ(new Decimal('239147599253876718.95044334911732723218139304606724752').toSignificantDigits(34, 2).valueOf(), '239147599253876718.9504433491173273');
    arktest.assertEQ(new Decimal('765497722616.896100364573390975890478644136740197458').toSignificantDigits(22, 6).valueOf(), '765497722616.8961003646');
    arktest.assertEQ(new Decimal('101979298537.6878817821900758850756870223612522530').toSignificantDigits(23, 3).valueOf(), '101979298537.68788178219');
    arktest.assertEQ(new Decimal('54563203006377285590954695127259934956.72304700').toSignificantDigits(5, 2).valueOf(), '54564000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('937851991874226678.191965007088457').toSignificantDigits(19, 0).valueOf(), '937851991874226678.2');
    arktest.assertEQ(new Decimal('9141378468539383564638997666387.513564869223').toSignificantDigits(27, 1).valueOf(), '9141378468539383564638997660000');
    arktest.assertEQ(new Decimal('20497199023806.45608949227987004920339939').toSignificantDigits(22, 5).valueOf(), '20497199023806.45608949');
    arktest.assertEQ(new Decimal('81603182.616632542195753770061305729184').toSignificantDigits(9, 2).valueOf(), '81603182.7');
    arktest.assertEQ(new Decimal('1288222268118271079758562419926059399472.4003871704903395624929468673346838726').toSignificantDigits(15, 2).valueOf(), '1288222268118280000000000000000000000000');
    arktest.assertEQ(new Decimal('484726059819531746596562275263672330.122283607').toSignificantDigits(14, 3).valueOf(), '484726059819530000000000000000000000');
    arktest.assertEQ(new Decimal('675623616750.993125132354').toSignificantDigits(2, 4).valueOf(), '680000000000');
}

function testToSignificantDigitsWithSpecificConfig013() {
    arktest.assertEQ(new Decimal('87687220439051554697153502010194056104.42861').toSignificantDigits(14, 1).valueOf(), '87687220439051000000000000000000000000');
    arktest.assertEQ(new Decimal('2844992735024778164396876428386392.397101880541673725').toSignificantDigits(29, 6).valueOf(), '2844992735024778164396876428400000');
    arktest.assertEQ(new Decimal('4910468313236974384325553954.558968559307060603568610').toSignificantDigits(32, 4).valueOf(), '4910468313236974384325553954.559');
    arktest.assertEQ(new Decimal('7677382468596942606325100814949164462.335409794').toSignificantDigits(49, 1).valueOf(), '7677382468596942606325100814949164462.335409794');
    arktest.assertEQ(new Decimal('6958724457786172481654532020985.043').toSignificantDigits(34, 2).valueOf(), '6958724457786172481654532020985.043');
    arktest.assertEQ(new Decimal('460705.8028929240198868371532360965971117977776').toSignificantDigits(4, 2).valueOf(), '460800');
    arktest.assertEQ(new Decimal('81763325373301098575053681.0095511047184850495148390898').toSignificantDigits(41, 5).valueOf(), '81763325373301098575053681.009551104718485');
    arktest.assertEQ(new Decimal('77298476544997406115741457621.48100777755241170668034650809').toSignificantDigits(4, 1).valueOf(), '77290000000000000000000000000');
    arktest.assertEQ(new Decimal('2862358127844547877974880690522726982.49409754').toSignificantDigits(33, 2).valueOf(), '2862358127844547877974880690522730000');
    arktest.assertEQ(new Decimal('901843956540041489372610614345.487929674').toSignificantDigits(26, 5).valueOf(), '901843956540041489372610610000');
    arktest.assertEQ(new Decimal('45031671217607058672836306.4835113355599').toSignificantDigits(34, 4).valueOf(), '45031671217607058672836306.48351134');
    arktest.assertEQ(new Decimal('92083948494159391284091144794664446.380059732506918919702007475113945').toSignificantDigits(58, 3).valueOf(), '92083948494159391284091144794664446.380059732506918919702');
    arktest.assertEQ(new Decimal('177322371367658020851649147602.975655295218030854293908400956').toSignificantDigits(24, 3).valueOf(), '177322371367658020851649000000');
    arktest.assertEQ(new Decimal('4239542600712473433498612753.2106014432176399255536577373285045425339').toSignificantDigits(51, 4).valueOf(), '4239542600712473433498612753.21060144321763992555366');
    arktest.assertEQ(new Decimal('989142489.56454046202042259').toSignificantDigits(29, 6).valueOf(), '989142489.56454046202042259');
    arktest.assertEQ(new Decimal('39188324760962.641710').toSignificantDigits(19, 0).valueOf(), '39188324760962.64171');
    arktest.assertEQ(new Decimal('1784374158866.4741261545314002975498').toSignificantDigits(11, 1).valueOf(), '1784374158800');
    arktest.assertEQ(new Decimal('4889335896817404.6714320041572203348794').toSignificantDigits(32, 3).valueOf(), '4889335896817404.6714320041572203');
    arktest.assertEQ(new Decimal('3179414685592371949442250485606819.26696750990296429734141944').toSignificantDigits(12, 6).valueOf(), '3179414685590000000000000000000000');
    arktest.assertEQ(new Decimal('2903.263500440').toSignificantDigits(13, 3).valueOf(), '2903.26350044');
    arktest.assertEQ(new Decimal('1491343.7894456317757614464191181515356911640598').toSignificantDigits(34, 3).valueOf(), '1491343.789445631775761446419118151');
    arktest.assertEQ(new Decimal('5500628286601049.8436636261819485793447808823573910').toSignificantDigits(31, 4).valueOf(), '5500628286601049.843663626181949');
    arktest.assertEQ(new Decimal('3974711265332457176491911497563.9361').toSignificantDigits(17, 4).valueOf(), '3974711265332457200000000000000');
    arktest.assertEQ(new Decimal('93.281054276673210359').toSignificantDigits(19, 2).valueOf(), '93.28105427667321036');
    arktest.assertEQ(new Decimal('79910228655846918061589185.101353368484158631096929283677512827019').toSignificantDigits(21, 0).valueOf(), '79910228655846918061600000');
    arktest.assertEQ(new Decimal('4389523222041915395738636382677027049670.297472800121808897652436').toSignificantDigits(36, 0).valueOf(), '4389523222041915395738636382677027050000');
    arktest.assertEQ(new Decimal('2894245566479613750113978122489420.56862841281958031877868147223357225').toSignificantDigits(53, 4).valueOf(), '2894245566479613750113978122489420.5686284128195803188');
}

function testToSignificantDigitsWithSpecificConfig014() {
    arktest.assertEQ(new Decimal('494724446222447272134436118293973049042208245586819785140130787553487687286873291327585520984443687211379965781311260864429223913901690493449434874.049').toSignificantDigits(136, 1).valueOf(), '494724446222447272134436118293973049042208245586819785140130787553487687286873291327585520984443687211379965781311260864429223913901690400000000000');
    arktest.assertEQ(new Decimal('3720280055397389210963282787773692958098570405431182098.1847115996658646089352534779938945383741184042776917885782743812931817237846474528789801').toSignificantDigits(51, 0).valueOf(), '3720280055397389210963282787773692958098570405431190000');
    arktest.assertEQ(new Decimal('759188909745876678229347606524330959678969850255912440680257260789793508531411911105662225225721357883400623102.881476662045026352517268398904498921045685377855106763679514746737986172605517588709155551610890784878010336772838372711497238975980355571170655725075915881399899070771273619112223837705596667049149981193').toSignificantDigits(67, 0).valueOf(), '759188909745876678229347606524330959678969850255912440680257260789800000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('4534304542844843415257043214798844424046834211704366877715023555011146990876705027962738522155031711274213446497330677820779740785545273317321438460934917531081249158109061429290323393161606391411793.277725444395').toSignificantDigits(69, 5).valueOf(), '4534304542844843415257043214798844424046834211704366877715023555011150000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('527244430907269587847819161754.09994360681344184942353237241149292559482211118493881252182204930127002210488011149670127752911923929048735411637853031498542495512941893603181502188788354208091361643795263910287').toSignificantDigits(20, 5).valueOf(), '527244430907269587850000000000');
    arktest.assertEQ(new Decimal('500274199038784701912854356303168604036329678593776947367198603066622399541094534586945152898619439257327038632128821666524202122658068057432922418958129519085677022999535035083.95723140131476417422632523022711778228314707489949388264508533995136005795122359991').toSignificantDigits(30, 5).valueOf(), '500274199038784701912854356303000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('5333760730338237.35812806107026532469961610375448079660529271972427104505398437046671245161686878493124070786536064182194621929641261600482019485630475569955198761522647338125822382150219203146448476750498789080703982026782691017063636782481300').toSignificantDigits(70, 5).valueOf(), '5333760730338237.358128061070265324699616103754480796605292719724271045');
    arktest.assertEQ(new Decimal('9387762026.73267349404211844460662261461066947142829472609767844561464154020').toSignificantDigits(75, 4).valueOf(), '9387762026.7326734940421184446066226146106694714282947260976784456146415402');
    arktest.assertEQ(new Decimal('67323858.2474107285426298930616170460110072782018778697292704532242132189290371593478382802207909998627911041939764517795222755210476244995305187831767572511625381158475111177037691940475964756887806055996078204829400918467078858084732712280713071351617031612115767601030103857883944259837811618215851568286334400246721991754374286168046011298802236522').toSignificantDigits(40, 2).valueOf(), '67323858.24741072854262989306161704601101');
    arktest.assertEQ(new Decimal('618280971398998509245590744944289036083345801467546352624671931393813580337957902732550378613100649571499058004021009366633559571.2699009128705660239887951528338903748244427453200035937983818076678373616991455573').toSignificantDigits(50, 5).valueOf(), '618280971398998509245590744944289036083345801467550000000000000000000000000000000000000000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('3625212716653626154269212443601969756896707.15631640883708299351163687263190725535805204906165678662625271').toSignificantDigits(95, 2).valueOf(), '3625212716653626154269212443601969756896707.1563164088370829935116368726319072553580520490616568');
    arktest.assertEQ(new Decimal('279794024756694785000215326702581040764281424692.94416985247012017560069970214089414772133468917746186164351199574782564560451796356697926186392269112385101341672332415427389576261072927099738586782426824776935594024947823170180686').toSignificantDigits(218, 6).valueOf(), '279794024756694785000215326702581040764281424692.94416985247012017560069970214089414772133468917746186164351199574782564560451796356697926186392269112385101341672332415427389576261072927099738586782426824776935594024948');
    arktest.assertEQ(new Decimal('35828450460504270469704936980280835584645741698641855543514563095433541570683138929790029105719699910361577052023426681751310480362480069300311248293.15348414532497112250157766681332507724698060759080257429160605105296732683871024').toSignificantDigits(103, 6).valueOf(), '35828450460504270469704936980280835584645741698641855543514563095433541570683138929790029105719699910360000000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('43761815435821313568192740041070231887308670966477475362204961804611459661458966510873830168.716748692490194916680548288518654884099098683415405589451450981728769866802762326738670533948709724357936162347635031179866244').toSignificantDigits(176, 2).valueOf(), '43761815435821313568192740041070231887308670966477475362204961804611459661458966510873830168.716748692490194916680548288518654884099098683415405589451450981728769866802762326739');
    arktest.assertEQ(new Decimal('9940289055903627512125768245502992585069900170399503819515972524.13568735283721983801757702683853241247769318405229726113276035561474837566466532875876277432576610830142188269641845050335097656359864498101396326004092685742327518181101473104745549408201476216574093693733574369506676184899821529267868141813088352688031339966846243166921375083445250920').toSignificantDigits(47, 4).valueOf(), '9940289055903627512125768245502992585069900170400000000000000000');
    arktest.assertEQ(new Decimal('47308500490651000672647597245353550222048575.8072142842392016063606507543350997595316613513575837073421103918284314912676250088610064455343463233565051257974791197641171878251624763412467738389496245200149213715671349161').toSignificantDigits(59, 4).valueOf(), '47308500490651000672647597245353550222048575.807214284239202');
    arktest.assertEQ(new Decimal('7242493899945961184657078755114354.827569440819898501534597610800051201555022451526536260195848454753026282935514547388140').toSignificantDigits(104, 1).valueOf(), '7242493899945961184657078755114354.8275694408198985015345976108000512015550224515265362601958484547530262');
    arktest.assertEQ(new Decimal('4984463523458535754905264911455285850008993268231053980.767410232956686367696506915282296296709367816894555456665962274719887159413483764425930136761749360042065357118267648394706165913383308575864809558314678492797545419').toSignificantDigits(91, 0).valueOf(), '4984463523458535754905264911455285850008993268231053980.767410232956686367696506915282296297');
    arktest.assertEQ(new Decimal('468731093093191466545712621915352193328466285392879958862956252122614534959310655638837578478177944653136138911.41840552882713558913968005741142943').toSignificantDigits(84, 5).valueOf(), '468731093093191466545712621915352193328466285392879958862956252122614534959310655639000000000000000000000000000');
    arktest.assertEQ(new Decimal('8722796170843759397737441130128801601794238071659409548210220188790585826046201831648630668253198051544084586185006178347748234759.9434511722086056368943').toSignificantDigits(128, 3).valueOf(), '8722796170843759397737441130128801601794238071659409548210220188790585826046201831648630668253198051544084586185006178347748234700');
    arktest.assertEQ(new Decimal('67926499505419960439966855715456849140496804252323842323150939357112.5926126887602628389494013461282357363300181486827630507100652279617012866942377850016203733098433364051').toSignificantDigits(83, 3).valueOf(), '67926499505419960439966855715456849140496804252323842323150939357112.592612688760262');
    arktest.assertEQ(new Decimal('1402457858069260843701831542961702005338943536372264316099532808299709640182133143.009979906034715196177295226439746913418590464152810027317068521003207924233662128607269663291833').toSignificantDigits(39, 1).valueOf(), '1402457858069260843701831542961702005330000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('1106350604097326705139313335803761878947015866282894852274609129769275668488439922176607333011405146481732245401758539986540.94882628447605933696571359709430425378075223612860671428378614749989787085552416655512628791179233').toSignificantDigits(183, 2).valueOf(), '1106350604097326705139313335803761878947015866282894852274609129769275668488439922176607333011405146481732245401758539986540.94882628447605933696571359709430425378075223612860671428379');
    arktest.assertEQ(new Decimal('9735483775685609464322320520902637418698700323093604913106065125437383414521799911105265912341567164193794949895999422938985317071738708834029942382911270457249378913787.1103832980594904520955978916205703025866141635300875243').toSignificantDigits(24, 1).valueOf(), '9735483775685609464322320000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('489529.683064034353387778259583221632556574514795517212567070880734204947495043604544481313566331313388535912869238239769635054162766247017293340447968973686886334804349955536713950306242814646600928789897172058572510482913298938731094634890755913526004290737129748224090241001770940').toSignificantDigits(95, 1).valueOf(), '489529.68306403435338777825958322163255657451479551721256707088073420494749504360454448131356633');
    arktest.assertEQ(new Decimal('5916066038388477.282777699215057097344308799257479415453402583912978373402137006855767835024785902898403750237821320238828483034980698804153055853498816858707539873771777970182477242160360615721826161461903035810812584828').toSignificantDigits(148, 5).valueOf(), '5916066038388477.282777699215057097344308799257479415453402583912978373402137006855767835024785902898403750237821320238828483034980698804153055853499');
    arktest.assertEQ(new Decimal('1492565201039912150941642914851.01886949776472714249189352099211297490645710215074707153310120372664663753678081131309106952434431280082529455546057203036546314199747521356134765285561227686693949347029078827406886675694947').toSignificantDigits(155, 6).valueOf(), '1492565201039912150941642914851.0188694977647271424918935209921129749064571021507470715331012037266466375367808113130910695243443128008252945554605720303655');
    arktest.assertEQ(new Decimal('9640.823515736412358055650895513069103270437767616274081289588691585664302621579615021666966091360826575410661596403164479300452152684973160363097309608066484458989787971253015176864265186897585966764740584557961889884222991340777882611717168551975010970695517417044829720982931484654856360759782530622').toSignificantDigits(154, 0).valueOf(), '9640.823515736412358055650895513069103270437767616274081289588691585664302621579615021666966091360826575410661596403164479300452152684973160363097309608067');
    arktest.assertEQ(new Decimal('2192917244999869343205916374337677.13670389941440829125766924482174747890075138055235242239241690213762402416630046739017177822').toSignificantDigits(99, 6).valueOf(), '2192917244999869343205916374337677.13670389941440829125766924482174747890075138055235242239241690214');
    arktest.assertEQ(new Decimal('63537288045407814614632100373927391371818274269554962.47539574376947926528280013008702010901972495291817972524796755262276716657735757278').toSignificantDigits(56, 2).valueOf(), '63537288045407814614632100373927391371818274269554962.476');
    arktest.assertEQ(new Decimal('2810271650889132939296.56710151819671988902086681773799853021233325407901168571657754153770180269049570864269302522698770485556146').toSignificantDigits(4, 0).valueOf(), '2811000000000000000000');
    arktest.assertEQ(new Decimal('315495111097618519547112164879699292983604105741729996616794447864849595569.4732429578938004170666887599581675624557787373713415172397346993864722937610311872942069612761272487587892003117667431497194318776589121114621644677670506783812384346194108237410689252765415071112547238246125156991556184611980318715999133743922164645414546327450499594633872455804056396617969187240487543621620').toSignificantDigits(69, 6).valueOf(), '315495111097618519547112164879699292983604105741729996616794447864850000000');
    arktest.assertEQ(new Decimal('204.054713812637763806802452').toSignificantDigits(28, 1).valueOf(), '204.054713812637763806802452');
}

function testToSignificantDigitsWithSpecificConfig015() {
    arktest.assertEQ(new Decimal('283401218864063396558389343623404646465639303535411612743365055.0226824948376888').toSignificantDigits(17, 6).valueOf(), '283401218864063400000000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('92229.6175832152848581295309179066678605862867934529122263631705230526731956188541578114356740475936695278089617909281727991815396972738914904573285717078598330120780900701772340175710598595012522421761264348230172003217').toSignificantDigits(103, 5).valueOf(), '92229.61758321528485812953091790666786058628679345291222636317052305267319561885415781143567404759366953');
    arktest.assertEQ(new Decimal('272730132423593904649945011230989.0744142799829675179643159659075079714180380184881796059492868547766243507357835076029870087842837823744527376015781771828747087898039912031366854731319283802355925346531872038489621488183273173863').toSignificantDigits(140, 2).valueOf(), '272730132423593904649945011230989.07441427998296751796431596590750797141803801848817960594928685477662435073578350760298700878428378237445274');
    arktest.assertEQ(new Decimal('834882716459088686085166110241864368.258556620963484419727257790577150207336809137411199524666254860110062671741122221715410199601367804869545194').toSignificantDigits(130, 2).valueOf(), '834882716459088686085166110241864368.2585566209634844197272577905771502073368091374111995246662548601100626717411222217154101996014');
    arktest.assertEQ(new Decimal('17069318170447559670824010313506648026528166586129880725091542343600550422997254786851016215940786230013064180480826097444101017117212949291335428804169206.4789414896747948034966909141270082719485508246318636726728954481').toSignificantDigits(125, 4).valueOf(), '17069318170447559670824010313506648026528166586129880725091542343600550422997254786851016215940786230013064180480826097444101000000000000000000000000000000');
    arktest.assertEQ(new Decimal('459159626088903852606631649784227594935229835299060973799218279323713336570937633103595944583645177507124617629953771475688119656568329467957028808451626080.2157052326772496024245144704394972923671967963191881122863487049025806804492486980444650765391761189341478929086845849828893436948962388818123422').toSignificantDigits(66, 1).valueOf(), '459159626088903852606631649784227594935229835299060973799218279323000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('8964893258158025627760261546686868902639753182992598513279568321959004014846384717934802529654264364024872333833301734962238301019.830652683').toSignificantDigits(45, 5).valueOf(), '8964893258158025627760261546686868902639753180000000000000000000000000000000000000000000000000000000000000000000000000000000000000');
}

function testToSignificantDigitsWithToExpNegAndToExpPosIsZero001() {
    Decimal.toExpNeg = 0;
    Decimal.toExpPos = 0;
    arktest.assertEQ(new Decimal('-1.1107056355595598537391E+949884').toSignificantDigits(18, 0).valueOf(), '-1.11070563555955986e+949884');
    arktest.assertEQ(new Decimal('6.310954382701729758393260889166675474531891695090516238E+4614231246519714').toSignificantDigits(6, 4).valueOf(), '6.31095e+4614231246519714');
    arktest.assertEQ(new Decimal('6.460903219125211021E+6677217159189').toSignificantDigits(9, 6).valueOf(), '6.46090322e+6677217159189');
    arktest.assertEQ(new Decimal('-3.95798433263891068927612875275729538E+483397').toSignificantDigits(36, 3).valueOf(), '-3.95798433263891068927612875275729538e+483397');
    arktest.assertEQ(new Decimal('-5.1495592729221424586275386661E+51785').toSignificantDigits(3, 0).valueOf(), '-5.15e+51785');
    arktest.assertEQ(new Decimal('-8.34129785438621486E+7759443961').toSignificantDigits(10, 2).valueOf(), '-8.341297854e+7759443961');
    arktest.assertEQ(new Decimal('-7.03903881158613994150693699720210776203561398962036350248E+648540530527').toSignificantDigits(7, 1).valueOf(), '-7.039038e+648540530527');
    arktest.assertEQ(new Decimal('8.860674096683951963219272593791310E+553520229219693').toSignificantDigits(18, 1).valueOf(), '8.86067409668395196e+553520229219693');
    arktest.assertEQ(new Decimal('9.8649879305169365348113E-490626048').toSignificantDigits(19, 1).valueOf(), '9.864987930516936534e-490626048');
    arktest.assertEQ(new Decimal('5.8917903635419392365831E+46492775').toSignificantDigits(17, 2).valueOf(), '5.8917903635419393e+46492775');
    arktest.assertEQ(new Decimal('3.4716518879894538148508E-7').toSignificantDigits(10, 6).valueOf(), '3.471651888e-7');
    arktest.assertEQ(new Decimal('3E+350468916').toSignificantDigits(1, 5).valueOf(), '3e+350468916');
    arktest.assertEQ(new Decimal('2.6468730222404974166287111790752920305E+187364576122710').toSignificantDigits(2, 4).valueOf(), '2.6e+187364576122710');
    arktest.assertEQ(new Decimal('-8.0817038755142757898362502198224521935080928E-7525724769').toSignificantDigits(27, 3).valueOf(), '-8.08170387551427578983625022e-7525724769');
    arktest.assertEQ(new Decimal('-5.820926E+137772541213403').toSignificantDigits(7, 1).valueOf(), '-5.820926e+137772541213403');
    arktest.assertEQ(new Decimal('5.82502531276069650637998468163265673623114802117E+8930012634273').toSignificantDigits(49, 0).valueOf(), '5.82502531276069650637998468163265673623114802117e+8930012634273');
    arktest.assertEQ(new Decimal('4.84406538101372598683728877E+6248').toSignificantDigits(22, 4).valueOf(), '4.844065381013725986837e+6248');
    arktest.assertEQ(new Decimal('-6.7242657973487816694742594494155083518497133669292619495E+7494242238106').toSignificantDigits(4, 1).valueOf(), '-6.724e+7494242238106');
    arktest.assertEQ(new Decimal('-9.44981616487963708105775536177061040387686718038130154486986E+8817051').toSignificantDigits(14, 3).valueOf(), '-9.4498161648797e+8817051');
    arktest.assertEQ(new Decimal('-7.7313723453361061E+7786945').toSignificantDigits(1, 1).valueOf(), '-7e+7786945');
    arktest.assertEQ(new Decimal('6.8917529279486620283885E+46766112525827').toSignificantDigits(7, 3).valueOf(), '6.891752e+46766112525827');
    arktest.assertEQ(new Decimal('-4.145441268822437756131965510163E-93725726753990').toSignificantDigits(1, 2).valueOf(), '-4e-93725726753990');
}

function testToSignificantDigitsWithToExpNegAndToExpPosIsZero002() {
    arktest.assertEQ(new Decimal('-1.118165921482066102466296293946894050652555175E+306023331171748').toSignificantDigits(3, 4).valueOf(), '-1.12e+306023331171748');
    arktest.assertEQ(new Decimal('-7.97933637481027805811243624604489711E+5501').toSignificantDigits(24, 2).valueOf(), '-7.97933637481027805811243e+5501');
    arktest.assertEQ(new Decimal('7.2100462602152007419973593468751658532E+398740640897153').toSignificantDigits(9, 4).valueOf(), '7.21004626e+398740640897153');
    arktest.assertEQ(new Decimal('-7.93600951E-43130').toSignificantDigits(4, 4).valueOf(), '-7.936e-43130');
    arktest.assertEQ(new Decimal('-1.80564432714468488631383756914281144946839361818525E+1359').toSignificantDigits(45, 6).valueOf(), '-1.80564432714468488631383756914281144946839362e+1359');
    arktest.assertEQ(new Decimal('-4.6285543240204252524099493990932424634348E+5815991268494').toSignificantDigits(16, 5).valueOf(), '-4.628554324020425e+5815991268494');
    arktest.assertEQ(new Decimal('-3.0E+15499').toSignificantDigits(5, 1).valueOf(), '-3e+15499');
    arktest.assertEQ(new Decimal('1.77529E+5442970395990').toSignificantDigits(6, 2).valueOf(), '1.77529e+5442970395990');
    arktest.assertEQ(new Decimal('-3.393352377819527802023568109492373656E+725647954123').toSignificantDigits(34, 1).valueOf(), '-3.393352377819527802023568109492373e+725647954123');
    arktest.assertEQ(new Decimal('-9.5111391392383063345115012182879189612993491E-868').toSignificantDigits(30, 5).valueOf(), '-9.51113913923830633451150121829e-868');
    arktest.assertEQ(new Decimal('-7.92283E+78175').toSignificantDigits(7, 5).valueOf(), '-7.92283e+78175');
    arktest.assertEQ(new Decimal('4.8416771331704368164342462331056980108405981E+30639158262').toSignificantDigits(28, 1).valueOf(), '4.841677133170436816434246233e+30639158262');
    arktest.assertEQ(new Decimal('4.45420164905655927661142848292149E+9325666044026').toSignificantDigits(2, 4).valueOf(), '4.5e+9325666044026');
    arktest.assertEQ(new Decimal('1.3393718841216445941247869301225022E+971860873519992').toSignificantDigits(2, 4).valueOf(), '1.3e+971860873519992');
    arktest.assertEQ(new Decimal('9.876242886531446435944603618104763457E+52605').toSignificantDigits(13, 1).valueOf(), '9.876242886531e+52605');
    arktest.assertEQ(new Decimal('6.330647937407025476696971741E+1768').toSignificantDigits(22, 2).valueOf(), '6.330647937407025476697e+1768');
    arktest.assertEQ(new Decimal('5.20689225056772082748E-3336').toSignificantDigits(17, 5).valueOf(), '5.2068922505677208e-3336');
    arktest.assertEQ(new Decimal('5.928089832984E+23046663').toSignificantDigits(7, 5).valueOf(), '5.92809e+23046663');
    arktest.assertEQ(new Decimal('-8.50136375387653962573285296809772512970778623629235925E+888365717321295').toSignificantDigits(11, 1).valueOf(), '-8.5013637538e+888365717321295');
    arktest.assertEQ(new Decimal('4.26475618712602071344874281748487391782E-62604').toSignificantDigits(27, 3).valueOf(), '4.26475618712602071344874281e-62604');
    arktest.assertEQ(new Decimal('0.9020713898414363807382133864186995').toSignificantDigits(25, 6).valueOf(), '9.020713898414363807382134e-1');
    arktest.assertEQ(new Decimal('7.0838E+2242265227092').toSignificantDigits(3, 0).valueOf(), '7.09e+2242265227092');
    arktest.assertEQ(new Decimal('-2.6239193037415483309716069E-459').toSignificantDigits(11, 2).valueOf(), '-2.6239193037e-459');
    arktest.assertEQ(new Decimal('20871405994227596829475710887.232303900189115941377511040').toSignificantDigits(18, 2).valueOf(), '2.08714059942275969e+28');
    arktest.assertEQ(new Decimal('-3.219846203541755078720574281120717190883E+66642478').toSignificantDigits(6, 2).valueOf(), '-3.21984e+66642478');
}

function testToSignificantDigitsWithToExpNegAndToExpPosIsZero003() {
    arktest.assertEQ(new Decimal('8.08261507044993075472382557081796361683467812687673683E-4976').toSignificantDigits(57, 0).valueOf(), '8.08261507044993075472382557081796361683467812687673683e-4976');
    arktest.assertEQ(new Decimal('5.51009648628502529901761683407913552').toSignificantDigits(23, 4).valueOf(), '5.5100964862850252990176e+0');
    arktest.assertEQ(new Decimal('1.712556251979341826880397489279E+623').toSignificantDigits(12, 3).valueOf(), '1.71255625197e+623');
    arktest.assertEQ(new Decimal('3.4816153525988774411598020196959004888E+9752693').toSignificantDigits(7, 3).valueOf(), '3.481615e+9752693');
    arktest.assertEQ(new Decimal('3.5098208203E+91098992').toSignificantDigits(7, 5).valueOf(), '3.509821e+91098992');
    arktest.assertEQ(new Decimal('9.8641537792703101567472055694976920744483687020935E-72452394265').toSignificantDigits(26, 2).valueOf(), '9.8641537792703101567472056e-72452394265');
    arktest.assertEQ(new Decimal('1.0279887207E+401293443271').toSignificantDigits(7, 4).valueOf(), '1.027989e+401293443271');
    arktest.assertEQ(new Decimal('4.7639E+44979245').toSignificantDigits(3, 4).valueOf(), '4.76e+44979245');
    arktest.assertEQ(new Decimal('-1.7306315078051716446467E+1059755325').toSignificantDigits(5, 4).valueOf(), '-1.7306e+1059755325');
    arktest.assertEQ(new Decimal('3.9456282632321519002673617475779957462749297912E+203687').toSignificantDigits(14, 4).valueOf(), '3.9456282632322e+203687');
    arktest.assertEQ(new Decimal('-8.53760630184147960E-534432343020843').toSignificantDigits(21, 2).valueOf(), '-8.5376063018414796e-534432343020843');
    arktest.assertEQ(new Decimal('8.652244144198672391205645713722739150532482412E+8606642533').toSignificantDigits(30, 2).valueOf(), '8.65224414419867239120564571373e+8606642533');
    arktest.assertEQ(new Decimal('-6173578265.071').toSignificantDigits(4, 4).valueOf(), '-6.174e+9');
    arktest.assertEQ(new Decimal('67930796.2462334092807242196421600').toSignificantDigits(22, 5).valueOf(), '6.793079624623340928072e+7');
    arktest.assertEQ(new Decimal('-2.36373930494803297524865796700721024881625741597833295816323E+838379').toSignificantDigits(36, 1).valueOf(), '-2.36373930494803297524865796700721024e+838379');
    arktest.assertEQ(new Decimal('-9E+5483611650120').toSignificantDigits(4, 3).valueOf(), '-9e+5483611650120');
    arktest.assertEQ(new Decimal('14349628.01922235388049374147').toSignificantDigits(2, 0).valueOf(), '1.5e+7');
    arktest.assertEQ(new Decimal('6.8614E+74393698866798').toSignificantDigits(8, 1).valueOf(), '6.8614e+74393698866798');
    arktest.assertEQ(new Decimal('-1.8311132335332706692286996219681430E+8406934').toSignificantDigits(29, 6).valueOf(), '-1.831113233533270669228699622e+8406934');
    arktest.assertEQ(new Decimal('771.45201840169939335428188371247934080056704899').toSignificantDigits(40, 2).valueOf(), '7.714520184016993933542818837124793408006e+2');
    arktest.assertEQ(new Decimal('-1.0732125537673403265457667040389769358981173800952578892E+6795813709').toSignificantDigits(3, 5).valueOf(), '-1.07e+6795813709');
    arktest.assertEQ(new Decimal('4.91401973E+7112074044').toSignificantDigits(7, 0).valueOf(), '4.91402e+7112074044');
    arktest.assertEQ(new Decimal('8.841940724E+76239602607').toSignificantDigits(3, 6).valueOf(), '8.84e+76239602607');
    arktest.assertEQ(new Decimal('4.794257702528707921549614769559147705541286916517125838578E+8949077983').toSignificantDigits(46, 1).valueOf(), '4.794257702528707921549614769559147705541286916e+8949077983');
    arktest.assertEQ(new Decimal('-3.14814925E+153331637489').toSignificantDigits(6, 5).valueOf(), '-3.14815e+153331637489');
}

function testToSignificantDigitsWithToExpNegAndToExpPosIsZero004() {
    arktest.assertEQ(new Decimal('6.04477481096739E-222583722').toSignificantDigits(18, 4).valueOf(), '6.04477481096739e-222583722');
    arktest.assertEQ(new Decimal('9.205025903021276187181944366817198E+96711').toSignificantDigits(36, 1).valueOf(), '9.205025903021276187181944366817198e+96711');
    arktest.assertEQ(new Decimal('9.4809482152213319440512616212233329955405089941555240223386E+351074489623').toSignificantDigits(33, 0).valueOf(), '9.48094821522133194405126162122334e+351074489623');
    arktest.assertEQ(new Decimal('3.97483400914941619365023958147427495495628532069146310E+3139').toSignificantDigits(55, 4).valueOf(), '3.9748340091494161936502395814742749549562853206914631e+3139');
}
