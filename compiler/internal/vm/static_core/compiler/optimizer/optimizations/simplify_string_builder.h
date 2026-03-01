/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_SIMPLIFY_STRING_BUILDER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_SIMPLIFY_STRING_BUILDER_H

#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"
#include "optimizer/optimizations/string_builder_utils.h"

namespace ark::compiler {

/**
 * 1. Removes unnecessary String Builder instances
 * 2. Replaces String Builder usage with string concatenation whenever optimal
 * 3. Optimizes String Builder concatenation loops
 * 4. Merges consecutive String Builder append string calls into one appendN call
 * 5. Merges consecutive String Builders into one String Builder if possible
 *
 * See compiler/docs/simplify_sb_doc.md for complete documentation
 */

constexpr size_t ARGS_NUM_1 = 1;
constexpr size_t ARGS_NUM_2 = 2;
constexpr size_t ARGS_NUM_3 = 3;
constexpr size_t ARGS_NUM_4 = 4;
constexpr size_t SB_APPEND_STRING_MAX_ARGS = ARGS_NUM_4;

class PANDA_PUBLIC_API SimplifyStringBuilder : public Optimization {
public:
    explicit SimplifyStringBuilder(Graph *graph);

    NO_MOVE_SEMANTIC(SimplifyStringBuilder);
    NO_COPY_SEMANTIC(SimplifyStringBuilder);
    ~SimplifyStringBuilder() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerSimplifyStringBuilder();
    }

    const char *GetPassName() const override
    {
        return "SimplifyStringBuilder";
    }
    void InvalidateAnalyses() override;

private:
    // 1. Removes unnecessary String Builder instances
    InstIter SkipToStringBuilderConstructorWithStringArg(InstIter begin, InstIter end);
    void OptimizeStringBuilderToString(BasicBlock *block);

    // 2. Replaces String Builder usage with string concatenation whenever optimal
    struct ConcatenationMatch {
        Inst *instance {nullptr};      // NOLINT(misc-non-private-member-variables-in-classes)
        Inst *ctorCall {nullptr};      // NOLINT(misc-non-private-member-variables-in-classes)
        Inst *toStringCall {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
        size_t appendCount {0};        // NOLINT(misc-non-private-member-variables-in-classes)
        std::array<IntrinsicInst *, ARGS_NUM_4>
            appendIntrinsics {};  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    InstIter SkipToStringBuilderDefaultConstructor(InstIter begin, InstIter end);
    InstIter SkipToStringBuilderDefaultOrStringArgConstructor(InstIter begin, InstIter end);
    void InsertNewAppendIntrinsic(ConcatenationMatch &match);
    IntrinsicInst *CreateConcatIntrinsic(const std::array<IntrinsicInst *, ARGS_NUM_4> &appendIntrinsics,
                                         size_t appendCount, DataType::Type type, SaveStateInst *saveState);
    bool MatchConcatenation(InstIter &begin, const InstIter &end, ConcatenationMatch &match);
    void FixBrokenSaveStates(Inst *source, Inst *target);
    void Check(const ConcatenationMatch &match);
    void InsertIntrinsicAndFixSaveStates(IntrinsicInst *concatIntrinsic,
                                         const std::array<IntrinsicInst *, ARGS_NUM_4> &appendIntrinsics,
                                         size_t appendCount, Inst *before);
    void ReplaceWithConcatIntrinsic(const ConcatenationMatch &match);
    void RemoveStringBuilderInstance(Inst *instance);
    void Cleanup(const ConcatenationMatch &match);
    void OptimizeStringConcatenation(BasicBlock *block);

    // 3. Optimizes String Builder concatenation loops

    struct ToStringLengthChain {
        Inst *toStringCallOrPhi {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
        Inst *nullCheckCall {nullptr};      // NOLINT(misc-non-private-member-variables-in-classes)
        Inst *lenArrayCall {nullptr};       // NOLINT(misc-non-private-member-variables-in-classes)
        Inst *shrLengthShift {nullptr};     // NOLINT(misc-non-private-member-variables-in-classes)
    };

    struct ConcatenationLoopMatch {
        /*
            This structure reflects the following string concatenation pattern:

            preheader:
                let initialValue: String = ...
            header:
                let accValue: String = preheader:initialValue | loop:accValue;
            loop:
                let preheaderInstance: StringBuilder;               // preheader.instance
                preheaderInstance = new StringBuilder();            // preheader.ctorCall
                preheaderInstance.append(accValue);                 // preheader.appendAccValue
                preheaderInstance.append(someArg0);                 // loop.appendInstructions[0]
                    ... Zero or more append non-accValue-arg calls
                let tempAccum0 = preheaderInstance.toString();      // temp[0].toStringCall

                    ... The total number of temporary instances maybe zero

                let tempInstanceN: StringBuilder                    // temp[N].instance
                tempInstanceN = new StringBuilder();                // temp[N].ctorCall
                tempInstanceN.append(tempAccumN);                   // temp[N].appendAccValue
                tempInstanceN.append(someArgN);                     // loop.appendInstructions[N]
                    ... Zero or more append non-accValue-arg calls
                accValue = tempInstanceN.toString();                // exit.toStringCall

            The pattern above is then transformed into:

            preheader:
                let preheaderInstance: StringBuilder;
                preheaderInstance = new StringBuilder();
                preheaderInstance.append(initialValue);
            loop:
                preheaderInstance.append(someArg0);
                    ...
                preheaderInstance.append(someArgN);
            exit:
                let accValue: String = preheaderInstance.toString();

            To accomplish such transformation, we
                1. Move all instructions collected in 'struct preheader' into loop preheader
                2. Relink all append intrinsics collected in 'struct loop' to an instance moved to preheader
                3. Move toString()-call instruction collected in 'struct exit' into loop post exit block
            and relink it to an instance moved to preheader
                4. Delete all temporary instructions

            Multiple independent patterns maybe in a loop.
        */

        explicit ConcatenationLoopMatch(ArenaAllocator *allocator) : loop {allocator}, temp {allocator->Adapter()} {}

        BasicBlock *block {nullptr};   // NOLINT(misc-non-private-member-variables-in-classes)
        PhiInst *accValue {nullptr};   // NOLINT(misc-non-private-member-variables-in-classes)
        Inst *initialValue {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)

        struct {
            Inst *instance {nullptr};        // NOLINT(misc-non-private-member-variables-in-classes)
            Inst *ctorCall {nullptr};        // NOLINT(misc-non-private-member-variables-in-classes)
            Inst *appendAccValue {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
        } preheader;                         // NOLINT(misc-non-private-member-variables-in-classes)

        struct Loop {
            explicit Loop(ArenaAllocator *allocator)
                : appendInstructions {allocator->Adapter()}, toStringLengthChains {allocator->Adapter()}
            {
            }
            ArenaVector<Inst *> appendInstructions;  // NOLINT(misc-non-private-member-variables-in-classes)
            ArenaVector<ToStringLengthChain>
                toStringLengthChains;  // NOLINT(misc-non-private-member-variables-in-classes)
        } loop;                        // NOLINT(misc-non-private-member-variables-in-classes)

        struct TemporaryInstructions {
            Inst *intermediateValue {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
            Inst *toStringCall {nullptr};       // NOLINT(misc-non-private-member-variables-in-classes)
            Inst *instance {nullptr};           // NOLINT(misc-non-private-member-variables-in-classes)
            Inst *ctorCall {nullptr};           // NOLINT(misc-non-private-member-variables-in-classes)
            Inst *appendAccValue {nullptr};     // NOLINT(misc-non-private-member-variables-in-classes)

            void Clear();
            bool IsEmpty() const;
        };

        ArenaVector<TemporaryInstructions> temp;  // NOLINT(misc-non-private-member-variables-in-classes)

        struct {
            Inst *toStringCall {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
        } exit;                            // NOLINT(misc-non-private-member-variables-in-classes)

        void Clear();
        bool IsInstanceHoistable() const;
    };

    struct StringBuilderUsage {
        StringBuilderUsage(Inst *pInstance, CallInst *pCtorCall, ArenaAllocator *allocator)
            : instance {pInstance},
              ctorCall {pCtorCall},
              appendInstructions {allocator->Adapter()},
              toStringCalls {allocator->Adapter()},
              toStringLengthChains {allocator->Adapter()}
        {
        }
        Inst *instance {nullptr};                               // NOLINT(misc-non-private-member-variables-in-classes)
        Inst *ctorCall {nullptr};                               // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<Inst *> appendInstructions;                 // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<Inst *> toStringCalls;                      // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<ToStringLengthChain> toStringLengthChains;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    IntrinsicInst *CreateIntrinsicStringBuilderAppendString(Inst *instance, Inst *arg, SaveStateInst *saveState);
    CallInst *CreateStringBuilderAppendString(Inst *instance, Inst *arg, SaveStateInst *saveState);
    Inst *NormalizeStringBuilderAppendInstructionUsers(Inst *instance, SaveStateInst *saveState);
    ArenaVector<Inst *> FindStringBuilderAppendInstructions(Inst *instance);

    void RemoveFromSaveStateInputs(Inst *inst, bool doMark = false);
    void RemoveFromAllExceptPhiInputs(Inst *inst);
    void ReconnectStringBuilderCascade(Inst *instance, Inst *inputInst, Inst *appendInstruction,
                                       SaveStateInst *saveState);
    void ReconnectStringBuilderCascades(const ConcatenationLoopMatch &match);
    RuntimeInterface::FieldPtr GetGetterStringBuilderStringLength();
    void ReconnectToStringLengthChains(const ConcatenationLoopMatch &match);
    void ReconnectInstructions(const ConcatenationLoopMatch &match);

    Inst *HoistInstructionToPreHeaderRecursively(BasicBlock *preHeader, Inst *lastInst, Inst *inst,
                                                 SaveStateInst *saveState);
    Inst *HoistInstructionToPreHeader(BasicBlock *preHeader, Inst *lastInst, Inst *inst, SaveStateInst *saveState);
    void HoistInstructionsToPreHeader(const ConcatenationLoopMatch &match, SaveStateInst *initSaveState);
    void HoistCheckCastInstructionUsers(Inst *inst, BasicBlock *loopBlock, BasicBlock *postExit);
    void HoistInstructionsToPostExit(const ConcatenationLoopMatch &match, SaveStateInst *saveState);

    void Cleanup(const ConcatenationLoopMatch &match);
    bool NeedRemoveInputFromSaveStateInstruction(Inst *inputInst);
    void CollectSaveStateInputsForRemoval(Inst *inst);
    void CleanupSaveStateInstructionInputs(Loop *loop);
    Inst *FindHoistedToStringCallInput(Inst *phi);
    void CleanupPhiInstructionInputs(Inst *phi);
    void CleanupPhiInstructionInputs(Loop *loop);
    bool HasNotHoistedUser(PhiInst *phi);
    void RemoveUnusedPhiInstructions(Loop *loop);
    void FixBrokenSaveStates(Loop *loop);
    void Cleanup(Loop *loop);

    bool AllUsersAreVisitedAppendInstructions(Inst *inst, Marker visited);
    Inst *UpdateIntermediateValue(const StringBuilderUsage &usage, Inst *intermediateValue,
                                  Marker appendInstructionVisited);
    bool MatchTemporaryInstruction(ConcatenationLoopMatch &match, ConcatenationLoopMatch::TemporaryInstructions &temp,
                                   Inst *appendInstruction, const Inst *accValue, Marker appendInstructionVisited);
    bool ValidateTemporaryStringLengthPhis(const ConcatenationLoopMatch &match, const StringBuilderUsage &usage);
    void MatchTemporaryInstructions(const StringBuilderUsage &usage, ConcatenationLoopMatch &match, Inst *accValue,
                                    Inst *intermediateValue, Marker appendInstructionVisited);
    Inst *MatchHoistableInstructions(const StringBuilderUsage &usage, ConcatenationLoopMatch &match,
                                     Marker appendInstructionVisited);
    void MatchStringBuilderUsage(Inst *instance, StringBuilderUsage &usage);
    const ArenaVector<ConcatenationLoopMatch> &MatchLoopConcatenation(Loop *loop);

    bool HasInputFromPreHeader(PhiInst *phi) const;
    bool HasToStringCallInput(PhiInst *phi) const;
    bool HasSbCtorStrInstructionUser(Inst *inst) const;
    bool HasAppendInstructionUser(Inst *inst) const;
    bool HasPhiOrAppendOrLengthUsersOnly(Inst *inst, Inst *ctorCall, Marker visited) const;
    bool HasAppendOrLengthUsersOnly(Inst *inst, Inst *ctorCall) const;
    bool HasInputInst(Inst *inputInst, Inst *inst) const;

    bool IsInstanceHoistable(const ConcatenationLoopMatch &match) const;
    bool IsToStringHoistable(const ConcatenationLoopMatch &match, Marker appendInstructionVisited) const;

    bool IsPhiAccumulatedValue(PhiInst *phi) const;
    ArenaVector<Inst *> GetPhiAccumulatedValues(Loop *loop);
    void StringBuilderUsagesDFS(Inst *inst, Loop *loop, Marker visited);
    const ArenaVector<StringBuilderUsage> &GetStringBuilderUsagesPO(Inst *accValue);

    // 4. Merges consecutive String Builder append string calls into one appendN call
    void OptimizeStringConcatenation(Loop *loop);
    IntrinsicInst *CreateIntrinsicStringBuilderAppendStrings(const ConcatenationMatch &match, SaveStateInst *saveState);
    void ReplaceWithAppendIntrinsic(const ConcatenationMatch &match);
    using StringBuilderCallsMap = ArenaMap<Inst *, InstVector>;
    const StringBuilderCallsMap &CollectStringBuilderCalls(BasicBlock *block);
    void ReplaceWithAppendIntrinsic(Inst *instance, const InstVector &calls, size_t from, size_t to);
    void OptimizeStringBuilderAppendChain(BasicBlock *block);

    // 5. Merges consecutive String Builders into one String Builder if possible
    using InstPair = std::pair<Inst *, Inst *>;
    using StringBuilderFirstLastCallsMap = ArenaMap<Inst *, InstPair>;
    void CollectStringBuilderFirstCalls(BasicBlock *block);
    void CollectStringBuilderLastCalls(BasicBlock *block);
    void CollectStringBuilderFirstLastCalls();
    void CollectStringBuilderChainCalls(Inst *instance, Inst *inputInstance);
    StringBuilderCallsMap &CollectStringBuilderChainCalls();
    bool CanMergeStringBuilders(Inst *instance, const InstPair &instanceCalls, Inst *inputInstance);
    void Cleanup(Inst *instance, Inst *instanceFirstAppendCall, Inst *inputInstanceToStringCall);
    void CleanupInstruction(Inst *inst);
    void FixBrokenSaveStatesForStringBuilderCalls(Inst *instance);
    void OptimizeStringBuilderChain();

    // 6. Removes extra call for StringBuilder::%%get-stringLength if possible
    void OptimizeStringBuilderStringLength(BasicBlock *block);

private:
    bool isApplied_ {false};
    SaveStateBridgesBuilder ssb_ {};
    ArenaStack<Inst *> instructionsStack_;
    ArenaVector<Inst *> instructionsVector_;
    ArenaVector<InputDesc> inputDescriptors_;
    ArenaVector<StringBuilderUsage> usages_;
    ArenaVector<ConcatenationLoopMatch> matches_;
    StringBuilderCallsMap stringBuilderCalls_;
    StringBuilderFirstLastCallsMap stringBuilderFirstLastCalls_;
    RuntimeInterface::FieldPtr sbStringLengthField_ {nullptr};
    ArenaMap<Inst *, Inst *> sbStringLengthCalls_;
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_SIMPLIFY_STRING_BUILDER_H
