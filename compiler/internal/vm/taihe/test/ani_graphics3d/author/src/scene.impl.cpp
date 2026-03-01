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
#include "scene.impl.hpp"
#include "scene.proj.hpp"
#include "sceneNodeParameters.h"
#include "stdexcept"
#include "taihe/runtime.hpp"

namespace {
// To be implemented.

class CameraImpl {
public:
    float fov_ = 5.24;

    CameraImpl() {}

    void SetFov(float fov)
    {
        this->fov_ = fov;
    }

    float GetFov()
    {
        return fov_;
    }
};

class LightImpl {
public:
    float intens_ = 5.24;

    LightImpl() {}

    void SetIntensity(float intens)
    {
        this->intens_ = intens;
    }

    float GetIntensity()
    {
        return intens_;
    }
};

class NodeImpl {
public:
    bool visible_ = true;
    ::taihe::string path = "test\ani_graphics3d";

    NodeImpl() {}

    void SetVisible(bool visible)
    {
        this->visible_ = visible;
    }

    bool GetVisible()
    {
        return visible_;
    }

    ::taihe::string GetPath()
    {
        return path;
    }
};

class SceneResourceParametersImpl {
public:
    std::string name_ = "Scene";

    SceneResourceParametersImpl() {}

    void SetName(::taihe::string_view name)
    {
        this->name_ = name;
    }

    ::taihe::string GetName()
    {
        return name_;
    }
};

class MaterialImpl {
public:
    int8_t materialType = 122;

    MaterialImpl() {}

    int8_t GetMaterialType()
    {
        return materialType;
    }
};

class ShaderImpl {
public:
    ShaderImpl() {}

    ::taihe::map<::taihe::string, int32_t> GetInputs()
    {
        ::taihe::map<::taihe::string, int32_t> res;
        static int32_t const input = 2025;
        res.emplace("banana", input);
        return res;
    }
};

class ImageImpl {
public:
    float width = 800;
    float height = 600;

    ImageImpl() {}

    float GetWidth()
    {
        return width;
    }

    float GetHeight()
    {
        return height;
    }
};

class EnvironmentImpl {
public:
    EnvironmentImpl() {}

    int32_t GetBackgroundType()
    {
        int32_t bkType = 0;
        return bkType;
    }
};

class SceneResourceFactoryImpl {
public:
    SceneResourceFactoryImpl() {}

    ::scene::Camera createCameraPro(::sceneNodeParameters::weak::SceneNodeParameters params)
    {
        return taihe::make_holder<CameraImpl, ::scene::Camera>();
    }

    ::scene::Light createLightPro(::sceneNodeParameters::weak::SceneNodeParameters params, ::scene::LightType lightType)
    {
        return taihe::make_holder<LightImpl, ::scene::Light>();
    }

    ::scene::Node createNodePro(::sceneNodeParameters::weak::SceneNodeParameters params)
    {
        return taihe::make_holder<NodeImpl, ::scene::Node>();
    }

    ::scene::Material createMaterialPro(::scene::weak::SceneResourceParameters params,
                                        ::scene::MaterialType materialType)
    {
        return taihe::make_holder<MaterialImpl, ::scene::Material>();
    }

    ::scene::Shader createShaderPro(::scene::weak::SceneResourceParameters params)
    {
        return taihe::make_holder<ShaderImpl, ::scene::Shader>();
    }

    ::scene::Image createImagePro(::scene::weak::SceneResourceParameters params)
    {
        return taihe::make_holder<ImageImpl, ::scene::Image>();
    }

    ::scene::Environment createEnvironmentPro(::scene::weak::SceneResourceParameters params)
    {
        return taihe::make_holder<EnvironmentImpl, ::scene::Environment>();
    }
};

::scene::SceneResourceFactory GetSceneResourceFactory()
{
    return taihe::make_holder<SceneResourceFactoryImpl, ::scene::SceneResourceFactory>();
}

::scene::Camera GetCamera()
{
    return taihe::make_holder<CameraImpl, ::scene::Camera>();
}

::scene::Light GetLight()
{
    return taihe::make_holder<LightImpl, ::scene::Light>();
}

::scene::Node GetNode()
{
    return taihe::make_holder<NodeImpl, ::scene::Node>();
}

::scene::SceneResourceParameters GetSceneResourceParameters()
{
    return taihe::make_holder<SceneResourceParametersImpl, ::scene::SceneResourceParameters>();
}

::scene::Material GetMaterial()
{
    return taihe::make_holder<MaterialImpl, ::scene::Material>();
}

::scene::Shader GetShader()
{
    return taihe::make_holder<ShaderImpl, ::scene::Shader>();
}

::scene::Image GetImage()
{
    return taihe::make_holder<ImageImpl, ::scene::Image>();
}

::scene::Environment GetEnvironment()
{
    return taihe::make_holder<EnvironmentImpl, ::scene::Environment>();
}
}  // namespace

TH_EXPORT_CPP_API_GetSceneResourceFactory(GetSceneResourceFactory);
TH_EXPORT_CPP_API_GetCamera(GetCamera);
TH_EXPORT_CPP_API_GetLight(GetLight);
TH_EXPORT_CPP_API_GetNode(GetNode);
TH_EXPORT_CPP_API_GetSceneResourceParameters(GetSceneResourceParameters);
TH_EXPORT_CPP_API_GetMaterial(GetMaterial);
TH_EXPORT_CPP_API_GetShader(GetShader);
TH_EXPORT_CPP_API_GetImage(GetImage);
TH_EXPORT_CPP_API_GetEnvironment(GetEnvironment);