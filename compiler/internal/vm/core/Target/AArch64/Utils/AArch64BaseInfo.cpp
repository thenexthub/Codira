//===-- AArch64BaseInfo.cpp - AArch64 Base encoding information------------===//
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
//
// This file provides basic encoding and assembly information for AArch64.
//
//===----------------------------------------------------------------------===//
#include "AArch64BaseInfo.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/Regex.h"

using namespace vm::core;

namespace vm::core {
  namespace AArch64AT {
#define GET_ATsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}


namespace vm::core {
  namespace AArch64DBnXS {
#define GET_DBnXSsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64DB {
#define GET_DBsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64DC {
#define GET_DCsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64IC {
#define GET_ICsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64ISB {
#define GET_ISBsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64TSB {
#define GET_TSBsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64PRFM {
#define GET_PRFMsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64SVEPRFM {
#define GET_SVEPRFMsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64RPRFM {
#define GET_RPRFMsList_IMPL
#include "AArch64GenSystemOperands.inc"
  } // namespace AArch64RPRFM
} // namespace vm::core

namespace vm::core {
  namespace AArch64SVEPredPattern {
#define GET_SVEPREDPATsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
namespace AArch64SVEVecLenSpecifier {
#define GET_SVEVECLENSPECIFIERsList_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64SVEVecLenSpecifier
} // namespace vm::core

namespace vm::core {
  namespace AArch64ExactFPImm {
#define GET_ExactFPImmsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64PState {
#define GET_PStateImm0_15sList_IMPL
#include "AArch64GenSystemOperands.inc"
#define GET_PStateImm0_1sList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
  namespace AArch64PSBHint {
#define GET_PSBsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
namespace AArch64PHint {
#define GET_PHintsList_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64PHint
} // namespace vm::core

namespace vm::core {
  namespace AArch64BTIHint {
#define GET_BTIsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

namespace vm::core {
namespace AArch64CMHPriorityHint {
#define GET_CMHPRIORITYHINT_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64CMHPriorityHint
} // namespace vm::core

namespace vm::core {
namespace AArch64TIndexHint {
#define GET_TINDEX_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64TIndexHint
} // namespace vm::core

namespace vm::core {
  namespace AArch64SysReg {
#define GET_SysRegsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}

uint32_t AArch64SysReg::parseGenericRegister(StringRef Name) {
  // Try to parse an S<op0>_<op1>_<Cn>_<Cm>_<op2> register name
  static const Regex GenericRegPattern("^S([0-3])_([0-7])_C([0-9]|1[0-5])_C([0-9]|1[0-5])_([0-7])$");

  std::string UpperName = Name.upper();
  SmallVector<StringRef, 5> Ops;
  if (!GenericRegPattern.match(UpperName, &Ops))
    return -1;

  uint32_t Op0 = 0, Op1 = 0, CRn = 0, CRm = 0, Op2 = 0;
  uint32_t Bits;
  Ops[1].getAsInteger(10, Op0);
  Ops[2].getAsInteger(10, Op1);
  Ops[3].getAsInteger(10, CRn);
  Ops[4].getAsInteger(10, CRm);
  Ops[5].getAsInteger(10, Op2);
  Bits = (Op0 << 14) | (Op1 << 11) | (CRn << 7) | (CRm << 3) | Op2;

  return Bits;
}

std::string AArch64SysReg::genericRegisterString(uint32_t Bits) {
  assert(Bits < 0x10000);
  uint32_t Op0 = (Bits >> 14) & 0x3;
  uint32_t Op1 = (Bits >> 11) & 0x7;
  uint32_t CRn = (Bits >> 7) & 0xf;
  uint32_t CRm = (Bits >> 3) & 0xf;
  uint32_t Op2 = Bits & 0x7;

  return "S" + utostr(Op0) + "_" + utostr(Op1) + "_C" + utostr(CRn) + "_C" +
         utostr(CRm) + "_" + utostr(Op2);
}

namespace vm::core {
namespace AArch64TLBI {
#define GET_TLBITable_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64TLBI
} // namespace vm::core

namespace vm::core {
namespace AArch64PLBI {
#define GET_PLBITable_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64PLBI
} // namespace vm::core

namespace vm::core {
namespace AArch64TLBIP {
#define GET_TLBIPTable_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64TLBIP

namespace AArch64MLBI {
#define GET_MLBITable_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64MLBI
} // namespace vm::core

namespace vm::core {
namespace AArch64GIC {
#define GET_GICTable_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64GIC
} // namespace vm::core

namespace vm::core {
namespace AArch64GICR {
#define GET_GICRTable_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64GICR
} // namespace vm::core

namespace vm::core {
namespace AArch64GSB {
#define GET_GSBTable_IMPL
#include "AArch64GenSystemOperands.inc"
} // namespace AArch64GSB
} // namespace vm::core

namespace vm::core {
  namespace AArch64SVCR {
#define GET_SVCRsList_IMPL
#include "AArch64GenSystemOperands.inc"
  }
}
