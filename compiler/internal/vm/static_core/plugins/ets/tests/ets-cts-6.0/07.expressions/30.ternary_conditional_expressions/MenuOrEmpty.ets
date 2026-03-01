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

/*---
desc: Type of the conditional expression is determined as the union of types of the second and the third expressions
tags: []
---*/

final class Menu {}

final class MenuProvider {
    menuList?: Array<Menu>
}

final class View {
    menu: Array<Menu>;
    constructor(mp: MenuProvider) {
        this.menu = mp.menuList == undefined ? [] : mp.menuList!
    }
}

function build(isEmpty: boolean = true): MenuProvider {
    let mp = new MenuProvider
    if(isEmpty) return mp

    let am = [new Menu, new Menu]
    mp.menuList = am;
    return mp;

}

function main() {
    let view: View = new View(build())
    arktest.assertEQ(0, view.menu.length)

    view = new View(build(false))
    arktest.assertEQ(2, view.menu.length)
}
