#include "language/Basic/SuccessorMap.h"
#include "toolchain/Support/raw_ostream.h"
#include <map>
#include <random>

const unsigned RandomSpread = 10;

int main(int argc, char **argv) {
  std::random_device randomDevice; // used for seeding
  std::default_random_engine generator(randomDevice());
  std::uniform_int_distribution<unsigned> distribution(0,RandomSpread);

  language::SuccessorMap<unsigned, unsigned> map;
  std::map<unsigned, unsigned> stdMap;

  if (argc < 0) map.dump(); // force this to be used

  auto next = [&] { return distribution(generator); };
  auto nextUnmappedKey = [&] {
    unsigned key;
    do {
      key = next();
    } while (stdMap.find(key) != stdMap.end());
    return key;
  };

  while (true) {
    auto operation = next();

    // Find.
    if (operation >= .7 * RandomSpread) {
      unsigned key = nextUnmappedKey();
      auto iter = stdMap.upper_bound(key);
      auto stdResult = (iter == stdMap.end() ? nullptr : &iter->second);

      toolchain::outs() << "  EXPECT_EQ(";
      if (stdResult) {
        toolchain::outs() << *stdResult << ", *";
      } else {
        toolchain::outs() << "InvalidValue, ";
      }
      toolchain::outs() << "map.findLeastUpperBound(" << key << "));\n";

      auto result = map.findLeastUpperBound(key);
      if (result && stdResult && *result != *stdResult) {
        toolchain::outs() << "FAILURE: found " << *result
                     << ", but should have found " << *stdResult << "\n";
        abort();
      } else if (!result && stdResult) {
        toolchain::outs() << "FAILURE: found nothing, but should have found "
                     << *stdResult << "\n";
        abort();
      } else if (result && !stdResult) {
        toolchain::outs() << "FAILURE: found " << *result
                     << ", but should have found nothing\n";
        abort();
      }

    } else if (operation >= .05 * RandomSpread) {
      unsigned key = nextUnmappedKey();
      unsigned value = next();

      toolchain::outs() << "  map.insert(" << key << ", " << value << ");\n";

      map.insert((unsigned) key, (unsigned) value);
      stdMap.insert(std::make_pair(key, value));
    } else {
      toolchain::outs() << "  map.clear();\n";
      map.clear();
      stdMap.clear();
    }

    toolchain::outs() << "  map.validate();\n";
    map.validate();
  }
}
