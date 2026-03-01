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

import { Decimal } from "@arkts.math.Decimal";

function testToStringDefault001() {
    arktest.assertEQ(new Decimal(0).toString(), '0');
    arktest.assertEQ(new Decimal('0').toString(), '0');
    arktest.assertEQ(new Decimal(NaN).toString(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').toString(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').toString(), 'Infinity');
    arktest.assertEQ(new Decimal(1).toString(), '1');
    arktest.assertEQ(new Decimal(9).toString(), '9');
    arktest.assertEQ(new Decimal(90).toString(), '90');
    arktest.assertEQ(new Decimal(90.12).toString(), '90.12');
    arktest.assertEQ(new Decimal(0.1).toString(), '0.1');
    arktest.assertEQ(new Decimal(0.01).toString(), '0.01');
    arktest.assertEQ(new Decimal(0.0123).toString(), '0.0123');
    arktest.assertEQ(new Decimal('111111111111111111111').toString(), '111111111111111111111');
    arktest.assertEQ(new Decimal('1111111111111111111111').toString(), '1111111111111111111111');
    arktest.assertEQ(new Decimal('11111111111111111111111').toString(), '11111111111111111111111');
    arktest.assertEQ(new Decimal(0.00001).toString(), '0.00001');
    arktest.assertEQ(new Decimal(0.000001).toString(), '0.000001');
}

function testToStringDefault002() {
    arktest.assertEQ(new Decimal(-0).toString(), '0');
    arktest.assertEQ(new Decimal('-0').toString(), '0');
    arktest.assertEQ(new Decimal('-Infinity').toString(), '-Infinity');
    arktest.assertEQ(new Decimal(-1).toString(), '-1');
    arktest.assertEQ(new Decimal(-9).toString(), '-9');
    arktest.assertEQ(new Decimal(-90).toString(), '-90');
    arktest.assertEQ(new Decimal(-90.12).toString(), '-90.12');
    arktest.assertEQ(new Decimal(-0.1).toString(), '-0.1');
    arktest.assertEQ(new Decimal(-0.01).toString(), '-0.01');
    arktest.assertEQ(new Decimal(-0.0123).toString(), '-0.0123');
    arktest.assertEQ(new Decimal( '-111111111111111111111').toString(), '-111111111111111111111');
    arktest.assertEQ(new Decimal('-1111111111111111111111').toString(), '-1111111111111111111111');
    arktest.assertEQ(new Decimal('-11111111111111111111111').toString(), '-11111111111111111111111');
    arktest.assertEQ(new Decimal(-0.00001).toString(), '-0.00001');
    arktest.assertEQ(new Decimal(-0.000001).toString(), '-0.000001');
}

function testToStringZeroExp001() {
    Decimal.toExpNeg = Decimal.toExpPos = 0;
    arktest.assertEQ(new Decimal(0.0000001).toString(), '1e-7');
    arktest.assertEQ(new Decimal(0.00000012).toString(), '1.2e-7');
    arktest.assertEQ(new Decimal(0.000000123).toString(), '1.23e-7');
    arktest.assertEQ(new Decimal(0.00000001).toString(), '1e-8');
    arktest.assertEQ(new Decimal(0.000000012).toString(), '1.2e-8');
    arktest.assertEQ(new Decimal(0.0000000123).toString(), '1.23e-8');
    arktest.assertEQ(new Decimal(-0.0000001).toString(), '-1e-7');
    arktest.assertEQ(new Decimal(-0.00000012).toString(), '-1.2e-7');
    arktest.assertEQ(new Decimal(-0.000000123).toString(), '-1.23e-7');
    arktest.assertEQ(new Decimal(-0.00000001).toString(), '-1e-8');
    arktest.assertEQ(new Decimal(-0.000000012).toString(), '-1.2e-8');
    arktest.assertEQ(new Decimal(-0.0000000123).toString(), '-1.23e-8');
}

function testToStringZeroExp002() {
    arktest.assertEQ(new Decimal('573447902457635.174479825134').toString(), '5.73447902457635174479825134e+14');
    arktest.assertEQ(new Decimal('10.7688').toString(), '1.07688e+1');
    arktest.assertEQ(new Decimal('3171194102379077141557759899.307946350455841').toString(), '3.171194102379077141557759899307946350455841e+27');
    arktest.assertEQ(new Decimal('49243534668981911776986533197425948906.34579').toString(), '4.924353466898191177698653319742594890634579e+37');
    arktest.assertEQ(new Decimal('6855582439265693973.28633907445409866949445343654692955').toString(), '6.85558243926569397328633907445409866949445343654692955e+18');
    arktest.assertEQ(new Decimal('1').toString(), '1e+0');
    arktest.assertEQ(new Decimal('21320000.803').toString(), '2.1320000803e+7');
    arktest.assertEQ(new Decimal('50878.741').toString(), '5.0878741e+4');
    arktest.assertEQ(new Decimal('519328982.88391').toString(), '5.1932898288391e+8');
    arktest.assertEQ(new Decimal('56906167781769560273078847259334103496043879156345.24').toString(), '5.690616778176956027307884725933410349604387915634524e+49');
    arktest.assertEQ(new Decimal('3250997805285754.572732466931476450').toString(), '3.25099780528575457273246693147645e+15');
    arktest.assertEQ(new Decimal('3').toString(), '3e+0');
    arktest.assertEQ(new Decimal('25811494197573.291905990947355226').toString(), '2.5811494197573291905990947355226e+13');
    arktest.assertEQ(new Decimal('56037225916983.3930471746454125').toString(), '5.60372259169833930471746454125e+13');
    arktest.assertEQ(new Decimal('12615810.663732236602461593613783').toString(), '1.2615810663732236602461593613783e+7');
    arktest.assertEQ(new Decimal('146543664492.66911507705477499035407722184965108377032').toString(), '1.4654366449266911507705477499035407722184965108377032e+11');
    arktest.assertEQ(new Decimal('649867355074489128571318329084239407570.0').toString(), '6.4986735507448912857131832908423940757e+38');
    arktest.assertEQ(new Decimal('3614.6989180120676857245474944').toString(), '3.6146989180120676857245474944e+3');
}

function testToStringZeroExp003() {
    arktest.assertEQ(new Decimal('99286.54762302286149994896625074').toString(), '9.928654762302286149994896625074e+4');
    arktest.assertEQ(new Decimal('3464.24170787806074650506079').toString(), '3.46424170787806074650506079e+3');
    arktest.assertEQ(new Decimal('1259343133553196664747525502046803.03068865719647').toString(), '1.25934313355319666474752550204680303068865719647e+33');
    arktest.assertEQ(new Decimal('1230141053376606511069780591.98916100450966081493207').toString(), '1.23014105337660651106978059198916100450966081493207e+27');
    arktest.assertEQ(new Decimal('138616471226716962499343.4287237').toString(), '1.386164712267169624993434287237e+23');
    arktest.assertEQ(new Decimal('2660.76369930322488334961932').toString(), '2.66076369930322488334961932e+3');
    arktest.assertEQ(new Decimal('93758.2568').toString(), '9.37582568e+4');
    arktest.assertEQ(new Decimal('13985364289472688399687574677.0529').toString(), '1.39853642894726883996875746770529e+28');
    arktest.assertEQ(new Decimal('319099.0').toString(), '3.19099e+5');
    arktest.assertEQ(new Decimal('30455710679878939630.3998723').toString(), '3.04557106798789396303998723e+19');
    arktest.assertEQ(new Decimal('1302461256.9115368830867934222004329653604418').toString(), '1.3024612569115368830867934222004329653604418e+9');
    arktest.assertEQ(new Decimal('235878.7483447172786').toString(), '2.358787483447172786e+5');
    arktest.assertEQ(new Decimal('51061444696531867454741670978.5208086304398889160563').toString(), '5.10614446965318674547416709785208086304398889160563e+28');
    arktest.assertEQ(new Decimal('146685947134456101512731.6115580').toString(), '1.46685947134456101512731611558e+23');
    arktest.assertEQ(new Decimal('36996010577134455415192825.6518906564810300119').toString(), '3.69960105771344554151928256518906564810300119e+25');
    arktest.assertEQ(new Decimal('26868315307.4628').toString(), '2.68683153074628e+10');
    arktest.assertEQ(new Decimal('2356565045684923.12232737219553793859212').toString(), '2.35656504568492312232737219553793859212e+15');
    arktest.assertEQ(new Decimal('775329244236121.5').toString(), '7.753292442361215e+14');
    arktest.assertEQ(new Decimal('1.56').toString(), '1.56e+0');
}

function testToStringZeroExp004() {
    arktest.assertEQ(new Decimal('1272818730367215461852.227991200704').toString(), '1.272818730367215461852227991200704e+21');
    arktest.assertEQ(new Decimal('11390070.0292988027871648046839423153789').toString(), '1.13900700292988027871648046839423153789e+7');
    arktest.assertEQ(new Decimal('3.3431').toString(), '3.3431e+0');
    arktest.assertEQ(new Decimal('145466549668194.02705').toString(), '1.4546654966819402705e+14');
    arktest.assertEQ(new Decimal('3053457353958055674247148914016675754664628301138.19').toString(), '3.05345735395805567424714891401667575466462830113819e+48');
    arktest.assertEQ(new Decimal('512.18945854639324441304933666460587').toString(), '5.1218945854639324441304933666460587e+2');
    arktest.assertEQ(new Decimal('995299.9008960').toString(), '9.95299900896e+5');
    arktest.assertEQ(new Decimal('1.21564537151562431991786620635').toString(), '1.21564537151562431991786620635e+0');
    arktest.assertEQ(new Decimal('40.16').toString(), '4.016e+1');
    arktest.assertEQ(new Decimal('18657032.6').toString(), '1.86570326e+7');
    arktest.assertEQ(new Decimal('133810.01727').toString(), '1.3381001727e+5');
    arktest.assertEQ(new Decimal('2639841700687.441886800225725227').toString(), '2.639841700687441886800225725227e+12');
    arktest.assertEQ(new Decimal('2.45').toString(), '2.45e+0');
    arktest.assertEQ(new Decimal('289.45').toString(), '2.8945e+2');
    arktest.assertEQ(new Decimal('1.23').toString(), '1.23e+0');
    arktest.assertEQ(new Decimal('1559806666149836070330006.415033').toString(), '1.559806666149836070330006415033e+24');
    arktest.assertEQ(new Decimal('3149.84566145310751826289711761375061645611777700983').toString(), '3.14984566145310751826289711761375061645611777700983e+3');
    arktest.assertEQ(new Decimal('309406.91333892283249774116223987').toString(), '3.0940691333892283249774116223987e+5');
}

function testToStringZeroExp005() {
    arktest.assertEQ(new Decimal('657276.6274013360381079275191108732606370177179594282').toString(), '6.572766274013360381079275191108732606370177179594282e+5');
    arktest.assertEQ(new Decimal('1470126.973337024').toString(), '1.470126973337024e+6');
    arktest.assertEQ(new Decimal('564.99').toString(), '5.6499e+2');
    arktest.assertEQ(new Decimal('28416297367859233303497847667.971781197616950846').toString(), '2.8416297367859233303497847667971781197616950846e+28');
    arktest.assertEQ(new Decimal('213649515681898365631024816255335383200511.63977').toString(), '2.1364951568189836563102481625533538320051163977e+41');
    arktest.assertEQ(new Decimal('87610861868753713708.0904679797').toString(), '8.76108618687537137080904679797e+19');
    arktest.assertEQ(new Decimal('6276835.73474251182408654509953633505286').toString(), '6.27683573474251182408654509953633505286e+6');
    arktest.assertEQ(new Decimal('8.91411').toString(), '8.91411e+0');
    arktest.assertEQ(new Decimal('90345428324109125783300211464.13119399').toString(), '9.034542832410912578330021146413119399e+28');
    arktest.assertEQ(new Decimal('7026094393430852002585511641212897686956.090955').toString(), '7.026094393430852002585511641212897686956090955e+39');
    arktest.assertEQ(new Decimal('188122210934.91505758565988678062').toString(), '1.8812221093491505758565988678062e+11');
    arktest.assertEQ(new Decimal('94355384924970501385802017349021810574680448.75').toString(), '9.435538492497050138580201734902181057468044875e+43');
    arktest.assertEQ(new Decimal('536793419620790391866.461').toString(), '5.36793419620790391866461e+20');
    arktest.assertEQ(new Decimal('231508926559040401256259959.2854156357726817712').toString(), '2.315089265590404012562599592854156357726817712e+26');
    arktest.assertEQ(new Decimal('749917074182888527.3030006066196546588710962').toString(), '7.499170741828885273030006066196546588710962e+17');
    arktest.assertEQ(new Decimal('339621.28305986').toString(), '3.3962128305986e+5');
}

function testToStringZeroExp006() {
    arktest.assertEQ(new Decimal('8179804565.10031304').toString(), '8.17980456510031304e+9');
    arktest.assertEQ(new Decimal('43945758768581.24185382').toString(), '4.394575876858124185382e+13');
    arktest.assertEQ(new Decimal('7881617323.629751701107428').toString(), '7.881617323629751701107428e+9');
    arktest.assertEQ(new Decimal('4.89').toString(), '4.89e+0');
    arktest.assertEQ(new Decimal('9852098946635208576857038817811940823561.23765').toString(), '9.85209894663520857685703881781194082356123765e+39');
    arktest.assertEQ(new Decimal('684932.9685').toString(), '6.849329685e+5');
    arktest.assertEQ(new Decimal('28262252.277815736355279617243060700437627773361').toString(), '2.8262252277815736355279617243060700437627773361e+7');
    arktest.assertEQ(new Decimal('1503736721.902').toString(), '1.503736721902e+9');
    arktest.assertEQ(new Decimal('265213505469049775997232325076980590625670.234690917845').toString(), '2.65213505469049775997232325076980590625670234690917845e+41');
    arktest.assertEQ(new Decimal('423.752645959719196604760963802412828187442060555521').toString(), '4.23752645959719196604760963802412828187442060555521e+2');
    arktest.assertEQ(new Decimal('9023159535576504097005203913521871601.640521009').toString(), '9.023159535576504097005203913521871601640521009e+36');
    arktest.assertEQ(new Decimal('469339457186380.276410136272120035011198438772754725').toString(), '4.69339457186380276410136272120035011198438772754725e+14');
    arktest.assertEQ(new Decimal('1281942.9130391792511503973184804508867728894').toString(), '1.2819429130391792511503973184804508867728894e+6');
    arktest.assertEQ(new Decimal('1977.8').toString(), '1.9778e+3');
    arktest.assertEQ(new Decimal('24566803598289376280246313067921853675726100.21').toString(), '2.456680359828937628024631306792185367572610021e+43');
    arktest.assertEQ(new Decimal('52.5389225018085571689046925802871155628').toString(), '5.25389225018085571689046925802871155628e+1');
    arktest.assertEQ(new Decimal('173370053.2107').toString(), '1.733700532107e+8');
    arktest.assertEQ(new Decimal('195610.99921').toString(), '1.9561099921e+5');
    arktest.assertEQ(new Decimal('334.09').toString(), '3.3409e+2');
}

function testToStringZeroExp007() {
    arktest.assertEQ(new Decimal('6.09858715556186').toString(), '6.09858715556186e+0');
    arktest.assertEQ(new Decimal('3206341068321063874823757.90792609337383007782520694').toString(), '3.20634106832106387482375790792609337383007782520694e+24');
    arktest.assertEQ(new Decimal('146347126003930100207.98881400').toString(), '1.46347126003930100207988814e+20');
    arktest.assertEQ(new Decimal('27177804497442101179955865615249870678071468.82').toString(), '2.717780449744210117995586561524987067807146882e+43');
    arktest.assertEQ(new Decimal('28675757263527037754017.0639151').toString(), '2.86757572635270377540170639151e+22');
    arktest.assertEQ(new Decimal('134883255415086.02487577920722101277063863557818').toString(), '1.3488325541508602487577920722101277063863557818e+14');
    arktest.assertEQ(new Decimal('19601373273643.6392').toString(), '1.96013732736436392e+13');
    arktest.assertEQ(new Decimal('479.8185890466').toString(), '4.798185890466e+2');
    arktest.assertEQ(new Decimal('1696622337138949329874242519485119916519.994150606').toString(), '1.696622337138949329874242519485119916519994150606e+39');
    arktest.assertEQ(new Decimal('55000057298497.0761183142593570950897913860587074643').toString(), '5.50000572984970761183142593570950897913860587074643e+13');
    arktest.assertEQ(new Decimal('49').toString(), '4.9e+1');
    arktest.assertEQ(new Decimal('2353405108.2447686661410').toString(), '2.353405108244768666141e+9');
    arktest.assertEQ(new Decimal('1.237978927714857736527530290155529').toString(), '1.237978927714857736527530290155529e+0');
    arktest.assertEQ(new Decimal('55.411301200').toString(), '5.54113012e+1');
    arktest.assertEQ(new Decimal('163970902313.1').toString(), '1.639709023131e+11');
    arktest.assertEQ(new Decimal('21532406012700120772.5970506357881').toString(), '2.15324060127001207725970506357881e+19');
    arktest.assertEQ(new Decimal('237353212182292976273161221409.5513784533409').toString(), '2.373532121822929762731612214095513784533409e+29');
    arktest.assertEQ(new Decimal('45883026736677354.807679611737881799804').toString(), '4.5883026736677354807679611737881799804e+16');
    arktest.assertEQ(new Decimal('2599671482034668932546.8319633061').toString(), '2.5996714820346689325468319633061e+21');
    arktest.assertEQ(new Decimal('8226419.280').toString(), '8.22641928e+6');
}

function testToStringZeroExp008() {
    arktest.assertEQ(new Decimal('256006014528479284199.702229871263269').toString(), '2.56006014528479284199702229871263269e+20');
    arktest.assertEQ(new Decimal('4301260132991159779386275.268219519443685').toString(), '4.301260132991159779386275268219519443685e+24');
    arktest.assertEQ(new Decimal('10527217903601656493308.88881').toString(), '1.052721790360165649330888881e+22');
    arktest.assertEQ(new Decimal('6.85257703973809064426443728').toString(), '6.85257703973809064426443728e+0');
    arktest.assertEQ(new Decimal('134120.6836').toString(), '1.341206836e+5');
    arktest.assertEQ(new Decimal('12936960838097459955801414.32072678134217648017629').toString(), '1.293696083809745995580141432072678134217648017629e+25');
    arktest.assertEQ(new Decimal('9818866111.83').toString(), '9.81886611183e+9');
    arktest.assertEQ(new Decimal('13').toString(), '1.3e+1');
    arktest.assertEQ(new Decimal('21.8521213416841175534202540526068340057495224337100').toString(), '2.185212134168411755342025405260683400574952243371e+1');
    arktest.assertEQ(new Decimal('50.9812942277266').toString(), '5.09812942277266e+1');
    arktest.assertEQ(new Decimal('11584122815047345945090459318707335999.3').toString(), '1.15841228150473459450904593187073359993e+37');
    arktest.assertEQ(new Decimal('29.46408').toString(), '2.946408e+1');
    arktest.assertEQ(new Decimal('7884325.3757257').toString(), '7.8843253757257e+6');
    arktest.assertEQ(new Decimal('41498295.32631829').toString(), '4.149829532631829e+7');
    arktest.assertEQ(new Decimal('97684440694466341543678251889467593158113516184.5733').toString(), '9.76844406944663415436782518894675931581135161845733e+46');
    arktest.assertEQ(new Decimal('13206341.09357604978481').toString(), '1.320634109357604978481e+7');
    arktest.assertEQ(new Decimal('123001170.44692162331376535732386421937').toString(), '1.2300117044692162331376535732386421937e+8');
    arktest.assertEQ(new Decimal('1793438222395303915587960015783941548469515.11735').toString(), '1.79343822239530391558796001578394154846951511735e+42');
    arktest.assertEQ(new Decimal('34622733.5968923941657647562338569').toString(), '3.46227335968923941657647562338569e+7');
    arktest.assertEQ(new Decimal('3608190113362925223.4652167').toString(), '3.6081901133629252234652167e+18');
}

function testToStringZeroExp009() {
    arktest.assertEQ(new Decimal('3417696145772103538342831.68068494').toString(), '3.41769614577210353834283168068494e+24');
    arktest.assertEQ(new Decimal('103669389.5').toString(), '1.036693895e+8');
    arktest.assertEQ(new Decimal('9840862048026534392868878603.161623504069221701').toString(), '9.840862048026534392868878603161623504069221701e+27');
    arktest.assertEQ(new Decimal('25643721123.86050').toString(), '2.56437211238605e+10');
    arktest.assertEQ(new Decimal('2645333.616435501').toString(), '2.645333616435501e+6');
    arktest.assertEQ(new Decimal('37.5834254646606787747061360998').toString(), '3.75834254646606787747061360998e+1');
    arktest.assertEQ(new Decimal('1215821.01247').toString(), '1.21582101247e+6');
    arktest.assertEQ(new Decimal('54.3').toString(), '5.43e+1');
    arktest.assertEQ(new Decimal('3146138040302845775365414203.2015').toString(), '3.1461380403028457753654142032015e+27');
    arktest.assertEQ(new Decimal('27303.9').toString(), '2.73039e+4');
    arktest.assertEQ(new Decimal('33491120770003982037357624.170').toString(), '3.349112077000398203735762417e+25');
    arktest.assertEQ(new Decimal('22939124755279469099609636986027.54526495697363').toString(), '2.293912475527946909960963698602754526495697363e+31');
    arktest.assertEQ(new Decimal('780057836.8').toString(), '7.800578368e+8');
    arktest.assertEQ(new Decimal('35033042650468351705005130834.32983735273').toString(), '3.503304265046835170500513083432983735273e+28');
    arktest.assertEQ(new Decimal('6521027589.563589728').toString(), '6.521027589563589728e+9');
    arktest.assertEQ(new Decimal('1266.04818273232').toString(), '1.26604818273232e+3');
    arktest.assertEQ(new Decimal('45844253.800756959854340115').toString(), '4.5844253800756959854340115e+7');
    arktest.assertEQ(new Decimal('251038877046091582159.79351198183').toString(), '2.5103887704609158215979351198183e+20');
    arktest.assertEQ(new Decimal('651707650180890013.98157674630438543').toString(), '6.5170765018089001398157674630438543e+17');
    arktest.assertEQ(new Decimal('7856796596557626379.41070216699747').toString(), '7.85679659655762637941070216699747e+18');
    arktest.assertEQ(new Decimal('6551137558348495871.45').toString(), '6.55113755834849587145e+18');
}

function testToStringZeroExp010() {
    arktest.assertEQ(new Decimal('137856413555592382324487860882977.704999616').toString(), '1.37856413555592382324487860882977704999616e+32');
    arktest.assertEQ(new Decimal('751530.486314140193').toString(), '7.51530486314140193e+5');
    arktest.assertEQ(new Decimal('13712642.4612295900110').toString(), '1.3712642461229590011e+7');
    arktest.assertEQ(new Decimal('894.5222111405724').toString(), '8.945222111405724e+2');
    arktest.assertEQ(new Decimal('174501389497524149414213254563953197394499747.444317').toString(), '1.74501389497524149414213254563953197394499747444317e+44');
    arktest.assertEQ(new Decimal('715832940418.45987824307132').toString(), '7.1583294041845987824307132e+11');
    arktest.assertEQ(new Decimal('12820079237037.83860923747442697572540049').toString(), '1.282007923703783860923747442697572540049e+13');
    arktest.assertEQ(new Decimal('566062517479.3381639446229222').toString(), '5.660625174793381639446229222e+11');
    arktest.assertEQ(new Decimal('20947.45267').toString(), '2.094745267e+4');
    arktest.assertEQ(new Decimal('84497877437844.6866210974502183131911750').toString(), '8.4497877437844686621097450218313191175e+13');
    arktest.assertEQ(new Decimal('170721710519742548800049.3702652714920318758323999364').toString(), '1.707217105197425488000493702652714920318758323999364e+23');
    arktest.assertEQ(new Decimal('2548743481407894811266791880125633535.3406914111636153').toString(), '2.5487434814078948112667918801256335353406914111636153e+36');
    arktest.assertEQ(new Decimal('797594425.5792483246376368330364').toString(), '7.975944255792483246376368330364e+8');
    arktest.assertEQ(new Decimal('110387100511276924654.53332862048').toString(), '1.1038710051127692465453332862048e+20');
    arktest.assertEQ(new Decimal('20214122542287.381656860062564183697682').toString(), '2.0214122542287381656860062564183697682e+13');
    arktest.assertEQ(new Decimal('78530.12025112').toString(), '7.853012025112e+4');
    arktest.assertEQ(new Decimal('7970.61651146928').toString(), '7.97061651146928e+3');
    arktest.assertEQ(new Decimal('15712956919.727392305361179349388').toString(), '1.5712956919727392305361179349388e+10');
    arktest.assertEQ(new Decimal('8248.0059').toString(), '8.2480059e+3');
}

function testToStringZeroExp011() {
    arktest.assertEQ(new Decimal('27192914642259123187.7279340940781443961397491').toString(), '2.71929146422591231877279340940781443961397491e+19');
    arktest.assertEQ(new Decimal('213.10').toString(), '2.131e+2');
    arktest.assertEQ(new Decimal('5534432.99017925367').toString(), '5.53443299017925367e+6');
    arktest.assertEQ(new Decimal('905996364.53').toString(), '9.0599636453e+8');
    arktest.assertEQ(new Decimal('2.86841011915001378943').toString(), '2.86841011915001378943e+0');
    arktest.assertEQ(new Decimal('35069191603464263520176796586.6239485795145676493').toString(), '3.50691916034642635201767965866239485795145676493e+28');
    arktest.assertEQ(new Decimal('1202682354165614270.283968131411421292910').toString(), '1.20268235416561427028396813141142129291e+18');
    arktest.assertEQ(new Decimal('48878.7290').toString(), '4.8878729e+4');
    arktest.assertEQ(new Decimal('90434.4910891').toString(), '9.04344910891e+4');
    arktest.assertEQ(new Decimal('387.232843764031').toString(), '3.87232843764031e+2');
    arktest.assertEQ(new Decimal('224621220135.33430').toString(), '2.246212201353343e+11');
    arktest.assertEQ(new Decimal('49291.6234816086643529027167741689023').toString(), '4.92916234816086643529027167741689023e+4');
    arktest.assertEQ(new Decimal('21773818234639052045922.630496').toString(), '2.1773818234639052045922630496e+22');
    arktest.assertEQ(new Decimal('4.787053626838151258480').toString(), '4.78705362683815125848e+0');
    arktest.assertEQ(new Decimal('27007620786984368467551987190.05').toString(), '2.700762078698436846755198719005e+28');
    arktest.assertEQ(new Decimal('3048979019986645132403593585746645256.51334171').toString(), '3.04897901998664513240359358574664525651334171e+36');
    arktest.assertEQ(new Decimal('3807300704307638993.582').toString(), '3.807300704307638993582e+18');
    arktest.assertEQ(new Decimal('7984.6840795076707340124614425632613353204').toString(), '7.9846840795076707340124614425632613353204e+3');
    arktest.assertEQ(new Decimal('66991.8558928').toString(), '6.69918558928e+4');
    arktest.assertEQ(new Decimal('51.1').toString(), '5.11e+1');
}

function testToStringZeroExp012() {
    arktest.assertEQ(new Decimal('18').toString(), '1.8e+1');
    arktest.assertEQ(new Decimal('262.9676971').toString(), '2.629676971e+2');
    arktest.assertEQ(new Decimal('6840204896776721228035449367237234736980.0788279670097').toString(), '6.8402048967767212280354493672372347369800788279670097e+39');
    arktest.assertEQ(new Decimal('4684145127602661593941.009299096573092581').toString(), '4.684145127602661593941009299096573092581e+21');
    arktest.assertEQ(new Decimal('1301013360035531356.4757759338788842').toString(), '1.3010133600355313564757759338788842e+18');
    arktest.assertEQ(new Decimal('1585279741139347329933729792401.70059').toString(), '1.58527974113934732993372979240170059e+30');
    arktest.assertEQ(new Decimal('182.49338722142728063286').toString(), '1.8249338722142728063286e+2');
    arktest.assertEQ(new Decimal('1.59410').toString(), '1.5941e+0');
    arktest.assertEQ(new Decimal('203375468381700824735399263269.26478252219475').toString(), '2.0337546838170082473539926326926478252219475e+29');
    arktest.assertEQ(new Decimal('4626.173').toString(), '4.626173e+3');
    arktest.assertEQ(new Decimal('6.04857').toString(), '6.04857e+0');
    arktest.assertEQ(new Decimal('9.55039030589038069713466331').toString(), '9.55039030589038069713466331e+0');
    arktest.assertEQ(new Decimal('3139778.42').toString(), '3.13977842e+6');
    arktest.assertEQ(new Decimal('44046554138.751').toString(), '4.4046554138751e+10');
    arktest.assertEQ(new Decimal('22713308806233554444100296.5096').toString(), '2.27133088062335544441002965096e+25');
    arktest.assertEQ(new Decimal('17289.5143').toString(), '1.72895143e+4');
    arktest.assertEQ(new Decimal('259665963383099309886.5197298367570').toString(), '2.59665963383099309886519729836757e+20');
    arktest.assertEQ(new Decimal('5428043754043010253177042.70939778493719619031067122726').toString(), '5.42804375404301025317704270939778493719619031067122726e+24');
    arktest.assertEQ(new Decimal('405.26054186532354119711729303068171063825508').toString(), '4.0526054186532354119711729303068171063825508e+2');
    arktest.assertEQ(new Decimal('1269602673.94273418410687782475849').toString(), '1.26960267394273418410687782475849e+9');
    arktest.assertEQ(new Decimal('56572062.12494798707700546288522044895183104747814298').toString(), '5.657206212494798707700546288522044895183104747814298e+7');
    arktest.assertEQ(new Decimal('4808346640474051961043.20584899449259286').toString(), '4.80834664047405196104320584899449259286e+21');
    arktest.assertEQ(new Decimal('5663929473368.7553228095').toString(), '5.6639294733687553228095e+12');
}

function testToStringZeroExp013() {
    arktest.assertEQ(new Decimal('308469899797006019074738182718775120203832280.4110').toString(), '3.08469899797006019074738182718775120203832280411e+44');
    arktest.assertEQ(new Decimal('673240183308911151638.82850963905830707247414517739').toString(), '6.7324018330891115163882850963905830707247414517739e+20');
    arktest.assertEQ(new Decimal('78').toString(), '7.8e+1');
    arktest.assertEQ(new Decimal('37511.576').toString(), '3.7511576e+4');
    arktest.assertEQ(new Decimal('497447374459220078725.59411117007').toString(), '4.9744737445922007872559411117007e+20');
    arktest.assertEQ(new Decimal('142.64855114407053894401398660016825255242638071603').toString(), '1.4264855114407053894401398660016825255242638071603e+2');
    arktest.assertEQ(new Decimal('17.972').toString(), '1.7972e+1');
    arktest.assertEQ(new Decimal('1082230759095.51421423442320791403363148864').toString(), '1.08223075909551421423442320791403363148864e+12');
    arktest.assertEQ(new Decimal('12799203232872856.8').toString(), '1.27992032328728568e+16');
    arktest.assertEQ(new Decimal('3234109987.56876789482263723951851692122375679').toString(), '3.23410998756876789482263723951851692122375679e+9');
    arktest.assertEQ(new Decimal('830905.8187826413886051933894555524364').toString(), '8.309058187826413886051933894555524364e+5');
    arktest.assertEQ(new Decimal('59126904485324084868266487306831291316268.437628598').toString(), '5.9126904485324084868266487306831291316268437628598e+40');
    arktest.assertEQ(new Decimal('35799182834122518.16470339246643').toString(), '3.579918283412251816470339246643e+16');
    arktest.assertEQ(new Decimal('82.92403288').toString(), '8.292403288e+1');
    arktest.assertEQ(new Decimal('739.431').toString(), '7.39431e+2');
    arktest.assertEQ(new Decimal('388.0259').toString(), '3.880259e+2');
    arktest.assertEQ(new Decimal('8356612898392420429.137009991722851').toString(), '8.356612898392420429137009991722851e+18');
    arktest.assertEQ(new Decimal('53959.03069').toString(), '5.395903069e+4');
    arktest.assertEQ(new Decimal('2084656696443.025813').toString(), '2.084656696443025813e+12');
    arktest.assertEQ(new Decimal('75467142063950722840810167300704265146277488188.8').toString(), '7.54671420639507228408101673007042651462774881888e+46');
    arktest.assertEQ(new Decimal('69').toString(), '6.9e+1');
}

function testToStringZeroExp014() {
    arktest.assertEQ(new Decimal('892.10').toString(), '8.921e+2');
    arktest.assertEQ(new Decimal('25119.6408').toString(), '2.51196408e+4');
    arktest.assertEQ(new Decimal('4250232599216602.7236666111862782').toString(), '4.2502325992166027236666111862782e+15');
    arktest.assertEQ(new Decimal('148181688637265610.577148846528720697801886').toString(), '1.48181688637265610577148846528720697801886e+17');
    arktest.assertEQ(new Decimal('6.578849006577789439801702').toString(), '6.578849006577789439801702e+0');
    arktest.assertEQ(new Decimal('43469056251469272131329.90652').toString(), '4.346905625146927213132990652e+22');
    arktest.assertEQ(new Decimal('5.5037893').toString(), '5.5037893e+0');
    arktest.assertEQ(new Decimal('2477316752674381.20023934691987').toString(), '2.47731675267438120023934691987e+15');
    arktest.assertEQ(new Decimal('3374873666.52276710575333974697197457').toString(), '3.37487366652276710575333974697197457e+9');
    arktest.assertEQ(new Decimal('1.0385387059229329627650733271').toString(), '1.0385387059229329627650733271e+0');
    arktest.assertEQ(new Decimal('983004944893186643.985967600066862369294062').toString(), '9.83004944893186643985967600066862369294062e+17');
    arktest.assertEQ(new Decimal('424581658127100743607231740.518633563667839856744').toString(), '4.24581658127100743607231740518633563667839856744e+26');
    arktest.assertEQ(new Decimal('314.222253457223322124561584676953981561133').toString(), '3.14222253457223322124561584676953981561133e+2');
    arktest.assertEQ(new Decimal('1155383314189149962356632226550269509644739078674.8').toString(), '1.1553833141891499623566322265502695096447390786748e+48');
    arktest.assertEQ(new Decimal('52.811').toString(), '5.2811e+1');
    arktest.assertEQ(new Decimal('992.5202445922857021945001443270353221818473047344').toString(), '9.925202445922857021945001443270353221818473047344e+2');
    arktest.assertEQ(new Decimal('15758.9410').toString(), '1.5758941e+4');
    arktest.assertEQ(new Decimal('6663001032856498005948891635876.7').toString(), '6.6630010328564980059488916358767e+30');
    arktest.assertEQ(new Decimal('149898377473153728100588907982263779724221092.732531').toString(), '1.49898377473153728100588907982263779724221092732531e+44');
    arktest.assertEQ(new Decimal('417523890818561653685.5').toString(), '4.175238908185616536855e+20');
}

function testToStringZeroExp015() {
    arktest.assertEQ(new Decimal('1192838736272799853174.021036238').toString(), '1.192838736272799853174021036238e+21');
    arktest.assertEQ(new Decimal('1145.038').toString(), '1.145038e+3');
    arktest.assertEQ(new Decimal('4097378662.6728889384598402998014750474268').toString(), '4.0973786626728889384598402998014750474268e+9');
    arktest.assertEQ(new Decimal('55038.104').toString(), '5.5038104e+4');
    arktest.assertEQ(new Decimal('683.895535917805849194871290958068199407518').toString(), '6.83895535917805849194871290958068199407518e+2');
    arktest.assertEQ(new Decimal('2.9716066182').toString(), '2.9716066182e+0');
    arktest.assertEQ(new Decimal('1').toString(), '1e+0');
    arktest.assertEQ(new Decimal('1.780634284813842592053313582311179350').toString(), '1.78063428481384259205331358231117935e+0');
    arktest.assertEQ(new Decimal('62777149761034257128377.1900').toString(), '6.277714976103425712837719e+22');
    arktest.assertEQ(new Decimal('137376909692642287134486582.232200547809845780076').toString(), '1.37376909692642287134486582232200547809845780076e+26');
    arktest.assertEQ(new Decimal('70255659498942180.908195').toString(), '7.0255659498942180908195e+16');
    arktest.assertEQ(new Decimal('1367584.12477').toString(), '1.36758412477e+6');
    arktest.assertEQ(new Decimal('289930165413233926392919.54727329719281958174').toString(), '2.8993016541323392639291954727329719281958174e+23');
    arktest.assertEQ(new Decimal('2.44').toString(), '2.44e+0');
    arktest.assertEQ(new Decimal('53987037407321267528605819634290402730400.8232').toString(), '5.39870374073212675286058196342904027304008232e+40');
    arktest.assertEQ(new Decimal('6450716065.4825').toString(), '6.4507160654825e+9');
    arktest.assertEQ(new Decimal('1216.64').toString(), '1.21664e+3');
    arktest.assertEQ(new Decimal('3.13108416362400780440861428855476299376486503').toString(), '3.13108416362400780440861428855476299376486503e+0');
    arktest.assertEQ(new Decimal('729604995293362211.98242592384915903149558006256202995').toString(), '7.2960499529336221198242592384915903149558006256202995e+17');
    arktest.assertEQ(new Decimal('5.67239328846178836850536139916737284448973').toString(), '5.67239328846178836850536139916737284448973e+0');
    arktest.assertEQ(new Decimal('7200.203059575197430640').toString(), '7.20020305957519743064e+3');
}

function testToStringZeroExp016() {
    arktest.assertEQ(new Decimal('1851154237800640737150325457.90701546649748120114').toString(), '1.85115423780064073715032545790701546649748120114e+27');
    arktest.assertEQ(new Decimal('1250212508367788933866.87012660759710902').toString(), '1.25021250836778893386687012660759710902e+21');
    arktest.assertEQ(new Decimal('23323707491301665555664068.5374833558659806110').toString(), '2.3323707491301665555664068537483355865980611e+25');
    arktest.assertEQ(new Decimal('25088.131581298507401113299236').toString(), '2.5088131581298507401113299236e+4');
    arktest.assertEQ(new Decimal('961232685056394315.5774866').toString(), '9.612326850563943155774866e+17');
    arktest.assertEQ(new Decimal('15411451717624829715428.9225338049499367447824').toString(), '1.54114517176248297154289225338049499367447824e+22');
    arktest.assertEQ(new Decimal('5.22').toString(), '5.22e+0');
    arktest.assertEQ(new Decimal('4046983054763095337838.97').toString(), '4.04698305476309533783897e+21');
    arktest.assertEQ(new Decimal('2620876536774240989563272.117908814902188002596311').toString(), '2.620876536774240989563272117908814902188002596311e+24');
    arktest.assertEQ(new Decimal('17290754650.750439926458970782158').toString(), '1.7290754650750439926458970782158e+10');
    arktest.assertEQ(new Decimal('8570789.332248').toString(), '8.570789332248e+6');
    arktest.assertEQ(new Decimal('12.1').toString(), '1.21e+1');
    arktest.assertEQ(new Decimal('9749134061639126502181192178140679940393318673.720443').toString(), '9.749134061639126502181192178140679940393318673720443e+45');
    arktest.assertEQ(new Decimal('126878.0').toString(), '1.26878e+5');
    arktest.assertEQ(new Decimal('1239159984.1950849289559651456348').toString(), '1.2391599841950849289559651456348e+9');
    arktest.assertEQ(new Decimal('17222011842766272461428.9256133342842086').toString(), '1.72220118427662724614289256133342842086e+22');
    arktest.assertEQ(new Decimal('1512063585971680.294584184272035496').toString(), '1.512063585971680294584184272035496e+15');
}

function config() {
    Decimal.precision = 20;
    Decimal.rounding = 4;
    Decimal.toExpNeg = -9e15;
    Decimal.toExpPos = 9e15;
    Decimal.minE = -9e15;
    Decimal.maxE = 9e15;
}

function main() {
    const suite = new arktest.ArkTestsuite("Decimal toString API tests");
    suite.addTest("Decimal toString use default", testToStringDefault001);
    suite.addTest("Decimal toString use default", testToStringDefault002);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp001);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp002);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp003);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp004);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp005);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp006);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp007);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp008);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp009);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp010);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp011);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp012);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp013);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp014);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp015);
    suite.addTest("Decimal toString toExpNeg and toExpPos is 0", testToStringZeroExp016);
    config();
    return suite.run();
}
