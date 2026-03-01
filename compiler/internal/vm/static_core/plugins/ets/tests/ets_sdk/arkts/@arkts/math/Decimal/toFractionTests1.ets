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
    const suite = new arktest.ArkTestsuite("Decimal toFraction API tests");
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig001);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig002);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig003);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig004);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig005);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig006);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig007);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig008);
    suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig009);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig010);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig011);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig012);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig013);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig014);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig015);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig016);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig017);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig018);
    // suite.addTest("Decimal toFraction with specific config", testToExponentialWithSpecificConfig019);
    return suite.run();
}

function testToExponentialWithSpecificConfig001() {
    arktest.assertEQ(new Decimal('0.1').toFraction().toString(), '1,10');
    arktest.assertEQ(new Decimal('-0.1').toFraction().toString(), '-1,10');
    arktest.assertEQ(new Decimal('0.01').toFraction().toString(), '1,100');
    arktest.assertEQ(new Decimal('-0.01').toFraction().toString(), '-1,100');
    arktest.assertEQ(new Decimal('0.001').toFraction().toString(), '1,1000');
    arktest.assertEQ(new Decimal('-0.001').toFraction().toString(), '-1,1000');
    arktest.assertEQ(new Decimal('543.017930').toFraction().toString(), '54301793,100000');
}

function testToExponentialWithSpecificConfig002() {
    arktest.assertEQ(new Decimal('-484693350.1482515744490').toFraction().toString(), '-484693350148251574449,1000000000000');
    arktest.assertEQ(new Decimal('-8004442981.622759206381160').toFraction().toString(), '-200111074540568980159529,25000000000000');
    arktest.assertEQ(new Decimal('-5.965700213104829').toFraction().toString(), '-5965700213104829,1000000000000000');
    arktest.assertEQ(new Decimal('-51241102097.39113904928179').toFraction().toString(), '-5124110209739113904928179,100000000000000');
    arktest.assertEQ(new Decimal('-8275085912667.602978324187705').toFraction().toString(), '-1655017182533520595664837541,200000000000000');
    arktest.assertEQ(new Decimal('-0.625').toFraction().toString(), '-5,8');
    arktest.assertEQ(new Decimal('-0.211973085').toFraction().toString(), '-42394617,200000000');
    arktest.assertEQ(new Decimal('-58.9814832695853167316').toFraction().toString(), '-147453708173963291829,2500000000000000000');
    arktest.assertEQ(new Decimal('-27.33').toFraction().toString(), '-2733,100');
    arktest.assertEQ(new Decimal('0.0').toFraction().toString(), '0,1');
}

function testToExponentialWithSpecificConfig003() {
    arktest.assertEQ(new Decimal('-259989755186639236.5731491641').toFraction().toString(), '-2599897551866392365731491641,10000000000');
    arktest.assertEQ(new Decimal('-1865.2662461530146742').toFraction().toString(), '-9326331230765073371,5000000000000000');
    arktest.assertEQ(new Decimal('-248.3061').toFraction().toString(), '-2483061,10000');
    arktest.assertEQ(new Decimal('-9286.4854181').toFraction().toString(), '-92864854181,10000000');
    arktest.assertEQ(new Decimal('-7.5851685022821519300').toFraction().toString(), '-758516850228215193,100000000000000000');
    arktest.assertEQ(new Decimal('-978520.404277979450').toFraction().toString(), '-19570408085559589,20000000000');
    arktest.assertEQ(new Decimal('-2.11').toFraction().toString(), '-211,100');
    arktest.assertEQ(new Decimal('-478.9334439841').toFraction().toString(), '-4789334439841,10000000000');
}

function testToExponentialWithSpecificConfig004() {
    arktest.assertEQ(new Decimal('0.2978320663077').toFraction().toString(), '2978320663077,10000000000000');
    arktest.assertEQ(new Decimal('-48.84').toFraction().toString(), '-1221,25');
    arktest.assertEQ(new Decimal('2491.25081072026620').toFraction().toString(), '12456254053601331,5000000000000');
    arktest.assertEQ(new Decimal('319.69983').toFraction().toString(), '31969983,100000');
    arktest.assertEQ(new Decimal('0.0796322528725').toFraction().toString(), '31852901149,400000000000');
    arktest.assertEQ(new Decimal('-45.6674099059').toFraction().toString(), '-456674099059,10000000000');
    arktest.assertEQ(new Decimal('-72.3815574').toFraction().toString(), '-361907787,5000000');
    arktest.assertEQ(new Decimal('-65.21').toFraction().toString(), '-6521,100');
}

function testToExponentialWithSpecificConfig005() {
    arktest.assertEQ(new Decimal('5.1582612935891').toFraction('3').toString(), '5,1');
    arktest.assertEQ(new Decimal('8.14969395596340').toFraction('4682').toString(), '14645,1797');
    arktest.assertEQ(new Decimal('4.28004634702').toFraction('82418').toString(), '350921,81990');
    arktest.assertEQ(new Decimal('9237545981131491025317993727057.58450549').toFraction().toString(), '923754598113149102531799372705758450549,100000000');
    arktest.assertEQ(new Decimal('478602688796297558085606261629921.9787246948249979233505266225354557384').toFraction('57466').toString(), '23665467152910525354658972818814752082,49447');
    arktest.assertEQ(new Decimal('6200562357067874056887067399198.66971421701').toFraction('2').toString(), '12401124714135748113774134798397,2');
    arktest.assertEQ(new Decimal('57474563252337328154609608649.83211610837351385715799969377').toFraction().toString(), '5747456325233732815460960864983211610837351385715799969377,100000000000000000000000000000');
    arktest.assertEQ(new Decimal('77.7705294446793765233206385561288346447').toFraction('86').toString(), '4744,61');
    arktest.assertEQ(new Decimal('2304066265154632547374984.87452583023685806305331639').toFraction().toString(), '230406626515463254737498487452583023685806305331639,100000000000000000000000000');
}

function testToExponentialWithSpecificConfig006() {
    arktest.assertEQ(new Decimal('1690563314483685903788823.9294').toFraction().toString(), '8452816572418429518944119647,5000');
    arktest.assertEQ(new Decimal('4823949947187.82944361297775810').toFraction().toString(), '48239499471878294436129777581,10000000000000000');
    arktest.assertEQ(new Decimal('607976237797612199239800166125309462.530995962577562').toFraction('89241').toString(), '45172634468362586403517152343110493066053,74300');
    arktest.assertEQ(new Decimal('196503207726.5013595186920201176903069645736').toFraction().toString(), '245629009658126699398365025147112883705717,1250000000000000000000000000000');
    arktest.assertEQ(new Decimal('39.150412162473168843124483511286812598').toFraction().toString(), '19575206081236584421562241755643406299,500000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('6216797667555655999484009912524.20481050135882289309385').toFraction().toString(), '124335953351113119989680198250484096210027176457861877,20000000000000000000000');
    arktest.assertEQ(new Decimal('623592869689166510218176940221.63661098254437249041482587').toFraction().toString(), '62359286968916651021817694022163661098254437249041482587,100000000000000000000000000');
    arktest.assertEQ(new Decimal('28125067840062442147004219.46').toFraction('90').toString(), '1406253392003122107350210973,50');
    arktest.assertEQ(new Decimal('736337826.0').toFraction('1').toString(), '736337826,1');
}

function testToExponentialWithSpecificConfig007() {
    arktest.assertEQ(new Decimal('759971.62007285863174441704674479534091075').toFraction('5946').toString(), '2294354321,3019');
    arktest.assertEQ(new Decimal('6851100853942420703719152.67578737093315792636847746').toFraction().toString(), '342555042697121035185957633789368546657896318423873,50000000000000000000000000');
    arktest.assertEQ(new Decimal('67530435688493088.2885316751682999639624370547426320').toFraction('73').toString(), '3511582655801640591,52');
    arktest.assertEQ(new Decimal('23813746213496766765469.63118').toFraction().toString(), '1190687310674838338273481559,50000');
    arktest.assertEQ(new Decimal('793697100207959259551935987.225181146039').toFraction('45').toString(), '31747884008318370382077439489,40');
    arktest.assertEQ(new Decimal('657598204949657212609163514.271348320970259397527780637349273932').toFraction('61325').toString(), '39451289109544785156061546711681,59993');
    arktest.assertEQ(new Decimal('59377524901417202631.7076617756627530296590420377861').toFraction('248').toString(), '3859539118592118171061,65');
    arktest.assertEQ(new Decimal('68819152052184900414056.449443703377131320899467509').toFraction('59208').toString(), '2529860848590369124121129138,36761');
    arktest.assertEQ(new Decimal('8665536.3781112033855216846406980971689').toFraction('2').toString(), '17331073,2');
}

function testToExponentialWithSpecificConfig008() {
    arktest.assertEQ(new Decimal('443550442787.75516').toFraction().toString(), '11088761069693879,25000');
    arktest.assertEQ(new Decimal('78688106453900733169.80801275320205353359142143262532').toFraction('517').toString(), '37298162459148947522489,474');
    arktest.assertEQ(new Decimal('124.11782390212011049587005051032466579057').toFraction().toString(), '12411782390212011049587005051032466579057,100000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('5767557784726048420.48952939300603759767694').toFraction().toString(), '288377889236302421024476469650301879883847,50000000000000000000000');
    arktest.assertEQ(new Decimal('64269805556832146.8921574533060466484388410550211').toFraction('2').toString(), '64269805556832147,1');
    arktest.assertEQ(new Decimal('509714111520840110.44623631764611004359625363775303233110').toFraction('2').toString(), '1019428223041680221,2');
    arktest.assertEQ(new Decimal('-26786975055.08927411782417555').toFraction().toString(), '-535739501101785482356483511,20000000000000000');
    arktest.assertEQ(new Decimal('-4.378004735466266').toFraction().toString(), '-2189002367733133,500000000000000');
    arktest.assertEQ(new Decimal('-4.135862533810979636019397675E+30').toFraction().toString(), '-4135862533810979636019397675000,1');
}

function testToExponentialWithSpecificConfig009() {
    arktest.assertEQ(new Decimal('-14984.699501047789').toFraction().toString(), '-14984699501047789,1000000000000');
    arktest.assertEQ(new Decimal('-264421342663174686266193.1503').toFraction().toString(), '-2644213426631746862661931503,10000');
    arktest.assertEQ(new Decimal('-418459851831499482818072203.4').toFraction().toString(), '-2092299259157497414090361017,5');
    arktest.assertEQ(new Decimal('-0.77').toFraction().toString(), '-77,100');
    arktest.assertEQ(new Decimal('-54.276151887').toFraction().toString(), '-54276151887,1000000000');
    arktest.assertEQ(new Decimal('-155873431650.7715545510526124').toFraction().toString(), '-389683579126928886377631531,2500000000000000');
    arktest.assertEQ(new Decimal('4678972616180711652.4384304297700033991397937633588752307').toFraction().toString(), '46789726161807116524384304297700033991397937633588752307,10000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('-919685803476294304529729.1137').toFraction().toString(), '-9196858034762943045297291137,10000');
    arktest.assertEQ(new Decimal('-510.31276').toFraction().toString(), '-12757819,25000');
    arktest.assertEQ(new Decimal('-55031602193293990566454389.46').toFraction().toString(), '-2751580109664699528322719473,50');
}

function testToExponentialWithSpecificConfig010() {
    arktest.assertEQ(new Decimal('-6985335369519331.161674806985').toFraction().toString(), '-1397067073903866232334961397,200000000000');
    arktest.assertEQ(new Decimal('-1.082730014296142061458358296E+70').toFraction().toString(), '-10827300142961420614583582960000000000000000000000000000000000000000000,1');
    arktest.assertEQ(new Decimal('9769.57491658').toFraction().toString(), '488478745829,50000000');
    arktest.assertEQ(new Decimal('-9.278814552584579630569660716').toFraction().toString(), '-2319703638146144907642415179,250000000000000000000000000');
    arktest.assertEQ(new Decimal('8.199609').toFraction().toString(), '8199609,1000000');
    arktest.assertEQ(new Decimal('-22650405638418062.97933719282').toFraction().toString(), '-1132520281920903148966859641,50000000000');
    arktest.assertEQ(new Decimal('-943537244.8691264711').toFraction().toString(), '-9435372448691264711,10000000000');
    arktest.assertEQ(new Decimal('-5.430432705270926972283794348E+42').toFraction().toString(), '-5430432705270926972283794348000000000000000,1');
    arktest.assertEQ(new Decimal('254.7106921').toFraction().toString(), '2547106921,10000000');
    arktest.assertEQ(new Decimal('-8053444243730075.893216120196').toFraction().toString(), '-2013361060932518973304030049,250000000000');
}

function testToExponentialWithSpecificConfig011() {
    arktest.assertEQ(new Decimal('-74767843302323687781923.63575').toFraction().toString(), '-299071373209294751127694543,4000');
    arktest.assertEQ(new Decimal('32.2522').toFraction().toString(), '161261,5000');
    arktest.assertEQ(new Decimal('-2149015945426180798371334522').toFraction().toString(), '-2149015945426180798371334522,1');
    arktest.assertEQ(new Decimal('-975242046181.3569384841892184').toFraction().toString(), '-1219052557726696173105236523,1250000000000000');
    arktest.assertEQ(new Decimal('-6635702255618430700913.831019').toFraction().toString(), '-6635702255618430700913831019,1000000');
    arktest.assertEQ(new Decimal('-42290924109879729530570068.73').toFraction().toString(), '-4229092410987972953057006873,100');
    arktest.assertEQ(new Decimal('8008767579580873470979426873.32875742929708200305858327085977609').toFraction().toString(), '800876757958087347097942687332875742929708200305858327085977609,100000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('-53309687933410961482738903.86').toFraction().toString(), '-2665484396670548074136945193,50');
    arktest.assertEQ(new Decimal('0.79581097').toFraction('74409').toString(), '30396,38195');
}

function testToExponentialWithSpecificConfig012() {
    arktest.assertEQ(new Decimal('646110.5424140842277760913716223149175043516912866283522989430448583852673477181054').toFraction('4314').toString(), '2125057574,3289');
    arktest.assertEQ(new Decimal('1140.688086121119279582816178812726653910').toFraction('37').toString(), '18251,16');
    arktest.assertEQ(new Decimal('30674382513540892238116444815711309458620542801128.559859020834676491532682063017825303001913079937413732204504474950978211332225018371420').toFraction('2').toString(), '61348765027081784476232889631422618917241085602257,2');
    arktest.assertEQ(new Decimal('9174193129457486826033417691391647830796882006449660.0558100148468970808679278895365813792370050009058167954469').toFraction('61710').toString(), '238354711696434965227174225040046402291933791409568617910,25981');
    arktest.assertEQ(new Decimal('919495974905217288703595469354435393822802085980318046767.1338999658963927824509929378965804777221331548975659545056488845295998946309871474144010438').toFraction('9').toString(), '7355967799241738309628763754835483150582416687842544374137,8');
    arktest.assertEQ(new Decimal('4547237716782743755943866989.8356560705677241346146759866583370949').toFraction('863202').toString(), '1125354937387110207464744146511518,247481');
    arktest.assertEQ(new Decimal('6684320.09797413893379904985072385250774250364224704531308514779353223').toFraction('14426').toString(), '58400904696,8737');
    arktest.assertEQ(new Decimal('9889359.28886025414277').toFraction('551989').toString(), '4765998700799,481932');
    arktest.assertEQ(new Decimal('84446319074008288318841993796319916.4290214679797552828327089099139280188').toFraction('1').toString(), '84446319074008288318841993796319916,1');
    arktest.assertEQ(new Decimal('7192411578339741566080.621098672624218880520368888346887640903578755437931806539999680994582938').toFraction('38').toString(), '208579935771852505416338,29');
}

function testToExponentialWithSpecificConfig013() {
    arktest.assertEQ(new Decimal('3005559325061644218561964167641260035161858920552843936039505077.70447752609278095294701366298104650604976387611568135143174113323780682').toFraction('7063').toString(), '1006862373895650813218257996159822111779222738385202718573234201031,335');
    arktest.assertEQ(new Decimal('41932.7940043445804168810482554121918927813650259').toFraction('895566').toString(), '12180512204,290477');
    arktest.assertEQ(new Decimal('53541607836.080957621939096312239677792878740188648258030024875').toFraction('31555').toString(), '972851014381591,18170');
    arktest.assertEQ(new Decimal('6501295322096.01707059843465672720153308').toFraction('88').toString(), '383576424003665,59');
    arktest.assertEQ(new Decimal('623290.372447884850674344473049816049238992052574671').toFraction('770999').toString(), '456059695649,731697');
    arktest.assertEQ(new Decimal('0.5716202415873544262986953173753852278285193332218646822626651').toFraction('2').toString(), '1,2');
    arktest.assertEQ(new Decimal('5470634478750193005701111314607953627282253109963257869896225013864965536493.782798186996737913044082377221550214034629772236938011947896897722075720890727218714').toFraction('2580').toString(), '13802410789886736953383903846755867001633124596437299605748175709981308048573814,2523');
    arktest.assertEQ(new Decimal('1720546690653497270570006270472130851.14006845543964920673232647491053082791301585407937041734157349507526701334664232654756777642845206').toFraction('1').toString(), '1720546690653497270570006270472130851,1');
    arktest.assertEQ(new Decimal('43917221872024973437243762385431.5005579559793624838126175850466341579997661657132448932942532299').toFraction('11453').toString(), '275492732803212658371830121443811803,6273');
    arktest.assertEQ(new Decimal('946380134802740069071108764912367186554505.02139414761736578247940104106009403877307019264568966335689948628094').toFraction('100').toString(), '88013352536654826423613115136850148349568967,93');
    arktest.assertEQ(new Decimal('0.16188799510329961').toFraction('2').toString(), '0,1');
    arktest.assertEQ(new Decimal('496155200156912588508379320070503930908190489069266499.364683462978985476107209780379101750930177884309470340386147403736530').toFraction('73').toString(), '31257777609885493076027897164441747647216000811363789460,63');
}

function testToExponentialWithSpecificConfig014() {
    arktest.assertEQ(new Decimal('-59838206470642867345.02864922').toFraction().toString(), '-2991910323532143367251432461,50000000');
    arktest.assertEQ(new Decimal('-9.727452008639720390120442885E+56').toFraction().toString(), '-972745200863972039012044288500000000000000000000000000000,1');
    arktest.assertEQ(new Decimal('-7.59').toFraction().toString(), '-759,100');
    arktest.assertEQ(new Decimal('-278272581514524949348632.0100').toFraction().toString(), '-27827258151452494934863201,100');
    arktest.assertEQ(new Decimal('-5692.53601618').toFraction().toString(), '-284626800809,50000000');
    arktest.assertEQ(new Decimal('-930270790.1030908648560428875').toFraction().toString(), '-74421663208247269188483431,80000000000000000');
    arktest.assertEQ(new Decimal('-1.082016448333999416859837516E+30').toFraction().toString(), '-1082016448333999416859837516000,1');
    arktest.assertEQ(new Decimal('-0.84').toFraction().toString(), '-21,25');
    arktest.assertEQ(new Decimal('-8405819271711162542.005901375').toFraction().toString(), '-67246554173689300336047211,8000000');
    arktest.assertEQ(new Decimal('-2.196812392790282979771359770E+39').toFraction().toString(), '-2196812392790282979771359770000000000000,1');
}

function testToExponentialWithSpecificConfig015() {
    arktest.assertEQ(new Decimal('-9.336763844064577288340288931E+31').toFraction().toString(), '-93367638440645772883402889310000,1');
    arktest.assertEQ(new Decimal('-3.598919971327247089389639764E+37').toFraction().toString(), '-35989199713272470893896397640000000000,1');
    arktest.assertEQ(new Decimal('-2.164121368433393987617956904E+28').toFraction().toString(), '-21641213684333939876179569040,1');
    arktest.assertEQ(new Decimal('-9.130912769213956601432703922E+64').toFraction().toString(), '-91309127692139566014327039220000000000000000000000000000000000000,1');
    arktest.assertEQ(new Decimal('-39828889546453238.24200113918').toFraction().toString(), '-1991444477322661912100056959,50000000000');
    arktest.assertEQ(new Decimal('-55.35158656167491485055927810').toFraction().toString(), '-553515865616749148505592781,10000000000000000000000000');
    arktest.assertEQ(new Decimal('-5473307660200784975387.242062').toFraction().toString(), '-2736653830100392487693621031,500000');
    arktest.assertEQ(new Decimal('-286982155661917456.2799154218').toFraction().toString(), '-1434910778309587281399577109,5000000000');
    arktest.assertEQ(new Decimal('0.7325643').toFraction().toString(), '7325643,10000000');
    arktest.assertEQ(new Decimal('81297534687091521.91211051855224273832281').toFraction().toString(), '8129753468709152191211051855224273832281,100000000000000000000000');
    arktest.assertEQ(new Decimal('4023214922452471497.25630571786057289667012386513324609').toFraction('72150416').toString(), '26846080372036394639591396,6672793');
    arktest.assertEQ(new Decimal('755847.328153061897408378264066684087805946605').toFraction().toString(), '151169465630612379481675652813336817561189321,200000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('48650152878444478499756261.104359').toFraction().toString(), '48650152878444478499756261104359,1000000');
}

function testToExponentialWithSpecificConfig016() {
    arktest.assertEQ(new Decimal('98942665638509321.1423434879780971949357099446690852').toFraction('4194429').toString(), '310291026486294288245548,3136069');
    arktest.assertEQ(new Decimal('9953232194581201656.11264913171363754779711915').toFraction('40003').toString(), '281148949800335203180214,28247');
    arktest.assertEQ(new Decimal('807099173.7').toFraction().toString(), '8070991737,10');
    arktest.assertEQ(new Decimal('842335058998719134097422.1715979897462986201981036').toFraction().toString(), '2105837647496797835243555428994974365746550495259,2500000000000000000000000');
    arktest.assertEQ(new Decimal('70505494067154490.07260719040610995681').toFraction().toString(), '7050549406715449007260719040610995681,100000000000000000000');
    arktest.assertEQ(new Decimal('14855216399811587657145.09531187624755829128849').toFraction().toString(), '1485521639981158765714509531187624755829128849,100000000000000000000000');
    arktest.assertEQ(new Decimal('451374367873296470301265441541162556657.33752912').toFraction('654').toString(), '179195624045698698709602380291841534992963,397');
    arktest.assertEQ(new Decimal('750455321520.1975303545477874468301224681956212').toFraction('6955').toString(), '60786881043136,81');
    arktest.assertEQ(new Decimal('489640170686365547452689.9639497075875189553277274425649764720088').toFraction('136').toString(), '54350058946186575767248586,111');
    arktest.assertEQ(new Decimal('82979325.34128477517548246164389960').toFraction().toString(), '207448313353211937938706154109749,2500000000000000000000000');
    arktest.assertEQ(new Decimal('931866401.54478486865').toFraction().toString(), '18637328030895697373,20000000000');
}

function testToExponentialWithSpecificConfig017() {
    arktest.assertEQ(new Decimal('7712723080.8472816894238711').toFraction().toString(), '77127230808472816894238711,10000000000000000');
    arktest.assertEQ(new Decimal('4160881936704043602173554309387122294.5709328').toFraction('7').toString(), '29126173556928305215214880165709856062,7');
    arktest.assertEQ(new Decimal('70518843391393.531604308619475952242596538499476346').toFraction('675048').toString(), '43333053556734019842,614489');
    arktest.assertEQ(new Decimal('95445.231562915140439462747444833608100863907').toFraction('1363996915').toString(), '53377793855957,559250504');
    arktest.assertEQ(new Decimal('299.9045833667611513140004251012084').toFraction('3206002475').toString(), '347448038601,1158528605');
    arktest.assertEQ(new Decimal('47235590253929.92686163722415183139').toFraction().toString(), '4723559025392992686163722415183139,100000000000000000000');
    arktest.assertEQ(new Decimal('98328691470714782625004163686209.3533896422549273776361277').toFraction('1811232885').toString(), '178091379773085237300450763077988415217238,1811184275');
    arktest.assertEQ(new Decimal('8055147248313052044208741856994.45105715750883').toFraction().toString(), '805514724831305204420874185699445105715750883,100000000000000');
    arktest.assertEQ(new Decimal('9.610016056348').toFraction('8529').toString(), '65819,6849');
}

function testToExponentialWithSpecificConfig018() {
    arktest.assertEQ(new Decimal('510670028720980.0871365544559472').toFraction().toString(), '319168767950612554460346534967,625000000000000');
    arktest.assertEQ(new Decimal('64170.7336772078034895641687140263').toFraction('92775').toString(), '5710296907,88986');
    arktest.assertEQ(new Decimal('1113176727893004996666617450271972.102427360371078853103050428478489704402').toFraction().toString(), '556588363946502498333308725135986051213680185539426551525214239244852201,500000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('666459967364703414254713245.421153056').toFraction().toString(), '20826873980146981695459788919411033,31250000');
    arktest.assertEQ(new Decimal('2499361049528080561.202511540068978284974386343138634241').toFraction().toString(), '2499361049528080561202511540068978284974386343138634241,1000000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('8365976145105381170540384726.81972262078895455779223713273658154').toFraction().toString(), '418298807255269058527019236340986131039447727889611856636829077,50000000000000000000000000000000000');
    arktest.assertEQ(new Decimal('69133242753235376897.24784638569909').toFraction().toString(), '6913324275323537689724784638569909,100000000000000');
    arktest.assertEQ(new Decimal('6981191730187792647202564832506918.8165').toFraction().toString(), '13962383460375585294405129665013837633,2000');
}

function testToExponentialWithSpecificConfig019() {
    arktest.assertEQ(new Decimal('123.45').toFraction().toString(), '2469,20');
    arktest.assertEQ(new Decimal('123.45').toFraction('2.1e1').toString(), '2469,20');
    arktest.assertEQ(new Decimal('123.45').toFraction(new Decimal(10)).toString(), '1111,9' );
    arktest.assertEQ(new Decimal('123.45').toFraction(new Decimal('123e399')).toString(), '2469,20');
    arktest.assertEQ(new Decimal('123.45').toFraction(new Decimal('123')).toString(), '2469,20');
}
