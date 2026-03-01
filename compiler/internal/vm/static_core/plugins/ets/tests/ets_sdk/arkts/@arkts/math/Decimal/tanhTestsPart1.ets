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
    const suite = new arktest.ArkTestsuite("Decimal tanh API tests");
    suite.addTest("Decimal tanh with specific config", testTanhhWithSpecificConfigPart001);
    suite.addTest("Decimal tanh with specific config", testTanhhWithSpecificConfigPart002);
    suite.addTest("Decimal tanh with specific config", testTanhhWithSpecificConfigPart003);
    return suite.run();
}

function checkTanhResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).tanh().valueOf(), expected);
}

function testTanhhWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkTanhResult('NaN', 40, 4, 'NaN');
    checkTanhResult('0', 20, 4, '0');
    checkTanhResult('-0', 20, 4, '-0');
    checkTanhResult('Infinity', 40, 4, '1');
    checkTanhResult('-Infinity', 40, 4, '-1');

    checkTanhResult('0.20269', 1, 0, '0.2');
    checkTanhResult('0.42', 6, 5, '0.39693');
    checkTanhResult('-0.35339119', 8, 1, '-0.33937958');
    checkTanhResult('0.153', 8, 1, '0.15181721');
    checkTanhResult('0.26', 1, 1, '0.2');
    checkTanhResult('-0.5', 4, 4, '-0.4621');
    checkTanhResult('0.5301167511', 10, 0, '0.4854703307');
    checkTanhResult('0.1', 6, 2, '0.099668');
    checkTanhResult('-0.1947', 8, 6, '-0.19227651');
    checkTanhResult('0.62055680437965736490569245685566214324448351639', 9, 3, '0.551515589');
    checkTanhResult('0.8653929782497126681556355800799811365862905422680464', 2, 5, '0.7');
    checkTanhResult('-0.223453445837184', 10, 6, '-0.2198071371');
    checkTanhResult('0.6228344', 3, 6, '0.553');
    checkTanhResult('0.3860621505234646348994570686682533147396620957827359578508068456', 2, 1, '0.36');
    checkTanhResult('0.5499', 3, 2, '0.501');
    checkTanhResult('0.27', 4, 0, '0.2637');
    checkTanhResult('-0.20874419', 6, 0, '-0.205765');
    checkTanhResult('-0.9', 9, 0, '-0.716297871');
    checkTanhResult('-0.66', 10, 0, '-0.5783634131');
    checkTanhResult('0.5', 8, 1, '0.46211715');
}

function testTanhhWithSpecificConfigPart002() {
    checkTanhResult('-0.8730255767917122199088529', 5, 1, '-0.7029');
    checkTanhResult('-0.3', 6, 4, '-0.291313');
    checkTanhResult('0.0772947', 5, 6, '0.077141');
    checkTanhResult('-0.1', 5, 4, '-0.099668');
    checkTanhResult('-0.5530719666927749554276959', 5, 2, '-0.50281');
    checkTanhResult('-0.76695591420216557012230781', 7, 0, '-0.6451559');
    checkTanhResult('-0.28931973768077305489831982152049', 1, 3, '-0.3');
    checkTanhResult('-0.631664433177866683720536966087102', 3, 5, '-0.559');
    checkTanhResult('0.4647093', 9, 2, '0.433914635');
    checkTanhResult('-0.293097', 2, 0, '-0.29');
    checkTanhResult('0.39968', 6, 2, '0.379676');
    checkTanhResult('-0.56478323116711527', 10, 5, '-0.5115177648');
    checkTanhResult('-0.60158', 2, 2, '-0.53');
    checkTanhResult('-0.566', 5, 6, '-0.51242');
    checkTanhResult('-0.5455', 1, 1, '-0.4');
    checkTanhResult('0.181743184255049315571704802510832099', 6, 5, '0.179768');
    checkTanhResult('-0.7', 2, 3, '-0.61');
    checkTanhResult('-0.410626', 8, 6, '-0.38900408');
    checkTanhResult('0.32', 10, 1, '0.3095069212');
    checkTanhResult('0.478', 9, 0, '0.444640449');
    checkTanhResult('-0.2127882653536975970432160655692', 1, 0, '-0.3');
    checkTanhResult('0.668', 9, 2, '0.583662747');
    checkTanhResult('0.1054319768894549379656717', 6, 5, '0.105043');
    checkTanhResult('-0.2159146041935965', 6, 3, '-0.212621');
    checkTanhResult('-0.53', 5, 6, '-0.48538');
}

function testTanhhWithSpecificConfigPart003() {
    checkTanhResult('0.35', 10, 1, '0.3363755443');
    checkTanhResult('0.3', 3, 2, '0.292');
    checkTanhResult('0.38276035385122733156525463', 1, 1, '0.3');
    checkTanhResult('-0.751550287803356896438162105272645', 31, 0, '-0.6360729215504306567971084126656');
    checkTanhResult('0.2958626290477698111067628542902641468424491987425491552317600599757584700042', 33, 4, '0.287521804057197325874867763687105');
    checkTanhResult('-0.1624241525', 60, 5, '-0.161010729348804314083016365256060397030839070169914629959771');
    checkTanhResult('-0.3246133', 23, 0, '-0.3136723151870226491541');
    checkTanhResult('0.7574510519751273', 52, 2, '0.6395731248592590736391239000170979980280431536354096');
    checkTanhResult('0.52677087', 9, 5, '0.482908861');
    checkTanhResult('0.01504298596871499648833185964254026', 19, 0, '0.01504185137182836531');
    checkTanhResult('-0.66543541401805503420777', 66, 4, '-0.581969286944420744706095110187298812957954793212156080645254407938');
    checkTanhResult('0.016497376645717697522865', 24, 0, '0.0164958801477289569077129');
    checkTanhResult('0.79228702880353249365', 45, 0, '0.659702681921377249169326566285194050648896939');
    checkTanhResult('-0.32047744', 35, 3, '-0.30993856125352144923141124282332793');
    checkTanhResult('0.78607526651121', 28, 5, '0.6561799353543745431928429623');
    checkTanhResult('-0.130782154855323731897567604029773075178369276247566438028657144457559876456195235141546572', 80, 3, '-0.13004158964612601126893011628900193563954656630020761998126577892190799298653838');
    checkTanhResult('0.543888568', 52, 5, '0.495925821522156920755419346170717806152690266078471');
    checkTanhResult('0.595946834719509046130665028731095130596', 61, 0, '0.5341591488247401589317435849855958281981025417248174269032896');
    checkTanhResult('0.8180710003422039739052425209159024776836992576782', 75, 5, '0.674018589934549659108491437885993668870523407722879795457767225797625110655');
    checkTanhResult('0.2002343534718092035', 32, 2, '0.19760053356302078332195382537244');
    checkTanhResult('0.4316', 73, 2, '0.4066575843743152347702788188048559212155195983052915894345749387848887185');
    checkTanhResult('0.0297026', 47, 0, '0.029693868096821512437865399347977108636469769826');
    checkTanhResult('0.53196297398345769364909690685747648864854530119854', 56, 1, '0.48688016788770163925717272861291678368256194265983517854');
    checkTanhResult('-0.86540495196', 57, 4, '-0.699031973057205236065847483268440159156292260431585544883');
    checkTanhResult('-0.0218982341955013979387277630591838547', 66, 6, '-0.021894734560604770532346703409220449766736745588134181835811694617');
}
