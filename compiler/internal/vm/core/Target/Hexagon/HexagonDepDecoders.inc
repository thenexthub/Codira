//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
// Automatically generated file, do not edit!
//===----------------------------------------------------------------------===//


#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

static DecodeStatus s6_0ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<6>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s31_1ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                    const MCDisassembler *Decoder) {
  signedDecoder<12>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s30_2ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                    const MCDisassembler *Decoder) {
  signedDecoder<13>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s29_3ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                    const MCDisassembler *Decoder) {
  signedDecoder<14>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s3_0ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<3>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s4_0ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<4>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s4_1ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<5>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s4_2ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<6>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s4_3ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<7>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s6_3ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<9>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}
static DecodeStatus s8_0ImmDecoder(MCInst &MI, unsigned tmp, uint64_t,
                                   const MCDisassembler *Decoder) {
  signedDecoder<8>(MI, tmp, Decoder);
  return MCDisassembler::Success;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

