/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef ECMASCRIPT_TOOLING_CLIENT_MANAGER_VARIABLE_MANAGER_H
#define ECMASCRIPT_TOOLING_CLIENT_MANAGER_VARIABLE_MANAGER_H

#include <iostream>
#include <map>

#include "tooling/dynamic/client/manager/stack_manager.h"
#include "tooling/dynamic/base/pt_json.h"
#include "tooling/dynamic/base/pt_returns.h"
#include "tooling/dynamic/base/pt_types.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
using PropertyDescriptor = panda::ecmascript::tooling::PropertyDescriptor;
using GetHeapUsageReturns = panda::ecmascript::tooling::GetHeapUsageReturns;
using PropertyDescriptor = panda::ecmascript::tooling::PropertyDescriptor;
using DescriptorMap = std::map<int32_t, std::unique_ptr<PropertyDescriptor>>;
using NodeData = std::variant<int32_t, std::map<int32_t, std::map<int32_t, std::string>>,
                              std::unique_ptr<PropertyDescriptor>, DescriptorMap>;
namespace OHOS::ArkCompiler::Toolchain {
class TreeNode {
public:
    NodeData data;
    std::vector<std::unique_ptr<TreeNode>> children;

    TreeNode(int32_t CallFrameId) : data(CallFrameId) {}
    TreeNode(const std::map<int32_t, std::map<int32_t, std::string>>& scopeInfo) : data(scopeInfo) {}
    TreeNode(std::unique_ptr<PropertyDescriptor> descriptor) : data(std::move(descriptor)) {}
    TreeNode(DescriptorMap&& descriptorMap) : data(std::move(descriptorMap)) {}

    void AddChild(std::unique_ptr<PropertyDescriptor> descriptor);
    void AddChild(DescriptorMap descriptorMap);
    void AddChild(std::unique_ptr<TreeNode> child);
    void Print(int depth = 0) const;
};

class Tree {
public:
    Tree(int32_t rootValue) : root_(std::make_unique<TreeNode>(rootValue)) {}
    Tree(const std::map<int32_t, std::map<int32_t, std::string>>& dataMap, int32_t index);

    void Clear();
    void Print() const;
    TreeNode* GetRoot() const;
    TreeNode* FindNodeWithObjectId(int32_t objectId) const;
    void AddVariableNode(TreeNode* parentNode, std::unique_ptr<PropertyDescriptor> descriptor);
    void AddObjectNode(TreeNode* parentNode, std::unique_ptr<PropertyDescriptor> descriptor);
    TreeNode* FindNodeWithCondition() const;
    void PrintRootAndImmediateChildren() const;
    int32_t FindObjectByIndex(int32_t index) const;

private:
    std::unique_ptr<TreeNode> root_ {};
    int32_t index_ {1};
    TreeNode* FindNodeWithObjectIdRecursive(TreeNode* node, int32_t objectId) const;
    TreeNode* FindNodeWithInnerKeyZero(TreeNode* node) const;
    int32_t FindObjectByIndexRecursive(const TreeNode* node, int32_t index) const;
};

class VariableManager final {
public:
    VariableManager(int32_t sessionId) : sessionId_(sessionId) {}
    void SetHeapUsageInfo(std::unique_ptr<GetHeapUsageReturns> heapUsageReturns);
    void ShowHeapUsageInfo() const;
    void ShowVariableInfos() const;
    void ClearVariableInfo();
    void InitializeTree(std::map<int32_t, std::map<int32_t, std::string>> dataMap, int index = 0);
    void PrintVariableInfo();
    TreeNode* FindNodeWithObjectId(int32_t objectId);
    int32_t FindObjectIdWithIndex(int index);
    void AddVariableInfo(TreeNode *parentNode, std::unique_ptr<PropertyDescriptor> variableInfo);
    TreeNode* FindNodeObjectZero();
    void Printinfo() const;

private:
    [[maybe_unused]] int32_t sessionId_;
    GetHeapUsageReturns heapUsageInfo_ {};
    Tree variableInfo_ {0};
    VariableManager(const VariableManager&) = delete;
    VariableManager& operator=(const VariableManager&) = delete;
};
} // OHOS::ArkCompiler::Toolchain
#endif