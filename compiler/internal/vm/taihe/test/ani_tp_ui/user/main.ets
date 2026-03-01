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
import {BusinessError} from "@ohos.base";
import * as ui from "ui"
import * as ui_impl from "ui_impl";
import * as ui_user from "ui_user";

loadLibrary("ani_tp_ui");

function main() {
    let body: ui.AntUserDialogBody = {
        title: "body_title",
        detail: "body_detail",
        traceParams: undefined
    };

    let controller: ui_impl.AntUserModalControllerClz =
        new ui_impl.AntUserModalControllerClz("controller1");
    let provider: ui_impl.AntUserUIProviderClz =
        new ui_impl.AntUserUIProviderClz(controller);
    ui_user.runNativeBusiness(provider, body);
}