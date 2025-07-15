#include "language/Basic/ClusteredBitVector.h"
#include "toolchain/Support/Format.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/ADT/Twine.h"
#include <vector>
#include "stdlib.h"

using namespace language;

const unsigned NV = 16;

/// Random number in [0,n)
unsigned randCount(unsigned n) {
  return unsigned(rand()) % n;
}
unsigned randNV() { return randCount(NV); }

static void checkConsistency(const Twine &name, const ClusteredBitVector &cbv,
                        const std::vector<bool> &vec, unsigned depth) {
  auto finish = [=]() {
    for (unsigned j = depth; j != 0; --j) {
      toolchain::outs().indent(2 * (j-1)) << "}\n";
    }
    abort();
  };

  auto n = cbv.size();
  if (n != vec.size()) {
    toolchain::outs().indent(2 * depth)
      << "assert(" << name << ".size() == " << vec.size() << ");\n";
    finish();
  }

  for (auto i = 0; i != n; ++i) {
    if (cbv[i] != vec[i]) {
      toolchain::outs().indent(2 * depth)
        << "assert(" << name << "[" << i << "] == "
        << (vec[i] ? "true" : "false") << ");\n";
      finish();
    }
  }
}

static void run() {
  toolchain::outs() << "#include \"language/Basic/ClusteredBitVector.h\"\n";
  toolchain::outs() << "using namespace language;\n";
  toolchain::outs() << "int main() {\n";
  toolchain::outs() << "  ClusteredBitVector cbvs[" << NV << "];\n";
  ClusteredBitVector cbvs[NV];
  std::vector<bool> vecs[NV];

  unsigned nextTemp = 0;

  while (true) {
    switch (randCount(10)) {
    case 0: {
      auto from = randNV();
      auto to = randNV();
      if (from == to) continue;
      toolchain::outs() << "  cbvs[" << to << "].append(cbvs[" << from << "]);\n";
      cbvs[to].append(cbvs[from]);
      vecs[to].insert(vecs[to].end(), vecs[from].begin(), vecs[from].end());
      break;
    }

    case 1: {
      auto from = randNV();
      auto to = randNV();
      if (from == to) continue;
      toolchain::outs() << "  cbvs[" << to << "] = cbvs[" << from << "];\n";
      cbvs[to] = cbvs[from];
      vecs[to] = vecs[from];
      break;
    }

    case 2: {
      auto from = randNV();
      auto to = randNV();
      if (from == to) continue;
      toolchain::outs() << "  cbvs[" << to << "] = std::move(cbvs[" << from << "]);\n";
      cbvs[to] = std::move(cbvs[from]);
      vecs[to] = std::move(vecs[from]);
      break;
    }

    case 3: {
      auto from = randNV();
      auto to = randNV();
      auto temp = nextTemp++;
      toolchain::outs() << "  { ClusteredBitVector temp" << temp << " = cbvs[" << from << "];\n";
      ClusteredBitVector tempCBV = cbvs[from];
      auto tempVec = vecs[from];
      checkConsistency("temp" + Twine(temp), tempCBV, tempVec, 2);
      toolchain::outs() << "    cbvs[" << to << "] = temp" << temp << "; }\n";
      cbvs[to] = tempCBV;
      vecs[to] = tempVec;
      break;
    }

    case 4: {
      auto from = randNV();
      auto to = randNV();
      auto temp = nextTemp++;
      toolchain::outs() << "  { ClusteredBitVector temp" << temp << " = std::move(cbvs[" << from << "]);\n";
      ClusteredBitVector tempCBV = std::move(cbvs[from]);
      auto tempVec = std::move(vecs[from]);
      checkConsistency("temp" + Twine(temp), tempCBV, tempVec, 2);
      toolchain::outs() << "    cbvs[" << to << "] = temp" << temp << "; }\n";
      cbvs[to] = tempCBV;
      vecs[to] = tempVec;
      break;
    }

    case 5: {
      auto from = randNV();
      auto to = randNV();
      auto temp = nextTemp++;
      toolchain::outs() << "  { ClusteredBitVector temp" << temp << " = cbvs[" << from << "];\n";
      ClusteredBitVector tempCBV = cbvs[from];
      auto tempVec = vecs[from];
      checkConsistency("temp" + Twine(temp), tempCBV, tempVec, 2);
      toolchain::outs() << "    cbvs[" << to << "] = std::move(temp" << temp << "); }\n";
      cbvs[to] = std::move(tempCBV);
      vecs[to] = std::move(tempVec);
      break;
    }

    case 6: {
      auto from = randNV();
      auto to = randNV();
      auto temp = nextTemp++;
      toolchain::outs() << "  { ClusteredBitVector temp" << temp << " = std::move(cbvs[" << from << "]);\n";
      ClusteredBitVector tempCBV = std::move(cbvs[from]);
      auto tempVec = std::move(vecs[from]);
      checkConsistency("temp" + Twine(temp), tempCBV, tempVec, 2);
      toolchain::outs() << "    cbvs[" << to << "] = std::move(temp" << temp << "); }\n";
      cbvs[to] = std::move(tempCBV);
      vecs[to] = std::move(tempVec);
      break;
    }

    case 7: {
      auto to = randNV();
      auto count = randCount(32);
      auto bits = randCount(1ULL << count);
      toolchain::outs() << "  cbvs[" << to << "].add(" << count << ", "
                   << toolchain::format_hex(bits, 18) << ");\n";
      cbvs[to].add(count, bits);
      while (count--) {
        vecs[to].push_back(bits & 1);
        bits >>= 1;
      }
      break;
    }

    case 8: {
      auto to = randNV();
      auto count = randCount(128);
      toolchain::outs() << "  cbvs[" << to << "].appendClearBits(" << count << ");\n";
      cbvs[to].appendClearBits(count);
      while (count--) vecs[to].push_back(false);
      break;
    }

    case 9: {
      auto to = randNV();
      auto count = randCount(128);
      toolchain::outs() << "  cbvs[" << to << "].appendSetBits(" << count << ");\n";
      cbvs[to].appendSetBits(count);
      while (count--) vecs[to].push_back(true);
      break;
    }
    }

    // Validate that everything's still okay.
    for (auto i = 0; i != NV; ++i) {
      checkConsistency("cbvs[" + Twine(i) + "]", cbvs[i], vecs[i], 1);
    }
  }
}

int main() {
  sranddev();
  run();
}
