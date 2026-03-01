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

#ifndef COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_MACROS_H
#define COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_MACROS_H

#ifndef ENABLE_LIBABCKIT
// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_ASSERT(cond) ASSERT((cond))
// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_EQ(lhs, rhs) CHECK_EQ(lhs, rhs)

// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_DO_IF_NOT(cond, func) ASSERT_DO((cond), func)
// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_DO_IF_NOT_VISITOR_INTERNAL(visitor, klass, cond, func) ASSERT_DO((cond), func)
// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_DO_IF_NOT_AND_PRINT(cond, func) ASSERT_DO((cond), func; PrintFailedMethodAndPass();)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(visitor, cond, func) \
    ASSERT_DO((cond), func; PrintFailedMethodAndPassVisitor(visitor);)
// CC-OFFNXT(G.FMT.16) project code style
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_IF_NOT_PRINT(cond) CHECKER_DO_IF_NOT_AND_PRINT(cond, )
// CC-OFFNXT(G.FMT.16) project code style
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_IF_NOT_PRINT_VISITOR(visitor, cond) CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(visitor, cond, )
// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_STATUS()
#else

// CC-OFFNXT(G.PRE.02) should be with define
#define ABCKIT_ASSERT(cond)                                    \
    if (UNLIKELY(!(cond))) {                                   \
        std::cerr << "The assertion is not true" << std::endl; \
        reinterpret_cast<InstChecker *>(v)->SetStatus(false);  \
    }
// CC-OFFNXT(G.PRE.02) should be with define
#define CHECKER_ASSERT(cond)                                          \
    if (reinterpret_cast<InstChecker *>(v)->GetGraph()->IsAbcKit()) { \
        ABCKIT_ASSERT((cond));                                        \
    } else {                                                          \
        ASSERT((cond));                                               \
    }
// CC-OFFNXT(G.PRE.02) should be with define
#define ABCKIT_CHECK_EQ(lhs, rhs)                             \
    if (UNLIKELY(!(lhs == rhs))) {                            \
        std::cerr << "The values are not equal" << std::endl; \
        reinterpret_cast<InstChecker *>(v)->SetStatus(false); \
    }
// CC-OFFNXT(G.PRE.02) should be with define
#define CHECKER_EQ(lhs, rhs)                                          \
    if (reinterpret_cast<InstChecker *>(v)->GetGraph()->IsAbcKit()) { \
        ABCKIT_CHECK_EQ((lhs), (rhs));                                \
    } else {                                                          \
        CHECK_EQ(lhs, rhs);                                           \
    }
// CC-OFFNXT(G.PRE.02) should be with define
#define ABCKIT_DO_IF_NOT_VISITOR(visitor, cond, func)                \
    if (UNLIKELY(!(cond))) {                                         \
        func;                                                        \
        reinterpret_cast<GraphChecker *>(visitor)->SetStatus(false); \
    }
// CC-OFFNXT(G.PRE.02) should be with define
#define ABCKIT_DO_IF_NOT(cond, func) \
    if (UNLIKELY(!(cond))) {         \
        func;                        \
        SetStatus(false);            \
    }
// CC-OFFNXT(G.PRE.02) should be with define
#define CHECKER_DO_IF_NOT(cond, func)   \
    if (GetGraph()->IsAbcKit()) {       \
        ABCKIT_DO_IF_NOT((cond), func); \
    } else {                            \
        ASSERT_DO(cond, func);          \
    }
// CC-OFFNXT(G.PRE.02) should be with define
#define CHECKER_DO_IF_NOT_VISITOR_INTERNAL(visitor, klass, cond, func) \
    if (reinterpret_cast<klass>(visitor)->GetGraph()->IsAbcKit()) {    \
        ABCKIT_DO_IF_NOT_VISITOR(visitor, (cond), func);               \
    } else {                                                           \
        ASSERT_DO(cond, func);                                         \
    }

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECKER_DO_IF_NOT_AND_PRINT(cond, func) CHECKER_DO_IF_NOT((cond), func; PrintFailedMethodAndPass();)
// CC-OFFNXT(G.PRE.02) should be with define
#define CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(visitor, cond, func) \
    CHECKER_DO_IF_NOT_VISITOR_INTERNAL(visitor, GraphChecker *, (cond), func; PrintFailedMethodAndPassVisitor(visitor);)
// CC-OFFNXT(G.PRE.02) should be with define
// CC-OFFNXT(G.FMT.16) project code style
#define CHECKER_IF_NOT_PRINT(cond) CHECKER_DO_IF_NOT_AND_PRINT(cond, )
// CC-OFFNXT(G.PRE.02) should be with define
// CC-OFFNXT(G.FMT.16) project code style
#define CHECKER_IF_NOT_PRINT_VISITOR(visitor, cond) CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(visitor, cond, )
// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_STATUS()                                        \
    if (!GetStatus() && GetGraph()->IsAbcKit()) {             \
        PrintFailedMethodAndPass();                           \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */ \
        return false;                                         \
    }
#endif

// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_MESSAGE_IF_NOT_AND_PRINT(cond, message) \
    CHECKER_DO_IF_NOT((cond), std::cerr << (message) << std::endl; PrintFailedMethodAndPass();)
// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(visitor, cond, message)                                     \
    CHECKER_DO_IF_NOT_VISITOR_INTERNAL(visitor, GraphChecker *, (cond), std::cerr << (message) << std::endl; \
                                       PrintFailedMethodAndPassVisitor(visitor);)

#endif  // COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_MACROS_H
