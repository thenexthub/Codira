/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef COMPILER_OPTIMIZER_IR_VISUALIZER_PRINTER_H
#define COMPILER_OPTIMIZER_IR_VISUALIZER_PRINTER_H

#include <iostream>
#include <string>

namespace ark::compiler {
// This class dump graph in C1Visualizer format
// Option: --compiler-visualizer-dump
// Open dump files:
//   1. Download c1visualizer (http://lafo.ssw.uni-linz.ac.at/c1visualizer/c1visualizer-1.7.zip).
//   2. Run c1visualizer for your OS from folder "bin".
//   3. Open files.

class VisualizerPrinter {
public:
    VisualizerPrinter(const Graph *graph, std::ostream *output, const char *passName)
        : graph_(graph), output_(output), passName_(passName)
    {
    }

    NO_MOVE_SEMANTIC(VisualizerPrinter);
    NO_COPY_SEMANTIC(VisualizerPrinter);
    ~VisualizerPrinter() = default;

    void Print();

private:
    void PrintBeginTag(const char *tag);

    void PrintEndTag(const char *tag);

    std::string MakeOffset();

    void PrintProperty(const char *prop, const ArenaString &value);

    void PrintProperty(const char *prop, int value);

    template <typename T>
    void PrintDependences(const std::string &preffix, const T &blocks);

    void PrintBasicBlock(BasicBlock *block);

    void PrintInsts(BasicBlock *block);

    void PrintInst(Inst *inst);

private:
    const Graph *graph_;
    std::ostream *output_;
    const char *passName_;
    uint32_t offset_ {0};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_VISUALIZER_PRINTER_H
