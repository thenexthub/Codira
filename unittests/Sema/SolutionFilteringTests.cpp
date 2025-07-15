//===--- SolutionFilteringTests.cpp ---------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "SemaFixture.h"
#include "language/Sema/ConstraintSystem.h"

using namespace language;
using namespace language::unittest;
using namespace language::constraints;

/// Make sure that minimization works as expected
/// and would remove solutions that score worse
/// than the best set even in ambiguity cases
/// (where there is more than one best solution).
TEST_F(SemaTest, TestFilteringBasedOnSolutionScore) {
  Score bestScore{};

  Score worseScore1{};
  Score worseScore2{};

  worseScore1.Data[SK_Unavailable] += 1;
  worseScore2.Data[SK_Fix] += 1;

  ConstraintSystem CS(DC, ConstraintSystemOptions());

  // Let's test worse solutions in different positions
  // in the list - beginning, middle, end.

  // To make "best" solutions ambiguous, let's use a single
  // type variable resolved into multiple distinct types.
  auto *ambiguousVar = CS.createTypeVariable(
      CS.getConstraintLocator({}), /*options=*/TVO_PrefersSubtypeBinding);

  auto formSolution = [&](Score score, Type type) -> Solution {
    Solution solution{CS, score};
    solution.typeBindings[ambiguousVar] = type;
    return solution;
  };

  // Nothing to remove
  {
    SmallVector<Solution, 4> solutions;

    solutions.push_back(formSolution(bestScore, getStdlibType("String")));
    solutions.push_back(formSolution(bestScore, getStdlibType("Int")));

    auto best = CS.findBestSolution(solutions, /*minimize=*/true);

    ASSERT_FALSE(best.has_value());
    ASSERT_EQ(solutions.size(), 2u);
    ASSERT_EQ(solutions[0].getFixedScore(), bestScore);
    ASSERT_EQ(solutions[1].getFixedScore(), bestScore);
  }

  // Worst solutions are at the front
  {
    SmallVector<Solution, 4> solutions;

    solutions.push_back(formSolution(worseScore1, getStdlibType("Int")));
    solutions.push_back(formSolution(worseScore2, getStdlibType("Int")));

    solutions.push_back(formSolution(bestScore, getStdlibType("String")));
    solutions.push_back(formSolution(bestScore, getStdlibType("Int")));

    auto best = CS.findBestSolution(solutions, /*minimize=*/true);

    ASSERT_FALSE(best.has_value());
    ASSERT_EQ(solutions.size(), 2u);
    ASSERT_EQ(solutions[0].getFixedScore(), bestScore);
    ASSERT_EQ(solutions[1].getFixedScore(), bestScore);
  }

  // Worst solutions are in the middle
  {
    SmallVector<Solution, 4> solutions;

    solutions.push_back(formSolution(bestScore, getStdlibType("String")));

    solutions.push_back(formSolution(worseScore1, getStdlibType("Int")));
    solutions.push_back(formSolution(worseScore2, getStdlibType("Int")));

    solutions.push_back(formSolution(bestScore, getStdlibType("Int")));

    auto best = CS.findBestSolution(solutions, /*minimize=*/true);

    ASSERT_FALSE(best.has_value());
    ASSERT_EQ(solutions.size(), 2u);
    ASSERT_EQ(solutions[0].getFixedScore(), bestScore);
    ASSERT_EQ(solutions[1].getFixedScore(), bestScore);
  }

  // Worst solutions are at the end
  {
    SmallVector<Solution, 4> solutions;

    solutions.push_back(formSolution(bestScore, getStdlibType("String")));
    solutions.push_back(formSolution(bestScore, getStdlibType("Int")));

    solutions.push_back(formSolution(worseScore1, getStdlibType("Int")));
    solutions.push_back(formSolution(worseScore2, getStdlibType("Int")));

    auto best = CS.findBestSolution(solutions, /*minimize=*/true);

    ASSERT_FALSE(best.has_value());
    ASSERT_EQ(solutions.size(), 2u);
    ASSERT_EQ(solutions[0].getFixedScore(), bestScore);
    ASSERT_EQ(solutions[1].getFixedScore(), bestScore);
  }

  // Worst solutions are spread out
  {
    SmallVector<Solution, 4> solutions;

    solutions.push_back(formSolution(worseScore1, getStdlibType("Int")));
    solutions.push_back(formSolution(bestScore, getStdlibType("String")));
    solutions.push_back(formSolution(worseScore1, getStdlibType("Int")));
    solutions.push_back(formSolution(worseScore2, getStdlibType("Int")));
    solutions.push_back(formSolution(bestScore, getStdlibType("Int")));
    solutions.push_back(formSolution(worseScore1, getStdlibType("Int")));
    solutions.push_back(formSolution(worseScore2, getStdlibType("Int")));

    auto best = CS.findBestSolution(solutions, /*minimize=*/true);

    ASSERT_FALSE(best.has_value());
    ASSERT_EQ(solutions.size(), 2u);
    ASSERT_EQ(solutions[0].getFixedScore(), bestScore);
    ASSERT_EQ(solutions[1].getFixedScore(), bestScore);
  }
}
