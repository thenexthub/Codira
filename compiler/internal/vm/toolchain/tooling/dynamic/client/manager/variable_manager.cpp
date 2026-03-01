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

#include "tooling/dynamic/client/manager/variable_manager.h"

using PtJson = panda::ecmascript::tooling::PtJson;
namespace OHOS::ArkCompiler::Toolchain {
void TreeNode::AddChild(std::unique_ptr<PropertyDescriptor> descriptor)
{
    children.push_back(std::make_unique<TreeNode>(std::move(descriptor)));
}

void TreeNode::AddChild(DescriptorMap descriptorMap)
{
    children.push_back(std::make_unique<TreeNode>(std::move(descriptorMap)));
}

void TreeNode::AddChild(std::unique_ptr<TreeNode> child)
{
    children.push_back(std::move(child));
}

void TreeNode::Print(int depth) const
{
    int actualIndent = 0;
    if (depth > 1) {
        actualIndent = (depth - 1) * 4; // 4: four spaces
    }
    std::string indent(actualIndent, ' ');

    if (std::holds_alternative<int32_t>(data)) {
        std::cout << indent << "CallFrameId: " << std::get<int32_t>(data) << std::endl;
    } else if (std::holds_alternative<std::map<int32_t, std::map<int32_t, std::string>>>(data)) {
        const auto& outerMap = std::get<std::map<int32_t, std::map<int32_t, std::string>>>(data);
        for (const auto& [key, innerMap] : outerMap) {
            for (const auto& [innerKey, value] : innerMap) {
                std::cout << key << ". " << value << std::endl;
            }
        }
    } else if (std::holds_alternative<std::unique_ptr<PropertyDescriptor>>(data)) {
        const auto &descriptor = std::get<std::unique_ptr<PropertyDescriptor>>(data);
        if (descriptor && descriptor->GetValue()) {
            if (descriptor->GetValue()->HasDescription()) {
                std::cout << indent << "   " << descriptor->GetName() << " = "
                      << descriptor->GetValue()->GetDescription() << std::endl;
            } else {
                std::cout << indent << "   " << descriptor->GetName() << " = "
                      << descriptor->GetValue()->GetType() << std::endl;
            }
        }
    } else if (std::holds_alternative<DescriptorMap>(data)) {
        const auto &descriptorMap = std::get<DescriptorMap>(data);
        for (const auto& [key, descriptor] : descriptorMap) {
            std::cout << indent << key << ". ";
            if (descriptor && descriptor->GetValue()) {
                std::cout << descriptor->GetName() << " = " << descriptor->GetValue()->GetDescription() << std::endl;
            }
        }
    }

    for (const auto &child : children) {
        child->Print(depth + 1);
    }
}

Tree::Tree(const std::map<int32_t, std::map<int32_t, std::string>>& dataMap, int32_t index)
{
    root_ = std::make_unique<TreeNode>(index);

    auto it = dataMap.find(index);
    if (it != dataMap.end()) {
        for (const auto& [key, value] : it->second) {
            std::map<int32_t, std::map<int32_t, std::string>> childData;
            childData[index_] = {{key, value}};
            root_->children.push_back(std::make_unique<TreeNode>(childData));
            index_++;
        }
    }
}

void Tree::PrintRootAndImmediateChildren() const
{
    if (std::holds_alternative<int32_t>(root_->data)) {
        std::cout << "CallFrame ID: " << std::get<int32_t>(root_->data) << std::endl;
    }

    for (const auto& child : root_->children) {
        if (std::holds_alternative<std::map<int32_t, std::map<int32_t, std::string>>>(child->data)) {
            const auto& outerMap = std::get<std::map<int32_t, std::map<int32_t, std::string>>>(child->data);
            for (const auto& [index, innerMap] : outerMap) {
                std::cout << "  Index: " << index << std::endl;
                for (const auto& [objectId, type] : innerMap) {
                    std::cout << "    Object ID: " << objectId << ", Type: " << type << std::endl;
                }
            }
        }
    }
}

void Tree::Clear()
{
    if (root_ != nullptr) {
        root_.reset();
    }
}

void Tree::Print() const
{
    if (root_) {
        root_->Print();
    }
}

TreeNode* Tree::FindNodeWithObjectId(int32_t objectId) const
{
    return FindNodeWithObjectIdRecursive(root_.get(), objectId);
}

void Tree::AddVariableNode(TreeNode* parentNode, std::unique_ptr<PropertyDescriptor> descriptor)
{
    if (!parentNode) {
        return;
    }
    parentNode->AddChild(std::move(descriptor));
}

void Tree::AddObjectNode(TreeNode* parentNode, std::unique_ptr<PropertyDescriptor> descriptor)
{
    if (!parentNode) {
        return;
    }
    DescriptorMap descriptorMap;
    descriptorMap[index_] = std::move(descriptor);
    parentNode->AddChild(std::move(descriptorMap));
    index_++;
}

TreeNode* Tree::GetRoot() const
{
    return root_.get();
}

TreeNode* Tree::FindNodeWithObjectIdRecursive(TreeNode* node, int32_t objectId) const
{
    if (!node) {
        return nullptr;
    }

    if (std::holds_alternative<std::map<int32_t, std::map<int32_t, std::string>>>(node->data)) {
        const auto& outerMap = std::get<std::map<int32_t, std::map<int32_t, std::string>>>(node->data);
        for (const auto& [key, innerMap] : outerMap) {
            if (innerMap.find(objectId) != innerMap.end()) {
                return node;
            }
        }
    } else if (std::holds_alternative<DescriptorMap>(node->data)) {
        const auto& descriptorMap = std::get<DescriptorMap>(node->data);
        for (const auto& [key, descriptor] : descriptorMap) {
            if (descriptor && descriptor->GetValue() && descriptor->GetValue()->GetObjectId() == objectId) {
                return node;
            }
        }
    }

    for (const auto& child : node->children) {
        TreeNode* foundNode = FindNodeWithObjectIdRecursive(child.get(), objectId);
        if (foundNode) {
            return foundNode;
        }
    }

    return nullptr;
}

TreeNode* Tree::FindNodeWithInnerKeyZero(TreeNode* node) const
{
    if (!node) {
        return nullptr;
    }

    if (std::holds_alternative<std::map<int32_t, std::map<int32_t, std::string>>>(node->data)) {
        const auto &outerMap = std::get<std::map<int32_t, std::map<int32_t, std::string>>>(node->data);
        for (const auto &innerMapPair : outerMap) {
            if (innerMapPair.second.find(0) != innerMapPair.second.end()) {
                return node;
            }
        }
    }

    for (const auto &child : node->children) {
        TreeNode* foundNode = FindNodeWithInnerKeyZero(child.get());
        if (foundNode) {
            return foundNode;
        }
    }

    return nullptr;
}
TreeNode* Tree::FindNodeWithCondition() const
{
    return FindNodeWithInnerKeyZero(root_.get());
}

int32_t Tree::FindObjectByIndex(int32_t index) const
{
    return FindObjectByIndexRecursive(root_.get(), index);
}

int32_t Tree::FindObjectByIndexRecursive(const TreeNode* node, int32_t index) const
{
    if (!node) {
        return 0;
    }

    if (std::holds_alternative<std::map<int32_t, std::map<int32_t, std::string>>>(node->data)) {
        const auto &outerMap = std::get<std::map<int32_t, std::map<int32_t, std::string>>>(node->data);
        auto it = outerMap.find(index);
        if (it != outerMap.end() && !it->second.empty()) {
            return it->second.begin()->first;
        }
    } else if (std::holds_alternative<DescriptorMap>(node->data)) {
        const auto &descriptorMap = std::get<DescriptorMap>(node->data);
        auto it = descriptorMap.find(index);
        if (it != descriptorMap.end() && it->second && it->second->GetValue()) {
            return it->second->GetValue()->GetObjectId();
        }
    }

    for (const auto& child : node->children) {
        int32_t result = FindObjectByIndexRecursive(child.get(), index);
        if (result != 0) {
            return result;
        }
    }

    return 0;
}

void VariableManager::SetHeapUsageInfo(std::unique_ptr<GetHeapUsageReturns> heapUsageReturns)
{
    heapUsageInfo_.SetUsedSize(heapUsageReturns->GetUsedSize());
    heapUsageInfo_.SetTotalSize(heapUsageReturns->GetTotalSize());
}

void VariableManager::ShowHeapUsageInfo() const
{
    std::cout << std::endl;
    std::cout << "UsedSize is: " << heapUsageInfo_.GetUsedSize() << std::endl;
    std::cout << "TotalSize is: " << std::fixed << std::setprecision(0) << heapUsageInfo_.GetTotalSize() << std::endl;
}

void VariableManager::ShowVariableInfos() const
{
    variableInfo_.Print();
}

void VariableManager::ClearVariableInfo()
{
    variableInfo_.Clear();
}
void VariableManager::InitializeTree(std::map<int32_t, std::map<int32_t, std::string>> dataMap, int index)
{
    variableInfo_ = Tree(dataMap, index);
}

void VariableManager::PrintVariableInfo()
{
    variableInfo_.Print();
}

TreeNode* VariableManager::FindNodeWithObjectId(int32_t objectId)
{
    return variableInfo_.FindNodeWithObjectId(objectId);
}

void VariableManager::AddVariableInfo(TreeNode *parentNode, std::unique_ptr<PropertyDescriptor> variableInfo)
{
    if (variableInfo && variableInfo->GetValue() && variableInfo->GetValue()->HasObjectId()) {
        variableInfo_.AddObjectNode(parentNode, std::move(variableInfo));
    } else {
        variableInfo_.AddVariableNode(parentNode, std::move(variableInfo));
    }
}

TreeNode* VariableManager::FindNodeObjectZero()
{
    return variableInfo_.FindNodeWithCondition();
}

int32_t VariableManager::FindObjectIdWithIndex(int index)
{
    return variableInfo_.FindObjectByIndex(index);
}

void VariableManager::Printinfo() const
{
    variableInfo_.PrintRootAndImmediateChildren();
}
}