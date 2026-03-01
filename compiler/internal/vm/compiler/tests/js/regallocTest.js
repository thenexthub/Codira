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


function func(num) {
  if (num > 0) {
    num = 0;
  } else {
    num += 1;
  }
  return num;
}

function func1(num1, num2) {
  const num = 1;
  try {
    if (num1 > num2) {
      num1 += num;
      console.log('succes.');
    }
    else {
      throw new Error('the number is low.');
    }
  }
  catch (err) {
    console.log('error message: ' + err);
  }
}

function func2(x, y) {
  var car = ["B", "V", "p", "F", "A"];
  var text = "";
  var i;
  for (i = 0; i < 5; i++) {
    text += car[i] + x + y;
  }
  return text;
}

function func3(a, b, c, d, e) {
  return a + b + c + d + e;
}

function func4(x, y) {
  var a = x + y;
  var b = x - y;
  var c = x * y;
  var d = x / y;
  var e = x % y;
  return func3(e, d, c, b, a);
}

function func5() {
  try {
    try {
      a = 1;
    } catch (e) {
      a;
    }
    if (a > 0) {
      a += 1;
    } else {
      throw new Error('the number is low.');
    }
  } catch (e) {
    print(e);
  }
}

function allocPhysical() {
  let v1 = 1;
  print(v1);
}

function allocFp(f) {
  let f1 = 1.5;
  let f2 = -3.4;
  print(f1 + f2);
  print(f2 * f);
}

function presplit(a) {
  let v1 = a;
  let v2 = a + v1;
  let v3 = v1 + v2;
  print(v1);
  print(v2);
  print(v3);
}

function split1(a) {
  let v1 = a + 1;
  let v2 = v1 * a;
  print(v1);
  print(v2);
}

function split2(a, b) {
  print(a, b);
}

function split3(a) {
  let v = 2;
  if (a > 0) {
    v = v * a;
  } else {
    v = v / a;
  }
  print(v, a);
}

function stat(a) {
  let v1 = a + 2;
  let f1 = a + 0.5;
  print(v1);
  print(f1);
}

function manyRegisters(a) {
  let v1 = a;
  let v2 = v1 + v1;
  let v3 = v1 + v2;
  let v4 = v2 + v3;
  let v5 = v1 + v2;
  let v6 = v4 + v5;
  let v7 = v1 + v6;
  let v8 = v4 + v6;
  let v9 = v1 + v3;
  let v10 = v8 + v2;
  let v11 = v10 + v1;
  let v12 = v10 + v9;
  let v13 = v11 + v10;
  let v14 = v9 + v13;
  let v15 = v10 + v8;
  let v16 = v12 + v7;
  let v17 = v7 + v1;
  let v18 = v11 + v10;
  let v19 = v10 + v8;
  let v20 = v17 + v8;
  let v21 = v11 + v8;
  let v22 = v3 + v21;
  let v23 = v13 + v20;
  let v24 = v3 + v20;
  let v25 = v19 + v23;
  let v26 = v20 + v25;
  let v27 = v10 + v21;
  let v28 = v12 + v7;
  let v29 = v10 + v13;
  let v30 = v18 + v7;
  let v31 = v30 + v1;
  let v32 = v30 + v20;
  let v33 = v1 + v28;
  let v34 = v17 + v19;
  let v35 = v11 + v6;
  let v36 = v30 + v4;
  let v37 = v32 + v21;
  let v38 = v22 + v34;
  let v39 = v24 + v25;
  let v40 = v16 + v32;
  let v41 = v10 + v10;
  let v42 = v16 + v6;
  let v43 = v38 + v36;
  let v44 = v33 + v14;
  let v45 = v33 + v37;
  let v46 = v23 + v16;
  let v47 = v16 + v42;
  let v48 = v11 + v16;
  let v49 = v7 + v42;
  let v50 = v32 + v28;
  let v51 = v2 + v41;
  let v52 = v6 + v3;
  let v53 = v46 + v37;
  let v54 = v7 + v11;
  let v55 = v38 + v33;
  let v56 = v40 + v31;
  let v57 = v13 + v12;
  let v58 = v35 + v1;
  let v59 = v50 + v58;
  let v60 = v13 + v45;
  let v61 = v7 + v27;
  let v62 = v23 + v22;
  let v63 = v33 + v59;
  let v64 = v9 + v6;
  let v65 = v43 + v44;
  let v66 = v51 + v65;
  let v67 = v20 + v11;
  let v68 = v36 + v55;
  let v69 = v10 + v21;
  let v70 = v49 + v31;
  let v71 = v37 + v14;
  let v72 = v48 + v65;
  let v73 = v7 + v31;
  let v74 = v41 + v13;
  let v75 = v33 + v27;
  let v76 = v10 + v8;
  let v77 = v70 + v72;
  let v78 = v30 + v26;
  let v79 = v10 + v49;
  let v80 = v75 + v59;
  let v81 = v33 + v51;
  let v82 = v20 + v34;
  let v83 = v5 + v67;
  let v84 = v76 + v17;
  let v85 = v62 + v72;
  let v86 = v54 + v77;
  let v87 = v44 + v39;
  let v88 = v41 + v21;
  let v89 = v69 + v48;
  let v90 = v7 + v48;
  let v91 = v32 + v58;
  let v92 = v22 + v56;
  let v93 = v60 + v41;
  let v94 = v59 + v58;
  let v95 = v57 + v14;
  let v96 = v60 + v42;
  let v97 = v51 + v52;
  let v98 = v76 + v93;
  let v99 = v83 + v93;
  let v100 = v4 + v80;
  let v101 = v8 + v86;
  let v102 = v9 + v74;
  let v103 = v65 + v88;
  let v104 = v40 + v31;
  let v105 = v35 + v43;
  let v106 = v26 + v1;
  let v107 = v93 + v71;
  let v108 = v87 + v91;
  let v109 = v6 + v87;
  let v110 = v77 + v99;
  let v111 = v46 + v75;
  let v112 = v48 + v95;
  let v113 = v76 + v109;
  let v114 = v58 + v75;
  let v115 = v63 + v3;
  let v116 = v76 + v102;
  let v117 = v22 + v11;
  let v118 = v23 + v7;
  let v119 = v36 + v70;
  let v120 = v23 + v69;
  let v121 = v14 + v21;
  let v122 = v119 + v1;
  let v123 = v34 + v80;
  let v124 = v3 + v14;
  let v125 = v41 + v77;
  let v126 = v68 + v67;
  let v127 = v124 + v61;
  let v128 = v44 + v23;
  let v129 = v109 + v115;
  let v130 = v120 + v93;
  let v131 = v19 + v16;
  let v132 = v94 + v3;
  let v133 = v82 + v100;
  let v134 = v38 + v54;
  let v135 = v22 + v60;
  let v136 = v56 + v17;
  let v137 = v98 + v105;
  let v138 = v48 + v136;
  let v139 = v75 + v107;
  let v140 = v4 + v60;
  let v141 = v37 + v113;
  let v142 = v2 + v17;
  let v143 = v76 + v85;
  let v144 = v64 + v55;
  let v145 = v23 + v104;
  let v146 = v101 + v23;
  let v147 = v96 + v35;
  let v148 = v111 + v138;
  let v149 = v83 + v83;
  let v150 = v139 + v72;
  let v151 = v75 + v85;
  let v152 = v101 + v140;
  let v153 = v116 + v119;
  let v154 = v7 + v19;
  let v155 = v27 + v93;
  let v156 = v80 + v6;
  let v157 = v53 + v118;
  let v158 = v1 + v157;
  let v159 = v75 + v118;
  let v160 = v144 + v96;
  let v161 = v78 + v71;
  let v162 = v57 + v85;
  let v163 = v96 + v120;
  let v164 = v28 + v58;
  let v165 = v142 + v46;
  let v166 = v33 + v162;
  let v167 = v5 + v142;
  let v168 = v84 + v63;
  let v169 = v20 + v118;
  let v170 = v20 + v84;
  let v171 = v145 + v164;
  let v172 = v59 + v150;
  let v173 = v7 + v46;
  let v174 = v73 + v62;
  let v175 = v148 + v60;
  let v176 = v26 + v45;
  let v177 = v169 + v101;
  let v178 = v116 + v29;
  let v179 = v159 + v37;
  let v180 = v119 + v176;
  let v181 = v168 + v95;
  let v182 = v180 + v112;
  let v183 = v31 + v79;
  let v184 = v85 + v72;
  let v185 = v119 + v61;
  let v186 = v95 + v150;
  let v187 = v48 + v53;
  let v188 = v172 + v172;
  let v189 = v141 + v163;
  let v190 = v119 + v53;
  let v191 = v179 + v67;
  let v192 = v123 + v35;
  let v193 = v95 + v48;
  let v194 = v70 + v51;
  let v195 = v121 + v103;
  let v196 = v135 + v62;
  let v197 = v48 + v1;
  let v198 = v180 + v158;
  let v199 = v88 + v45;
  let v200 = v180 + v182;
  let v201 = v59 + v17;
  let v202 = v167 + v131;
  let v203 = v41 + v193;
  let v204 = v182 + v79;
  let v205 = v188 + v201;
  let v206 = v20 + v87;
  let v207 = v120 + v157;
  let v208 = v131 + v173;
  let v209 = v14 + v90;
  let v210 = v35 + v203;
  let v211 = v94 + v37;
  let v212 = v183 + v82;
  let v213 = v81 + v11;
  let v214 = v129 + v106;
  let v215 = v118 + v17;
  let v216 = v206 + v59;
  let v217 = v35 + v207;
  let v218 = v200 + v71;
  let v219 = v25 + v150;
  let v220 = v22 + v103;
  let v221 = v189 + v209;
  let v222 = v127 + v199;
  let v223 = v178 + v86;
  let v224 = v102 + v133;
  let v225 = v218 + v72;
  let v226 = v22 + v5;
  let v227 = v11 + v72;
  let v228 = v174 + v31;
  let v229 = v199 + v64;
  let v230 = v58 + v163;
  let v231 = v81 + v174;
  let v232 = v39 + v75;
  let v233 = v51 + v208;
  let v234 = v194 + v38;
  let v235 = v179 + v129;
  let v236 = v197 + v178;
  let v237 = v114 + v40;
  let v238 = v16 + v7;
  let v239 = v176 + v92;
  let v240 = v139 + v15;
  let v241 = v166 + v89;
  let v242 = v132 + v158;
  let v243 = v160 + v174;
  let v244 = v150 + v185;
  let v245 = v136 + v24;
  let v246 = v32 + v113;
  let v247 = v169 + v35;
  let v248 = v91 + v143;
  let v249 = v194 + v19;
  let v250 = v178 + v42;
  let v251 = v28 + v179;
  let v252 = v60 + v245;
  let v253 = v88 + v118;
  let v254 = v176 + v97;
  let v255 = v166 + v46;
  let v256 = v162 + v143;
  let v257 = v93 + v105;
  let v258 = v99 + v233;
  let v259 = v75 + v137;
  let v260 = v145 + v183;
  print(v1);
  print(v2);
  print(v3);
  print(v4);
  print(v5);
  print(v6);
  print(v7);
  print(v8);
  print(v9);
  print(v10);
  print(v11);
  print(v12);
  print(v13);
  print(v14);
  print(v15);
  print(v16);
  print(v17);
  print(v18);
  print(v19);
  print(v20);
  print(v21);
  print(v22);
  print(v23);
  print(v24);
  print(v25);
  print(v26);
  print(v27);
  print(v28);
  print(v29);
  print(v30);
  print(v31);
  print(v32);
  print(v33);
  print(v34);
  print(v35);
  print(v36);
  print(v37);
  print(v38);
  print(v39);
  print(v40);
  print(v41);
  print(v42);
  print(v43);
  print(v44);
  print(v45);
  print(v46);
  print(v47);
  print(v48);
  print(v49);
  print(v50);
  print(v51);
  print(v52);
  print(v53);
  print(v54);
  print(v55);
  print(v56);
  print(v57);
  print(v58);
  print(v59);
  print(v60);
  print(v61);
  print(v62);
  print(v63);
  print(v64);
  print(v65);
  print(v66);
  print(v67);
  print(v68);
  print(v69);
  print(v70);
  print(v71);
  print(v72);
  print(v73);
  print(v74);
  print(v75);
  print(v76);
  print(v77);
  print(v78);
  print(v79);
  print(v80);
  print(v81);
  print(v82);
  print(v83);
  print(v84);
  print(v85);
  print(v86);
  print(v87);
  print(v88);
  print(v89);
  print(v90);
  print(v91);
  print(v92);
  print(v93);
  print(v94);
  print(v95);
  print(v96);
  print(v97);
  print(v98);
  print(v99);
  print(v100);
  print(v101);
  print(v102);
  print(v103);
  print(v104);
  print(v105);
  print(v106);
  print(v107);
  print(v108);
  print(v109);
  print(v110);
  print(v111);
  print(v112);
  print(v113);
  print(v114);
  print(v115);
  print(v116);
  print(v117);
  print(v118);
  print(v119);
  print(v120);
  print(v121);
  print(v122);
  print(v123);
  print(v124);
  print(v125);
  print(v126);
  print(v127);
  print(v128);
  print(v129);
  print(v130);
  print(v131);
  print(v132);
  print(v133);
  print(v134);
  print(v135);
  print(v136);
  print(v137);
  print(v138);
  print(v139);
  print(v140);
  print(v141);
  print(v142);
  print(v143);
  print(v144);
  print(v145);
  print(v146);
  print(v147);
  print(v148);
  print(v149);
  print(v150);
  print(v151);
  print(v152);
  print(v153);
  print(v154);
  print(v155);
  print(v156);
  print(v157);
  print(v158);
  print(v159);
  print(v160);
  print(v161);
  print(v162);
  print(v163);
  print(v164);
  print(v165);
  print(v166);
  print(v167);
  print(v168);
  print(v169);
  print(v170);
  print(v171);
  print(v172);
  print(v173);
  print(v174);
  print(v175);
  print(v176);
  print(v177);
  print(v178);
  print(v179);
  print(v180);
  print(v181);
  print(v182);
  print(v183);
  print(v184);
  print(v185);
  print(v186);
  print(v187);
  print(v188);
  print(v189);
  print(v190);
  print(v191);
  print(v192);
  print(v193);
  print(v194);
  print(v195);
  print(v196);
  print(v197);
  print(v198);
  print(v199);
  print(v200);
  print(v201);
  print(v202);
  print(v203);
  print(v204);
  print(v205);
  print(v206);
  print(v207);
  print(v208);
  print(v209);
  print(v210);
  print(v211);
  print(v212);
  print(v213);
  print(v214);
  print(v215);
  print(v216);
  print(v217);
  print(v218);
  print(v219);
  print(v220);
  print(v221);
  print(v222);
  print(v223);
  print(v224);
  print(v225);
  print(v226);
  print(v227);
  print(v228);
  print(v229);
  print(v230);
  print(v231);
  print(v232);
  print(v233);
  print(v234);
  print(v235);
  print(v236);
  print(v237);
  print(v238);
  print(v239);
  print(v240);
  print(v241);
  print(v242);
  print(v243);
  print(v244);
  print(v245);
  print(v246);
  print(v247);
  print(v248);
  print(v249);
  print(v250);
  print(v251);
  print(v252);
  print(v253);
  print(v254);
  print(v255);
  print(v256);
  print(v257);
  print(v258);
  print(v259);
  print(v260);
}

