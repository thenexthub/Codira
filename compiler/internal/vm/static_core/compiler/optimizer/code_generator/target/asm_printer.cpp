/**
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

#include "asm_printer.h"

namespace ark::compiler {
#ifdef PANDA_COMPILER_TARGET_AARCH32
template <>
void PrinterLabelHolder<AssemblyPrinter<aarch32::Aarch32Encoder>>::BindLabel(LabelId id)
{
    auto encoder = reinterpret_cast<AssemblyPrinter<aarch32::Aarch32Encoder> *>(GetEncoder());
    auto stream = encoder->GetStream();
    auto str = labels_[id];
    *stream << "." << str << ":\n";
}
template <>
AssemblyPrinter<aarch32::Aarch32Encoder>::AssemblyPrinter(ArenaAllocator *aa, aarch32::Aarch32Encoder *enc)
    : Encoder(aa, enc->GetArch()), enc_(enc)
{
    labels_ = aa->template New<PrinterLabelHolder<AssemblyPrinter<aarch32::Aarch32Encoder>>>(this);
}

template <>
void AssemblyPrinter<aarch32::Aarch32Encoder>::MakeCall(const void *entryPoint)
{
    *str_ << "blx " << std::hex << entryPoint << "\n";
}
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
template <>
void PrinterLabelHolder<AssemblyPrinter<aarch64::Aarch64Encoder>>::BindLabel(LabelId id)
{
    auto encoder = reinterpret_cast<AssemblyPrinter<aarch64::Aarch64Encoder> *>(GetEncoder());
    auto stream = encoder->GetStream();
    auto str = labels_[id];
    *stream << "." << str << ":\n";
}
template <>
AssemblyPrinter<aarch64::Aarch64Encoder>::AssemblyPrinter(ArenaAllocator *aa, aarch64::Aarch64Encoder *enc)
    : Encoder(aa, enc->GetArch()), enc_(enc)
{
    labels_ = aa->template New<PrinterLabelHolder<AssemblyPrinter<aarch64::Aarch64Encoder>>>(this);
}
template <>
void AssemblyPrinter<aarch64::Aarch64Encoder>::MakeCall(const void *entryPoint)
{
    *str_ << "bl " << std::hex << entryPoint << "\n";
}
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
template <>
void PrinterLabelHolder<AssemblyPrinter<amd64::Amd64Encoder>>::BindLabel(LabelId id)
{
    auto encoder = reinterpret_cast<AssemblyPrinter<amd64::Amd64Encoder> *>(GetEncoder());
    auto stream = encoder->GetStream();
    auto str = labels_[id];
    *stream << "." << str << ":\n";
}
template <>
AssemblyPrinter<amd64::Amd64Encoder>::AssemblyPrinter(ArenaAllocator *aa, amd64::Amd64Encoder *enc)
    : Encoder(aa, enc->GetArch()), enc_(enc)
{
    labels_ = aa->template New<PrinterLabelHolder<AssemblyPrinter<amd64::Amd64Encoder>>>(this);
}
template <>
void AssemblyPrinter<amd64::Amd64Encoder>::MakeCall(const void *entryPoint)
{
    *str_ << "callq " << std::hex << entryPoint << "@plt\n";
}
#endif
}  // namespace ark::compiler
