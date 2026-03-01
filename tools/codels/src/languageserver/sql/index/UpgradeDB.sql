/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "../Prologue.inc.sql"


/* NOTE: SQL sentences must be declared in version ascending order and must be
 monotonic. If database cannot be upgraded properly for one version it's better
 to remove all previous upgrade scripts. */

// we have to have at least one update statement.
SQL(0,
    PRAGMA user_version = 0;
)

// current update scenario - the DB recreation
