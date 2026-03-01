/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

/* These #undef's are necessary because some header defines these macros and
   they will be expanded to literal 1 or 0 in SQL queries which is unwanted.
   Unfortunately, there are no any other suitable solutions for this problem.
   Currently NULL, TRUE and FALSE are the only macros which needs to be
   #undef'ined, but there maybe some others like min/max, etc. Fortunately for
   us, most of them will lead to an error during preparation of the SQL
   statements damaged by these macros. */
#ifdef NULL
#undef NULL
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif
