/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Multi-line comments
---*/

/**/

/*
  block
*/

/*// nested one-liner
  // nested one-liner
*/

// /* one-liner comment

// */ one-liner comment

class/**/C/**/{/***/public/****/foo/*****/
    ( /**/ p /***/ : /****/ int /******/ )
    : /***********************************
      /*
        /*
          comment
    */
    int
    /* method body with comments: */
    {return/***/p/***/*/***/2;}
}

function main() {

    /*1*/let/*let;*/x/*x*/:/*colon :*/int/***int type*/
    /*2*/let /***********************************************
                         ::: let statement :::
             ********************************************/////
    y // var name

    /*
        separation
        colon
    */

    :/* separation colon */

    /* /*
        comment
        // another comment
        // */

    int/* element type */[ /*array dimension*/ ]=[
      /*** 1-st array element ***/44
      ,
      /*** 2-nd array element ***/55
    ]

    let s1: string = `
        // this is not a comment
        /* this is not a comment either */
    `

    // arktest.assertTrue() calls:
    arktest.assertTrue( y[0] == 44/*1*/ && y[1] == 55 /*2*/ && x == 0/*3*/ )
    arktest.assertTrue( new C().foo(2) == 4 /*4*/ )

    /* // */ 
}
