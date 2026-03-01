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


#ifndef CODIRA_DECOMPRESS_H
#define CODIRA_DECOMPRESS_H

#include <string>
#include <vector>
#include <tuple>

namespace Codira {
constexpr size_t NPOS = static_cast<size_t>(-1);

template<typename T>
class DeCompression {
public:
    /**
     * @brief The constructor of class DeCompression.
     *
     * @param mangled The name to decompress.
     * @return DeCompression The instance of DeCompression.
     */
    explicit DeCompression(const T& mangled) : isRecord(false)
    {
        mangledName = mangled;
    }

    /**
     * @brief Main entry of mangler decompression.
     *
     * @param isType Whether the demangled name is type.
     * @return T The demangled name after decompression.
     */
    T CODEMangledDeCompression(bool isType = false);

    /**
     * @brief Get the index at the end of Class type.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardClassType(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of Generic types. e.g. I<type>+E
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardGenericTypes(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of Function type.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardFunctionType(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of Tuple type.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardTupleType(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of CPointer type.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardCPointer(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of Array type.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardArrayType(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of Generic type. e.g. G<type>
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardGenericType(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of package name.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardPackageName(T& mangled, size_t& cnt, size_t idx);

    /**
     * @brief Get the index at the end of the type.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardType(T& mangled, size_t& cnt, size_t idx = 0);

    /**
     * @brief Get the index at the end of the types.
     *
     * @param mangled The name needs to decompress.
     * @param cnt Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardTypes(T& mangled, size_t& cnt, size_t idx = 0);

    /**
     * @brief Get the index at the end of the number.
     *
     * @param mangled The name needs to decompress.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardNumber(T& mangled, size_t idx = 0);

    /**
     * @brief Get the index at the end of the name.
     *
     * @param mangled The name needs to decompress.
     * @param idx The start index of the demangled name.
     * @return size_t The end index of the demangled name.
     */
    size_t ForwardName(T& mangled, size_t idx = 0);

    /**
     * @brief Check whether the demangle name is compressed.
     *
     * @param mangled The name needs to decompress.
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsCompressed(T& mangled);

    /**
     * @brief Check whether the demangle name is variable decl.
     *
     * @param mangled The name needs to decompress.
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsVarDeclEncode(T& mangled);

    /**
     * @brief Check whether the demangle name is global name.
     *
     * @param mangled The name needs to decompress.
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsGlobalEncode(T& mangled);

    /**
     * @brief Check whether the demangle name is default param function.
     *
     * @param mangled The name needs to decompress.
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsDefaultParamFuncEncode(T& mangled);

    /**
     * @brief Check if the first and the second have the same prefix.
     *
     * @param first The first string.
     * @param second The second string.
     * @param idx The start index of the first and the second.
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsSamePrefix(T& first, T second, size_t idx);

    /**
     * @brief Check if the treeIdMap vector contains duplicates.
     *
     * @param mangled The name needs to decompress.
     * @param pos The position which is a pair consisting of start index and length.
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool HasDuplicates(T& mangled, std::tuple<size_t, size_t>& pos);

    /**
     * @brief Check if the treeIdMap vector contains duplicates.
     *
     * @param mangled The name needs to decompress.
     * @param mid The mid-th element in the treeIdMap vector.
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool HasDuplicates(T& mangled, size_t mid);

    /**
     * @brief Update the compressed name.
     *
     * @param compressed The name needs to update.
     * @param idx The start index of the compressed name.
     * @return std::tuple<size_t, size_t> A pair consisting of the change count and the end index.
     */
    std::tuple<size_t, size_t> UpdateCompressedName(T& compressed, size_t idx);

    /**
     * @brief Update the compressed name.
     *
     * @param compressed The name needs to update.
     * @param sid The start index which the compressed name has been updated.
     * @param eid The end index which the compressed name has been updated.
     * @return size_t The real end index which the compressed name has been updated.
     */
    size_t UpdateCompressedName(T& compressed, size_t sid, size_t eid);

    /**
     * @brief Try to decompress extend path.
     *
     * @param mangled The compressed name will be updated.
     * @param count Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the compressed name.
     * @param entityId The index of treeIdMap vector whose item needs to update the end index.
     * @param curMangled The origin compressed name.
     * @return size_t The end index which the compressed name has been updated.
     */
    size_t TryExtendPath(T& mangled, size_t& count, size_t idx, size_t entityId, T& curMangled);

    /**
     * @brief Try to decompress lambda path.
     *
     * @param mangled The compressed name will be updated.
     * @param count Record the number of new elements added to the treeIdMap.
     * @param idx The start index of the compressed name.
     * @param entityId The index of treeIdMap vector whose item needs to update the end index.
     * @param change Indicates whether the compressed name has been updated.
     * @return size_t The end index which the compressed name has been updated.
     */
    size_t TryLambdaPath(T& mangled, size_t& count, size_t idx, size_t entityId, size_t change);

    /**
     * @brief Try to decompress generic prefix path.
     *
     * @param mangled The compressed name will be updated.
     * @param count Record the number of new elements added to the treeIdMap.
     * @param curMangled The origin compressed name.
     * @param rParams A tuple consisting of the start index, entity id and the end index.
     * @return size_t The end index which the compressed name has been updated.
     */
    size_t TryGenericPrefixPath(T& mangled, size_t& count, T& curMangled, std::tuple<size_t, size_t, size_t> rParams);

    /**
     * @brief Try to decompress name prefix path.
     *
     * @param mangled The compressed name will be updated.
     * @param count Record the number of new elements added to the treeIdMap.
     * @param curMangled The origin compressed name.
     * @param rParams A tuple consisting of the start index, entity id and the change.
     * @return size_t The end index which the compressed name has been updated.
     */
    size_t TryNamePrefixPath(T& mangled, size_t& count, T& curMangled, std::tuple<size_t, size_t, size_t> rParams);

    /**
     * @brief Try to decompress path.
     *
     * @param mangled The compressed name will be updated.
     * @return size_t The end index which the compressed name has been updated.
     */
    size_t TryParsePath(T mangled);

    /**
     * @brief Generate variable decl decompressed name.
     */
    void SpanningVarDeclTree();

    /**
     * @brief Generate function decl decompressed name.
     */
    void SpanningFuncDeclTree();

    /**
     * @brief Pop elements of treeIdMap vector.
     *
     * @param from The start index of treeIdMap vector.
     * @param to The end index of treeIdMap vector.
     */
    void TreeIdMapPop(size_t& from, size_t to);

    /**
     * @brief Push elements of treeIdMap vector.
     *
     * @param mangled The compressed name.
     * @param pos The element needs to push.
     * @return bool If push success, true is returned. Otherwise, false is returned.
     */
    bool TreeIdMapPushBack(T& mangled, std::tuple<size_t, size_t>& pos);

    /**
     * @brief Erase elements of treeIdMap vector.
     *
     * @param mangled The compressed name.
     * @param cnt The treeIdMap size.
     * @param eid The entity id.
     * @param sid The start index of the compressed name(mangled).
     * @return T The compressed name(mangled) has been updated.
     */
    T TreeIdMapErase(T& mangled, size_t& cnt, size_t eid, size_t sid);

    /**
     * @brief Assign elements of treeIdMap vector.
     *
     * @param mangled The compressed name.
     * @param mangledCopy The origin compressed name.
     * @param mapId The index of treeIdMap vector whose item needs to update the end index.
     * @param cnt The treeIdMap size.
     * @param eleInfo A pair consisting of start index and end index.
     */
    void TreeIdMapAssign(T& mangled, T& mangledCopy, size_t mapId, size_t& cnt,
        std::tuple<size_t, size_t>& eleInfo);

private:
    size_t pid{};
    bool isRecord;
    T mangledName;
    size_t currentIndex{};
    T decompressed;
    std::vector<std::tuple<size_t, size_t>> treeIdMap;
};
}
#endif // CODIRA_DECOMPRESS_H
