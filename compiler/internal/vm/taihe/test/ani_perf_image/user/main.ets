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
import {image} from "@ohos.multimedia.image"

loadLibrary("ani_perf_image");

// let imageSourceApi: image.ImageSource;
// let pixelMap: image.PixelMap;
// let filesDir: string | undefined
// let cacheDir: string | undefined
// let fdNumber: number;
// let globalimageSource: image.ImageSource;
// let globalpixelmap: image.PixelMap

function runTestFunction(prompt: string, n: int, fn: () => void): void {
    const s = Chrono.nanoNow();
    for (let i: int = 0; i < n; i++) {
        fn();
    }
    const t = Chrono.nanoNow();
    const u = Chrono.nanoNow();
    for (let i: int = 0; i < n; i++) {
    }
    const v = Chrono.nanoNow();
    let durationNs = ((t - s) - (v - u)) * 1.0 / n;
    console.log(`>> Done ${prompt}: ${durationNs} ns.`);
}

function testImage() {
    // create image source test
    const imageSource: image.ImageSource = image.createImageSource();

    // create pixel map test
    const defaultWidth: int = 41;
    const defaultHeight: int = 41;

    const desiredSize: image.Size|
        undefined = {width: defaultWidth, height: defaultHeight};
    const desiredRegion: image.Region|undefined = {
        size: {width: defaultWidth, height: defaultHeight},
        x: 0,
        y: 0
    };
    const decodeOpts: image.DecodingOptions = {
        index: 0,
        sampleSize: 1,
        rotate: 0,
        editable: true,
        desiredSize,
        desiredRegion,
        desiredPixelFormat: image.PixelMapFormat.RGBA_8888,
        fitDensity: 1
    };
    const pixelMap = imageSource.createPixelMapSync(decodeOpts);
}
function main() {
    runTestFunction("preheat", 500, testImage);
    runTestFunction("test", 2000, testImage);
}
