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

#include "libarkbase/macros.h"
#include "unit_test.h"
#include "inst_generator.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/optimizations/locations_builder.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"

#include <optional>

namespace ark::compiler {
class InstGeneratorTest : public GraphTest {
public:
    InstGeneratorTest()
    {
        // Disable regalloc verification as generated code may operate on
        // registers/stack slots containing uninitialized values
        g_options.SetCompilerVerifyRegalloc(false);
    }
};

class CodegenStatisticGenerator : public StatisticGenerator {
public:
    using StatisticGenerator::StatisticGenerator;

    ~CodegenStatisticGenerator() override = default;

    NO_COPY_SEMANTIC(CodegenStatisticGenerator);
    NO_MOVE_SEMANTIC(CodegenStatisticGenerator);

    void Generate() override
    {
        for (const auto &op : instGenerator_.GetMap()) {
            ASSERT(op.first != Opcode::Builtin);
            if (op.first == Opcode::Intrinsic) {
                continue;
            }
            if (graphCreator_.GetRuntimeTargetArch() == Arch::AARCH32 && op.first == Opcode::StoreArrayPair) {
                // StoreArrayPair requires 4 registers for inputs + 1 temp register,
                // which is more than the number of available registers for AARCH32
                // That's why we skip StoreArrayPair generation for AARCH32 target
                continue;
            }
            auto it = instGenerator_.Generate(op.first);
            FullInstStat fullInstStat = tmplt_;
            for (auto &i : it) {
                ASSERT(graphCreator_.GetAllocator()->GetAllocatedSize() == 0U);
                auto graph = graphCreator_.GenerateGraph(i);
                graph->RunPass<RegAllocLinearScan>();
                bool status = graph->RunPass<Codegen>();
                fullInstStat[i->GetType()] = static_cast<std::map<DataType::Type, int8_t>::mapped_type>(status);
                allInstNumber_++;
                positiveInstNumber_ += static_cast<int>(status);
                // To consume less memory
                graph->~Graph();
                graphCreator_.GetAllocator()->Resize(0U);
            }
            statistic_.first.insert({op.first, fullInstStat});
        }
        GenerateIntrinsics();
        for (auto i = 0; i != static_cast<int>(Opcode::NUM_OPCODES); ++i) {
            auto opc = static_cast<Opcode>(i);
            if (opc == Opcode::NOP || opc == Opcode::Intrinsic || opc == Opcode::Builtin) {
                continue;
            }
            allOpcodeNumber_++;
            implementedOpcodeNumber_ += static_cast<int>(statistic_.first.find(opc) != statistic_.first.end());
        }
    }

private:
    void GenerateIntrinsics()
    {
        auto intrinsics = instGenerator_.Generate(Opcode::Intrinsic);
        for (auto &intrinsic : intrinsics) {
            ASSERT(graphCreator_.GetAllocator()->GetAllocatedSize() == 0U);
            auto graph = graphCreator_.GenerateGraph(intrinsic);
            auto id = intrinsic->CastToIntrinsic()->GetIntrinsicId();
#include "compiler/generated/inst_generator_test_ext.inl.h"
            graph->RunPass<RegAllocLinearScan>();
            bool status = graph->RunPass<Codegen>();
            statistic_.second[id] = status;
            allInstNumber_++;
            positiveInstNumber_ += static_cast<int>(status);
            graph->~Graph();
            graphCreator_.GetAllocator()->Resize(0U);
        }
    }
};

TEST_F(InstGeneratorTest, AllInstTestARM64)
{
    ArenaAllocator instAlloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator instGen(instAlloc);

    ArenaAllocator graphAlloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graphLocalAlloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graphCreator(graphAlloc, graphLocalAlloc);

    // ARM64
    CodegenStatisticGenerator statGenArm64(instGen, graphCreator);
    statGenArm64.Generate();
    statGenArm64.GenerateHTMLPage("CodegenStatisticARM64.html");
}

TEST_F(InstGeneratorTest, AllInstTestARM32)
{
    ArenaAllocator instAlloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator instGen(instAlloc);

    ArenaAllocator graphAlloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graphLocalAlloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graphCreator(graphAlloc, graphLocalAlloc);

    // ARM32
    graphCreator.SetRuntimeTargetArch(Arch::AARCH32);
    CodegenStatisticGenerator statGenArm32(instGen, graphCreator);
    statGenArm32.Generate();
    statGenArm32.GenerateHTMLPage("CodegenStatisticARM32.html");
}

TEST_F(InstGeneratorTest, AllInstTestAMD64)
{
    ArenaAllocator instAlloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator instGen(instAlloc);

    ArenaAllocator graphAlloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graphLocalAlloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graphCreator(graphAlloc, graphLocalAlloc);

    // AMD64
    graphCreator.SetRuntimeTargetArch(Arch::X86_64);
    CodegenStatisticGenerator statGenAmd64(instGen, graphCreator);
    statGenAmd64.Generate();
    statGenAmd64.GenerateHTMLPage("CodegenStatisticAMD64.html");
}

}  // namespace ark::compiler

#ifdef USE_VIXL_ARM64
#include "vixl_exec_module.h"
#include <random>
#include <cstring>

#else
#error "Not supported!"
#endif

// NOLINTBEGIN(hicpp-signed-bitwise)
namespace ark::compiler {
class ArithGenerator : public CodegenStatisticGenerator {
    // Seed for random generator
    static constexpr uint64_t SEED = 0x1234;

public:
#ifndef PANDA_NIGHTLY_TEST_ON
    static constexpr uint64_t ITERATION = 20;
#else
    static constexpr uint64_t ITERATION = 250;
#endif
    ArithGenerator() = delete;

    explicit ArithGenerator(InstGenerator &instGenerator, GraphCreator &graphCreator)
        : CodegenStatisticGenerator(instGenerator, graphCreator),
          execModule_(graphCreator_.GetAllocator(), graphCreator_.GetRuntime()) {};

    uint64_t GetRandomData()
    {
        static auto randomGen = std::mt19937_64(SEED);  // NOLINT(cert-msc51-cpp)
        return randomGen();
    };

    template <class T>
    // CC-OFFNXT(huge_method, huge_cyclomatic_complexity, huge_cca_cyclomatic_complexity, G.FUN.01-CPP) big switch-case
    void FixParams([[maybe_unused]] T *param1, [[maybe_unused]] T *param2, [[maybe_unused]] T *param3, Opcode opc)
    {
        switch (opc) {
            case Opcode::Neg:
            case Opcode::NegSR:
            case Opcode::Abs:
            case Opcode::Not:
            case Opcode::Cast:
            case Opcode::Min:
            case Opcode::Max: {
                // single instructions - do not need to fix
                return;
            }
            case Opcode::Add:
            case Opcode::Sub:

            case Opcode::And:
            case Opcode::Or:
            case Opcode::Xor:
            case Opcode::AddI:
            case Opcode::SubI:
            case Opcode::AndI:
            case Opcode::OrI:
            case Opcode::XorI:
            case Opcode::AndNot:
            case Opcode::OrNot:
            case Opcode::XorNot: {
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    // shift parameters to prevent overflow
                    *param1 >>= 2U;
                    *param2 >>= 2U;
                    return;
                }
            }
            case Opcode::Mul:
            case Opcode::Div:
            case Opcode::Mod:
            case Opcode::MNeg: {
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    // shift parameters to prevent overflow
                    *param1 >>= sizeof(T) * 4U;
                    *param2 >>= (sizeof(T) * 4U + 1U);
                    if (*param2 == 0U) {
                        *param2 = *param1 + 1U;
                    }
                    if (*param2 == 0U) {
                        *param2 = *param2 + 1U;
                    };
                    return;
                }
            }
            case Opcode::MAdd:
            case Opcode::MSub:
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    // shift parameters to prevent overflow
                    *param1 >>= sizeof(T) * 4U;
                    *param2 >>= (sizeof(T) * 4U + 2U);
                    *param3 >>= sizeof(T) * 4U;
                    if (*param2 == 0U) {
                        *param2 = *param1 + 1U;
                    }
                    if (*param2 == 0U) {
                        *param2 = *param2 + 1U;
                    };
                    if (*param3 == 0U) {
                        *param3 = *param3 + 1U;
                    }
                }
                return;
            case Opcode::Shl:
            case Opcode::Shr:
            case Opcode::AShr:
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    *param1 >>= 2U;
                    return;
                }
            case Opcode::ShlI:
            case Opcode::ShrI:
            case Opcode::AShrI:
            case Opcode::AddSR:
            case Opcode::SubSR:
            case Opcode::AndSR:
            case Opcode::OrSR:
            case Opcode::XorSR:
            case Opcode::AndNotSR:
            case Opcode::OrNotSR:
            case Opcode::XorNotSR:
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    *param1 >>= 2U;
                    // mask for shift
                    *param2 &= (sizeof(T) - 1U);
                    return;
                }
            default:
                ASSERT_DO(0U, std::cerr << static_cast<int>(opc) << "\n");
        }
    }

    bool IsImmOps(Opcode opc)
    {
        return (opc == Opcode::AddI || opc == Opcode::SubI || opc == Opcode::ShlI || opc == Opcode::ShrI ||
                opc == Opcode::AShrI || opc == Opcode::AndI || opc == Opcode::OrI || opc == Opcode::XorI);
    }

    bool IsUnaryShiftedRegisterOps(Opcode opc)
    {
        return opc == Opcode::NegSR;
    }

    bool IsBinaryShiftedRegisterOps(Opcode opc)
    {
        return (opc == Opcode::AddSR || opc == Opcode::SubSR || opc == Opcode::AndSR || opc == Opcode::OrSR ||
                opc == Opcode::XorSR || opc == Opcode::AndNotSR || opc == Opcode::OrNotSR || opc == Opcode::XorNotSR);
    }

    bool IsTernary(Opcode opc)
    {
        return opc == Opcode::MAdd || opc == Opcode::MSub;
    }

    std::tuple<uint64_t, uint64_t, uint64_t> GetRandValues()
    {
        return {GetRandomData(), GetRandomData(), GetRandomData()};
    }

    uint64_t GetRandValue()
    {
        return GetRandomData();
    }

    template <class ParamType>
    void Generate(Opcode opc, std::pair<ParamType, ParamType> vals)
    {
        Generate(opc, std::make_tuple(vals.first, vals.second, static_cast<ParamType>(0U)));
    }

    template <class ParamType>
    std::optional<ParamType> ExecModule(Graph *graph, Opcode opc, ParamType param1, ParamType param2, ParamType param3)
    {
        graph->RunPass<RegAllocLinearScan>();
        if (!graph->RunPass<Codegen>()) {
            return std::nullopt;
        };
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        execModule_.SetInstructions(codeEntry, codeExit);

        execModule_.SetDump(true);

        execModule_.SetParameter(0U, param1);

        if (!IsImmOps(opc)) {
            execModule_.SetParameter(1U, param2);
        }

        if (IsTernary(opc)) {
            execModule_.SetParameter(2U, param3);
        }

        execModule_.Execute();
        return execModule_.GetRetValue<ParamType>();
    }

    template <class ParamType>
    void CheckData(ParamType calcData, ParamType retData, std::tuple<ParamType, ParamType, ParamType> vals, Inst *inst)
    {
        if (calcData != retData) {
            auto param1 = std::get<0U>(vals);
            auto param2 = std::get<1U>(vals);
            auto param3 = std::get<2U>(vals);
            std::cout << "  data " << retData << " sizeof type  " << static_cast<uint64_t>(sizeof(ParamType) * 8U)
                      << " \n";
            std::cout << std::hex << "parameter_1 = " << param1 << " parameter_2 = " << param2
                      << "parameter_3 = " << param3 << "\n";
            inst->Dump(&std::cerr);
            std::cout << "calculated = " << calcData << " returned " << retData << "\n";
            execModule_.SetDump(true);
            execModule_.PrintInstructions();
            execModule_.SetParameter(0U, param1);
            execModule_.SetParameter(1U, param2);
            execModule_.SetParameter(2U, param3);
            execModule_.Execute();
            execModule_.SetDump(false);
        }
        ASSERT_EQ(calcData, retData);
    }

    template <class ParamType>
    void Generate(Opcode opc, std::tuple<ParamType, ParamType, ParamType> vals)
    {
        auto it = instGenerator_.Generate(opc);
        for (auto &inst : it) {
            auto type = inst->GetType();
            auto shiftType = ShiftType::INVALID_SHIFT;
            if (VixlExecModule::GetType<ParamType>() != type) {
                continue;
            }
            auto param1 = std::get<0U>(vals);
            auto param2 = std::get<1U>(vals);
            auto param3 = std::get<2U>(vals);
            FixParams<ParamType>(&param1, &param2, &param3, opc);
            if (IsImmOps(opc)) {
                static_cast<BinaryImmOperation *>(inst)->SetImm(param2);
            } else if (IsUnaryShiftedRegisterOps(opc)) {
                static_cast<UnaryShiftedRegisterOperation *>(inst)->SetImm(param2);
                shiftType = static_cast<UnaryShiftedRegisterOperation *>(inst)->GetShiftType();
            } else if (IsBinaryShiftedRegisterOps(opc)) {
                static_cast<BinaryShiftedRegisterOperation *>(inst)->SetImm(param3);
                shiftType = static_cast<BinaryShiftedRegisterOperation *>(inst)->GetShiftType();
            }
            auto graph = graphCreator_.GenerateGraph(inst);
            auto finalizer = [&graph]([[maybe_unused]] void *ptr) {
                if (graph != nullptr) {
                    graph->~Graph();
                }
            };
            std::unique_ptr<void, decltype(finalizer)> fin(&finalizer, finalizer);
            auto retData = ExecModule(graph, opc, param1, param2, param3);
            if (!retData.has_value()) {
                return;
            }
            auto calcData = DoLogic<ParamType>(opc, param1, param2, param3, shiftType,
                                               DataType::GetTypeSize(type, graph->GetArch()));
            CheckData<ParamType>(calcData, retData.value(), vals, inst);
        }
    }

    template <class ParamType, class ResultType>
    std::optional<ResultType> ExecModule(Graph *graph, ParamType param1)
    {
        graph->RunPass<RegAllocLinearScan>();
        if (!graph->RunPass<Codegen>()) {
            return std::nullopt;
        };
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        execModule_.SetInstructions(codeEntry, codeExit);

        execModule_.SetDump(false);
        execModule_.SetParameter(0U, param1);
        execModule_.Execute();

        return execModule_.GetRetValue<ResultType>();
    }

    template <class ParamType, class ResultType>
    ResultType DoCast(ParamType param1)
    {
        if constexpr ((GetCommonType(VixlExecModule::GetType<ResultType>()) == DataType::Type::INT64) &&
                      (std::is_same_v<ParamType, float> || std::is_same_v<ParamType, double>)) {
            if (param1 > static_cast<ParamType>(std::numeric_limits<ResultType>::max())) {
                return std::numeric_limits<ResultType>::max();
            }
            if (param1 < static_cast<ParamType>(std::numeric_limits<ResultType>::min())) {
                return std::numeric_limits<ResultType>::min();
            }
            return static_cast<ResultType>(param1);
        }
        return static_cast<ResultType>(param1);
    }

    template <class ParamType, class ResultType>
    void CheckData(ResultType calcData, ResultType retData, ParamType param1, Inst *inst)
    {
        if (calcData != retData) {
            if constexpr (std::is_same_v<ResultType, double> || std::is_same_v<ParamType, double>) {
                std::cout << std::hex << " in parameter = " << param1 << "\n";
                std::cout << std::hex << "parameter_1 = " << static_cast<double>(param1) << "\n";
            }

#ifndef NDEBUG
            std::cout << " cast from " << DataType::ToString(VixlExecModule::GetType<ParamType>()) << " to "
                      << DataType::ToString(VixlExecModule::GetType<ResultType>()) << "\n";
#endif
            std::cout << "  data " << retData << " sizeof type  " << static_cast<uint64_t>(sizeof(ParamType) * 8U)
                      << " \n";
            inst->Dump(&std::cerr);
            execModule_.SetDump(true);
            execModule_.SetParameter(0U, param1);
            execModule_.Execute();
            execModule_.SetDump(false);
        }
        ASSERT_EQ(calcData, retData);
    }

    template <class ParamType, class ResultType>
    void GenCast(ParamType val)
    {
        auto opc = Opcode::Cast;

        auto it = instGenerator_.Generate(opc);
        for (auto &inst : it) {
            auto type = inst->GetType();
            if (VixlExecModule::GetType<ParamType>() != type) {
                continue;
            }
            ParamType param1 = val;

            auto graph = graphCreator_.GenerateGraph(inst);

            auto finalizer = [&graph]([[maybe_unused]] void *ptr) {
                if (graph != nullptr) {
                    graph->~Graph();
                }
            };
            std::unique_ptr<void, decltype(finalizer)> fin(&finalizer, finalizer);

            // Reset type for Cast-destination:
            inst->SetType(VixlExecModule::GetType<ResultType>());
            // Fix return value
            for (auto &iter : inst->GetUsers()) {
                iter.GetInst()->SetType(VixlExecModule::GetType<ResultType>());
            }
            auto retData = ExecModule<ParamType, ResultType>(graph, param1);
            if (!retData.has_value()) {
                return;
            }
            auto calcData = DoCast<ParamType, ResultType>(param1);
            CheckData(calcData, retData.value(), param1, inst);
        }
    }

    template <class T>
    T DoShift(T value, ShiftType shiftType, uint64_t scale, uint8_t typeSize)
    {
        switch (shiftType) {
            case ShiftType::LSL:
                return static_cast<uint64_t>(value) << scale;
            case ShiftType::ROR:
                return (static_cast<uint64_t>(value) >> scale) |
                       (value << (typeSize - scale));  // NOLINT(hicpp-signed-bitwise)
            case ShiftType::LSR:
                if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>) {
                    return static_cast<uint32_t>(value) >> static_cast<uint32_t>(scale);
                }
                if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
                    return static_cast<uint64_t>(value) >> scale;
                }
            /* fall-through */
            case ShiftType::ASR:
                return static_cast<int64_t>(value) >> scale;  // NOLINT(hicpp-signed-bitwise)
            default:
                UNREACHABLE();
        }
    }

    // NOLINTBEGIN(hicpp-signed-bitwise)
    // Make logic with parameters (default - first parameter)
    template <class T>
    // CC-OFFNXT(huge_method, huge_cyclomatic_complexity, huge_cca_cyclomatic_complexity, G.FUN.01-CPP) big switch-case
    T DoLogic(Opcode opc, T param1, [[maybe_unused]] T param2, [[maybe_unused]] T param3,
              [[maybe_unused]] ShiftType shiftType, [[maybe_unused]] uint8_t typeSize)
    {
        constexpr DataType::Type TYPE = ConstantInst::GetTypeFromCType<T>();
        constexpr bool ARITHMETIC_TYPE = (TYPE == DataType::INT64);
        switch (opc) {
            case Opcode::Neg:
                return -param1;
            case Opcode::MAdd:
                return param1 * param2 + param3;
            case Opcode::MSub:
                return param3 - param1 * param2;
            case Opcode::Not:
                return (-param1 - 1L);
            case Opcode::Add:
            case Opcode::AddI:
                return (param1 + param2);
            case Opcode::Sub:
            case Opcode::SubI:
                return (param1 - param2);
            case Opcode::Mul:
                return (param1 * param2);
            case Opcode::MNeg:
                return -(param1 * param2);
            case Opcode::Div:
                ASSERT_PRINT(param2 != 0U, "If you got this assert, you may change SEED");
                return (param1 / param2);
            case Opcode::Mod:
                if constexpr (ARITHMETIC_TYPE) {
                    ASSERT_PRINT(param2 != 0U, "If you got this assert, you may change SEED");
                    return param1 % param2;
                } else {
                    return fmod(param1, param2);
                }
            case Opcode::Min:
                return std::min(param1, param2);
            case Opcode::Max:
                return std::max(param1, param2);

            case Opcode::NegSR:
                if constexpr (ARITHMETIC_TYPE) {
                    return -(DoShift(param1, shiftType, param2, typeSize));
                }
                /* fall-through */
            case Opcode::ShlI:
                if constexpr (ARITHMETIC_TYPE) {
                    return DoShift(param1, ShiftType::LSL, param2, typeSize);
                }
                /* fall-through */
            case Opcode::Shl:
                if constexpr (ARITHMETIC_TYPE) {
                    return DoShift(param1, ShiftType::LSL, param2 & (typeSize - 1L), typeSize);
                }
                /* fall-through */
            case Opcode::ShrI:
                if constexpr (ARITHMETIC_TYPE) {
                    return DoShift(param1, ShiftType::LSR, param2, typeSize);
                }
                /* fall-through */
            case Opcode::Shr:
                if constexpr (ARITHMETIC_TYPE) {
                    return DoShift(param1, ShiftType::LSR, param2 & (typeSize - 1L), typeSize);
                }
                /* fall-through */
            case Opcode::AShr:
                if constexpr (ARITHMETIC_TYPE) {
                    return DoShift(param1, ShiftType::ASR, param2 & (typeSize - 1L), typeSize);
                }
                /* fall-through */
            case Opcode::AShrI:
                if constexpr (ARITHMETIC_TYPE) {
                    return DoShift(param1, ShiftType::ASR, param2, typeSize);
                }
                /* fall-through */
            case Opcode::And:
            case Opcode::AndI:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 & param2;
                }
                /* fall-through */
            case Opcode::AndSR:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 & DoShift(param2, shiftType, param3, typeSize);
                }
                /* fall-through */
            case Opcode::Or:
            case Opcode::OrI:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 | param2;
                }
                /* fall-through */
            case Opcode::OrSR:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 | DoShift(param2, shiftType, param3, typeSize);
                }
                /* fall-through */
            case Opcode::Xor:
            case Opcode::XorI:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 ^ param2;
                }
                /* fall-through */
            case Opcode::XorSR:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 ^ DoShift(param2, shiftType, param3, typeSize);
                }
                /* fall-through */
            case Opcode::AndNot:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 & (~param2);
                }
                /* fall-through */
            case Opcode::AndNotSR:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 & (~DoShift(param2, shiftType, param3, typeSize));
                }
                /* fall-through */
            case Opcode::OrNot:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 | (~param2);
                }
                /* fall-through */
            case Opcode::OrNotSR:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 | (~DoShift(param2, shiftType, param3, typeSize));
                }
                /* fall-through */
            case Opcode::XorNot:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 ^ (~param2);
                }
                /* fall-through */
            case Opcode::XorNotSR:
                if constexpr (ARITHMETIC_TYPE) {
                    return param1 ^ (~DoShift(param2, shiftType, param3, typeSize));
                }
                /* fall-through */
            case Opcode::Abs:
                if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
                              std::is_same_v<T, int64_t>) {
                    return std::abs(param1);
                }
                /* fall-through */
            default:
                ASSERT_DO(false, std::cerr << "Unsupported!" << static_cast<int>(opc) << "\n");
                return -1L;
        }
    }
    // NOLINTEND(hicpp-signed-bitwise)

private:
    VixlExecModule execModule_;
};
// NOLINTEND(hicpp-signed-bitwise)

void OneTest(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<int8_t>(opc, gen.GetRandValues());
    gen.Generate<int16_t>(opc, gen.GetRandValues());
    gen.Generate<int32_t>(opc, gen.GetRandValues());
    gen.Generate<int64_t>(opc, gen.GetRandValues());
    gen.Generate<uint8_t>(opc, gen.GetRandValues());
    gen.Generate<uint16_t>(opc, gen.GetRandValues());
    gen.Generate<uint32_t>(opc, gen.GetRandValues());
    gen.Generate<uint64_t>(opc, gen.GetRandValues());
}

static void OneTestCastSignedInt(ArithGenerator &gen)
{
    gen.GenCast<int8_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int8_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int8_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int8_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int8_t, float>(gen.GetRandValue());
    gen.GenCast<int8_t, double>(gen.GetRandValue());

    gen.GenCast<int16_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int16_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int16_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int16_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int16_t, float>(gen.GetRandValue());
    gen.GenCast<int16_t, double>(gen.GetRandValue());

    gen.GenCast<int32_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int32_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int32_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int32_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int32_t, float>(gen.GetRandValue());
    gen.GenCast<int32_t, double>(gen.GetRandValue());

    gen.GenCast<int64_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int64_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int64_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int64_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int64_t, float>(gen.GetRandValue());
    gen.GenCast<int64_t, double>(gen.GetRandValue());
}

static void OneTestCastUnsignedInt(ArithGenerator &gen)
{
    gen.GenCast<uint8_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, float>(gen.GetRandValue());
    gen.GenCast<uint8_t, double>(gen.GetRandValue());

    gen.GenCast<uint16_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, float>(gen.GetRandValue());
    gen.GenCast<uint16_t, double>(gen.GetRandValue());

    gen.GenCast<uint32_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, float>(gen.GetRandValue());
    gen.GenCast<uint32_t, double>(gen.GetRandValue());

    gen.GenCast<uint64_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, float>(gen.GetRandValue());
    gen.GenCast<uint64_t, double>(gen.GetRandValue());
}

void OneTestCast(ArithGenerator &gen)
{
    OneTestCastSignedInt(gen);
    OneTestCastUnsignedInt(gen);

    // We DON'T support cast from float32 to int8/16.
    gen.GenCast<float, uint32_t>(gen.GetRandValue());
    gen.GenCast<float, uint64_t>(gen.GetRandValue());
    gen.GenCast<float, int32_t>(gen.GetRandValue());
    gen.GenCast<float, int64_t>(gen.GetRandValue());
    gen.GenCast<float, float>(gen.GetRandValue());
    gen.GenCast<float, double>(gen.GetRandValue());

    // We DON'T support cast from float64 to int8/16.
    gen.GenCast<double, uint32_t>(gen.GetRandValue());
    gen.GenCast<double, uint64_t>(gen.GetRandValue());
    gen.GenCast<double, int32_t>(gen.GetRandValue());
    gen.GenCast<double, int64_t>(gen.GetRandValue());
    gen.GenCast<double, float>(gen.GetRandValue());
    gen.GenCast<double, double>(gen.GetRandValue());
}

void OneTestSign(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<int8_t>(opc, gen.GetRandValues());
    gen.Generate<int16_t>(opc, gen.GetRandValues());
    gen.Generate<int32_t>(opc, gen.GetRandValues());
    gen.Generate<int64_t>(opc, gen.GetRandValues());
}

void OneTestFP(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<float>(opc, gen.GetRandValues());
    gen.Generate<double>(opc, gen.GetRandValues());
}

// NOLINTBEGIN(readability-magic-numbers)
void OneTestShift(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<uint64_t>(opc, {0x8899aabbccddeeffU, 32U});
    gen.Generate<uint64_t>(opc, {0x8899aabbccddeeffU, 32U + 64U});
    gen.Generate<int64_t>(opc, {0x8899aabbccddeeffU, 32U});
    gen.Generate<int64_t>(opc, {0x8899aabbccddeeffU, 32U + 64U});
    gen.Generate<uint32_t>(opc, {0xccddeeffU, 16U});
    gen.Generate<uint32_t>(opc, {0xccddeeffU, 16U + 32U});
    gen.Generate<int32_t>(opc, {0xccddeeffU, 0xffffffffU});
    gen.Generate<int32_t>(opc, {0xccddeeffU, 16U});
    gen.Generate<int32_t>(opc, {0xccddeeffU, 16U + 32U});
    gen.Generate<uint16_t>(opc, {0xeeffU, 8U});
    gen.Generate<uint16_t>(opc, {0xeeffU, 8U + 16U});
    gen.Generate<int16_t>(opc, {0xeeffU, 8U});
    gen.Generate<int16_t>(opc, {0xeeffU, 8U + 16U});
    gen.Generate<uint8_t>(opc, {0xffU, 4U});
    gen.Generate<uint8_t>(opc, {0xffU, 4U + 8U});
    gen.Generate<int8_t>(opc, {0xffU, 4U});
    gen.Generate<int8_t>(opc, {0xffU, 4U + 8U});
}
// NOLINTEND(readability-magic-numbers)

// There is not enough memory in the arena allocator, so it is divided into 2 parts
void RandomTestsPart1()
{
    ArenaAllocator alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator localAlloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator instGen(alloc);
    GraphCreator graphCreator(alloc, localAlloc);
    ArithGenerator statGen(instGen, graphCreator);

    OneTest(statGen, Opcode::Add);
    OneTestFP(statGen, Opcode::Add);

    OneTest(statGen, Opcode::Sub);
    OneTestFP(statGen, Opcode::Sub);

    OneTest(statGen, Opcode::Mul);
    OneTestFP(statGen, Opcode::Mul);

    OneTest(statGen, Opcode::Div);
    OneTestFP(statGen, Opcode::Div);

    OneTest(statGen, Opcode::Mod);
    // Disabled, because external fmod() call is currently x86_64 -- incompatible with aarch64 runtime :(
    // stat_gen  Opcode::Mod

    OneTest(statGen, Opcode::Min);
    OneTestFP(statGen, Opcode::Min);

    OneTest(statGen, Opcode::Max);
    OneTestFP(statGen, Opcode::Max);

    OneTest(statGen, Opcode::Shl);
    OneTest(statGen, Opcode::Shr);
    OneTest(statGen, Opcode::AShr);
}

void RandomTestsPart2()
{
    ArenaAllocator alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator localAlloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator instGen(alloc);
    GraphCreator graphCreator(alloc, localAlloc);
    ArithGenerator statGen(instGen, graphCreator);

    OneTest(statGen, Opcode::And);
    // Float unsupported

    OneTest(statGen, Opcode::Or);
    // Float unsupported

    OneTest(statGen, Opcode::Xor);
    // Float unsupported

    OneTest(statGen, Opcode::Neg);

    OneTestSign(statGen, Opcode::Abs);

    OneTest(statGen, Opcode::Not);

    OneTest(statGen, Opcode::AddI);

    OneTest(statGen, Opcode::SubI);

    OneTest(statGen, Opcode::ShlI);

    OneTest(statGen, Opcode::ShrI);

    OneTest(statGen, Opcode::AShrI);

    OneTest(statGen, Opcode::AndI);

    OneTest(statGen, Opcode::OrI);

    OneTest(statGen, Opcode::XorI);

    // Special case for Case-instruction - generate inputs types.
    OneTestCast(statGen);
}

void NotRandomTests()
{
    ArenaAllocator alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator localAlloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator instGen(alloc);
    GraphCreator graphCreator(alloc, localAlloc);
    ArithGenerator statGen(instGen, graphCreator);

    statGen.Generate<uint64_t>(Opcode::Min, {UINT64_MAX, 0U});
    statGen.Generate<uint64_t>(Opcode::Min, {0U, UINT64_MAX});
    statGen.Generate<int64_t>(Opcode::Min, {0U, UINT64_MAX});
    statGen.Generate<int64_t>(Opcode::Min, {0U, UINT64_MAX});
    OneTestShift(statGen, Opcode::Shl);
    OneTestShift(statGen, Opcode::Shr);
    OneTestShift(statGen, Opcode::AShr);
}

TEST_F(InstGeneratorTest, GenArithVixlCode)
{
    for (uint64_t i = 0; i < ArithGenerator::ITERATION; ++i) {
        RandomTestsPart1();
        RandomTestsPart2();
    }
    NotRandomTests();
}

static void DestroyIfNotNull(Graph *graph)
{
    if (graph != nullptr) {
        graph->~Graph();
    }
}

/**
 * Check that all possible instructions that introduce a reference as a result
 * are handled in analysis.  On failed test add a case to AliasVisitor.
 */
TEST_F(InstGeneratorTest, AliasAnalysisSupportTest)
{
    ArenaAllocator instAlloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator instGen(instAlloc);

    ArenaAllocator graphAlloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graphLocalAlloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graphCreator(graphAlloc, graphLocalAlloc);

    for (const auto &op : instGen.GetMap()) {
        auto it = instGen.Generate(op.first);
        for (auto &i : it) {
            if (i->GetType() != DataType::REFERENCE) {
                continue;
            }
            auto graph = graphCreator.GenerateGraph(i);

            auto finalizer = [&graph]([[maybe_unused]] void *ptr) { DestroyIfNotNull(graph); };
            std::unique_ptr<void, decltype(finalizer)> fin(&finalizer, finalizer);

            graph->RunPass<AliasAnalysis>();
            EXPECT_TRUE(graph->IsAnalysisValid<AliasAnalysis>());
        }
    }
}
}  // namespace ark::compiler
