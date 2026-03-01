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

#include "libabckit/cpp/abckit_cpp.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/src/logger.h"

#include <gtest/gtest.h>

#include <optional>

namespace {

class GTestAssertErrorHandler final : public abckit::IErrorHandler {
public:
    void HandleError(abckit::Exception &&err) override
    {
        EXPECT_TRUE(false) << "Abckit expection raised: " << err.what();
    }
};

constexpr auto API_CLASS_NAME = "ApiControl";
constexpr auto API_METHOD_NAME = "fixOrientationLinearLayout";
constexpr auto API_MODULE_NAME = "modules/ApiControl";
constexpr auto ANNOTATION_INTERFACE_NAME = "CallSiteReplacement";
constexpr auto TARGET_ORIENTATION = 5;

struct ReplaceCommand {
    struct Subject {
        abckit::core::Class klass;
        abckit::core::Function method;
    };
    Subject subject;

    struct Target {
        std::string className;
        std::string methodName;
    };
    Target target;
};

bool HasTargetMethod(const abckit::Instruction &inst, const ReplaceCommand::Target &target)
{
    if (inst.GetGraph()->DynIsa().GetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return false;
    }

    bool hasNewObjUser = false;
    inst.VisitUsers([&](const abckit::Instruction &user) -> bool {
        hasNewObjUser |= user.GetGraph()->DynIsa().GetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE;
        return !hasNewObjUser;
    });
    if (!hasNewObjUser) {
        return false;
    }

    bool found = false;
    inst.VisitUsers([&](const abckit::Instruction &user) -> bool {
        if (user.GetGraph()->DynIsa().GetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME) {
            found |= target.className == user.GetString();
        }
        return !found;
    });

    return found;
}

bool HasTargetMethod(const abckit::core::Function &method, const ReplaceCommand::Target &target)
{
    bool found = false;

    auto graph = method.CreateGraph();
    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) -> bool {
        for (abckit::Instruction inst = bb.GetFirstInst(); !!inst && !found; inst = inst.GetNext()) {
            found |= HasTargetMethod(inst, target);
        }
        return !found;
    });

    return found;
}

std::optional<abckit::Instruction> FindFirstInst(const abckit::Graph &graph, AbckitIsaApiDynamicOpcode opcode)
{
    std::optional<abckit::Instruction> found;
    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) -> bool {
        for (auto inst = bb.GetFirstInst(); !!inst && !found; inst = inst.GetNext()) {
            if (inst.GetGraph()->DynIsa().GetOpcode(inst) == opcode) {
                found = inst;
            }
        }
        return !found;
    });
    return found;
}

void ReplaceCallSite(const abckit::core::Function &method, const ReplaceCommand &command)
{
    std::optional<abckit::arkts::Module> targetModule;
    method.GetFile()->EnumerateModules([&](const abckit::core::Module &mod) -> bool {
        if (mod.GetName() == API_MODULE_NAME) {
            targetModule = abckit::arkts::Module(mod);
            return false;
        }
        return true;
    });
    EXPECT_TRUE(targetModule.has_value());

    abckit::arkts::Module arktsMethMod(method.GetModule());
    abckit::core::ImportDescriptor idesc =
        arktsMethMod.AddImportFromArktsV1ToArktsV1(*targetModule, API_CLASS_NAME, API_CLASS_NAME);

    abckit::Graph graph = method.CreateGraph();

    std::optional<abckit::Instruction> maybeConstInst;
    auto maybeNewobjInst = FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE);
    EXPECT_TRUE(maybeNewobjInst.has_value());

    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) -> bool {
        for (abckit::Instruction inst = bb.GetFirstInst(); !!inst; inst = inst.GetNext()) {
            if (inst.GetGraph()->DynIsa().GetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1) {
                continue;
            }
            maybeConstInst = maybeConstInst ? maybeConstInst : graph.FindOrCreateConstantI32(TARGET_ORIENTATION);

            auto ldExternal = graph.DynIsa().CreateLdexternalmodulevar(idesc).InsertAfter(inst);
            auto classThrow = graph.DynIsa()
                                  .CreateThrowUndefinedifholewithname(ldExternal, command.subject.klass.GetName())
                                  .InsertAfter(ldExternal);
            auto ldObj =
                graph.DynIsa().CreateLdobjbyname(ldExternal, command.subject.method.GetName()).InsertAfter(classThrow);
            auto staticCall =
                graph.DynIsa().CreateCallthis2(ldObj, ldExternal, *maybeNewobjInst, *maybeConstInst).InsertAfter(ldObj);
        }
        return true;
    });

    method.SetGraph(graph);
}

void ReplaceCallSite(const abckit::core::Class &klass, const ReplaceCommand &command)
{
    klass.EnumerateMethods([&](const abckit::core::Function &method) -> bool {
        if (HasTargetMethod(method, command.target)) {
            ReplaceCallSite(method, command);
        }
        return true;
    });
}

struct AnnotationTrack {
    abckit::core::Class klass;
    abckit::core::Function method;
    abckit::core::Annotation anno;
};

void GetAnnotationTracks(const abckit::core::Class &klass, const abckit::core::Function &method,
                         std::vector<AnnotationTrack> &tracks)
{
    method.EnumerateAnnotations([&](auto anno) {
        if (anno.GetInterface().GetName() == ANNOTATION_INTERFACE_NAME) {
            tracks.push_back({klass, method, anno});
        }
        return true;
    });
}

std::vector<AnnotationTrack> GetAnnotationTracks(const abckit::core::Module &mod)
{
    std::vector<AnnotationTrack> tracks;
    mod.EnumerateClasses([&](const auto &klass) {
        klass.EnumerateMethods([&](const auto &method) {
            GetAnnotationTracks(klass, method, tracks);
            return true;
        });
        return true;
    });
    return tracks;
}

ReplaceCommand GetReplaceCommand(const AnnotationTrack &track)
{
    ReplaceCommand::Target target;
    track.anno.EnumerateElements([&](const abckit::core::AnnotationElement &el) -> bool {
        auto name = el.GetName();
        if (name == "targetClass") {
            target.className = el.GetValue().GetString();
        } else if (name == "methodName") {
            target.methodName = el.GetValue().GetString();
        }
        return true;
    });
    EXPECT_TRUE(!target.className.empty());
    EXPECT_TRUE(!target.methodName.empty());

    return ReplaceCommand {ReplaceCommand::Subject {track.klass, track.method}, target};
}

void GetAnnotationInfo(const abckit::core::Module &mod, std::vector<ReplaceCommand> &replaceCommands)
{
    const std::vector<AnnotationTrack> tracks = GetAnnotationTracks(mod);
    for (const auto &track : tracks) {
        replaceCommands.push_back(GetReplaceCommand(track));
    }
}

}  // namespace

namespace libabckit::test {

class AbckitScenarioCppTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestCppDynamicReplaceCallSiteClean)
{
    const std::string sandboxPath = ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/replace_call_site/";
    const std::string inputPath = sandboxPath + "replace_call_site.abc";
    const std::string outputPath = sandboxPath + "replace_call_site_modified.abc";

    auto output = helpers::ExecuteDynamicAbc(inputPath, "replace_call_site");
    EXPECT_EQ(output, "3\n");

    abckit::File file(inputPath, std::make_unique<GTestAssertErrorHandler>());

    std::vector<ReplaceCommand> replaceCommands;
    file.EnumerateModules([&](const abckit::core::Module &mod) {
        if (mod.GetName() == API_MODULE_NAME) {
            GetAnnotationInfo(mod, replaceCommands);
        }
        return true;
    });
    EXPECT_EQ(replaceCommands.size(), 1);
    const auto &command = replaceCommands.front();

    // "modules/ApiControl" "modules/ApiControl"
    EXPECT_EQ(command.subject.klass.GetName(), API_CLASS_NAME);
    EXPECT_EQ(command.subject.method.GetName(), API_METHOD_NAME);

    file.EnumerateModules([&](const abckit::core::Module &mod) {
        if (mod.GetName() != "replace_call_site") {
            return true;
        }
        mod.EnumerateClasses([&](const abckit::core::Class &klass) {
            ReplaceCallSite(klass, command);
            return true;
        });
        return true;
    });

    file.WriteAbc(outputPath);

    output = helpers::ExecuteDynamicAbc(outputPath, "replace_call_site");
    EXPECT_TRUE(helpers::Match(output,
                               "fixOrientationLinearLayout was called\n"
                               "5\n"));
}

}  // namespace libabckit::test
