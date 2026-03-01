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


#ifndef LIB_CODIRA_DEMANGLE
#define LIB_CODIRA_DEMANGLE

#include <string>
#include <vector>

namespace Codira {
struct DemangleData {
    /**
     * @brief The constructor of struct DemangleData.
     *
     * @param pkgName The package name.
     * @param fullName The full demangled name.
     * @param isFunctionLike Whether is function like.
     * @param isValid Whether The demangled name is valid.
     * @return DemangleData The instance of DemangleData.
     */
    DemangleData(std::string pkgName, std::string fullName, bool isFunctionLike, bool isValid)
        : pkgName(pkgName), fullName(fullName), isFunctionLike(isFunctionLike), isValid(isValid) {}

    /**
     * @brief The destructor of struct DemangleData.
     */
    ~DemangleData() {}

    /**
     * @brief Get package name.
     *
     * @return std::string The package name.
     */
    std::string GetPkgName() const;

    /**
     * @brief Get full name.
     *
     * @return std::string The full name.
     */
    std::string GetFullName() const;

    /**
     * @brief Check if it functions like a function.
     *
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsFunctionLike() const;

    /**
     * @brief Check if it is valid.
     *
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsValid() const;

    /**
     * @brief Set private declaration.
     *
     * @param isPrivate It is used to assign a value to isPrivateDeclaration.
     */
    void SetPrivateDeclaration(bool isPrivate);

    /**
     * @brief Check if it is private declaration.
     *
     * @return bool If yes, true is returned. Otherwise, false is returned.
     */
    bool IsPrivateDeclaration() const;

private:
    std::string pkgName;
    std::string fullName;
    bool isFunctionLike = false;
    bool isValid;
    bool isPrivateDeclaration = false;
};

/**
 * @brief Demangle the string.
 *
 * @param mangled The name to be demangled.
 * @param genericVec The generic vector.
 * @return DemangleData The demangled information.
 */
DemangleData Demangle(const std::string& mangled, const std::vector<std::string>& genericVec);

/**
 * @brief Demangle the string.
 *
 * @param mangled The name to be demangled.
 * @param scopeRes The scope resolution.
 * @param genericVec The generic vector.
 * @return DemangleData The demangled information.
 */
DemangleData Demangle(const std::string& mangled, const std::string& scopeRes,
    const std::vector<std::string>& genericVec);

/**
 * @brief Demangle the string.
 *
 * @param mangled The name to be demangled.
 * @return DemangleData The demangled information.
 */
DemangleData Demangle(const std::string& mangled);

/**
 * @brief Demangle the string.
 *
 * @param mangled The name to be demangled.
 * @param scopeRes The scope resolution.
 * @return DemangleData The demangled information.
 */
DemangleData Demangle(const std::string& mangled, const std::string& scopeRes);

/**
 * @brief Demangle the type.
 *
 * @param mangled The type name to demangle.
 * @return DemangleData The demangled information.
 */
DemangleData DemangleType(const std::string& mangled);

/**
 * @brief Demangle the type.
 *
 * @param mangled The type name to demangle.
 * @param scopeRes The scope resolution.
 * @return DemangleData The demangled information.
 */
DemangleData DemangleType(const std::string& mangled, const std::string& scopeRes);
} // namespace Codira
#endif // LIB_CODIRA_DEMANGLE
