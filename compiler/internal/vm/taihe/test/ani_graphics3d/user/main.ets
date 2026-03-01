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
import * as SceneTest from "scene";
import * as SceneNodeTest from "sceneNodeParameters";

loadLibrary("ani_graphics3d");

let sResFactory: SceneTest.SceneResourceFactory =
    SceneTest.getSceneResourceFactory();
let sNodePara: SceneNodeTest.SceneNodeParameters =
    SceneNodeTest.getSceneNodeParameters();
let sResPara: SceneTest.SceneResourceParameters =
    SceneTest.getSceneResourceParameters();
function test_ani_graphics3d_scene_createCamera() {
    let param = sNodePara;
    let info = SceneTest.getCamera();
    let err: BusinessError = new BusinessError();
    try {
        info = sResFactory.createCameraPro(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createCamera is: " + info.fov);
    arktest.assertEQ(info.fov, 5.24f);
}

function test_ani_graphics3d_scene_createCamera_promise() {
    let param = sNodePara;
    let info = SceneTest.getCamera();
    let err: BusinessError = new BusinessError();
    try {
        info = await sResFactory.createCamera(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createCamera is: " + info.fov);
    arktest.assertEQ(info.fov, 5.24f);
}

function test_ani_graphics3d_scene_createCamera_sync() {
    let param = sNodePara
    let promise = new Promise<SceneTest.Camera>((resolve, reject): void => {
        sResFactory.createCamera(
            param,
            (err: BusinessError|null, data: SceneTest.Camera|undefined) => {
                if (err != null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });

    let info = SceneTest.getCamera();
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createCamera is: " + info.fov);
    arktest.assertEQ(info.fov, 5.24f);
}

function test_ani_graphics3d_scene_createLight() {
    let param = sNodePara;
    let lightType = SceneTest.LightType.SPOT;
    let info = SceneTest.getLight();
    let err: BusinessError = new BusinessError();
    try {
        info = sResFactory.createLightPro(param, lightType);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createLightPro is: " + info.intensity);
    arktest.assertEQ(info.intensity, 5.24f);
}

function test_ani_graphics3d_scene_createLight_promise() {
    let param = sNodePara;
    let lightType = SceneTest.LightType.SPOT;
    let info = SceneTest.getLight();
    let err: BusinessError = new BusinessError();
    try {
        info = await sResFactory.createLight(param, lightType);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createLightPro is: " + info.intensity);
    arktest.assertEQ(info.intensity, 5.24f);
}

function test_ani_graphics3d_scene_createLight_sync() {
    let param = sNodePara;
    let lightType = SceneTest.LightType.SPOT;
    let promise = new Promise<SceneTest.Light>((resolve, reject) => {
        sResFactory.createLight(
            param, lightType,
            (err: BusinessError|null, data: SceneTest.Light|undefined) => {
                if (err != null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });

    let info = SceneTest.getLight();
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createLightPro is: " + info.intensity);
    arktest.assertEQ(info.intensity, 5.24f);
}

function test_ani_graphics3d_scene_createNode() {
    let param = sNodePara;
    let info = SceneTest.getNode();
    let err: BusinessError = new BusinessError();
    try {
        info = sResFactory.createNodePro(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createNode is: " + info.visible);
    arktest.assertEQ(info.visible, true);
}

function test_ani_graphics3d_scene_createNode_promise() {
    let param = sNodePara;
    let info = SceneTest.getNode();
    let err: BusinessError = new BusinessError();
    try {
        info = await sResFactory.createNode(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createNode is: " + info.visible);
    arktest.assertEQ(info.visible, true);
}

function test_ani_graphics3d_scene_createNode_sync() {
    let param = sNodePara;
    let promise = new Promise<SceneTest.Node>((resolve, reject) => {
        sResFactory.createNode(
            param,
            (err: BusinessError|null, data: SceneTest.Node|undefined) => {
                if (err != null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });

    let info = SceneTest.getNode();
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createNode is: " + info.visible);
    arktest.assertEQ(info.visible, true);
}

function test_ani_graphics3d_scene_createMaterial() {
    let param = sResPara;
    let materialType = SceneTest.MaterialType.SHADER;
    let info = SceneTest.getMaterial();
    let err: BusinessError = new BusinessError();
    try {
        info = sResFactory.createMaterialPro(param, materialType);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createMaterial is: " + info.materialType);
    arktest.assertEQ(info.materialType, 122 as byte);
}

function test_ani_graphics3d_scene_createMaterial_promise() {
    let param = sResPara;
    let materialType = SceneTest.MaterialType.SHADER;
    let info = SceneTest.getMaterial();
    let err: BusinessError = new BusinessError();
    try {
        info = await sResFactory.createMaterial(param, materialType);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createMaterial is: " + info.materialType);
    arktest.assertEQ(info.materialType, 122 as byte);
}

function test_ani_graphics3d_scene_createMaterial_sync() {
    let param = sResPara;
    let materialType = SceneTest.MaterialType.SHADER;
    let promise = new Promise<SceneTest.Material>((resolve, reject) => {
        sResFactory.createMaterial(
            param, materialType,
            (err: BusinessError|null, data: SceneTest.Material|undefined) => {
                if (err != null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });

    let info = SceneTest.getMaterial();
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createMaterial is: " + info.materialType);
    arktest.assertEQ(info.materialType, 122 as byte);
}

function test_ani_graphics3d_scene_createShader() {
    let param = sResPara;
    let info = SceneTest.getShader();
    let err: BusinessError = new BusinessError();
    try {
        info = sResFactory.createShaderPro(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createShader is: " + info.inputs);
    arktest.assertEQ(info.inputs.size, 1);
}

function test_ani_graphics3d_scene_createShader_promise() {
    let param = sResPara;
    let info = SceneTest.getShader();
    let err: BusinessError = new BusinessError();
    try {
        info = await sResFactory.createShader(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createShader is: " + info.inputs);
    arktest.assertEQ(info.inputs.size, 1);
}

function test_ani_graphics3d_scene_createShader_sync() {
    let param = sResPara;
    let promise = new Promise<SceneTest.Shader>((resolve, reject) => {
        sResFactory.createShader(
            param,
            (err: BusinessError|null, data: SceneTest.Shader|undefined) => {
                if (err != null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });

    let info = SceneTest.getShader();
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createShader is: " + info.inputs);
    arktest.assertEQ(info.inputs.size, 1);
}

function test_ani_graphics3d_scene_createImage() {
    let param = sResPara;
    let info = SceneTest.getImage();
    let err: BusinessError = new BusinessError();
    try {
        info = sResFactory.createImagePro(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createImage is: " + info.width);
    arktest.assertEQ(info.width, 800);
}

function test_ani_graphics3d_scene_createImage_promise() {
    let param = sResPara;
    let info = SceneTest.getImage();
    let err: BusinessError = new BusinessError();
    try {
        info = await sResFactory.createImage(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createImage is: " + info);
    arktest.assertEQ(info.height, 600);
}

function test_ani_graphics3d_scene_createImage_sync() {
    let param = sResPara;
    let promise = new Promise<SceneTest.Image>((resolve, reject) => {
        sResFactory.createImage(
            param,
            (err: BusinessError|null, data: SceneTest.Image|undefined) => {
                if (err != null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });

    let info = SceneTest.getImage();
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createImage is: " + info);
    arktest.assertEQ(info.height, 600);
}

function test_ani_graphics3d_scene_createEnvironment() {
    let param = sResPara;
    let info = SceneTest.getEnvironment();
    let err: BusinessError = new BusinessError();
    try {
        info = sResFactory.createEnvironmentPro(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createEnvironment is: " + info);
    arktest.assertEQ(info.backgroundType, 0);
}

function test_ani_graphics3d_scene_createEnvironment_promise() {
    let param = sResPara;
    let info = SceneTest.getEnvironment();
    let err: BusinessError = new BusinessError();
    try {
        info = await sResFactory.createEnvironment(param);
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createEnvironment is: " + info);
    arktest.assertEQ(info.backgroundType, 0);
}

function test_ani_graphics3d_scene_createEnvironment_sync() {
    let param = sResPara;
    let promise = new Promise<SceneTest.Environment>((resolve, reject) => {
        sResFactory.createEnvironment(
            param,
            (err: BusinessError|null,
             data: SceneTest.Environment|undefined) => {
                if (err != null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });

    let info = SceneTest.getEnvironment();
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("createEnvironment is: " + info);
    arktest.assertEQ(info.backgroundType, 0);
}

function main() {
    console.log("############## start ani_graphics3d #############");
    const suite = new arktest.ArkTestsuite("ani_graphics3d Tests");

    suite.addTest(
        "test_ani_graphics3d_scene_createCamera",
        test_ani_graphics3d_scene_createCamera)
    suite.addTest(
        "test_ani_graphics3d_scene_createCamera_promise",
        test_ani_graphics3d_scene_createCamera_promise)
    suite.addTest(
        "test_ani_graphics3d_scene_createCamera_sync",
        test_ani_graphics3d_scene_createCamera_sync)
    suite.addTest(
        "test_ani_graphics3d_scene_createLight",
        test_ani_graphics3d_scene_createLight)
    suite.addTest(
        "test_ani_graphics3d_scene_createLight_promise",
        test_ani_graphics3d_scene_createLight_promise)
    suite.addTest(
        "test_ani_graphics3d_scene_createLight_sync",
        test_ani_graphics3d_scene_createLight_sync)
    suite.addTest(
        "test_ani_graphics3d_scene_createNode",
        test_ani_graphics3d_scene_createNode)
    suite.addTest(
        "test_ani_graphics3d_scene_createNode_promise",
        test_ani_graphics3d_scene_createNode_promise)
    suite.addTest(
        "test_ani_graphics3d_scene_createNode_sync",
        test_ani_graphics3d_scene_createNode_sync)
    suite.addTest(
        "test_ani_graphics3d_scene_createMaterial",
        test_ani_graphics3d_scene_createMaterial)
    suite.addTest(
        "test_ani_graphics3d_scene_createMaterial_promise",
        test_ani_graphics3d_scene_createMaterial_promise)
    suite.addTest(
        "test_ani_graphics3d_scene_createMaterial_sync",
        test_ani_graphics3d_scene_createMaterial_sync)
    suite.addTest(
        "test_ani_graphics3d_scene_createShader",
        test_ani_graphics3d_scene_createShader)
    suite.addTest(
        "test_ani_graphics3d_scene_createShader_promise",
        test_ani_graphics3d_scene_createShader_promise)
    suite.addTest(
        "test_ani_graphics3d_scene_createShader_promise",
        test_ani_graphics3d_scene_createShader_sync)
    suite.addTest(
        "test_ani_graphics3d_scene_createImage",
        test_ani_graphics3d_scene_createImage)
    suite.addTest(
        "test_ani_graphics3d_scene_createImage_promise",
        test_ani_graphics3d_scene_createImage_promise)
    suite.addTest(
        "test_ani_graphics3d_scene_createImage_sync",
        test_ani_graphics3d_scene_createImage_sync)
    suite.addTest(
        "test_ani_graphics3d_scene_createEnvironment",
        test_ani_graphics3d_scene_createEnvironment)
    suite.addTest(
        "test_ani_graphics3d_scene_createEnvironment_promise",
        test_ani_graphics3d_scene_createEnvironment_promise)
    suite.addTest(
        "test_ani_graphics3d_scene_createEnvironment_promise",
        test_ani_graphics3d_scene_createEnvironment_sync)

    exit(suite.run())
}