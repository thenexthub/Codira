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


#ifndef CODIRA_DEMANGLER_STD_STRING_H
#define CODIRA_DEMANGLER_STD_STRING_H

#include <string>

namespace Codira {
/**
 * @brief This is a std::string proxy type.
 * Since the Demangler and DemanglerInfo template classes are based on the CString type we created in the Base library,
 * we need to maintain API consistency with the CString class. The following methods must all be implemented. This type
 * also helps remove the dependency on CString and boundscheck (an indirect dependency) of the cangjie-demangle library
 * that we export to other users.
 */
class StdString : public std::string {
public:
    /**
     * @brief The constructor of class StdString with none.
     *
     * @return StdString The instance of StdString.
     */
    StdString() : std::string() {}

    /**
     * @brief The constructor of class StdString with character.
     *
     * @param c The character.
     * @return StdString The instance of StdString.
     */
    StdString(char c) : std::string(1, c) {}

    /**
     * @brief The constructor of class StdString with char*.
     *
     * @param initStr The pointer to character.
     * @return StdString The instance of StdString.
     */
    StdString(const char* initStr) : std::string(initStr) {}

    /**
     * @brief The constructor of class StdString with std::string.
     *
     * @param other The string.
     * @return StdString The instance of StdString.
     */
    StdString(const std::string& other) : std::string(other) {}

    /**
     * @brief The constructor of class StdString with StdString.
     *
     * @param other The StdString.
     * @return StdString The instance of StdString.
     */
    StdString(const StdString& other) : std::string(other.Str()) {}

    /**
     * @brief This function ensures that StdString self-assignment.
     *
     * @param other The StdString.
     * @return StdString The new instance of StdString.
     */
    StdString& operator=(const StdString& other)
    {
        std::string::operator=(other.Str());
        return *this;
    }

    /**
     * @brief Get the length of StdString object.
     *
     * @return size_t The length.
     */
    size_t Length() const { return this->size(); }

    /**
     * @brief Get the "char *" format of StdString object.
     *
     * @return char* The string.
     */
    const char* Str() const noexcept { return this->c_str(); }

    /**
     * @brief Determine if StdString object is empty.
     *
     * @return bool Return true if the StdString object is empty, Otherwise, false is returned.
     */
    bool IsEmpty() const { return this->empty(); }

    /**
     * @brief Search for the first occurrence of the specified pattern in the string starting from the given position.
     *
     * @param pattern The specified pattern.
     * @param begin The string starting position.
     * @return int Return the index of the first match, or -1 if no match is found.
     */
    int Find(const char* pattern, size_t begin = 0) const { return this->find(pattern, begin); }

    /**
     * @brief Search for the first occurrence of the specified character in the string starting from
     * the given position.
     *
     * @param pattern The specified character.
     * @param begin The string starting position.
     * @return int Return the index of the first match, or -1 if no match is found.
     */
    int Find(const char pattern, size_t begin = 0) const { return this->find(pattern, begin); }

    /**
     * @brief Return a substring starting from the specified index.
     *
     * @param begin The string starting position.
     * @return StdString The substring.
     */
    StdString SubStr(size_t index) const { return this->substr(index); }

    /**
     * @brief Return a substring starting from the specified index and with the specified length.
     *
     * @param begin The string starting position.
     * @param len The substring length.
     * @return StdString The substring.
     */
    StdString SubStr(size_t index, size_t len) const { return this->substr(index, len); }

    /**
     * @brief Check whether the string ends with the specified suffix.
     *
     * @param suffix The specified suffix.
     * @return bool Return true if the string ends with the specified suffix, Otherwise, false is returned.
     */
    bool EndsWith(const StdString& suffix) const
    {
        return size() >= suffix.size() && substr(size() - suffix.size()) == suffix;
    }

    /**
     * @brief Truncate the string to the specified index.
     *
     * @param index The truncate position.
     * @return StdString The truncated string.
     */
    StdString& Truncate(size_t index)
    {
        this->resize(index);
        return *this;
    }
};
} // namespace Codira
#endif // CODIRA_DEMANGLER_STD_STRING_H
