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

#include <gtest/gtest.h>

#include "libarkbase/panda_gen_options/generated/logger_options.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ani/ani.h"

namespace ark::ets::test {

// NOLINTBEGIN(google-runtime-int)
extern "C" ani_long SkoalaCreateRedrawerPeer([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object /*any*/)
{
    return 1;
}
extern "C" ani_long SkoalaGetFrame([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long peer /*KNativePointer*/,
                                   [[maybe_unused]] ani_int a, [[maybe_unused]] ani_int b)
{
    return 1;
}
// CC-OFFNXT(G.FUN.01) solid logic
extern "C" ani_long SkoalaInitRedrawer([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_int width,
                                       [[maybe_unused]] ani_int height, [[maybe_unused]] ani_float scale,
                                       [[maybe_unused]] ani_long peer /*KNativePointer*/,
                                       [[maybe_unused]] ani_long frame /*KNativePointer*/)
{
    return 1;
}
extern "C" ani_long SkoalaPaint1nMake([[maybe_unused]] ani_env *env)
{
    return 1;
}
// CC-OFFNXT(G.FUN.01) solid logic
extern "C" ani_long SkoalaPictureRecorder1nBeginRecording(
    // CC-OFFNXT(G.FMT.06) false positive
    [[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long ptr /*KNativePointer*/, [[maybe_unused]] ani_float left,
    [[maybe_unused]] ani_float top, [[maybe_unused]] ani_float right, [[maybe_unused]] ani_float bottom)
{
    return 1;
}
extern "C" ani_long SkoalaPictureRecorder1nFinishRecordingAsDrawable([[maybe_unused]] ani_env *env,
                                                                     [[maybe_unused]] ani_long ptr /*KNativePointer*/)
{
    return 1;
}
// CC-OFFNXT(G.FUN.01) solid logic
extern "C" ani_long SkoalaPictureRecorder1nFinishRecordingAsPictureWithCull(
    // CC-OFFNXT(G.FMT.06) false positive
    [[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long ptr /*KNativePointer*/, [[maybe_unused]] ani_float left,
    [[maybe_unused]] ani_float top, [[maybe_unused]] ani_float right, [[maybe_unused]] ani_float bottom)
{
    return 1;
}
extern "C" ani_long SkoalaPictureRecorder1nMake([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphParagraphBuilder1nMake(
    // CC-OFFNXT(G.FMT.06) false positive
    [[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long paragraphStylePtr /*KNativePointer*/,
    [[maybe_unused]] ani_long fontCollectionPtr /*KNativePointer*/)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphParagraphBuilder1nGetFinalizer([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphParagraphBuilder1nBuild([[maybe_unused]] ani_env *env,
                                                           [[maybe_unused]] ani_long ptr /*KNativePointer*/)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphFontCollection1nMake([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaFontMgr1nDefault([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphParagraphStyle1nMake([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphTextStyle1nMake([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaManagedString1nMake([[maybe_unused]] ani_env *env,
                                              [[maybe_unused]] ani_fixedarray_byte textStr /*KStringPtr*/)
{
    return 1;
}
extern "C" ani_long SkoalaPaint1nGetFinalizer([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaImplRefCntGetFinalizer([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphParagraphStyle1nGetFinalizer([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphTextStyle1nGetFinalizer([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaParagraphParagraph1nGetFinalizer([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long SkoalaManagedString1nGetFinalizer([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long GetPeerFactory([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_long GetEngine([[maybe_unused]] ani_env *env)
{
    return 1;
}
extern "C" ani_int SkoalaGetFrameWidth([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long peer /*KNativePointer*/,
                                       [[maybe_unused]] ani_long frame /*KNativePointer*/)
{
    return 1;
}
extern "C" ani_int SkoalaGetFrameHeight([[maybe_unused]] ani_env *env,
                                        [[maybe_unused]] ani_long peer /*KNativePointer*/,
                                        [[maybe_unused]] ani_long frame /*KNativePointer*/)
{
    return 1;
}
extern "C" ani_int SkoalaCanvas1nSave([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long ptr /*KNativePointer*/)
{
    return 1;
}
extern "C" void SkoalaDrawPicture([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long picture /*KNativePointer*/,
                                  [[maybe_unused]] ani_long data /*KNativePointer*/,
                                  [[maybe_unused]] ani_object cb /*any*/, [[maybe_unused]] ani_boolean sync)
{
}
extern "C" void SkoalaProvidePeerFactory([[maybe_unused]] ani_env *env,
                                         [[maybe_unused]] ani_long func /*KNativePointer*/,
                                         [[maybe_unused]] ani_long arg /*KNativePointer*/)
{
}
extern "C" void SkoalaSetPlatformApi([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object api /*any*/) {}
extern "C" void SkoalaCanvas1nDrawDrawable([[maybe_unused]] ani_env *env,
                                           [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                           [[maybe_unused]] ani_long drawablePtr /*KNativePointer*/,
                                           [[maybe_unused]] ani_long matrixArr /*KFloatPtr*/)
{
}
extern "C" void SkoalaCanvas1nRestore([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long ptr /*KNativePointer*/)
{
}
extern "C" void SkoalaPaint1nSetColor([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                      [[maybe_unused]] ani_int color)
{
}
// CC-OFFNXT(G.FUN.01) solid logic
extern "C" void SkoalaCanvas1nDrawOval([[maybe_unused]] ani_env *env,
                                       [[maybe_unused]] ani_long canvasPtr /*KNativePointer*/,
                                       [[maybe_unused]] ani_float left, [[maybe_unused]] ani_float top,
                                       [[maybe_unused]] ani_float right, [[maybe_unused]] ani_float bottom,
                                       [[maybe_unused]] ani_long paintPtr /*KNativePointer*/)
{
}
extern "C" void SkoalaParagraphParagraphBuilder1nPushStyle([[maybe_unused]] ani_env *env,
                                                           [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                                           [[maybe_unused]] ani_long textStylePtr /*KNativePointer*/)
{
}
extern "C" void SkoalaParagraphParagraphBuilder1nAddText([[maybe_unused]] ani_env *env,
                                                         [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                                         [[maybe_unused]] ani_long textString /*KStringPtr*/)
{
}
extern "C" void SkoalaParagraphFontCollection1nSetDefaultFontManager(
    // CC-OFFNXT(G.FMT.06) false positive
    [[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long ptr /*KNativePointer*/,
    [[maybe_unused]] ani_long fontManagerPtr /*KNativePointer*/,
    [[maybe_unused]] ani_long defaultFamilyNameStr /*KStringPtr*/)
{
}
extern "C" void SkoalaParagraphParagraph1nLayout([[maybe_unused]] ani_env *env,
                                                 [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                                 [[maybe_unused]] ani_long width)
{
}
extern "C" void SkoalaParagraphParagraph1nPaint([[maybe_unused]] ani_env *env,
                                                [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                                [[maybe_unused]] ani_long canvasPtr /*KNativePointer*/,
                                                [[maybe_unused]] ani_float x, [[maybe_unused]] ani_float y)
{
}
extern "C" void SkoalaParagraphTextStyle1nSetColor([[maybe_unused]] ani_env *env,
                                                   [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                                   [[maybe_unused]] ani_int color)
{
}
extern "C" void SkoalaParagraphTextStyle1nSetFontSize([[maybe_unused]] ani_env *env,
                                                      [[maybe_unused]] ani_long ptr /*KNativePointer*/,
                                                      [[maybe_unused]] ani_float size)
{
}
extern "C" void SkoalaImplManagedInvokeFinalizer([[maybe_unused]] ani_env *env,
                                                 [[maybe_unused]] ani_long finalizer /*KNativePointer*/,
                                                 [[maybe_unused]] ani_long obj /*KNativePointer*/)
{
}
extern "C" void SkoalaEnqueueRun([[maybe_unused]] ani_env *env,
                                 [[maybe_unused]] ani_long redrawerPeerPtr /*KNativePointer*/)
{
}
// NOLINTEND(google-runtime-int)

class EtsVMConfingNapi : public testing::Test {
public:
    void CreateRuntime(const std::string &stdlibAbc, const std::string &pathAbc, bool enableJit,
                       const std::string &pathAn = "")
    {
        std::string bootPandaFilesOption = "--ext:boot-panda-files=" + stdlibAbc + ":" + pathAbc;
        std::string compilerEnableJitOption =
            std::string("--ext:compiler-enable-jit=") + (enableJit ? "true" : "false");
        std::vector<ani_option> options = {
            {"--ext:log-level=info", nullptr},
            {bootPandaFilesOption.c_str(), nullptr},
            {"--ext:gc-trigger-type=heap-trigger", nullptr},
            {compilerEnableJitOption.c_str(), nullptr},
        };
        std::string optAotFilesOption;
        if (!pathAn.empty()) {
            options.push_back({"--ext:--enable-an", nullptr});
            optAotFilesOption = "--ext:aot-files=" + pathAn;
            options.push_back({optAotFilesOption.c_str(), nullptr});
        }
        ani_options opts {options.size(), options.data()};
        ASSERT_EQ(ANI_CreateVM(&opts, ANI_VERSION_1, &vm_), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env_), ANI_OK) << "Cannot get ani env";
    }

    // CC-OFFNXT(G.NAM.03,G.NAM.03)-CPP) project code style
    static constexpr char const *PEAS_MODULE_NAME = "bouncing_peas_unit_native";

    bool InitExports()
    {
        const std::array<ani_native_function, 43> impls = {
            {{"_skoala_createRedrawerPeer", nullptr, reinterpret_cast<void *>(SkoalaCreateRedrawerPeer)},
             {"_skoala_drawPicture", nullptr, reinterpret_cast<void *>(SkoalaDrawPicture)},
             {"_skoala_getFrame", nullptr, reinterpret_cast<void *>(SkoalaGetFrame)},
             {"_skoala_getFrameWidth", nullptr, reinterpret_cast<void *>(SkoalaGetFrameWidth)},
             {"_skoala_getFrameHeight", nullptr, reinterpret_cast<void *>(SkoalaGetFrameHeight)},
             {"_skoala_initRedrawer", nullptr, reinterpret_cast<void *>(SkoalaInitRedrawer)},
             {"_skoala_providePeerFactory", nullptr, reinterpret_cast<void *>(SkoalaProvidePeerFactory)},
             {"_skoala_setPlatformAPI", nullptr, reinterpret_cast<void *>(SkoalaSetPlatformApi)},
             {"_skoala_Canvas__1nDrawDrawable", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nDrawDrawable)},
             {"_skoala_Canvas__1nRestore", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nRestore)},
             {"_skoala_Canvas__1nSave", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nSave)},
             {"_skoala_Paint__1nMake", nullptr, reinterpret_cast<void *>(SkoalaPaint1nMake)},
             {"_skoala_Paint__1nSetColor", nullptr, reinterpret_cast<void *>(SkoalaPaint1nSetColor)},
             {"_skoala_PictureRecorder__1nBeginRecording", nullptr,
              reinterpret_cast<void *>(SkoalaPictureRecorder1nBeginRecording)},
             {"_skoala_PictureRecorder__1nFinishRecordingAsDrawable", nullptr,
              reinterpret_cast<void *>(SkoalaPictureRecorder1nFinishRecordingAsDrawable)},
             {"_skoala_PictureRecorder__1nFinishRecordingAsPictureWithCull", nullptr,
              reinterpret_cast<void *>(SkoalaPictureRecorder1nFinishRecordingAsPictureWithCull)},
             {"_skoala_PictureRecorder__1nMake", nullptr, reinterpret_cast<void *>(SkoalaPictureRecorder1nMake)},
             {"_skoala_Canvas__1nDrawOval", nullptr, reinterpret_cast<void *>(SkoalaCanvas1nDrawOval)},
             {"_skoala_paragraph_ParagraphBuilder__1nMake", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nMake)},
             {"_skoala_paragraph_ParagraphBuilder__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nGetFinalizer)},
             {"_skoala_paragraph_ParagraphBuilder__1nPushStyle", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nPushStyle)},
             {"_skoala_paragraph_ParagraphBuilder__1nAddText", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nAddText)},
             {"_skoala_paragraph_ParagraphBuilder__1nBuild", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphBuilder1nBuild)},
             {"_skoala_paragraph_FontCollection__1nMake", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphFontCollection1nMake)},
             {"_skoala_paragraph_FontCollection__1nSetDefaultFontManager", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphFontCollection1nSetDefaultFontManager)},
             {"_skoala_FontMgr__1nDefault", nullptr, reinterpret_cast<void *>(SkoalaFontMgr1nDefault)},
             {"_skoala_paragraph_ParagraphStyle__1nMake", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphStyle1nMake)},
             {"_skoala_paragraph_Paragraph__1nLayout", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraph1nLayout)},
             {"_skoala_paragraph_Paragraph__1nPaint", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraph1nPaint)},
             {"_skoala_paragraph_TextStyle__1nMake", nullptr, reinterpret_cast<void *>(SkoalaParagraphTextStyle1nMake)},
             {"_skoala_paragraph_TextStyle__1nSetColor", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphTextStyle1nSetColor)},
             {"_skoala_paragraph_TextStyle__1nSetFontSize", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphTextStyle1nSetFontSize)},
             {"_skoala_ManagedString__1nMake", nullptr, reinterpret_cast<void *>(SkoalaManagedString1nMake)},
             {"_skoala_Paint__1nGetFinalizer", nullptr, reinterpret_cast<void *>(SkoalaPaint1nGetFinalizer)},
             {"_skoala_impl_Managed__invokeFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaImplManagedInvokeFinalizer)},
             {"_skoala_impl_RefCnt__getFinalizer", nullptr, reinterpret_cast<void *>(SkoalaImplRefCntGetFinalizer)},
             {"_skoala_paragraph_ParagraphStyle__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraphStyle1nGetFinalizer)},
             {"_skoala_paragraph_TextStyle__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphTextStyle1nGetFinalizer)},
             {"_skoala_paragraph_Paragraph__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaParagraphParagraph1nGetFinalizer)},
             {"_skoala_ManagedString__1nGetFinalizer", nullptr,
              reinterpret_cast<void *>(SkoalaManagedString1nGetFinalizer)},
             {"_getPeerFactory", nullptr, reinterpret_cast<void *>(GetPeerFactory)},
             {"_getEngine", nullptr, reinterpret_cast<void *>(GetEngine)},
             {"_skoala_enqueue_run", nullptr, reinterpret_cast<void *>(SkoalaEnqueueRun)}}};

        ani_module module {};
        auto status = env_->FindModule(PEAS_MODULE_NAME, &module);
        if (status != ANI_OK) {
            return false;
        }

        return env_->Module_BindNativeFunctions(module, impls.data(), impls.size()) == ANI_OK;
    }

    bool DestroyRuntime() const
    {
        return vm_->DestroyVM() == ANI_OK;
    }

    int ExecuteMain() const
    {
        ani_module module {};
        [[maybe_unused]] auto status = env_->FindModule(PEAS_MODULE_NAME, &module);
        ASSERT(status == ANI_OK);
        ani_function fn {};
        status = env_->Module_FindFunction(module, "main", ":i", &fn);
        ASSERT(status == ANI_OK);
        ani_int result = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env_->Function_Call_Int(fn, &result);
        ASSERT(status == ANI_OK);
        return result;
    }

private:
    ani_vm *vm_ {};
    ani_env *env_ {};
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
TEST_F(EtsVMConfingNapi, PeasINT)
{
    std::string stdlibAbc;
    std::string pathAbc;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());

    CreateRuntime(stdlibAbc, pathAbc, false);
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}

TEST_F(EtsVMConfingNapi, PeasJIT)
{
    std::string stdlibAbc;
    std::string pathAbc;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());

    CreateRuntime(stdlibAbc, pathAbc, true);
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}

// NOTE(dslynko, #32431): enable the test after fixing issue with incorrect AOT file used
TEST_F(EtsVMConfingNapi, DISABLED_PeasAOT)
{
#ifdef HOST_CROSSCOMPILING
    GTEST_SKIP();
#endif

    std::string stdlibAbc;
    std::string pathAbc;
    std::string pathAn;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
    pathAn = ETS_UNIT_STRING(PATH_AN_ARK);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());
    ASSERT_FALSE(pathAn.empty());

    CreateRuntime(stdlibAbc, pathAbc, false, pathAn);
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}

// NOTE(dslynko, #32431): enable the test after fixing issue with incorrect AOT file used
TEST_F(EtsVMConfingNapi, DISABLED_PeasLLVMAOT)
{
#ifdef HOST_CROSSCOMPILING
    GTEST_SKIP();
#endif

#ifndef PANDA_LLVM_AOT
    GTEST_SKIP();
#endif

    std::string stdlibAbc;
    std::string pathAbc;
    std::string pathAn;

#if defined(STDLIB_ABC) && defined(PATH_ABC)
#define ETS_UNIT_STRING_STEP(s) #s
#define ETS_UNIT_STRING(s) ETS_UNIT_STRING_STEP(s)
    stdlibAbc = ETS_UNIT_STRING(STDLIB_ABC);
    pathAbc = ETS_UNIT_STRING(PATH_ABC);
    pathAn = ETS_UNIT_STRING(PATH_AN_LLVM);
#undef ETS_UNIT_STRING
#undef ETS_UNIT_STRING_STEP
#endif

    ASSERT_FALSE(stdlibAbc.empty());
    ASSERT_FALSE(pathAbc.empty());
    ASSERT_FALSE(pathAn.empty());

    CreateRuntime(stdlibAbc, pathAbc, false, pathAn);
    EXPECT_TRUE(InitExports());
    EXPECT_TRUE(ExecuteMain() == 0);
    ASSERT_TRUE(DestroyRuntime());
}
// NOLINTEND(cppcoreguidelines-macro-usage)

}  // namespace ark::ets::test
