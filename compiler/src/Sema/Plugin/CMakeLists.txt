# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Codira project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://codira-lang.cn/pages/LICENSE for license information.

file(GLOB PLUGIN_SRC *.cpp)
set(SEMA_SRC ${SEMA_SRC} ${PLUGIN_SRC} PARENT_SCOPE)