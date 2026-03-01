..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Functions
*********



.. rst-class:: doc-code-block

  :kw:`export`
  decodeURIComponent(uriComponent\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The decodeURIComponent() function decodes a Uniform Resource Identifier (URI) component previously created by encodeURIComponent() or by a similar routine.
|
| **Returns\:** decoded uri
|
| **Arguments\:**

 - uriComponent\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`export`
  encodeURI(uri\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The encodeURI() function encodes a URI by replacing each instance of certain characters by one, two, three, or four escape sequences representing the UTF-8 encoding of the character (will only be four escape sequences for characters composed of two surrogate characters). Compared to encodeURIComponent(), this function encodes fewer characters, preserving those that are part of the URI syntax.
|
| **Returns\:** encoded uri
|
| **Arguments\:**

 - uri\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`export`
  encodeURIComponent(uriComponent\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The encodeURIComponent() function encodes a URI by replacing each instance of certain characters by one, two, three, or four escape sequences representing the UTF-8 encoding of the character (will only be four escape sequences for characters composed of two surrogate characters). Compared to encodeURI(), this function encodes more characters, including those that are part of the URI syntax.
|
| **Returns\:** encoded uri
|
| **Arguments\:**

 - uriComponent\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`export`
  escape(str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| **DEPRECATED\:** This feature is no longer recommended. Though some browsers might still support it, it may have already been removed from the relevant web standards, may be in the process of being dropped, or may only be kept for compatibility purposes. Avoid using it, and update existing code if possible; see the compatibility table at the bottom of this page to guide your decision. Be aware that this feature may cease to work at any time.
|
| **Returns\:** string with hexadecimal escape sequences
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| **Note\:** escape() is a non-standard function implemented by browsers and was only standardized for cross-engine compatibility. It is not required to be implemented by all JavaScript engines and may not work everywhere. Use encodeURIComponent() or encodeURI() if possible.
| The escape() function computes a new string in which certain characters have been replaced by hexadecimal escape sequences.
|

------


.. rst-class:: doc-code-block

  :kw:`export`
  unescape(str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| **DEPRECATED\:** This feature is no longer recommended. Though some browsers might still support it, it may have already been removed from the relevant web standards, may be in the process of being dropped, or may only be kept for compatibility purposes. Avoid using it, and update existing code if possible; see the compatibility table at the bottom of this page to guide your decision. Be aware that this feature may cease to work at any time.
|
| **Returns\:** unexcaped string
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| **Note\:** unescape() is a non-standard function implemented by browsers and was only standardized for cross-engine compatibility. It is not required to be implemented by all JavaScript engines and may not work everywhere. Use decodeURIComponent() or decodeURI() if possible.
| The unescape() function computes a new string in which hexadecimal escape sequences are replaced with the characters that they represent. The escape sequences might be introduced by a function like escape().
|

------


Classes
*******


.. _LeLsLcLoLmLpLaLt.UALrLrLaLy:

Array\<T\>
==========



.. rst-class:: doc-code-block

  :kw:`export`

| Represents JS API-compatible Array
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: T | :kw:`null`

| Takes an integer value and returns the item at that index, allowing for positive and negative integers. Negative integers count back from the last item in the array.
|
| **Returns\:** The element in the array matching the given index. Returns null if \`index\` \< \`-length()\` or \`index\` \>= \`length()\`.
|
| **Arguments\:**

 - index\: :kw:`number`  Zero-based index of the array element to be returned. Negative index counts back from the end of the array — if \`index\` \< 0, index + \`array.length()\` is accessed.

------


.. rst-class:: doc-code-block

  :kw:`public`
  concat(other\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Creates a new \`Array\` from this \`Array\` instance and given \`Array\` instance.
|
| **Returns\:** New \`Array\` instance, constructed from \`this\` and given \`other\` instances of \`Array\` class.
|
| **Arguments\:**

 - other\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>  to concatenate into a new array.

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates a new empty instance of Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: T\[\])\: :kw:`void`

| Creates a new instance of Array based on object\[\]
|
| **Arguments\:**

 - d\: T\[\]  Array initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| target\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
|
| **Returns\:** this array after transformation
|
| **Arguments\:**

 - target\: :kw:`number`  index at which to copy the sequence
 - start\: :kw:`number`  index at which to start copying elements from
 - end\: :kw:`number`  index at which to end copying elements from

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(target\: :kw:`number`, start\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
|
| **Returns\:** this array after transformation
|
| **Arguments\:**

 - target\: :kw:`number`  index at which to copy the sequence
 - start\: :kw:`number`  index at which to start copying elements from

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(target\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
|
| **Returns\:** this array after transformation
|
| **Arguments\:**

 - target\: :kw:`number`  index at which to copy the sequence

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (v\: T, k\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Tests whether all elements in the array pass the test implemented by the provided function. It returns a Boolean value.
|
| **Returns\:** \`true\` if \`fn\` returns a \`true\` value for every array element. Otherwise, \`false\`.
|
| **Arguments\:**

 - fn\: (v\: T, k\: :kw:`number`) =\> :kw:`boolean`  function to execute for each element in the array. It should return a \`true\` to indicate the element passes the test, and a \`false\` value otherwise.

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (v\: T) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Tests whether all elements in the array pass the test implemented by the provided function. It returns a Boolean value.
|
| **Returns\:** \`true\` if \`fn\` returns a \`true\` value for every array element. Otherwise, \`false\`.
|
| **Arguments\:**

 - fn\: (v\: T) =\> :kw:`boolean`  function to execute for each element in the array. It should return a \`true\` to indicate the element passes the test, and a \`false\` value otherwise.

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: T,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Changes all elements in the Array to a static value, from a start index to an end index
|
| **Returns\:** this array after transformation
|
| **Arguments\:**

 - value\: T  to fill the array with
 - start\: :kw:`number`  index at which to start filling
 - end\: :kw:`number`  index at which to end filling, but not including

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: T)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Changes all elements in the Array to a static value
|
| **Returns\:** this array after transformation
|
| **Arguments\:**

 - value\: T  to fill the array with

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (v\: T, k\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Constructs a new \`Array\` instance and populates it with portion of a given array, filtered down to just the elements from the given array that pass the test implemented by the provided function.
|
| **Returns\:** New \`Array\` instance constructed from \`this\` with elements filtered using test function \`fn\`.
|
| **Arguments\:**

 - fn\: (v\: T, k\: :kw:`number`) =\> :kw:`boolean`  test function, applied to each element of an array.

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (v\: T) =\> :kw:`boolean`
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Creates a new \`Array\` instance and populates it with portion of a given array, filtered down to just the elements from the given array that pass the test implemented by the provided function.
|
| **Returns\:** New \`Array\` instance constructed from \`this\` with elements filtered using test function \`fn\`.
|
| **Arguments\:**

 - fn\: (v\: T) =\> :kw:`boolean`  test function, applied to each element of an array.

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (elem\: T) =\> :kw:`boolean`
  )\: T | :kw:`null`

| Returns the first element in the provided array that satisfies the provided testing function
|
| **Returns\:** found element or null otherwise
|
| **Arguments\:**

 - fn\: (elem\: T) =\> :kw:`boolean`  testing function

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (elem\: T) =\> :kw:`boolean`
  )\: :kw:`number`

| Returns the index of the first element in an array that satisfies the provided testing function
|
| **Returns\:** found element index or -1 otherwise
|
| **Arguments\:**

 - fn\: (elem\: T) =\> :kw:`boolean`  testing function

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (elem\: T) =\> :kw:`boolean`
  )\: T | :kw:`null`

| Iterates the array in reverse order and returns the value of the first element that satisfies the provided testing function
|
| **Returns\:** found element or null otherwise
|
| **Arguments\:**

 - fn\: (elem\: T) =\> :kw:`boolean`  testing function

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (element\: T) =\> :kw:`boolean`
  )\: :kw:`number`

| Iterates the array in reverse order and returns the index of the first element that satisfies the provided testing function. If no elements satisfy the testing function, -1 is returned.
|
| **Returns\:** index of first element satisfying to fn, -1 if no such element
|
| **Arguments\:**

 - fn\: (element\: T) =\> :kw:`boolean`  testing function

------


.. rst-class:: doc-code-block

  :kw:`public`
  flat(depth\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<:ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`\>

| Creates a new Array with all sub-array elements concatenated into it recursively up to the specified depth.
|
| **Returns\:** a flattened Array with respect to depth
|
| **Arguments\:**

 - depth\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  flat()\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<:ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`\>

| Creates a new Array with all sub-array elements concatenated
|
| **Returns\:** a flattened Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  flatMap(
  |nbsp| fn\: (v\: T, k\: :kw:`number`) =\> :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<:ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`\>

| Applies flat and than map
| fn a function to apply
|
| **Return\:** new Array after map and than flat
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  flatMap(
  |nbsp| fn\: (v\: T) =\> :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<:ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`\>

| Applies flat and than map
| fn a function to apply
|
| **Return\:** new Array after map and than flat
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (a\: T) =\> :kw:`void`
  )\: :kw:`void`

| Executes a provided function once for each array element.
|
| **Arguments\:**

 - fn\: (a\: T) =\> :kw:`void`  to apply for each element of the Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(val\: T)\: :kw:`boolean`

| Checks whether an Array includes a certain value among its entries, returning true or false as appropriate.
|
| **Returns\:** true if val is in Array
|
| **Arguments\:**

 - val\: T  value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(val\: T)\: :kw:`number`

| Returns the first index at which a given element can be found in the array, or -1 if it is not present.
|
| **Returns\:** index of val, -1 otherwise
|
| **Arguments\:**

 - val\: T  value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(sep\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Creates and returns a new string by concatenating all of the elements in an \`Array\`, separated by a specified separator string. If the array has only one item, then that item will be returned without using the separator.
|
| **Returns\:** A string with all array elements joined. If \`length()\` is 0, the empty string is returned.
|
| **Arguments\:**

 - sep\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  specifies a separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Creates and returns a new string by concatenating all of the elements in an \`Array\`, separated by a comma. If the array has only one item, then that item will be returned without using the separator.
|
| **Returns\:** A string with all array elements joined. If \`length()\` is 0, the empty string is returned.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns an iterator over all indices
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(element\: T, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns the last index at which a given element can be found in the array, or -1 if it is not present. The array is searched backwards, starting at fromIndex.
|
| **Returns\:** The last index of the element in the array; -1 if not found.\\
|
| **Arguments\:**

 - element\: T  element to locate in the array.
 - fromIndex\: :kw:`number`  zero-based index at which to start searching backwards. Negative index counts back from the end of the array — if \`fromIndex\` \< 0, \`fromIndex\` + \`length()\` is used. If \`fromIndex\` \< \`-length()\`, the array is not searched and -1 is returned. If \`fromIndex\` \>= \`length()\` then \`array.length - 1\` is used, causing the entire array to be searched.

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(element\: T)\: :kw:`number`

| Returns the last index at which a given element can be found in the array, or -1 if it is not present.
|
| **Returns\:** The last index of the element in the array; -1 if not found.
|
| **Arguments\:**

 - element\: T  to find in the array.

------


.. rst-class:: doc-code-block

  :kw:`public`
  length()\: :kw:`number`

| Returns the number of elements in the Array.
|
| **Returns\:** Element count in the \`Array\` instance.
|

------

.. rst-class:: doc-code-block

  :kw:`public`
  of(items: T\[\]): Array\<T\>

| Creates a new \`Array\` object from initializer
|
| **Returns\:** \`Array\` instance, constructed from \`this\` and given argument.
|
| **Arguments\:**

 - items\: T\[\]

------


.. rst-class:: doc-code-block

  :kw:`public`
  map\<U\>(
  |nbsp| fn\: (v\: T, k\: :kw:`number`) =\> U
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<U\>

| Creates a new \`Array\` object and populates it with the results of calling a provided function on every element in \`this\` instance of \`Array\` class.
|
| **Returns\:** \`Array\` instance, constructed from \`this\` and given function.
|
| **Arguments\:**

 - fn\: (v\: T, k\: :kw:`number`) =\> U  mapping function, applied to each element of an array.

------


.. rst-class:: doc-code-block

  :kw:`public`
  pop()\: T | :kw:`null`

| Removes the last element from an array and returns that element. This method changes the length of the array.
|
| **Returns\:** removed element
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  push(val\: T)\: :kw:`number`

| Adds the specified elements to the end of an array and returns the new length of the array.
|
| **Returns\:** new length
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce\<U\>(
  |nbsp| fn\: (a\: U, b\: T) =\> U,
  |nbsp| initVal\: U
  )\: U

| Executes a user-supplied \"reducer\" callback function on each element of the array, in order, passing in the return value from the calculation on the preceding element. The final result of running the reducer across all elements of the array is a single value. Order is from left-to-right.
|
| **Returns\:** a result after applying fn over all elements of the Array
|
| **Arguments\:**

 - fn\: (a\: U, b\: T) =\> U  reduce function
 - initVal\: U  start value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (a\: T, b\: T) =\> T
  )\: T

| Executes a user-supplied \"reducer\" callback function on each element of the array, in order, passing in the return value from the calculation on the preceding element. The final result of running the reducer across all elements of the array is a single value. Order is from left-to-right.
|
| **Returns\:** a result after applying fn over all elements of the Array
|
| **Arguments\:**

 - fn\: (a\: T, b\: T) =\> T  reduce function

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight\<U\>(
  |nbsp| fn\: (a\: U, b\: T) =\> U,
  |nbsp| initVal\: U
  )\: U

| Executes a user-supplied \"reducer\" callback function on each element of the array, in order, passing in the return value from the calculation on the preceding element. The final result of running the reducer across all elements of the array is a single value. Order is from right-to-left.
|
| **Returns\:** a result after applying fn over all elements of the Array
|
| **Arguments\:**

 - fn\: (a\: U, b\: T) =\> U  reduce function
 - initVal\: U  start value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (a\: T, b\: T) =\> T
  )\: T

| Executes a user-supplied \"reducer\" callback function on each element of the array, in order, passing in the return value from the calculation on the preceding element. The final result of running the reducer across all elements of the array is a single value. Order is from right-to-left.
|
| **Returns\:** a result after applying fn over all elements of the Array
|
| **Arguments\:**

 - fn\: (a\: T, b\: T) =\> T  reduce function

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :kw:`void`

| Modifies \`this\` instance of \`Array\` class and populates it with same elements ordered towards the direction opposite to that previously stated.
|
| **Note\:** Mutating method
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  shift()\: T | :kw:`null`

| Removes the first element from an array and returns that removed element. This method changes the length of the array.
|
| **Returns\:** shifted element, i.e. that was at index zero
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(start\: :kw:`number`, end\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Creates a new \`Array\` object and populates it with elements of \`this\` instance of \`Array\` class selected from \`start\` to \`end\` (\`end\` not included) where \`start\` and \`end\` represent the index of items in that array.
|
| **Returns\:** \`Array\` instance, constructed from extracted elements of \`this\` instance.
|
| **Arguments\:**

 - start\: :kw:`number`  zero-based index at which to start extraction
 - end\: :kw:`number`  zero-based index at which to end extraction. \`slice()\` extracts up to but not including end.

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(start\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Creates a new \`Array\` object and populates it with elements of \`this\` instance of \`Array\` class selected from \`start\` to \`Int.MAX_VALUE\`, which means 'to the end of an array'.
|
| **Returns\:** \`Array\` instance, constructed from extracted elements of \`this\` instance.
|
| **Arguments\:**

 - start\: :kw:`number`  zero-based index at which to start extraction

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Creates a new \`Array\` object and populates it with elements of \`this\` instance of \`Array\` class
|
| **Returns\:** \`Array\` instance, constructed all elements of \`this\` instance.
|
| **Note\:** This method creates full copy of original \`Array\` instance.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (v\: T, k\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Tests whether at least one element in the array pass the test implemented by the provided function. It returns a Boolean value.
|
| **Returns\:** \`true\` if \`fn\` returns a \`true\` value for at least one array element. Otherwise, \`false\`.
|
| **Arguments\:**

 - fn\: (v\: T, k\: :kw:`number`) =\> :kw:`boolean`  function to execute for each element in the array. It should return a \`true\` to indicate the element passes the test, and a \`false\` value otherwise.

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| comparator\: (a\: T, b\: T) =\> :kw:`number`
  )\: :kw:`void`

| Reorders elements of \`this\` using comparator function.
|
| **Arguments\:**

 - comparator\: (a\: T, b\: T) =\> :kw:`number`  function that defines the sort order.

| **Note\:** Mutating method
| TODO clarify UTF-16 or UTF-8
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :kw:`void`

| Reorders elements of \`this\` using a default comparator. Elements sorted in ascending order built upon converting the elements into strings, then comparing their sequences of UTF-16 code units values.
|
| **Note\:** Mutating method
| TODO clarify UTF-16 or UTF-8
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  splice(start\: :kw:`number`, delete\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Changes the contents of an array by removing or replacing existing elements and/or adding new elements in place.
|
| **Returns\:** an Array with deleted elements
|
| **Arguments\:**

 - start\: :kw:`number`  index
 - delete\: :kw:`number`  number of items after start index

------


.. rst-class:: doc-code-block

  :kw:`public`
  splice(start\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Changes the contents of an array by removing or replacing existing elements and/or adding new elements in place.
|
| **Returns\:** an Array with deleted elements from start to the last element of the current instance
|
| **Arguments\:**

 - start\: :kw:`number`  index

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a locale string representing the specified array and its elements.
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a locale string representing the specified array and its elements.
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a locale string representing the specified array and its elements.
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Copying version of the reverse() method. It returns a new array with the elements in reversed order.
|
| **Returns\:** reversed copy of the current Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Copying version of the sort() method. It returns a new array with the elements sorted in ascending order.
|
| **Returns\:** sorted copy of hte current instance using default comparator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted(
  |nbsp| comparator\: (a\: T, b\: T) =\> :kw:`number`
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Copying version of the sort() method. It returns a new array with the elements sorted in ascending order.
|
| **Returns\:** sorted copy of the current instance comparator
|
| **Arguments\:**

 - comparator\: (a\: T, b\: T) =\> :kw:`number`  function to compare to elements of the Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSpliced(start\: :kw:`number`, delete\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Copying version of the splice() method.
|
| **Returns\:** a new Array with some elements removed and/or replaced at a given index.
|
| **Arguments\:**

 - start\: :kw:`number`  index
 - delete\: :kw:`number`  number of items after start index

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSpliced(start\: :kw:`number`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Copying version of the splice() method.
|
| **Returns\:** a new Array with some elements removed and/or replaced at a given index.
|
| **Arguments\:**

 - start\: :kw:`number`  index

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representing the specified array and its elements.
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<T\>

| Returns an iterator over all values
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: T)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Copying version of using the bracket notation to change the value of a given index. It returns a new Array with the element at the given index replaced with the given value.
|
| **Returns\:** a new Array with the element at the given index replaced with the given value
|
| **Arguments\:**

 - index\: :kw:`number`  to replace
 - value\: T  new value

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  find\<T\>(
  |nbsp| fn\: (elem\: T) =\> :kw:`boolean`,
  |nbsp| thisArg\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>
  )\: T | :kw:`null`

| Returns the first element in the provided array that satisfies the provided testing function
|
| **Returns\:** found element or null otherwise
|
| **Arguments\:**

 - fn\: (elem\: T) =\> :kw:`boolean`  testing function
 - thisArg\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>  an Array to search

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  findIndex\<T\>(
  |nbsp| fn\: (elem\: T) =\> :kw:`boolean`,
  |nbsp| thisArg\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>
  )\: :kw:`number`

| Returns the index of the first element in an array that satisfies the provided testing function
|
| **Returns\:** found element index or -1 otherwise
|
| **Arguments\:**

 - fn\: (elem\: T) =\> :kw:`boolean`  testing function
 - thisArg\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>  an Array to search

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  findLast\<T\>(
  |nbsp| fn\: (elem\: T) =\> :kw:`boolean`,
  |nbsp| thisArg\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>
  )\: T | :kw:`null`

| Iterates the array in reverse order and returns the value of the first element that satisfies the provided testing function
|
| **Returns\:** found element or null otherwise
|
| **Arguments\:**

 - fn\: (elem\: T) =\> :kw:`boolean`  testing function
 - thisArg\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>  an Array to search

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  from\<T\>(arr\: T\[\])\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Creates a new \`Array\` instance from \`object\[\]\` primitive array.
|
| **Returns\:** \`Array\` intance constructed from \`object\[\]\` primitive array.
|
| **Arguments\:**

 - arr\: T\[\]  primitive 'object' array to be converted to \`Array\` instance.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  from\<T, U\>(
  |nbsp| arr\: T\[\],
  |nbsp| fn\: (v\: T, k\: :kw:`number`) =\> U
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<U\>

| Creates a new \`Array\` instance from \`object\[\]\` primitive array.
|
| **Returns\:** \`Array\` intance constructed from \`object\[\]\` primitive array and given function.
|
| **Arguments\:**

 - arr\: T\[\]  primitive 'object' array, converted to \`Array\` instance.
 - fn\: (v\: T, k\: :kw:`number`) =\> U  map function to call on every element of the array. Every value to be added to the array is first passed through this function, and \`fn\`'s return value is added to the array instead.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  from\<U\>(
  |nbsp| str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`,
  |nbsp| fn\: (v\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, k\: :kw:`number`) =\> U
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<U\>

| Creates a new \`Array\` instance from characters of \`string\` and mapping function.
|
| **Returns\:** \`Array\` intance constructed from characters of source \`string\` and given function.
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  source string to be converted to array of character's \`string\`
 - fn\: (v\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, k\: :kw:`number`) =\> U  map function to call on every character of source string.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  from\<U\>(
  |nbsp| str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`,
  |nbsp| fn\: (v\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, k\: :kw:`number`) =\> U
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<U\>

| Creates a new \`Array\` instance from characters of \`string\` and mapping function.
|
| **Returns\:** \`Array\` intance constructed from characters of source \`string\` and given function.
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  source string to be converted to array of character's \`string\`
 - fn\: (v\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, k\: :kw:`number`) =\> U  map function to call on every character of source string.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  from(str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<:ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`\>

| Creates a new \`Array\` instance from characters of \`string\`.
|
| **Returns\:** \`Array\` intance constructed from characters of source \`string\`.
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  source string to be converted to array of character's \`string\`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  fromAsync\<T, U\>(
  |nbsp| arrLike\: T\[\],
  |nbsp| mapFn\: (a\: T, i\: :kw:`number`) =\> U
  )\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<U\>

| Creates a new Array from array-like or iterable
|
| **Returns\:** a new instance of an Array
|
| **Arguments\:**

 - arrLike\: T\[\]  array-like or an iterable object
 - mapFn\: (a\: T, i\: :kw:`number`) =\> U  a function to apply over all elements of arrLike

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  fromAsync\<T\>(arrLike\: T\[\])\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

| Creates a new Array from array-like or iterable
|
| **Returns\:** new instance of an Array
|
| **Arguments\:**

 - arrLike\: T\[\]  array-like or an iterable object

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isArray\<T\>(arr\: T\[\])\: :kw:`boolean`

| Checks whether the passed value is an Array.
|
| **Returns\:** true is arr is a non-null array, false otherwise
|
| **Arguments\:**

 - arr\: T\[\]

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isArray\<T\>(arr\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>)\: :kw:`boolean`

| Checks whether the passed value is an Array.
|
| **Returns\:** true is arr is a non-null and non-empty array, false otherwise
|
| **Arguments\:**

 - arr\: :ref:`Array<LeLsLcLoLmLpLaLt.UALrLrLaLy>`\<T\>

------

Properties
----------

.. rst-class:: doc-code-block

  - length\: :kw:`number`



.. _LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr:

ArrayBuffer
===========

.. rst-class:: doc-code-block

  :kw:`export`

| **Class\:** JS ArrayBuffer API-compatible class
|

Methods
-------

.. rst-class:: doc-code-block

  :kw:`public`
  resize(newLen\: :kw:`number`)\: :kw:`void`

| Resizes the ArrayBuffer
|
| **Arguments\:**

 - newLen\: :kw:`number`  new length

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`

| Creates a new ArrayBuffer with copy of bytes in range \[begin;end)
|
| **Returns\:** data taken from current ArrayBuffer with respect to begin and end parameters
|
| **Arguments\:**

 - begin\: :kw:`number`  an inclusive index to start copying with
 - end\: :kw:`number`  a last exclusive index to stop copying

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isView(obj\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :kw:`boolean`

| Checks if the passed object is a View
|
| **Returns\:** true if obj is instance of typed array
|
| **Arguments\:**

 - obj\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  to check

------

Properties
----------

.. rst-class:: doc-code-block

  - byteLength\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UALtLoLmLiLcLs:

Atomics
=======



.. rst-class:: doc-code-block

  :kw:`export`

| **Class\:** Represents JS API-compatible Atomics
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  add(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Adds a given \[value\] at a given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  and(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise AND of the given \[value\] and the value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  compareExchange(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| expectedValue\: :kw:`number`,
  |nbsp| replacementValue\: :kw:`number`
  )\: :kw:`number`

| Exchanges a given \[replacementValue\] at a given \[index\] in the \[typedArray\], if a given \[expectedValue\] equals the old value. Returns the old value at that position whether it was equal to the expected value or not.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - expectedValue\: :kw:`number`
 - replacementValue\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  exchange(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isLockFree(size\: :kw:`number`)\: :kw:`boolean`

| isLockFree(n) checks whether atomic operations for typed arrays of the given element size use hardware atomics instructions instead of locks.
|
| **Returns\:** a boolean result
|
| **Arguments\:**

 - size\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  load(typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`, index\: :kw:`number`)\: :kw:`number`

| Returns a value at the given \[index\] in the \[typedArray\].
|
| **Returns\:** the read value
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  notify(typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`, offset\: :kw:`number`)\: :kw:`number`

| Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index. (index = typedArray.byteOffset + offset \* 4)
| Note\: This method also wakes up threads suspended by the BigInt64Array Atomics.wait(t64, offset64) calls. But if and only if 't64' views the same ArrayBuffer as 'typedArray' and 'offset64' and 'offset' point at the same index in that ArrayBuffer.
|
| **Returns\:** the number of notified threads
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - offset\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  notify(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| offset\: :kw:`number`,
  |nbsp| count\: :kw:`number`
  )\: :kw:`number`

| Operates exactly like Atomics.notify(Int32Array, int) but specifies the maximum number of threads to notify using 'count'.
|
| **Returns\:** the number of notified threads
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - offset\: :kw:`number`
 - count\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  notify(typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`, offset\: :kw:`number`)\: :kw:`number`

| Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index. (index = typedArray.byteOffset + offset \* 8)
| Note\: This method also wakes up threads suspended by the Int32Array Atomics.wait(t32, offset32) calls. But if and only if 't32' views the same ArrayBuffer as 'typedArray' and 'offset32' and 'offset' point at the same index in that ArrayBuffer.
|
| **Returns\:** the number of notified threads
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - offset\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  notify(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| offset\: :kw:`number`,
  |nbsp| count\: :kw:`number`
  )\: :kw:`number`

| Operates exactly like Atomics.notify(BigInt64Array, int) but specifies the maximum number of threads to notify using 'count'.
|
| **Returns\:** the number of notified threads
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - offset\: :kw:`number`
 - count\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  or(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise OR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  store(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Stores a given \[value\] at a given \[index\] in the \[typedArray\] and returns that value.
|
| **Returns\:** the new value (i.e. the \[value\] parameter)
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sub(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Subtracts a given \[value\] at a given \[index\] in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  wait(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| offset\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Suspends the current thread if \"typedArray\[offset\] != value\" until it is notified by Atomics.notify. Note\: An Atomics.notify call will wake up this thread even if \"typedArray\[offset\] == value\" is true. In other words, the \"typedArray\[offset\] != value\" condition is checked only once.
|
| **Returns\:** \"not-equal\" if the the value the the given \[offset\] was not equal to the given \[value\], otherwise after being notified returns \"ok\"
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - offset\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  wait(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| offset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| timeout\: :kw:`number`
  )\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Operates exactly like Atomics.wait(Int32Array, int, int) but also returns if the given \[timeout\] (in ms.) passes.
|
| **Returns\:** \"not-equal\" and \"ok\" like Atomics.wait(Int32Array, int, int), but also \"timed-out\" if the timeout passes
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - offset\: :kw:`number`
 - value\: :kw:`number`
 - timeout\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  wait(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| offset\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Suspends the current thread if \"typedArray\[offset\] != value\" until it is notified by Atomics.notify. Note 1\: An Atomics.notify call will wake up this thread even if \"typedArray\[offset\] == value\" is true. In other words, the \"typedArray\[offset\] != value\" condition is checked only once. Note 2\: A call to Atomic.notify(Int32Array, int) will wake up this thread, but only if both offsets point at the same index in the underlying ArrayBuffer. In the other words, a notification issued to the right 32-bit half of the 64-bit integer will not wake up this thread.
|
| **Returns\:** \"not-equal\" if the the value the the given \[offset\] was not equal to the given \[value\], otherwise after being notified returns \"ok\"
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - offset\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  wait(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| offset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| timeout\: :kw:`number`
  )\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Operates exactly like Atomics.wait(BigInt64Array, int, long) but also returns if the given \[timeout\] (in ms.) passes.
|
| **Returns\:** \"not-equal\" and \"ok\" like Atomics.wait(Int32Array, int, int), but also \"timed-out\" if the timeout passes
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - offset\: :kw:`number`
 - value\: :kw:`number`
 - timeout\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  xor(
  |nbsp| typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| value\: :kw:`number`
  )\: :kw:`number`

| Computes a bitwise XOR of the given \[value\] and the value at the given \[index\] in the \[typedArray\]. Updates the value in the array and returns the old value at that position.
|
| **Returns\:** the old value at that position
|
| **Arguments\:**

 - typedArray\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`
 - index\: :kw:`number`
 - value\: :kw:`number`

------


.. _LeLsLcLoLmLpLaLt.UBLiLgUILnLt:

BigInt
======



.. rst-class:: doc-code-block

  :kw:`export`

| BigInt class stub
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs new BigInt
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: :kw:`number`)\: :kw:`void`

| Constructs new BigInt from int
|
| **Arguments\:**

 - d\: :kw:`number`  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs new BigInt from string
|
| **Arguments\:**

 - d\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: :kw:`boolean`)\: :kw:`void`

| Constructs new BigInt from string
|
| **Arguments\:**

 - d\: :kw:`boolean`  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :kw:`void`

| Constructs new BigInt from string
|
| **Arguments\:**

 - d\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: :kw:`number`)\: :kw:`void`

| Constructs new BigInt from string
|
| **Arguments\:**

 - d\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: :kw:`number`)\: :kw:`void`

| Constructs new BigInt from number
|
| **Arguments\:**

 - d\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a locale string representing the specified array and its elements.
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a locale string representing the specified array and its elements.
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a locale string representing the specified array and its elements.
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns string representation
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  valueOf()\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Returns a BigInt instance
|
| **Returns\:** a BigInt instance
|

------


.. _LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy:

BigInt64Array
=============



.. rst-class:: doc-code-block

  :kw:`export`

| JS BigInt64Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty BigInt64Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an BigInt64Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type long in newly created BigInt64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an BigInt64Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an BigInt64Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of BigInt64Array.
|
| **Arguments\:**

 - other\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of BigInt64Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of BigInt64Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigInt64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigInt64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigInt64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Fills the BigInt64Array with specified value
|
| **Returns\:** modified BigInt64Array
|
| **Arguments\:**

 - value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, start\: :kw:`number`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Fills the BigInt64Array with specified value
|
| **Returns\:** modified BigInt64Array
|
| **Arguments\:**

 - value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Fills the BigInt64Array with specified value
|
| **Returns\:** modified BigInt64Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| creates a new BigInt64Array from current BigInt64Array based on a condition fn
|
| **Returns\:** a new BigInt64Array with elements from current BigInt64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a new BigInt64Array from current BigInt64Array based on a condition fn.
|
| **Returns\:** a new BigInt64Array with elements from current BigInt64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| creates a new BigInt64Array from current BigInt64Array based on a condition fn
|
| **Returns\:** a new BigInt64Array with elements from current BigInt64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the first element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return long | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the last element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigInt64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of BigInt64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of BigInt64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of BigInt64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates an BigInt64Array from array-like argument
|
| **Returns\:** new BigInt64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize BigInt64Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates an BigInt64Array from array-like argument
|
| **Returns\:** new BigInt64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize BigInt64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates an BigInt64Array from array-like argument
|
| **Returns\:** new BigInt64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize BigInt64Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in BigInt64Array
|
| **Returns\:** true if e is in BigInt64Array, false otherwise
|
| **Arguments\:**

 - e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in BigInt64Array
|
| **Returns\:** true if e is in BigInt64Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns keys of the BigInt64Array
|
| **Returns\:** iterator over keys
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a new BigInt64Array using fn(arr\[i\]) over all elements of current BigInt64Array
|
| **Returns\:** a new BigInt64Array where for each element from current BigInt64Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a function to apply for each element of current BigInt64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a new BigInt64Array using fn(arr\[i\]) over all elements of current BigInt64Array.
|
| **Returns\:** a new BigInt64Array where for each element from current BigInt64Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a function to apply for each element of current BigInt64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a new BigInt64Array using initializer
|
| **Returns\:** a new BigInt64Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition
 - init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition
 - init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a new BigInt64Array using reversed data from the current one
|
| **Returns\:** a new BigInt64Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(long\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current BigInt64Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\])\: :kw:`void`

| Copies all elements of arr to the current BigInt64Array.
|
| **Arguments\:**

 - arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a slice of current BigInt64Array using range \[begin, end)
|
| **Returns\:** a new BigInt64Array with elements of current BigInt64Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a slice of current BigInt64Array using range \[begin, this.length).
|
| **Returns\:** a new BigInt64Array with elements of current BigInt64Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a slice of current BigInt64 with all elements.
|
| **Returns\:** a new BigInt64Array with elements of current BigInt64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigInt64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigInt64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigInt64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted BigInt64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, b\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`number`
  )\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted BigInt64Array
|
| **Arguments\:**

 - fn\: (a\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, b\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a BigInt64Array with the same underlying ArrayBuffer
|
| **Returns\:** new BigInt64Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a BigInt64Array with the same ArrayBuffer
|
| **Returns\:** new BigInt64Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a BigInt64Array with the same ArrayBuffer
|
| **Returns\:** new BigInt64Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts BigInt64Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts BigInt64Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts BigInt64Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the BigInt64Array
|
| **Returns\:** a string representation of the BigInt64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns array values iterator
|
| **Returns\:** an iterator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`BigInt64Array<LeLsLcLoLmLpLaLt.UBLiLgUILnLtU6U4UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an BigInt64Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy:

BigUint64Array
==============



.. rst-class:: doc-code-block

  :kw:`export`

| JS BigUint64Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty BigUint64Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an BigUint64Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type BigInt in newly created BigUint64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an BigUint64Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an BigUint64Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of BigUint64Array.
|
| **Arguments\:**

 - other\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of BigUint64Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of BigUint64Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigUint64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigUint64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigUint64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigUint64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of BigUint64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Fills the BigUint64Array with specified value
|
| **Returns\:** modified BigUint64Array
|
| **Arguments\:**

 - value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, start\: :kw:`number`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Fills the BigUint64Array with specified value
|
| **Returns\:** modified BigUint64Array
|
| **Arguments\:**

 - value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Fills the BigUint64Array with specified value
|
| **Returns\:** modified BigUint64Array
|
| **Arguments\:**

 - value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a new BigUint64Array from current BigUint64Array based on a condition fn.
|
| **Returns\:** a new BigUint64Array with elements from current BigUint64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| creates a new BigUint64Array from current BigUint64Array based on a condition fn
|
| **Returns\:** a new BigUint64Array with elements from current BigUint64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| creates a new BigUint64Array from current BigUint64Array based on a condition fn
|
| **Returns\:** a new BigUint64Array with elements from current BigUint64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a new BigUint64Array from current BigUint64Array based on a condition fn.
|
| **Returns\:** a new BigUint64Array with elements from current BigUint64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| creates a new BigUint64Array from current BigUint64Array based on a condition fn
|
| **Returns\:** a new BigUint64Array with elements from current BigUint64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return BigInt | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return BigInt | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Finds the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the BigUint64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :kw:`void`

| Applies a function over all elements of BigUint64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :kw:`void`

| Applies a function over all elements of BigUint64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :kw:`void`

| Applies a function over all elements of BigUint64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :kw:`void`

| Applies a function over all elements of BigUint64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :kw:`void`

| Applies a function over all elements of BigUint64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates an BigUint64Array from array-like argument
|
| **Returns\:** new BigUint64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize BigUint64Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates an BigUint64Array from array-like argument
|
| **Returns\:** new BigUint64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize BigUint64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates an BigUint64Array from array-like argument
|
| **Returns\:** new BigUint64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize BigUint64Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates an BigUint64Array from array-like argument
|
| **Returns\:** new BigUint64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize BigUint64Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in BigUint64Array
|
| **Returns\:** true if e is in BigUint64Array, false otherwise
|
| **Arguments\:**

 - e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :kw:`boolean`

| Checks if specified argument is in BigUint64Array
|
| **Returns\:** true if e is in BigUint64Array, false otherwise
|
| **Arguments\:**

 - e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a new BigUint64Array using fn(arr\[i\]) over all elements of current BigUint64Array.
|
| **Returns\:** a new BigUint64Array where for each element from current BigUint64Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a function to apply for each element of current BigUint64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a new BigUint64Array using fn(arr\[i\]) over all elements of current BigUint64Array
|
| **Returns\:** a new BigUint64Array where for each element from current BigUint64Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a function to apply for each element of current BigUint64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a new BigUint64Array using fn(arr\[i\]) over all elements of current BigUint64Array.
|
| **Returns\:** a new BigUint64Array where for each element from current BigUint64Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  a function to apply for each element of current BigUint64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`bigint`\[\])\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a new BigUint64Array using initializer
|
| **Returns\:** a new BigUint64Array from data
|
| **Arguments\:**

 - data\: :kw:`bigint`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition
 - init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition
 - init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition
 - init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`,
  |nbsp| init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition
 - init\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`
  )\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curVal\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, curIndex\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a new BigUint64Array using reversed data from the current one
|
| **Returns\:** a new BigUint64Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(BigInt\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current BigUint64Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\])\: :kw:`void`

| Copies all elements of arr to the current BigUint64Array.
|
| **Arguments\:**

 - arr\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a slice of current BigUint64Array using range \[begin, end)
|
| **Returns\:** a new BigUint64Array with elements of current BigUint64Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a slice of current BigUint64Array using range \[begin, this.length).
|
| **Returns\:** a new BigUint64Array with elements of current BigUint64Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a slice of current BigUint64 with all elements.
|
| **Returns\:** a new BigUint64Array with elements of current BigUint64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigUint64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigUint64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigUint64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigUint64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`, array\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of BigUint64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted BigUint64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, b\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`number`
  )\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted BigUint64Array
|
| **Arguments\:**

 - fn\: (a\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`, b\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a BigUint64Array with the same underlying ArrayBuffer
|
| **Returns\:** new BigUint64Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a BigUint64Array with the same ArrayBuffer
|
| **Returns\:** new BigUint64Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a BigUint64Array with the same ArrayBuffer
|
| **Returns\:** new BigUint64Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts BigUint64Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts BigUint64Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts BigUint64Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the BigUint64Array
|
| **Returns\:** a string representation of the BigUint64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`)\: :ref:`BigUint64Array<LeLsLcLoLmLpLaLt.UBLiLgUULiLnLtU6U4UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an BigUint64Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :ref:`BigInt<LeLsLcLoLmLpLaLt.UBLiLgUILnLt>`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`



.. _LsLtLd.LcLoLrLe.UBLoLoLlLeLaLn:

Boolean
=======



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

| Represents boxed boolean value and related operations
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new Boolean with false value
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(value\: :kw:`boolean`)\: :kw:`void`

| Constructs a new Boolean with provided value
|
| **Arguments\:**

 - value\: :kw:`boolean` ---  value to construct class instance with

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(value\: :ref:`Boolean<LsLtLd.LcLoLrLe.UBLoLoLlLeLaLn>`)\: :kw:`void`

| Constructs a new Boolean with provided value
|
| **Arguments\:**

 - value\: :ref:`Boolean<LsLtLd.LcLoLrLe.UBLoLoLlLeLaLn>` ---  value to construct class instance with

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts this object to a string
|
| **Returns\:** \"True\" if this instance is true, \"False\" otherwise
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  valueOf(b\: :kw:`boolean`)\: :ref:`Boolean<LsLtLd.LcLoLrLe.UBLoLoLlLeLaLn>`

| Static method that converts primitive boolean to boxed version
|
| **Returns\:** boxed value that represents provided primitive value
|
| **Arguments\:**

 - b\: :kw:`boolean` ---  value to be converted

------

.. _LeLsLcLoLmLpLaLt.UDLaLtLaUVLiLeLw:

DataView
========



.. rst-class:: doc-code-block

  :kw:`export`

| DataView representation
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Constructs view
|
| **Arguments\:**

 - ArrayBuffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  underlying ArrayBuffer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Constructs view
|
| **Arguments\:**

 - ArrayBuffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  underlying ArrayBuffer
 - byteOffset\: :kw:`number`  offset to start from

| **Throws\:**

 - :ref:`RangeError<LeLsLcLoLmLpLaLt.URLaLnLgLeUELrLrLoLr>`  if offset is out of array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| ArrayBuffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| byteLength\: :kw:`number`
  )\: :kw:`void`

| Constructs view
|
| **Arguments\:**

 - ArrayBuffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  underlying ArrayBuffer
 - byteOffset\: :kw:`number`  offset to start from
 - byteLength\: :kw:`number`  lenth of bytes to take

| **Throws\:**

 - :ref:`RangeError<LeLsLcLoLmLpLaLt.URLaLnLgLeUELrLrLoLr>`  if provided indicies are invalid

------


.. rst-class:: doc-code-block

  :kw:`public`
  getBigInt64(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getBigInt64(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getBigUint64(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getBigUint64(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getFloat32(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getFloat32(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getFloat64(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getFloat64(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getInt16(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getInt16(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getInt32(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getInt32(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getInt8(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUint16(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUint16(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUint32(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUint32(byteOffset\: :kw:`number`, littleEndian\: :kw:`boolean`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUint8(byteOffset\: :kw:`number`)\: :kw:`number`

| Read bytes as they represent given type
|
| **Returns\:** read value (big endian)
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to read

------


.. rst-class:: doc-code-block

  :kw:`public`
  setBigInt64(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setBigInt64(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setBigUint64(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setBigUint64(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setFloat32(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setFloat32(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setFloat64(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setFloat64(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setInt16(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setInt16(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setInt32(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setInt32(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setInt8(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUint16(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUint16(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUint32(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUint32(
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| value\: :kw:`number`,
  |nbsp| littleEndian\: :kw:`boolean`
  )\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write
 - value\: :kw:`number`
 - littleEndian\: :kw:`boolean`  read as little or big endian

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUint8(byteOffset\: :kw:`number`, value\: :kw:`number`)\: :kw:`void`

| Sets bytes as they represent given type
|
| **Arguments\:**

 - byteOffset\: :kw:`number`  zero index to write (big endian)
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UDLaLtLe:

Date
====



.. rst-class:: doc-code-block

  :kw:`export`

| Date JS API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Default constructor.
|
| **Description\:** Initializes Date instance with current time.
|
| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(d\: :ref:`Date<LeLsLcLoLmLpLaLt.UDLaLtLe>`)\: :kw:`void` :kw:`throws`

| \`Date\` constructor.
|
| **Description\:** Initializes \`Date\` instance with another \`Date\` instance.
|
| **Arguments\:**

 - d\: :ref:`Date<LeLsLcLoLmLpLaLt.UDLaLtLe>`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(ms\: :kw:`number`)\: :kw:`void`

| \`Date\` constructor.
|
| **Description\:** Initialize \`Date\` instance with milliseconds given.
|
| **Arguments\:**

 - ms\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(year\: :kw:`number`, month\: :kw:`number`)\: :kw:`void`

| \`Date\` constructor.
|
| **Description\:** Initialize \`Date\` instance with year and month given.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Description\:** Initialize \`Date\` instance with year, month and day given.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Description\:** Initialize \`Date\` instance with year, month, day and hours given.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Description\:** Initialize \`Date\` instance with year, month, day, hours and minutes given.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`
 - minutes\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Description\:** Initialize \`Date\` instance with year, month, day, hours, minutes and seconds given.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`
 - minutes\: :kw:`number`
 - seconds\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`,
  |nbsp| ms\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Description\:** Initialize \`Date\` instance with year, month, day, hours, minutes, seconds and milliseconds given.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`
 - minutes\: :kw:`number`
 - seconds\: :kw:`number`
 - ms\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(ms\: :kw:`number`)\: :kw:`void`

| \`Date\` constructor.
|
| **Arguments\:**

 - ms\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(year\: :kw:`number`, month\: :kw:`number`)\: :kw:`void`

| \`Date\` constructor.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`
 - minutes\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`
 - minutes\: :kw:`number`
 - seconds\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`,
  |nbsp| ms\: :kw:`number`
  )\: :kw:`void`

| \`Date\` constructor.
|
| **Arguments\:**

 - year\: :kw:`number`
 - month\: :kw:`number`
 - day\: :kw:`number`
 - hours\: :kw:`number`
 - minutes\: :kw:`number`
 - seconds\: :kw:`number`
 - ms\: :kw:`number`

| **See\:** ECMA-262, 21.4.2.1
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getDate()\: :kw:`number`

| The \`getDate()\` method returns the day of the month for the specified date according to local time.
|
| **Returns\:** An integer number, between 1 and 31, representing the day of the month for the given date according to local time.
|
| **See\:** ECMA-262, 21.4.4.2
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getDay()\: :kw:`number`

| Returns the day of the week for the specified date according to local time, where 0 represents Sunday. For the day of the month, see ``getDayOfMonth`` .
|
| **Returns\:** An integer number, between 0 and 6, corresponding to the day of the week for the given date, according to local time\: 0 for Sunday, 1 for Monday, 2 for Tuesday, and so on.
|
| **See\:** ECMA-262, 21.4.4.3
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getTime()\: :kw:`number` :kw:`throws`

| Returns the number of milliseconds since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the milliseconds elapsed between 1 January 1970 00\:00\:00 UTC and the given date.
|
| **See\:** ECMA-262, 21.4.4.10
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getTimezoneOffset()\: :kw:`number`

| Returns the difference, in minutes, between a date as evaluated in the UTC time zone, and the same date as evaluated in the local time zone.
|
| **Returns\:** the difference, in minutes, between a date as evaluated in the UTC time zone, and the same date as evaluated in the local time zone.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCDate()\: :kw:`number`

| Returns the day of the month (from 1 to 31) in the specified date according to universal time.
|
| **Returns\:** An integer number, between 1 and 31, representing the day of the month for the given date according to local time.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCDay()\: :kw:`number`

| Returns the day of the week in the specified date according to universal time, where 0 represents Sunday.
|
| **Returns\:** An integer number, between 0 and 6, corresponding to the day of the week for the given date, according to local time\: 0 for Sunday, 1 for Monday, 2 for Tuesday, and so on.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCFullYear()\: :kw:`number`

| Returns the year of the specified date according to local time.
|
| **Description\:** The value returned by \`getUTCFullYear()\` is an absolute number. For dates between the years 1000 and 9999, \`getUTCFullYear()\` returns a four-digit number, for example, 1995. Use this function to make sure a year is compliant with years after 2000.
|
| **Returns\:** A year of the specified date according to local time. year
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCHours()\: :kw:`number`

| Returns the hours in the specified date according to universal time.
|
| **Returns\:** An integer number, between 0 and 23, representing the hour for the given date according to UTC time.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCMilliseconds()\: :kw:`number`

| Returns the milliseconds portion of the time object's value according to universal time.
|
| **Returns\:** the milliseconds portion of the time object's value according to universal time.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCMinutes()\: :kw:`number`

| Returns the minutes in the specified date according to universal time.
|
| **Returns\:** the minutes in the specified date according to universal time.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCMonth()\: :kw:`number`

| Returns the month of the specified date according to universal time, as a zero-based value (where zero indicates the first month of the year).
|
| **Returns\:** An integer number, between 0 and 11, representing the month in the given date according to UTC time. 0 corresponds to January, 1 to February, and so on.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getUTCSeconds()\: :kw:`number`

| Returns the seconds in the specified date according to universal time.
|
| **Returns\:** the seconds in the specified date according to universal time.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  getYear()\: :kw:`number`

| Returns the year of the specified date according to local time.
|
| **Returns\:** year
|
| **See\:** ECMA-262, 21.4.4.4 deprecated
|
| **Note\:** This function is an alias to ``getFullYear`` and left for compatibility with ECMA-262.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  isDateValid()\: :kw:`boolean`

| The \`isDateValid()\` method checks if constructed date is maximum of ±100,000,000 (one hundred million) days relative to January 1, 1970 UTC (that is, April 20, 271821 BCE ~ September 13, 275760 CE) can be represented by the standard Date object (equivalent to ±8,640,000,000,000,000 milliseconds).
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  setDate(value\: :kw:`number`)\: :kw:`void`

| Changes the day of the month of a given Date instance, based on local time.
|
| **Arguments\:**

 - value\: :kw:`number`  new day.

------


.. rst-class:: doc-code-block

  :kw:`public`
  setDay(value\: :kw:`number`)\: :kw:`void`

| Alias to ``setDate`` and left for compatibility with ECMA-262.
|
| **Arguments\:**

 - value\: :kw:`number`  new day.

------


.. rst-class:: doc-code-block

  :kw:`public`
  setFullYear(value\: :kw:`number`)\: :kw:`void`

| Sets the full year for a specified date according to local time.
|
| **Arguments\:**

 - value\: :kw:`number`  new year

------


.. rst-class:: doc-code-block

  :kw:`public`
  setHours(value\: :kw:`number`)\: :kw:`void`

| Sets the hours for a specified date according to local time.
|
| **Arguments\:**

 - value\: :kw:`number`  new hours

------


.. rst-class:: doc-code-block

  :kw:`public`
  setMilliseconds(value\: :kw:`number`)\: :kw:`void`

| Sets the milliseconds for a specified date according to local time.
|
| **Arguments\:**

 - value\: :kw:`number`  new ms

------


.. rst-class:: doc-code-block

  :kw:`public`
  setMinutes(value\: :kw:`number`)\: :kw:`void`

| Sets the minutes for a specified date according to local time.
|
| **Arguments\:**

 - value\: :kw:`number`  new minutes

------


.. rst-class:: doc-code-block

  :kw:`public`
  setMonth(month\: :kw:`number`)\: :kw:`void`

| Sets the month for a specified date according to the currently set year.
|
| **Arguments\:**

 - month\: :kw:`number`  new month

------


.. rst-class:: doc-code-block

  :kw:`public`
  setSeconds(value\: :kw:`number`)\: :kw:`void`

| Sets the seconds for a specified date according to local time.
|
| **Arguments\:**

 - value\: :kw:`number`  new seconds

------


.. rst-class:: doc-code-block

  :kw:`public`
  setTime(value\: :kw:`number`)\: :kw:`void`

| Sets the number of milliseconds since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the milliseconds elapsed between 1 January 1970 00\:00\:00 UTC and the given date.
|
| **Arguments\:**

 - value\: :kw:`number`  new ms

| **See\:** ECMA-262, 21.4.4.10
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  setTimezoneOffset(value\: :kw:`number`)\: :kw:`number`

| Sets the difference, in minutes, between a date as evaluated in the UTC time zone, and the same date as evaluated in the local time zone.
|
| **Arguments\:**

 - value\: :kw:`number`  new timezone offset

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCDate(value\: :kw:`number`)\: :kw:`void`

| Changes the day of the month of a given Date instance, based on UTC time.
|
| **Arguments\:**

 - value\: :kw:`number`  new day.

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCDay(value\: :kw:`number`)\: :kw:`void`

| Changes the day of the month of a given Date instance, based on UTC time.
|
| **Arguments\:**

 - value\: :kw:`number`  new day.

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCFullYear(value\: :kw:`number`)\: :kw:`void`

| Sets the full year for a specified date according to universal time.
|
| **Arguments\:**

 - value\: :kw:`number`  new year

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCHours(value\: :kw:`number`)\: :kw:`void`

| Sets the hour for a specified date according to universal time.
|
| **Arguments\:**

 - value\: :kw:`number`  new hours

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCMilliseconds(value\: :kw:`number`)\: :kw:`void`

| Sets the milliseconds for a specified date according to universal time.
|
| **Arguments\:**

 - value\: :kw:`number`  new ms

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCMinutes(value\: :kw:`number`)\: :kw:`void`

| Sets the minutes for a specified date according to universal time.
|
| **Arguments\:**

 - value\: :kw:`number`  new minutes

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCMonth(month\: :kw:`number`)\: :kw:`void`

| Sets the month for a specified date according to universal time.
|
| **Arguments\:**

 - month\: :kw:`number`  new month

------


.. rst-class:: doc-code-block

  :kw:`public`
  setUTCSeconds(value\: :kw:`number`)\: :kw:`void`

| Sets the seconds for a specified date according to universal time.
|
| **Arguments\:**

 - value\: :kw:`number`  new seconds

------


.. rst-class:: doc-code-block

  :kw:`public`
  setYear(value\: :kw:`number`)\: :kw:`void`

| This function is an alias to ``setFullYear`` and left for compatibility with ECMA-262.
|
| **Arguments\:**

 - value\: :kw:`number`  new year

------


.. rst-class:: doc-code-block

  :kw:`public`
  toJSON()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Date object.
|
| **Returns\:** JSON representation of the current instance
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleDatestring()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Gets a string with a language-sensitive representation of the date portion of the specified date in the user agent's timezone.
|
| **Returns\:** a string with a language-sensitive representation of the date portion of the specified date in the user agent's timezone.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleDatestring(locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string with a language-sensitive representation of the date portion of the specified date in the user agent's timezone.
|
| **Returns\:** a string with a language-sensitive representation of the date portion of the specified date in the user agent's timezone.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Gets a string with a language-sensitive representation of this date.
|
| **Returns\:** a language-sensitive representation of this date.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Gets a string with a language-sensitive representation of this date with respect to locale.
|
| **Returns\:** a language-sensitive representation of this date.
|
| **Arguments\:**

 - locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleTimestring()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Gets a string with a language-sensitive representation of the time portion of the date.
|
| **Returns\:** a language-sensitive representation of the time portion of the date.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleTimestring(locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Gets a string with a language-sensitive representation of the time portion of the date with respect to locale.
|
| **Returns\:** a language-sensitive representation of the time portion of the date with respect to locale.
|
| **Arguments\:**

 - locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  valueOf()\: :kw:`number` :kw:`throws`

| The \`valueOf()\` method returns the primitive value of a \`Date\` object.
|
| **Returns\:** The number of milliseconds between 1 January 1970 00\:00\:00 UTC and the given date.
| throws InvalidDate - Throws if Date object is invalid (``isDateValid`` is \`false\`).
|
| **See\:** ECMA-262, 21.4.4.44
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(d\: :ref:`Date<LeLsLcLoLmLpLaLt.UDLaLtLe>`)\: :kw:`number` :kw:`throws`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - d\: :ref:`Date<LeLsLcLoLmLpLaLt.UDLaLtLe>`  to be converted to milliseconds.

| **See\:** ECMA-262, 21.4.3.1
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(year\: :kw:`number`)\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(year\: :kw:`number`, month\: :kw:`number`)\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.
 - minutes\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.
 - minutes\: :kw:`number`  to be converted to milliseconds.
 - seconds\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`,
  |nbsp| ms\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.
 - minutes\: :kw:`number`  to be converted to milliseconds.
 - seconds\: :kw:`number`  to be converted to milliseconds.
 - ms\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(year\: :kw:`number`)\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(year\: :kw:`number`, month\: :kw:`number`)\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.
 - minutes\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.
 - minutes\: :kw:`number`  to be converted to milliseconds.
 - seconds\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  UTC(
  |nbsp| year\: :kw:`number`,
  |nbsp| month\: :kw:`number`,
  |nbsp| day\: :kw:`number`,
  |nbsp| hours\: :kw:`number`,
  |nbsp| minutes\: :kw:`number`,
  |nbsp| seconds\: :kw:`number`,
  |nbsp| ms\: :kw:`number`
  )\: :kw:`number`

| Returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Arguments\:**

 - year\: :kw:`number`  to be converted to milliseconds.
 - month\: :kw:`number`  to be converted to milliseconds.
 - day\: :kw:`number`  to be converted to milliseconds.
 - hours\: :kw:`number`  to be converted to milliseconds.
 - minutes\: :kw:`number`  to be converted to milliseconds.
 - seconds\: :kw:`number`  to be converted to milliseconds.
 - ms\: :kw:`number`  to be converted to milliseconds.

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  getLocalTimezoneOffset()\: :kw:`number`

| Gets local time offset.
|
| **Returns\:** local time offset.
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  getLocalestring(
  |nbsp| format\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`,
  |nbsp| locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`,
  |nbsp| ms\: :kw:`number`,
  |nbsp| isUTC\: :kw:`boolean`
  )\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Gets locale string representation according to format.
|
| **Returns\:** locale string in the specified format.
|
| **Arguments\:**

 - format\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - ms\: :kw:`number`
 - isUTC\: :kw:`boolean`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  getTimezoneName()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Gets time zone name.
|
| **Returns\:** time zone name.
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  now()\: :kw:`number`

| The \`now()\` static method returns the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **Returns\:** A number representing the number of milliseconds elapsed since the epoch, which is defined as the midnight at the beginning of January 1, 1970, UTC.
|
| **See\:** ECMA-262, 21.4.3.1
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  parse(dateStr\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`number` :kw:`throws`

| Parses a string representation of a date, and returns the number of milliseconds since January 1, 1970, 00\:00\:00 UTC or raises \`InvalidDate\` if the string is unrecognized or, in some cases, contains illegal date values (e.g. 2015-02-31).
| Only the `ISO 8601 format <https://tc39.es/ecma262/#sec-date-time-string-format>`_ (YYYY-MM-DDTHH\:mm\:ss.sssZ) is explicitly specified to be supported. Other formats are implementation-defined and may not work across all browsers (targets). A library can help if many different formats are to be accommodated.
|
| **Returns\:** A number representing the milliseconds elapsed since January 1, 1970, 00\:00\:00 UTC and the date obtained by parsing the given string representation of a date. If the argument doesn't represent a valid date, \`InvalidDate\` exception is thrown.
|
| **Arguments\:**

 - dateStr\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  to be parsed

| **See\:** ECMA-262, 21.4.3.2
|

------


.. _LeLsLcLoLmLpLaLt.UELrLrLoLr:

Error
=====



.. rst-class:: doc-code-block

  :kw:`export`

| Strores information about stacktrace and cause in case of an error. Serves as a base class for all error classes.
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new empty error instance
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(msg\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new error instance with provided message
|
| **Arguments\:**

 - msg\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  message of the error

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(msg\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :kw:`void`

| Constructs a new error instance with provided message and cause
|
| **Arguments\:**

 - msg\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  message of the error
 - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  cause of the error

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts this error to a string Result includes error message and the stacktrace
|
| **Returns\:** result of the conversion
|

------

Properties
----------

.. rst-class:: doc-code-block

  - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  - message\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - name\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - stack\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`


.. _LeLsLcLoLmLpLaLt.UELvLaLlUELrLrLoLr:

EvalError
=========



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`Error<LeLsLcLoLmLpLaLt.UELrLrLoLr>`

| **Class\:** Represents an error that occurs when global eval() function fails
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new instance of error
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------

Properties
----------

.. rst-class:: doc-code-block

  - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  - message\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - name\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - stack\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`



.. _LeLsLcLoLmLpLaLt.UFLiLnLaLlLiLzLaLtLiLoLnURLeLgLiLsLtLrLy:

FinalizationRegistry<T>
=======================


.. rst-class:: doc-code-block

  :kw:`export`

| **Interface\:** Represents a FinalizationRegistry
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  register(target: :ref:`WeakKey<LeLsLcLoLmLpLaLt.UWLeLaLkUKLeLy>`, heldValue: T, unregisterToken?: :ref:`WeakKey<LeLsLcLoLmLpLaLt.UWLeLaLkUKLeLy>`): void;

|

.. rst-class:: doc-code-block

  :kw:`public`
  unregister(unregisterToken: :ref:`WeakKey<LeLsLcLoLmLpLaLt.UWLeLaLkUKLeLy>`)\: :kw:`void`

|

------

.. _LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy:

Float32Array
============



.. rst-class:: doc-code-block

  :kw:`export`

| JS Float32Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Float32Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Float32Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type float in newly created Float32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Float32Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Float32Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Float32Array.
|
| **Arguments\:**

 - other\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Float32Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Float32Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Float32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Float32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Float32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Fills the Float32Array with specified value
|
| **Returns\:** modified Float32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Fills the Float32Array with specified value
|
| **Returns\:** modified Float32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Fills the Float32Array with specified value
|
| **Returns\:** modified Float32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| creates a new Float32Array from current Float32Array based on a condition fn
|
| **Returns\:** a new Float32Array with elements from current Float32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a new Float32Array from current Float32Array based on a condition fn.
|
| **Returns\:** a new Float32Array with elements from current Float32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| creates a new Float32Array from current Float32Array based on a condition fn
|
| **Returns\:** a new Float32Array with elements from current Float32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Float32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Float32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return float | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Float32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Float32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Float32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Float32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Float32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Float32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Float32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Float32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Float32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Float32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Float32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Float32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Float32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates an Float32Array from array-like argument
|
| **Returns\:** new Float32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Float32Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates an Float32Array from array-like argument
|
| **Returns\:** new Float32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Float32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates an Float32Array from array-like argument
|
| **Returns\:** new Float32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Float32Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Float32Array
|
| **Returns\:** true if e is in Float32Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Float32Array
|
| **Returns\:** true if e is in Float32Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns keys of the Float32Array
|
| **Returns\:** iterator over keys
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a new Float32Array using fn(arr\[i\]) over all elements of current Float32Array
|
| **Returns\:** a new Float32Array where for each element from current Float32Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Float32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a new Float32Array using fn(arr\[i\]) over all elements of current Float32Array.
|
| **Returns\:** a new Float32Array where for each element from current Float32Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Float32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a new Float32Array using initializer
|
| **Returns\:** a new Float32Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a new Float32Array using reversed data from the current one
|
| **Returns\:** a new Float32Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(float\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Float32Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Float32Array.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a slice of current Float32Array using range \[begin, end)
|
| **Returns\:** a new Float32Array with elements of current Float32Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a slice of current Float32Array using range \[begin, this.length).
|
| **Returns\:** a new Float32Array with elements of current Float32Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a slice of current Float32 with all elements.
|
| **Returns\:** a new Float32Array with elements of current Float32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Float32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Float32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Float32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Float32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Float32Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a Float32Array with the same underlying ArrayBuffer
|
| **Returns\:** new Float32Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a Float32Array with the same ArrayBuffer
|
| **Returns\:** new Float32Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a Float32Array with the same ArrayBuffer
|
| **Returns\:** new Float32Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Float32Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Float32Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Float32Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Float32Array
|
| **Returns\:** a string representation of the Float32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns array values iterator
|
| **Returns\:** an iterator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Float32Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU3U2UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Float32Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`




.. _LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy:

Float64Array
============



.. rst-class:: doc-code-block

  :kw:`export`

| JS Float64Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Float64Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Float64Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type number in newly created Float64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Float64Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Float64Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Float64Array.
|
| **Arguments\:**

 - other\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Float64Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Float64Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Float64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Float64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Float64Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Fills the Float64Array with specified value
|
| **Returns\:** modified Float64Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Fills the Float64Array with specified value
|
| **Returns\:** modified Float64Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Fills the Float64Array with specified value
|
| **Returns\:** modified Float64Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| creates a new Float64Array from current Float64Array based on a condition fn
|
| **Returns\:** a new Float64Array with elements from current Float64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a new Float64Array from current Float64Array based on a condition fn.
|
| **Returns\:** a new Float64Array with elements from current Float64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| creates a new Float64Array from current Float64Array based on a condition fn
|
| **Returns\:** a new Float64Array with elements from current Float64Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Float64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Float64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Float64Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Float64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Float64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Float64Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Float64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Float64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Float64Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Float64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Float64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Float64Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Float64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Float64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Float64Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates an Float64Array from array-like argument
|
| **Returns\:** new Float64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Float64Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates an Float64Array from array-like argument
|
| **Returns\:** new Float64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Float64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates an Float64Array from array-like argument
|
| **Returns\:** new Float64Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Float64Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Float64Array
|
| **Returns\:** true if e is in Float64Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Float64Array
|
| **Returns\:** true if e is in Float64Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns keys of the Float64Array
|
| **Returns\:** iterator over keys
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a new Float64Array using fn(arr\[i\]) over all elements of current Float64Array
|
| **Returns\:** a new Float64Array where for each element from current Float64Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Float64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a new Float64Array using fn(arr\[i\]) over all elements of current Float64Array.
|
| **Returns\:** a new Float64Array where for each element from current Float64Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Float64Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: ::kw:`number`\[\])\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a new Float64Array using initializer
|
| **Returns\:** a new Float64Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a new Float64Array using reversed data from the current one
|
| **Returns\:** a new Float64Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(number\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Float64Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a slice of current Float64Array using range \[begin, end)
|
| **Returns\:** a new Float64Array with elements of current Float64Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a slice of current Float64Array using range \[begin, this.length).
|
| **Returns\:** a new Float64Array with elements of current Float64Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a slice of current Float64 with all elements.
|
| **Returns\:** a new Float64Array with elements of current Float64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Float64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Float64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Float64Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Float64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Float64Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a Float64Array with the same underlying ArrayBuffer
|
| **Returns\:** new Float64Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a Float64Array with the same ArrayBuffer
|
| **Returns\:** new Float64Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a Float64Array with the same ArrayBuffer
|
| **Returns\:** new Float64Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Float64Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Float64Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Float64Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Float64Array
|
| **Returns\:** a string representation of the Float64Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns array values iterator
|
| **Returns\:** an iterator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Float64Array<LeLsLcLoLmLpLaLt.UFLlLoLaLtU6U4UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Float64Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy:

Int16Array
==========



.. rst-class:: doc-code-block

  :kw:`export`

| JS Int16Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Int16Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Int16Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type short in newly created Int16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Int16Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Int16Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Int16Array.
|
| **Arguments\:**

 - other\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Int16Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Int16Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Fills the Int16Array with specified value
|
| **Returns\:** modified Int16Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Fills the Int16Array with specified value
|
| **Returns\:** modified Int16Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Fills the Int16Array with specified value
|
| **Returns\:** modified Int16Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| creates a new Int16Array from current Int16Array based on a condition fn
|
| **Returns\:** a new Int16Array with elements from current Int16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a new Int16Array from current Int16Array based on a condition fn.
|
| **Returns\:** a new Int16Array with elements from current Int16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| creates a new Int16Array from current Int16Array based on a condition fn
|
| **Returns\:** a new Int16Array with elements from current Int16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return short | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates an Int16Array from array-like argument
|
| **Returns\:** new Int16Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int16Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates an Int16Array from array-like argument
|
| **Returns\:** new Int16Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates an Int16Array from array-like argument
|
| **Returns\:** new Int16Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int16Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Int16Array
|
| **Returns\:** true if e is in Int16Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Int16Array
|
| **Returns\:** true if e is in Int16Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns keys of the Int16Array
|
| **Returns\:** iterator over keys
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a new Int16Array using fn(arr\[i\]) over all elements of current Int16Array
|
| **Returns\:** a new Int16Array where for each element from current Int16Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Int16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a new Int16Array using fn(arr\[i\]) over all elements of current Int16Array.
|
| **Returns\:** a new Int16Array where for each element from current Int16Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Int16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a new Int16Array using initializer
|
| **Returns\:** a new Int16Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a new Int16Array using reversed data from the current one
|
| **Returns\:** a new Int16Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(short\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Int16Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Int16Array.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a slice of current Int16Array using range \[begin, end)
|
| **Returns\:** a new Int16Array with elements of current Int16Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a slice of current Int16Array using range \[begin, this.length).
|
| **Returns\:** a new Int16Array with elements of current Int16Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a slice of current Int16 with all elements.
|
| **Returns\:** a new Int16Array with elements of current Int16Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Int16Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Int16Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a Int16Array with the same underlying ArrayBuffer
|
| **Returns\:** new Int16Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a Int16Array with the same ArrayBuffer
|
| **Returns\:** new Int16Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a Int16Array with the same ArrayBuffer
|
| **Returns\:** new Int16Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int16Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int16Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int16Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Int16Array
|
| **Returns\:** a string representation of the Int16Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns array values iterator
|
| **Returns\:** an iterator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Int16Array<LeLsLcLoLmLpLaLt.UILnLtU1U6UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Int16Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy:

Int32Array
==========



.. rst-class:: doc-code-block

  :kw:`export`

| JS Int32Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Int32Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Int32Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type int in newly created Int32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Int32Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Int32Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Int32Array.
|
| **Arguments\:**

 - other\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Int32Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Int32Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Fills the Int32Array with specified value
|
| **Returns\:** modified Int32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Fills the Int32Array with specified value
|
| **Returns\:** modified Int32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Fills the Int32Array with specified value
|
| **Returns\:** modified Int32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| creates a new Int32Array from current Int32Array based on a condition fn
|
| **Returns\:** a new Int32Array with elements from current Int32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a new Int32Array from current Int32Array based on a condition fn.
|
| **Returns\:** a new Int32Array with elements from current Int32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| creates a new Int32Array from current Int32Array based on a condition fn
|
| **Returns\:** a new Int32Array with elements from current Int32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return int | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates an Int32Array from array-like argument
|
| **Returns\:** new Int32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int32Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates an Int32Array from array-like argument
|
| **Returns\:** new Int32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates an Int32Array from array-like argument
|
| **Returns\:** new Int32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int32Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Int32Array
|
| **Returns\:** true if e is in Int32Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Int32Array
|
| **Returns\:** true if e is in Int32Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns keys of the Int32Array
|
| **Returns\:** iterator over keys
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a new Int32Array using fn(arr\[i\]) over all elements of current Int32Array
|
| **Returns\:** a new Int32Array where for each element from current Int32Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Int32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a new Int32Array using fn(arr\[i\]) over all elements of current Int32Array.
|
| **Returns\:** a new Int32Array where for each element from current Int32Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Int32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a new Int32Array using initializer
|
| **Returns\:** a new Int32Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a new Int32Array using reversed data from the current one
|
| **Returns\:** a new Int32Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(int\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Int32Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Int32Array.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a slice of current Int32Array using range \[begin, end)
|
| **Returns\:** a new Int32Array with elements of current Int32Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a slice of current Int32Array using range \[begin, this.length).
|
| **Returns\:** a new Int32Array with elements of current Int32Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a slice of current Int32 with all elements.
|
| **Returns\:** a new Int32Array with elements of current Int32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Int32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Int32Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a Int32Array with the same underlying ArrayBuffer
|
| **Returns\:** new Int32Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a Int32Array with the same ArrayBuffer
|
| **Returns\:** new Int32Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a Int32Array with the same ArrayBuffer
|
| **Returns\:** new Int32Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int32Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int32Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int32Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Int32Array
|
| **Returns\:** a string representation of the Int32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns array values iterator
|
| **Returns\:** an iterator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Int32Array<LeLsLcLoLmLpLaLt.UILnLtU3U2UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Int32Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy:

Int8Array
=========



.. rst-class:: doc-code-block

  :kw:`export`

| JS Int8Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Int8Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Int8Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type byte in newly created Int8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Int8Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Int8Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Int8Array.
|
| **Arguments\:**

 - other\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Int8Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Int8Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Int8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Fills the Int8Array with specified value
|
| **Returns\:** modified Int8Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Fills the Int8Array with specified value
|
| **Returns\:** modified Int8Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Fills the Int8Array with specified value
|
| **Returns\:** modified Int8Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| creates a new Int8Array from current Int8Array based on a condition fn
|
| **Returns\:** a new Int8Array with elements from current Int8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a new Int8Array from current Int8Array based on a condition fn.
|
| **Returns\:** a new Int8Array with elements from current Int8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| creates a new Int8Array from current Int8Array based on a condition fn
|
| **Returns\:** a new Int8Array with elements from current Int8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return byte | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Int8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Int8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Int8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Int8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Int8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates an Int8Array from array-like argument
|
| **Returns\:** new Int8Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int8Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates an Int8Array from array-like argument
|
| **Returns\:** new Int8Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates an Int8Array from array-like argument
|
| **Returns\:** new Int8Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Int8Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Int8Array
|
| **Returns\:** true if e is in Int8Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Int8Array
|
| **Returns\:** true if e is in Int8Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns keys of the Int8Array
|
| **Returns\:** iterator over keys
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a new Int8Array using fn(arr\[i\]) over all elements of current Int8Array
|
| **Returns\:** a new Int8Array where for each element from current Int8Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Int8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a new Int8Array using fn(arr\[i\]) over all elements of current Int8Array.
|
| **Returns\:** a new Int8Array where for each element from current Int8Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Int8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a new Int8Array using initializer
|
| **Returns\:** a new Int8Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a new Int8Array using reversed data from the current one
|
| **Returns\:** a new Int8Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(byte\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Int8Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Int8Array.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a slice of current Int8Array using range \[begin, end)
|
| **Returns\:** a new Int8Array with elements of current Int8Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a slice of current Int8Array using range \[begin, this.length).
|
| **Returns\:** a new Int8Array with elements of current Int8Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a slice of current Int8 with all elements.
|
| **Returns\:** a new Int8Array with elements of current Int8Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Int8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Int8Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Int8Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a Int8Array with the same underlying ArrayBuffer
|
| **Returns\:** new Int8Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a Int8Array with the same ArrayBuffer
|
| **Returns\:** new Int8Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a Int8Array with the same ArrayBuffer
|
| **Returns\:** new Int8Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int8Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int8Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Int8Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Int8Array
|
| **Returns\:** a string representation of the Int8Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<:kw:`number`\>

| Returns array values iterator
|
| **Returns\:** an iterator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Int8Array<LeLsLcLoLmLpLaLt.UILnLtU8UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Int8Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr:

IterableIterator\<T\>
=============================


.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`Iterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLr>`\<T\>

| IterableIterator
|

Methods
-------

.. rst-class:: doc-code-block

  :kw:`public`
  next(args: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`\[\]): :ref:`IteratorResult<LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLrURLeLsLuLlLt>`

| next
|
| **Arguments\:**
| args: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`\[\]
|
| **Returns**
| IteratorResult


------

.. rst-class:: doc-code-block

  :kw:`public`
  return(value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`): :ref:`IteratorResult<LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLrURLeLsLuLlLt>`

| return
|
| **Arguments\:**
| value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
|
| **Returns**
| IteratorResult


------

.. rst-class:: doc-code-block

  :kw:`public`
  throw(value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`): :ref:`IteratorResult<LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLrURLeLsLuLlLt>`

| throw
|
| **Arguments\:**
| value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
|
| **Returns**
| IteratorResult


------

.. _LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLr:

Iterator\<T\, TReturn, TNext>
=============================


.. rst-class:: doc-code-block

  :kw:`export`

| Iterator
|

Methods
-------

.. rst-class:: doc-code-block

  :kw:`public`
  next(args: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`\[\]): :ref:`IteratorResult<LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLrURLeLsLuLlLt>`

| next
|
| **Arguments\:**
| args: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`\[\]
|
| **Returns**
| IteratorResult


------

.. rst-class:: doc-code-block

  :kw:`public`
  return(value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`): :ref:`IteratorResult<LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLrURLeLsLuLlLt>`

| return
|
| **Arguments\:**
| value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
|
| **Returns**
| IteratorResult


------

.. rst-class:: doc-code-block

  :kw:`public`
  throw(value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`): :ref:`IteratorResult<LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLrURLeLsLuLlLt>`

| throw
|
| **Arguments\:**
| value: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
|
| **Returns**
| IteratorResult


------


.. _LeLsLcLoLmLpLaLt.UILtLeLrLaLtLoLrURLeLsLuLlLt:

IteratorResult\<V\>
===================


.. rst-class:: doc-code-block

  :kw:`export`

| IteratorResult
|


Properties
----------


.. rst-class:: doc-code-block

  - done\: :kw:`boolean` | :kw:`undefined`
  - value\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------



.. _LeLsLcLoLmLpLaLt.UJUSUOUN:

JSON
====



.. rst-class:: doc-code-block

  :kw:`export`

| JSON class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts byte to JSON format
|
| **Returns\:** string - JSON representation of byte
|
| **Arguments\:**

 - d\: :kw:`number`  \: :kw:`number` - byte to be converted to a JSON as a number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts short to JSON format
|
| **Returns\:** string - JSON representation of short
|
| **Arguments\:**

 - d\: :kw:`number`  \: :kw:`number` - short to be converted to a JSON as a number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts int to JSON format
|
| **Returns\:** string - JSON representation of int
|
| **Arguments\:**

 - d\: :kw:`number`  \: :kw:`number` - int to be converted to a JSON as a number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts long to JSON format
|
| **Returns\:** string - JSON representation of long
|
| **Arguments\:**

 - d\: :kw:`number`  \: :kw:`number` - long to be converted to a JSON as a number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts float to JSON format
|
| **Returns\:** string - JSON representation of float
|
| **Arguments\:**

 - d\: :kw:`number`  \: :kw:`number` - float to be converted to a JSON as a number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts number to JSON format
|
| **Returns\:** string - JSON representation of number
|
| **Arguments\:**

 - d\: :kw:`number`  \: :kw:`number` - number to be converted to a JSON as a number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`char`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts char to JSON format
|
| **Returns\:** string - JSON representation of char
|
| **Arguments\:**

 - d\: :kw:`char`  \: char - char to be converted to a JSON as a string

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`boolean`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts boolean to JSON format
|
| **Returns\:** string - JSON representation of boolean
|
| **Arguments\:**

 - d\: :kw:`boolean`  \: boolean - boolean to be converted to a JSON as a Boolean literal

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts string to JSON format
|
| **Returns\:** string - JSON representation of byte
|
| **Arguments\:**

 - d\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  \: string - byte to be converted to a JSON as a string

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts object to JSON format
|
| **Returns\:** string - JSON representation of object
|
| **Arguments\:**

 - d\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  \: object - byte to be converted to a JSON as an object

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts bytes array to JSON format
|
| **Returns\:** string - JSON representation of bytes array
|
| **Arguments\:**

 - d\: :kw:`number`\[\]  \: :kw:`number`\[\] - bytes array to be converted to a JSON as an Array of numbers

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts shorts array to JSON format
|
| **Returns\:** string - JSON representation of shorts array
|
| **Arguments\:**

 - d\: :kw:`number`\[\]  \: :kw:`number`\[\] - shorts array to be converted to a JSON as an Array of numbers

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts ints array to JSON format
|
| **Returns\:** string - JSON representation of ints array
|
| **Arguments\:**

 - d\: :kw:`number`\[\]  \: :kw:`number`\[\] - ints array to be converted to a JSON as an Array of numbers

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts longs array to JSON format
|
| **Returns\:** string - JSON representation of longs array
|
| **Arguments\:**

 - d\: :kw:`number`\[\]  \: :kw:`number`\[\] - longs array to be converted to a JSON as an Array of numbers

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts array of bytes to JSON format
|
| **Returns\:** string - JSON representation of array of bytes
|
| **Arguments\:**

 - d\: :kw:`number`\[\]  \: :kw:`number`\[\] - array of byte to be converted to a JSON as an Array of numbers

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`number`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts numbers array to JSON format
|
| **Returns\:** string - JSON representation of numbers array
|
| **Arguments\:**

 - d\: :kw:`number`\[\]  \: :kw:`number`\[\] - numbers array to be converted to a JSON as an Array of numbers

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`char`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts chars array to JSON format
|
| **Returns\:** string - JSON representation of chars array
|
| **Arguments\:**

 - d\: :kw:`char`\[\]  \: char\[\] - chars array to be converted to a JSON as an Array of numbers

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  stringify(d\: :kw:`boolean`\[\])\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts booleans array to JSON format
|
| **Returns\:** string - JSON representation of booleans array
|
| **Arguments\:**

 - d\: :kw:`boolean`\[\]  \: boolean\[\] - booleans array to be converted to a JSON as an Array of Boolean literals

------


.. _LeLsLcLoLmLpLaLt.UMLaLp:

Map\<K, V\>
===========



.. rst-class:: doc-code-block

  :kw:`export`

| **Class\:** Map
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  clear()\: :kw:`void`

| Deletes all elements from the Map
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs an empty Map
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  delete(k\: K)\: :kw:`void`

| Removes an Entry with specified key from the Map
|
| **Arguments\:**

 - k\: K  the key to remove

------


.. rst-class:: doc-code-block

  :kw:`public`
  entries()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`

| Returns elements from the Map as an array of Entries. TODO: return type is incorrect
|
| **Returns\:** an array of Entries
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (v\: V, k\: K) =\> :kw:`void`
  )\: :kw:`void`

| Applies a function over all elements of the Map
|
| **Arguments\:**

 - fn\: (v\: V, k\: K) =\> :kw:`void`  to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (v\: V) =\> :kw:`void`
  )\: :kw:`void`

| Applies a function over all elements of the Map
|
| **Arguments\:**

 - fn\: (v\: V) =\> :kw:`void`  to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  get(k\: K)\: V | undefined

| Returns a value assosiated with key if present
|
| **Returns\:** value if assosiated with key presents.
|
| **Arguments\:**

 - k\: K  the key to find in the Map

------


.. rst-class:: doc-code-block

  :kw:`public`
  has(k\: K)\: :kw:`boolean`

| Checks if a key is in the Map
|
| **Returns\:** true if the value is in the Map
|
| **Arguments\:**

 - k\: K  the key to find in the Map

------


.. rst-class:: doc-code-block

  :kw:`public`
  keys()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<K\>

| Returns elements from the Map as an keys Iterator. TODO: return type is incorrect
|
| **Returns\:** ValueIterator with map keys
|

------

.. rst-class:: doc-code-block

  :kw:`public`
  set(k\: K, v\: V)\: :kw:`void`

| Puts a pair of key and value into the Map
|
| **Arguments\:**

 - k\: K  the key to put into the Map
 - v\: V  the value to put into the Map

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<V\>

| Returns elements from the Map as an values Iterator. TODO: return type is incorrect
|
| **Returns\:** ValueIterator with map values
|

------


Properties
----------

.. rst-class:: doc-code-block

  - size\: :kw:`number`



.. _LeLsLcLoLmLpLaLt.UMLaLtLh:

Math
====



.. rst-class:: doc-code-block

  :kw:`export`

| **Class\:** The Math class contains static properties and methods for mathematical constants and functions.
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  asin(x\: :kw:`number`)\: :kw:`number`

| Arcsine of angle \`v\`
|
| **Returns\:** Arcsine of angle \`v\`
|
| **Arguments\:**

 - x\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  atan2(y\: :kw:`number`, x\: :kw:`number`)\: :kw:`number`

| Method returns the angle in the plane (in radians) between the positive x-axis and the ray from (0, 0) to the point (x, y), for Math.atan2(y, x).
|
| **Returns\:** The angle in radians (between -π and π, inclusive) between the positive x-axis and the ray from (0, 0) to the point (x, y).
|
| **Remark\:** The atan2() method measures the counterclockwise angle θ, in radians, between the positive x-axis and the point (x, y). Note that the arguments to this function pass the y-coordinate first and the x-coordinate second.
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  cbrt(x\: :kw:`number`)\: :kw:`number`

| Cube root of a number.
|
| **Arguments\:**

 - x\: :kw:`number`  arbitrary number

| **Remark\:** Math.𝚌brt(𝚡) = ∛x = the unique y such that y³ = x.
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  fround(x\: :kw:`number`)\: :kw:`number`

| \"fround\" returns nearest 32-bit single fp representation of a number in a 64-bit form
| Math.fround(1.337) == 1.337 // false, result would be 1.3370000123977661 Math.fround(1.5) == 1.5 // true Math.fround(-5.05) == -5.05 //false, result would be -5.050000190734863 Math.fround(Infinity) // Infinity Math.fround(NaN) // NaN
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  hypot(u\: :kw:`number`, v\: :kw:`number`)\: :kw:`number`

| Square root of the sum of squares of \`v\` and \`u\`
|
| **Returns\:** The square root of the sum of squares of its arguments
|
| **Arguments\:**

 - u\: :kw:`number`  arbitrary number
 - v\: :kw:`number`  arbitrary number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  imul(a\: :kw:`number`, b\: :kw:`number`)\: :kw:`number`

| Method returns the result of the C-like 32-bit manipulation of the two parameters
|
| **Returns\:** (a \* b) % 2 \*\* 32
| Math.imul(Infinity, 1) = 0 Math.imul(NaN, 1) = 0 Math.imul(2.5, 2.5) = 4 Math.imul(-5, 5.05) = 25
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  max(u\: :kw:`number`, v\: :kw:`number`)\: :kw:`number`

| Largest value of \`u\` and \`v\`
|
| **Returns\:** Largest value of \`u\` and \`v\`
|
| **Arguments\:**

 - u\: :kw:`number`  arbitrary number
 - v\: :kw:`number`  arbitrary number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  min(u\: :kw:`number`, v\: :kw:`number`)\: :kw:`number`

| Smallest value of \`u\` and \`v\`
|
| **Returns\:** Smallest value of \`u\` and \`v\`
|
| **Arguments\:**

 - u\: :kw:`number`  arbitrary number
 - v\: :kw:`number`  arbitrary number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  random()\: :kw:`number`

| Pseudo-random number in the range \[0.0, 1.0)
|
| **Returns\:** A floating-point, pseudo-random number that's greater than or equal to 0 and less than 1, with approximately uniform distribution over that range — which you can then scale to your desired range. Initial seed to the random number generator algorithm can be given using ``seedRandom()`` function.
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  sign(x\: :kw:`number`)\: :kw:`number`

| **Returns\:** -1 if \`x\` is negative, 1 if \`x\` is positive, 0 if \`x\` is close to zero (epsilon is 1e-13)
|
| **Arguments\:**

 - x\: :kw:`number`  arbitrary number

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  trunc(x\: :kw:`number`)\: :kw:`number`

| Integer part of \`v\`
|
| **Returns\:** The integer part of a number by removing any fractional digits.
|
| **Arguments\:**

 - x\: :kw:`number`

| **Notes\:** If arg is +Infinity or -Infinity, it is returned unmodified. If arg is +0 or -0, it is returned unmodified. If arg is NaN, NaN is returned
|

------

Properties
----------

.. rst-class:: doc-code-block

  - static E\: :kw:`number`
  - static LN10\: :kw:`number`
  - static LN2\: :kw:`number`
  - static LOG10E\: :kw:`number`
  - static LOG2E\: :kw:`number`
  - static PI\: :kw:`number`
  - static SQRT1_2\: :kw:`number`
  - static SQRT2\: :kw:`number`




.. _LsLtLd.LcLoLrLe.UDLoLuLbLlLe:

Number
======

.. rst-class:: doc-code-block

  :kw:`export`

| Represents boxed number value and related operations
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new number instance with initial value zero
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(value\: :kw:`number`)\: :kw:`void`

| Constructs a new number instance with provided initial value
|
| **Arguments\:**

 - value\: :kw:`number` ---  the initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(value\: :kw:`number`)\: :kw:`void`

| Constructs a new number instance with provided initial value
|
| **Arguments\:**

 - value\: :kw:`number` ---  the initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  isFinite()\: :kw:`boolean`

| isFinite() checks if the underlying number is a floating point value (not a NaN or infinity)
|
| **Returns\:** true if the underlying number is a floating point value
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  isInteger()\: :kw:`boolean`

| Checks if the underlying number is similar to an integer value
|
| **Returns\:** true if the underlying number is similar to an integer value
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  isNaN()\: :kw:`boolean`

| isNaN() checks if the underlying number is NaN (not a number)
|
| **Returns\:** true if the underlying number is NaN
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  isSafeInteger()\: :kw:`boolean`

| Checks if number is a safe integer value
|
| **Returns\:** true if the underlying number is a safe integer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toExponential(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| toExponential(number) returns string representing the underlying number in exponential notation
|
| **Returns\:** the result of conversion
|
| **Arguments\:**

 - d\: :kw:`number` ---  the exponent (rounded to nearest integer); must be in \[0, 100\]

| **Note\:** If d = new number(0.25); d.toExponential(2) -\> \"2.50e-1\" If d = new number(0.25); d.toExponential(2.5) -\> \"2.50e-1\" If d = new number(0.25); d.toExponential(1) -\> \"2.5e-1\" If d = new number(12345.01); d.toExponential(10) -\> \"1.2345010000e+4\" If d = new number(NaN); d.toExponential(10) -\> \"NaN\"; If d = new number(number.POSITIVE_INFINITY); d.toExponential(10) -\> \"Infinity\"; \"-Infinity\" for negative
|
| **Remark\:** Implemented as native function,
|
| **See\:** \`toExponential()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#673).
| ECMA reference\: https\://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.prototype.toexponential
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toExponential()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| toExponential() returns string representing the underlying number in exponential notation
|
| **Returns\:** the result of conversion
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toFixed(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| toFixed(number) returns string representing the underlying number using fixed-point notation
|
| **Returns\:** the result of conversion
|
| **Arguments\:**

 - d\: :kw:`number` ---  fixed size (integer part); must be in \[0, 100\]

| **Note\:** If d = new number(0.1); d.toFixed(0) -\> \"0\" If d = new number(0.7); d.toFixed(0) -\> \"1\" If d = new number(0.12345); d.toFixed(1) -\> \"0.1\" If d = new number(0.12345); d.toFixed(3) -\> \"0.123\" If d = new number(number.POSITIVE_INFINITY); d.toFixed(3) -\> \"Infinity\" If d = new number(number.NaN); d.toFixed(3) -\> \"NaN\" If d = new number(0.25); d.toFixed(200) -\> thrown ArgumentOutOfRangeError
|
| **Remark\:** Implemented as native function,
|
| **See\:** \`toFixed()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#693).
| ECMA reference\: https\://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.prototype.tofixed
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toFixed()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| toFixed(number) returns string representing the underlying number using fixed-point notation
|
| **Returns\:** the result of conversion
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)

| Accepts a locale and returns string in language-sensitive representation
|
| **Returns\:** result of the conversion in a local representation
|
| **Remark\:** Implemented as native function,
|
| **See\:** \`toLocaleString()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#741).
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Accepts a locale and returns string in language-sensitive representation
|
| **Returns\:** result of the conversion in a local representation
|
| **Remark\:** Implemented as native function,
|
| **See\:** \`toLocaleString()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#741).
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Without an argument method returns just toString value
|
| **Returns\:** result of the conversion
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toPrecision(d\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| toPrecision(number) returns string representing the underlying number on the specified precision
|
| **Returns\:** the result of conversion
|
| **Arguments\:**

 - d\: :kw:`number` ---  precision (rounded to nearest integer); must be in \[1, 100\]

| **Note\:** If d = new number(0.25); d.toPrecision(4) -\> \"0.2500\" If d = new number(1.01); d.toPrecision(4.7) -\> \"1.010\" If d = new number(0.25); d.toPrecision(0) -\> thrown ArgumentOutOfRangeError If d = new number(12345.123455); d.toPrecision(10) -\> \"12345.12346\"
|
| **Remark\:** Implemented as native function,
|
| **See\:** \`toPrecision()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#683).
| ECMA reference\: https\://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.prototype.toprecision
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toPrecision()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| toPrecision() returns string representing the underlying number in exponential notation basically, if toPrecision called with no argument it's just toString according to spec
|
| **Returns\:** the result of conversion
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts this object to a string
|
| **Returns\:** result of the conversion
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  valueOf()\: :kw:`number`

| Returns the value of this number
|
| **Returns\:** the value of this number
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isFinite(v\: :kw:`number`)\: :kw:`boolean`

| isFinite(number) checks if number is a floating point value (not a NaN or infinity)
|
| **Returns\:** true if the argument is a floating point value
|
| **Arguments\:**

 - v\: :kw:`number` ---  the number to test

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isInteger(v\: :kw:`number`)\: :kw:`boolean`

| Checks if number is similar to an integer value
|
| **Returns\:** true if the argument is similar to an integer value
|
| **Arguments\:**

 - v\: :kw:`number` ---  the number to test

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isNaN(v\: :kw:`number`)\: :kw:`boolean`

| isNaN(number) checks if number is NaN (not a number)
|
| **Returns\:** true if the argument is NaN
|
| **Arguments\:**

 - v\: :kw:`number` ---  the number to test

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  isSafeInteger(v\: :kw:`number`)\: :kw:`boolean`

| Checks if number is a safe integer value
|
| **Returns\:** true if the argument is integer ans less than MAX_SAFE_INTEGER
|
| **Arguments\:**

 - v\: :kw:`number` ---  the number to test

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  parseFloat(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`number`

| parseFloat(string) converts string to number
|
| **Returns\:** the result of conversion
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  the string to convert

| **Note\:** If arg is \"+Infinity\", \"Infinity\" or \"-Infinity\", return value is \`inf\` or \`-inf\` respectively. If arg is \"+0\" or \"-0\", return value is 0 or -0. If arg has leading zeroes, it's ignored\: \"0001.5\" -\> 1.5, \"-0001.5\" -\> -1.5 If arg starts from \".\", leading zero is implied\: \".5\" -\> 0.5, \"-.5\" -\> -0.5 If arg successfully parsed, trailing non-digits ignored\: \"-.6ffg\" -\> -0.6 If arg can not be parsed into a number, NaN is returned
|
| **Remark\:** Implemented as native function,
|
| **See\:** \`parseFloat()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#653).
| ECMA reference\: https\://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.parsefloat
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  parseInt(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, r\: :kw:`number`)\: :kw:`number`

| parseInt(string, int) parses from string an integer of specified radix
|
| **Returns\:** the result of parsing
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  the string to convert
 - r\: :kw:`number` ---  the radix of conversion; should be \[2, 36\]; 0 assumed to be 10

| **Note\:** If args (\"10\", 1) -\> thrown ArgumentOutOfRangeError, (\"10\", 37) -\> thrown ArgumentOutOfRangeError If args (\"10\", 2) -\> 2 If args (\"10\", 10) -\> 10, (\"10\", 0) -\> 10 If args (\"ff\", 16) -\> 255 etc.
|
| **Remark\:** Implemented as native function,
|
| **See\:** \`parseInt()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#663).
| ECMA reference\: https\://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.parseint
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  parseInt(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`number`

| parseInt(string) parses from string an integer of radix 10
|
| **Returns\:** the result of parsing
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  the string to convert

------


.. _LsLtLd.LcLoLrLe.UOLbLjLeLcLt:


Object
======



.. rst-class:: doc-code-block

  :kw:`export`

| Common ancestor amongst all other classes
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new blank object
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts this object to a string
|
| **Returns\:** result of the conversion
|

------


.. _LsLtLd.LcLoLrLe.UPLrLoLmLiLsLe:

Promise\<T\>
============



.. rst-class:: doc-code-block

  :kw:`export`

| Class represents a result of an asynchronous operation in the future.
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  catch\<U\>(
  |nbsp| onRejected\: (error\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`) =\> U
  )\: :ref:`Promise<LsLtLd.LcLoLrLe.UPLrLoLmLiLsLe>`\<U\>

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| callback\: (resolve\: (value\: T) =\> :kw:`void`) =\> :kw:`void`
  )\: :kw:`void`

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| callback\: (resolve\: (value\: T) =\> :kw:`void`, reject\: (error\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`void`) =\> :kw:`void`
  )\: :kw:`void`

------


.. rst-class:: doc-code-block

  :kw:`public`
  finally(
  |nbsp| onFinally\: () =\> :kw:`void`
  )\: :ref:`Promise<LsLtLd.LcLoLrLe.UPLrLoLmLiLsLe>`\<T\>

------


.. rst-class:: doc-code-block

  :kw:`public`
  then\<U\>(
  |nbsp| onFulfilled\: () =\> U
  )\: :ref:`Promise<LsLtLd.LcLoLrLe.UPLrLoLmLiLsLe>`\<U\>

------


.. rst-class:: doc-code-block

  :kw:`public`
  then\<U\>(
  |nbsp| onFulfilled\: (value\: T) =\> U
  )\: :ref:`Promise<LsLtLd.LcLoLrLe.UPLrLoLmLiLsLe>`\<U\>

------


.. rst-class:: doc-code-block

  :kw:`public`
  then\<U\>(
  |nbsp| onFulfilled\: (value\: T) =\> U,
  |nbsp| onRejected\: (error\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`) =\> U
  )\: :ref:`Promise<LsLtLd.LcLoLrLe.UPLrLoLmLiLsLe>`\<U\>

------

.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  resolve\<U\>(value\: U)\: :ref:`Promise<LsLtLd.LcLoLrLe.UPLrLoLmLiLsLe>`\<U\>

------


.. _LeLsLcLoLmLpLaLt.URLaLnLgLeUELrLrLoLr:

RangeError
==========



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`Error<LeLsLcLoLmLpLaLt.UELrLrLoLr>`

| **Class\:** Represents an error that occurs when provided collection index is out of range
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new instance of error
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------

Properties
----------

.. rst-class:: doc-code-block

  - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  - message\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - name\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - stack\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`



.. _LeLsLcLoLmLpLaLt.URLeLfLeLrLeLnLcLeUELrLrLoLr:

ReferenceError
==============



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`Error<LeLsLcLoLmLpLaLt.UELrLrLoLr>`

| **Class\:** Represents an error that occurs when a variable that doesn't exist (or hasn't yet been initialized) in the current scope is referenced
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new instance of error
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------

Properties
----------

.. rst-class:: doc-code-block

  - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  - message\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - name\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - stack\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`



.. _LeLsLcLoLmLpLaLt.URLeLgUELxLp:

RegExp
======



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

| **Class\:** Regular expression
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  compile(pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, flags\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Recompiles a regular expression with new source and flags after the RegExp object has already been created
|
| **Arguments\:**

 - pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  the text of the regular expression
 - flags\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  any combination of flag values

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new RegExp using pattern
|
| **Arguments\:**

 - pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  description of a pattern

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, flags\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new RegExp using pattern and flags
|
| **Arguments\:**

 - pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  description of a pattern
 - flags\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  description of flags to be used

------


.. rst-class:: doc-code-block

  :kw:`public`
  exec(str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`RegExpExecArray<LeLsLcLoLmLpLaLt.URLeLgUELxLpUELxLeLcURLeLsLuLlLt>`

| Executes a search for a match in a specified string and returns a result array
|
| **Returns\:** RegExp result
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  the string against which to match the regular expression

| **See\:** RegExpExecArray
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  test(str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`boolean`

| Executes a search for a match between a regular expression and specified string
|
| **Returns\:** true if match was found
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  the string against which to match the regular expression

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representing the given object
|
| **Returns\:** a string representing the given object
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`static`
  advancestringIndex(
  |nbsp| s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`,
  |nbsp| index\: :kw:`number`,
  |nbsp| unicode\: :kw:`boolean`
  )\: :kw:`number`

| Returns next index from a passed one
|
| **Returns\:** new index
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - index\: :kw:`number`  start position
 - unicode\: :kw:`boolean`  true if unicode is used

------

Properties
----------

.. rst-class:: doc-code-block

  - dotAll\: :kw:`boolean`
  - flags\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - global\: :kw:`boolean`
  - hasIndices\: :kw:`boolean`
  - ignoreCase\: :kw:`boolean`
  - lastIndex\: :kw:`number`
  - multiline\: :kw:`boolean`
  - source\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - sticky\: :kw:`boolean`
  - unicode\: :kw:`boolean`


.. _LeLsLcLoLmLpLaLt.URLeLgUELxLpUELxLeLcURLeLsLuLlLt:

RegExpExecArray
================

TODO: align with TS spec

.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

| **Class\:** Regular expression result descriptor
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| isCorrect\: :kw:`boolean`,
  |nbsp| index\: :kw:`number`,
  |nbsp| input\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`,
  |nbsp| result\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`\[\]
  )\: :kw:`void`

| Creates a RegExpExecArray
|
| **Arguments\:**

 - isCorrect\: :kw:`boolean`
 - index\: :kw:`number`
 - input\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - result\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`\[\]

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  equals(other\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`)\: :kw:`boolean`

| Creates a RegExpExecArray
|
| **Arguments\:**

 - other\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>` | :kw:`null`

------


.. rst-class:: doc-code-block

  :kw:`public`
  get(index\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns result string by index
|
| **Returns\:** resulting string
|
| **Arguments\:**

 - index\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  get(index\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns result string by index
|
| **Returns\:** resulting string
|
| **Arguments\:**

 - index\: :kw:`number`

------


.. _LeLsLcLoLmLpLaLt.USLeLt:

Set\<K\>
========



.. rst-class:: doc-code-block

  :kw:`export`

| **Class\:** Set implementation via tree
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  add(v\: K)\: :kw:`void`

| Puts a value into the Set
|
| **Arguments\:**

 - v\: K  the value to put into the Set

------


.. rst-class:: doc-code-block

  :kw:`public`
  clear()\: :kw:`void`

| Deletes all elements from the Set
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs an empty TreeSet
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  delete(v\: K)\: :kw:`void`

| Removes a value from the Set
|
| **Arguments\:**

 - v\: K  the value to remove

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (k\: K) =\> :kw:`void`
  )\: :kw:`void`

| Applies a function over all elements of the Set
|
| **Arguments\:**

 - fn\: (k\: K) =\> :kw:`void`  to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  has(v\: K)\: :kw:`boolean`

| Checks if a value is in the Set
|
| **Returns\:** true if the value is in the Set
|
| **Arguments\:**

 - v\: K  the value to find in the Set

------


.. rst-class:: doc-code-block

  :kw:`public`
  length()\: :kw:`number`

| Returns number of unique elements in the Set
|
| **Returns\:** number of unique elements in the Set
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  values()\: :ref:`IterableIterator<LeLsLcLoLmLpLaLt.UILtLeLrLaLbLlLeUILtLeLrLaLtLoLr>`\<K\>

| Returns elements from the Set as an array
|
| **Returns\:** an array of Set values
|

------

Properties
----------

.. rst-class:: doc-code-block

  - size\: :kw:`number`



.. _LsLtLd.LcLoLrLe.USLtLrLiLnLg:

String
======



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

| Unicode string
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  charAt(index\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Getter for char at some index
|
| **Returns\:** char value at index
| throws stringIndexOutOfBoundsException if index is negative or \>= length
|
| **Arguments\:**

 - index\: :kw:`number` ---  index in char array inside string

| **Remarks\:** Implemented as native function,
|
| **See\:** \`charAt()\` intrinsic \[declaration\](https\://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L585).
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  charCodeAt(index\: :kw:`number`)\: :kw:`number`

| The charCodeAt() method returns an integer between 0 and 65535 representing the UTF-16 code unit at the given index.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  concat(to\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Concatenation of this and another string.
|
| **Returns\:** new string which is a concatenation of this + to
|
| **Arguments\:**

 - to\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  string to concat with  throws NullPointerException if to param is null

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs an empty string
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(data\: :kw:`char`\[\])\: :kw:`void`

| Constructs string from chars array initializer
|
| **Arguments\:**

 - data\: :kw:`char`\[\] ---  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(otherStr\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs string from another string
|
| **Arguments\:**

 - otherStr\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, fromIndex\: :kw:`number`)\: :kw:`number`

| Finds the first occurrence of another string in this string. The search starts from the specified index.
|
| **Returns\:** index of the str from the beginning of this string, or -1 if not found
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  to find
 - fromIndex\: :kw:`number` ---  to start searching from  throws NullPointerException if str param is null throws stringIndexOutOfBoundsException if fromIndex param is negative or \>= length throws AssertionError if length of str is greater than the length of this string

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, fromIndex\: :kw:`number`)\: :kw:`number`

| Finds the last occurrence of another string in this string. The search starts from the specified index.
|
| **Returns\:** index of the str from the beginning of this string, or -1 if not found
|
| **Arguments\:**

 - str\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  to find
 - fromIndex\: :kw:`number` ---  to start searching from  throws NullPointerException if str param is null throws stringIndexOutOfBoundsException if fromIndex param is negative or \>= length throws AssertionError if length of str in sum of specified index is greater than the length of this string

------


.. rst-class:: doc-code-block

  :kw:`public`
  localeCompare(another\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` | :kw:`null`)\: :kw:`number`

| Comparison between this string and another one based on locale. The result is -1 if this string sorts before the another string, 0 if they are equal, and 1 otherwise.
|
| **Returns\:** the comparison result
|
| **Arguments\:**

 - another\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  string to compare with
 - locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` | :kw:`null` ---  string representing the BCP47 language tag  throws RangeError if the locale tag is invalid or not found throws NullPointerException if another or locale is null

------


.. rst-class:: doc-code-block

  :kw:`public`
  localeCompare(another\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`number`

| Comparison between this string and another one based on default host locale. The result is -1 if this string sorts before the another string, 0 if they are equal, and 1 otherwise.
|
| **Returns\:** the comparison result
|
| **Arguments\:**

 - another\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  string to compare with  throws RangeError if the locale tag is invalid or not found throws NullPointerException if another param is null

------


.. rst-class:: doc-code-block

  :kw:`public`
  match(regexp\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`\[\]

| regexp match
|
| **Arguments\:**

 - regexp\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  match(regexp\: :ref:`RegExp<LeLsLcLoLmLpLaLt.URLeLgUELxLp>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`\[\]

| Retrieves the result of matching a string against a regular expression
|
| **Returns\:** If the regexp.global is true, all results matching the complete regular expression will be returned, but capturing groups are not included Otherwise, only the first complete match and its related capturing groups are returned
|
| **Arguments\:**

 - regexp\: :ref:`RegExp<LeLsLcLoLmLpLaLt.URLeLgUELxLp>` ---  a regular expression object

------


.. rst-class:: doc-code-block

  :kw:`public`
  replace(w1\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, w2\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| replace
|
| **Returns\:** new string
|
| **Arguments\:**

 - w1\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  - w2\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  replace(pattern\: :ref:`RegExp<LeLsLcLoLmLpLaLt.URLeLgUELxLp>`, replacement\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a new string with one, some, or all matches of a pattern replaced by a replacement. The pattern can be a string or a RegExp, and the replacement can be a string or a function called for each match. If pattern is a string, only the first occurrence will be replaced. The original string is left unchanged.
|
| **Returns\:** new string
|
| **Arguments\:**

 - pattern\: :ref:`RegExp<LeLsLcLoLmLpLaLt.URLeLgUELxLp>`  - replacement\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  search(reg\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`number`

| search
|
| **Returns\:** int
|
| **Arguments\:**

 - reg\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  search(regexp\: :ref:`RegExp<LeLsLcLoLmLpLaLt.URLeLgUELxLp>`)\: :kw:`number`

| Executes a search for a match between a regular expression and this string object.
|
| **Returns\:** the index of the first match between the regular expression and the given string, or -1 if no match was found.
|
| **Arguments\:**

 - regexp\: :ref:`RegExp<LeLsLcLoLmLpLaLt.URLeLgUELxLp>` ---  a regular expression object

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The slice() method extracts a section of a string and returns it as a new string, without modifying the original string.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  split(pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, limit\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`\[\]

| Splits this string by pattern and returns ordered array of substrings. The order of the resulted array corresponds to the order of the passage of this string from beginning to end. The pattern is excluded from substrings. The array is limited by some specified value.
|
| **Returns\:** string array contains substrings from this string
|
| **Arguments\:**

 - pattern\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>` ---  string to split by
 - limit\: :kw:`number` ---  max length of the returned array. If it's negative then there is no limit.  throws NullPointerException if pattern param is null

------


.. rst-class:: doc-code-block

  :kw:`public`
  substr(begin\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The substr() method returns a portion of the string, starting at the specified index and extending for a given number of characters afterwards.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  substr(begin\: :kw:`number`, length\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The substr() method returns a portion of the string, starting at the specified index and extending for a given number of characters afterwards.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  substring(begin\: :kw:`number`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Selects a substring of this string, starting at a specified index and ending at the end of this string.
|
| **Returns\:** new string which is a substring of this string
|
| **Arguments\:**

 - begin\: :kw:`number` ---  to start substring  throws stringIndexOutOfBoundsException if begin param is negative or \>= length

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleLowerCase(locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The toLocaleLowerCase() method returns the calling string value converted to lower case, according to any locale-specific case mappings.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleLowerCase()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The toLocaleLowerCase() method returns the calling string value converted to lower case, according to any locale-specific case mappings.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleUpperCase(locale\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The toLocaleUpperCase() method returns the calling string value converted to upper case, according to any locale-specific case mappings.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleUpperCase()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The toLocaleUpperCase() method returns the calling string value converted to upper case, according to any locale-specific case mappings.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLowerCase()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Creates new string similar to this string but with all characters in lower case.
|
| **Returns\:** new string with all characters in lower case
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The \`toString()\` method returns the string representation of the given string in the form of a copy of the original object.
|
| **Returns\:** a copy of the original string
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toUpperCase()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Creates new string similar to this string but with all characters in upper case.
|
| **Returns\:** new string with all characters in upper case
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  valueOf()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| The valueOf() method returns the primitive value of a string object.
|


Properties
----------

.. rst-class:: doc-code-block

  - length\: :kw:`number`



.. _LeLsLcLoLmLpLaLt.USLyLnLtLaLxUELrLrLoLr:

SyntaxError
===========



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`Error<LeLsLcLoLmLpLaLt.UELrLrLoLr>`

| **Class\:** Represents an error that occurs when trying to interpret syntactically invalid code
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new instance of error
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------

Properties
----------

.. rst-class:: doc-code-block

  - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  - message\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - name\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - stack\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`



.. _LeLsLcLoLmLpLaLt.UUURUIUELrLrLoLr:

URIError
========



.. rst-class:: doc-code-block

  :kw:`export`
  :kw:`extends` :ref:`Error<LeLsLcLoLmLpLaLt.UELrLrLoLr>`

| **Class\:** Represents an error that occurs when a global URI handling function was used in a wrong way
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new instance of error
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`, cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :kw:`void`

| Constructs a new instance of error
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
 - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------

Properties
----------

.. rst-class:: doc-code-block

  - cause\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
  - message\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - name\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`
  - stack\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`


.. _LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy:

Uint16Array
===========



.. rst-class:: doc-code-block

  :kw:`export`

| JS Uint16Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Uint16Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Uint16Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type number in newly created Uint16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Uint16Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Uint16Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Uint16Array.
|
| **Arguments\:**

 - other\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Uint16Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Uint16Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint16Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Fills the Uint16Array with specified value
|
| **Returns\:** modified Uint16Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Fills the Uint16Array with specified value
|
| **Returns\:** modified Uint16Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Fills the Uint16Array with specified value
|
| **Returns\:** modified Uint16Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a new Uint16Array from current Uint16Array based on a condition fn.
|
| **Returns\:** a new Uint16Array with elements from current Uint16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| creates a new Uint16Array from current Uint16Array based on a condition fn
|
| **Returns\:** a new Uint16Array with elements from current Uint16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| creates a new Uint16Array from current Uint16Array based on a condition fn
|
| **Returns\:** a new Uint16Array with elements from current Uint16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a new Uint16Array from current Uint16Array based on a condition fn.
|
| **Returns\:** a new Uint16Array with elements from current Uint16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| creates a new Uint16Array from current Uint16Array based on a condition fn
|
| **Returns\:** a new Uint16Array with elements from current Uint16Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint16Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint16Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates an Uint16Array from array-like argument
|
| **Returns\:** new Uint16Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint16Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates an Uint16Array from array-like argument
|
| **Returns\:** new Uint16Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates an Uint16Array from array-like argument
|
| **Returns\:** new Uint16Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint16Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates an Uint16Array from array-like argument
|
| **Returns\:** new Uint16Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint16Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint16Array
|
| **Returns\:** true if e is in Uint16Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint16Array
|
| **Returns\:** true if e is in Uint16Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a new Uint16Array using fn(arr\[i\]) over all elements of current Uint16Array.
|
| **Returns\:** a new Uint16Array where for each element from current Uint16Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a new Uint16Array using fn(arr\[i\]) over all elements of current Uint16Array
|
| **Returns\:** a new Uint16Array where for each element from current Uint16Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a new Uint16Array using fn(arr\[i\]) over all elements of current Uint16Array.
|
| **Returns\:** a new Uint16Array where for each element from current Uint16Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint16Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a new Uint16Array using initializer
|
| **Returns\:** a new Uint16Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a new Uint16Array using reversed data from the current one
|
| **Returns\:** a new Uint16Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(number\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Uint16Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Uint16Array.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a slice of current Uint16Array using range \[begin, end)
|
| **Returns\:** a new Uint16Array with elements of current Uint16Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a slice of current Uint16Array using range \[begin, this.length).
|
| **Returns\:** a new Uint16Array with elements of current Uint16Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a slice of current Uint16 with all elements.
|
| **Returns\:** a new Uint16Array with elements of current Uint16Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint16Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Uint16Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Uint16Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a Uint16Array with the same underlying ArrayBuffer
|
| **Returns\:** new Uint16Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a Uint16Array with the same ArrayBuffer
|
| **Returns\:** new Uint16Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a Uint16Array with the same ArrayBuffer
|
| **Returns\:** new Uint16Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint16Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint16Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint16Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Uint16Array
|
| **Returns\:** a string representation of the Uint16Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Uint16Array<LeLsLcLoLmLpLaLt.UULiLnLtU1U6UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Uint16Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy:

Uint32Array
===========



.. rst-class:: doc-code-block

  :kw:`export`

| JS Uint32Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Uint32Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Uint32Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type number in newly created Uint32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Uint32Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Uint32Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Uint32Array.
|
| **Arguments\:**

 - other\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Uint32Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Uint32Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint32Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Fills the Uint32Array with specified value
|
| **Returns\:** modified Uint32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Fills the Uint32Array with specified value
|
| **Returns\:** modified Uint32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Fills the Uint32Array with specified value
|
| **Returns\:** modified Uint32Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a new Uint32Array from current Uint32Array based on a condition fn.
|
| **Returns\:** a new Uint32Array with elements from current Uint32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| creates a new Uint32Array from current Uint32Array based on a condition fn
|
| **Returns\:** a new Uint32Array with elements from current Uint32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| creates a new Uint32Array from current Uint32Array based on a condition fn
|
| **Returns\:** a new Uint32Array with elements from current Uint32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a new Uint32Array from current Uint32Array based on a condition fn.
|
| **Returns\:** a new Uint32Array with elements from current Uint32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| creates a new Uint32Array from current Uint32Array based on a condition fn
|
| **Returns\:** a new Uint32Array with elements from current Uint32Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint32Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint32Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates an Uint32Array from array-like argument
|
| **Returns\:** new Uint32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint32Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates an Uint32Array from array-like argument
|
| **Returns\:** new Uint32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates an Uint32Array from array-like argument
|
| **Returns\:** new Uint32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint32Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates an Uint32Array from array-like argument
|
| **Returns\:** new Uint32Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint32Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint32Array
|
| **Returns\:** true if e is in Uint32Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint32Array
|
| **Returns\:** true if e is in Uint32Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a new Uint32Array using fn(arr\[i\]) over all elements of current Uint32Array.
|
| **Returns\:** a new Uint32Array where for each element from current Uint32Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a new Uint32Array using fn(arr\[i\]) over all elements of current Uint32Array
|
| **Returns\:** a new Uint32Array where for each element from current Uint32Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a new Uint32Array using fn(arr\[i\]) over all elements of current Uint32Array.
|
| **Returns\:** a new Uint32Array where for each element from current Uint32Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint32Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a new Uint32Array using initializer
|
| **Returns\:** a new Uint32Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a new Uint32Array using reversed data from the current one
|
| **Returns\:** a new Uint32Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(number\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Uint32Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Uint32Array.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a slice of current Uint32Array using range \[begin, end)
|
| **Returns\:** a new Uint32Array with elements of current Uint32Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a slice of current Uint32Array using range \[begin, this.length).
|
| **Returns\:** a new Uint32Array with elements of current Uint32Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a slice of current Uint32 with all elements.
|
| **Returns\:** a new Uint32Array with elements of current Uint32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint32Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Uint32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Uint32Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a Uint32Array with the same underlying ArrayBuffer
|
| **Returns\:** new Uint32Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a Uint32Array with the same ArrayBuffer
|
| **Returns\:** new Uint32Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a Uint32Array with the same ArrayBuffer
|
| **Returns\:** new Uint32Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint32Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint32Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint32Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Uint32Array
|
| **Returns\:** a string representation of the Uint32Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Uint32Array<LeLsLcLoLmLpLaLt.UULiLnLtU3U2UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Uint32Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy:

Uint8Array
==========



.. rst-class:: doc-code-block

  :kw:`export`

| JS Uint8Array API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Uint8Array.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Uint8Array with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type number in newly created Uint8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Uint8Array with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Uint8Array with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Uint8Array.
|
| **Arguments\:**

 - other\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Uint8Array.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Uint8Array.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8Array satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Fills the Uint8Array with specified value
|
| **Returns\:** modified Uint8Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Fills the Uint8Array with specified value
|
| **Returns\:** modified Uint8Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Fills the Uint8Array with specified value
|
| **Returns\:** modified Uint8Array
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a new Uint8Array from current Uint8Array based on a condition fn.
|
| **Returns\:** a new Uint8Array with elements from current Uint8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| creates a new Uint8Array from current Uint8Array based on a condition fn
|
| **Returns\:** a new Uint8Array with elements from current Uint8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| creates a new Uint8Array from current Uint8Array based on a condition fn
|
| **Returns\:** a new Uint8Array with elements from current Uint8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a new Uint8Array from current Uint8Array based on a condition fn.
|
| **Returns\:** a new Uint8Array with elements from current Uint8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| creates a new Uint8Array from current Uint8Array based on a condition fn
|
| **Returns\:** a new Uint8Array with elements from current Uint8Array that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8Array that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8Array
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates an Uint8Array from array-like argument
|
| **Returns\:** new Uint8Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates an Uint8Array from array-like argument
|
| **Returns\:** new Uint8Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates an Uint8Array from array-like argument
|
| **Returns\:** new Uint8Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates an Uint8Array from array-like argument
|
| **Returns\:** new Uint8Array
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8Array
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint8Array
|
| **Returns\:** true if e is in Uint8Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint8Array
|
| **Returns\:** true if e is in Uint8Array, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a new Uint8Array using fn(arr\[i\]) over all elements of current Uint8Array.
|
| **Returns\:** a new Uint8Array where for each element from current Uint8Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a new Uint8Array using fn(arr\[i\]) over all elements of current Uint8Array
|
| **Returns\:** a new Uint8Array where for each element from current Uint8Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a new Uint8Array using fn(arr\[i\]) over all elements of current Uint8Array.
|
| **Returns\:** a new Uint8Array where for each element from current Uint8Array fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint8Array

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a new Uint8Array using initializer
|
| **Returns\:** a new Uint8Array from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a new Uint8Array using reversed data from the current one
|
| **Returns\:** a new Uint8Array using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(number\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Uint8Array starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Uint8Array.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a slice of current Uint8Array using range \[begin, end)
|
| **Returns\:** a new Uint8Array with elements of current Uint8Array\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a slice of current Uint8Array using range \[begin, this.length).
|
| **Returns\:** a new Uint8Array with elements of current Uint8Array\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a slice of current Uint8 with all elements.
|
| **Returns\:** a new Uint8Array with elements of current Uint8Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8Array satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Uint8Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Uint8Array
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a Uint8Array with the same underlying ArrayBuffer
|
| **Returns\:** new Uint8Array with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a Uint8Array with the same ArrayBuffer
|
| **Returns\:** new Uint8Array with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a Uint8Array with the same ArrayBuffer
|
| **Returns\:** new Uint8Array with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint8Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint8Array to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint8Array to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Uint8Array
|
| **Returns\:** a string representation of the Uint8Array
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Uint8Array<LeLsLcLoLmLpLaLt.UULiLnLtU8UALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Uint8Array with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`


.. _LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy:

Uint8ClampedArray
=================



.. rst-class:: doc-code-block

  :kw:`export`

| JS Uint8ClampedArray API-compatible class
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  at(index\: :kw:`number`)\: :kw:`number`

| Returns an instance of primitive type at passed index.
|
| **Returns\:** a primitive at index
|
| **Arguments\:**

 - index\: :kw:`number`  index to look at

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Creates an empty Uint8ClampedArray.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(
  |nbsp| buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`,
  |nbsp| byteOffset\: :kw:`number`,
  |nbsp| length\: :kw:`number`
  )\: :kw:`void`

| Creates an Uint8ClampedArray with respect to data, byteOffset and length.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf
 - length\: :kw:`number`  size of elements of type number in newly created Uint8ClampedArray

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`, byteOffset\: :kw:`number`)\: :kw:`void`

| Creates an Uint8ClampedArray with respect to buf and byteOffset.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer
 - byteOffset\: :kw:`number`  byte offset from begin of the buf

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`)\: :kw:`void`

| Creates an Uint8ClampedArray with respect to buf.
|
| **Arguments\:**

 - buf\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor(other\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`)\: :kw:`void`

| Creates a copy of Uint8ClampedArray.
|
| **Arguments\:**

 - other\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`  data initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(
  |nbsp| insertPos\: :kw:`number`,
  |nbsp| startPos\: :kw:`number`,
  |nbsp| endPos\: :kw:`number`
  )\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to endPos.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from
 - endPos\: :kw:`number`  last index to end copy from, excluded  See rules of parameters normalization on `MDN <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`, startPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from startPos to end of Uint8ClampedArray.
|
| **Arguments\:**

 - insertPos\: :kw:`number`  insert index to place copied elements
 - startPos\: :kw:`number`  start index to begin copy from  See rules of parameters normalization `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_

------


.. rst-class:: doc-code-block

  :kw:`public`
  copyWithin(insertPos\: :kw:`number`)\: :kw:`void`

| Makes a copy of internal elements to insertPos from begin to end of Uint8ClampedArray.
| See rules of parameters normalization\: `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/Array/copyWithin>`_
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8ClampedArray satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8ClampedArray satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8ClampedArray satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8ClampedArray satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  every(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that all elements of Uint8ClampedArray satisfy the passed function
|
| **Returns\:** true if all elements satisfy fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(
  |nbsp| value\: :kw:`number`,
  |nbsp| start\: :kw:`number`,
  |nbsp| end\: :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Fills the Uint8ClampedArray with specified value
|
| **Returns\:** modified Uint8ClampedArray
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`
 - end\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`, start\: :kw:`number`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Fills the Uint8ClampedArray with specified value
|
| **Returns\:** modified Uint8ClampedArray
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy
 - start\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  fill(value\: :kw:`number`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Fills the Uint8ClampedArray with specified value
|
| **Returns\:** modified Uint8ClampedArray
|
| **Arguments\:**

 - value\: :kw:`number`  new valuy

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a new Uint8ClampedArray from current Uint8ClampedArray based on a condition fn.
|
| **Returns\:** a new Uint8ClampedArray with elements from current Uint8ClampedArray that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| creates a new Uint8ClampedArray from current Uint8ClampedArray based on a condition fn
|
| **Returns\:** a new Uint8ClampedArray with elements from current Uint8ClampedArray that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| creates a new Uint8ClampedArray from current Uint8ClampedArray based on a condition fn
|
| **Returns\:** a new Uint8ClampedArray with elements from current Uint8ClampedArray that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a new Uint8ClampedArray from current Uint8ClampedArray based on a condition fn.
|
| **Returns\:** a new Uint8ClampedArray with elements from current Uint8ClampedArray that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  filter(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| creates a new Uint8ClampedArray from current Uint8ClampedArray based on a condition fn
|
| **Returns\:** a new Uint8ClampedArray with elements from current Uint8ClampedArray that satisfy condition fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the first element that satisfies fn TODO\: return number | undefined as in JS
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  the condition to apply for each element

------


.. rst-class:: doc-code-block

  :kw:`public`
  find(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the first element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the first element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLast(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the last element that satisfies fn
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  findLastIndex(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`number`

| Finds an index of the last element in the Uint8ClampedArray that satisfies the condition
|
| **Returns\:** the index of the last element that satisfies fn, -1 otherwise
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8ClampedArray
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8ClampedArray
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8ClampedArray
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8ClampedArray
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  forEach(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :kw:`void`

| Applies a function over all elements of Uint8ClampedArray
|
| **Returns\:** undefined
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  function to apply

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates an Uint8ClampedArray from array-like argument
|
| **Returns\:** new Uint8ClampedArray
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8ClampedArray
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates an Uint8ClampedArray from array-like argument
|
| **Returns\:** new Uint8ClampedArray
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8ClampedArray

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates an Uint8ClampedArray from array-like argument
|
| **Returns\:** new Uint8ClampedArray
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8ClampedArray
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  from(
  |nbsp| o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`,
  |nbsp| mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates an Uint8ClampedArray from array-like argument
|
| **Returns\:** new Uint8ClampedArray
|
| **Arguments\:**

 - o\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`  array-like object to initialize Uint8ClampedArray
 - mapFn\: (e\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, index\: :kw:`number`) =\> :kw:`number`  function to apply for each

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint8ClampedArray
|
| **Returns\:** true if e is in Uint8ClampedArray, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  includes(e\: :kw:`number`)\: :kw:`boolean`

| Checks if specified argument is in Uint8ClampedArray
|
| **Returns\:** true if e is in Uint8ClampedArray, false otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element
 - fromIndex\: :kw:`number`  start index to search from

------


.. rst-class:: doc-code-block

  :kw:`public`
  indexOf(e\: :kw:`number`)\: :kw:`number`

| Returns index of specified element
|
| **Returns\:** index of element if it presents, -1 otherwise
|
| **Arguments\:**

 - e\: :kw:`number`  search element

------


.. rst-class:: doc-code-block

  :kw:`public`
  join(s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation
|
| **Arguments\:**

 - s\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`  separator

------


.. rst-class:: doc-code-block

  :kw:`public`
  join()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Joins data to a string
|
| **Returns\:** joined representation with comma separator
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`, fromIndex\: :kw:`number`)\: :kw:`number`

| Moves backwards starting at fromIndex to 0 and search val.
|
| **Returns\:** right-most index of val. It must be less or equal than fromIndex. -1 if val not found `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/lastIndexOf>`_
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search
 - fromIndex\: :kw:`number`  the first index to search val at, i.e. fromIndex is included in search space

------


.. rst-class:: doc-code-block

  :kw:`public`
  lastIndexOf(val\: :kw:`number`)\: :kw:`number`

| Moves backwards and search val.
|
| **Returns\:** right-most index of val. -1 if val not found
|
| **Arguments\:**

 - val\: :kw:`number`  a value to search

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a new Uint8ClampedArray using fn(arr\[i\]) over all elements of current Uint8ClampedArray.
|
| **Returns\:** a new Uint8ClampedArray where for each element from current Uint8ClampedArray fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint8ClampedArray

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a new Uint8ClampedArray using fn(arr\[i\]) over all elements of current Uint8ClampedArray
|
| **Returns\:** a new Uint8ClampedArray where for each element from current Uint8ClampedArray fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint8ClampedArray

------


.. rst-class:: doc-code-block

  :kw:`public`
  map(
  |nbsp| fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a new Uint8ClampedArray using fn(arr\[i\]) over all elements of current Uint8ClampedArray.
|
| **Returns\:** a new Uint8ClampedArray where for each element from current Uint8ClampedArray fn was applied
|
| **Arguments\:**

 - fn\: (val\: :kw:`number`, index\: :kw:`number`) =\> :kw:`number`  a function to apply for each element of current Uint8ClampedArray

------


.. rst-class:: doc-code-block

  :kw:`public`
  of(data\: :kw:`number`\[\])\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a new Uint8ClampedArray using initializer
|
| **Returns\:** a new Uint8ClampedArray from data
|
| **Arguments\:**

 - data\: :kw:`number`\[\]  initializer

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduce(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using left-to-right traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`,
  |nbsp| init\: :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition
 - init\: :kw:`number`  initial value

------


.. rst-class:: doc-code-block

  :kw:`public`
  reduceRight(
  |nbsp| fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`
  )\: :kw:`number`

| Reduces data into a single value using right-to-left traversal
|
| **Returns\:** reduction result
|
| **Arguments\:**

 - fn\: (acc\: :kw:`number`, curVal\: :kw:`number`, curIndex\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`number`  condition

------


.. rst-class:: doc-code-block

  :kw:`public`
  reverse()\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a new Uint8ClampedArray using reversed data from the current one
|
| **Returns\:** a new Uint8ClampedArray using reversed data from the current one
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(insertPos\: :kw:`number`, val\: :kw:`number`)\: :kw:`void`

| Assigns val as element on insertPos.
|
| **Description\:** Added to avoid (un)packing a single value into array to use overloaded set(number\[\], insertPos)
|
| **Arguments\:**

 - insertPos\: :kw:`number`  index to change
 - val\: :kw:`number`  value to set

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\], insertPos1\: :kw:`number`)\: :kw:`void`

| Copies all elements of arr to the current Uint8ClampedArray starting from insertPos.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from
 - insertPos1\: :kw:`number`

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(arr\: :kw:`number`\[\])\: :kw:`void`

| Copies all elements of arr to the current Uint8ClampedArray.
|
| **Arguments\:**

 - arr\: :kw:`number`\[\]  array to copy data from

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a slice of current Uint8ClampedArray using range \[begin, end)
|
| **Returns\:** a new Uint8ClampedArray with elements of current Uint8ClampedArray\[begin;end) where end index is excluded `https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_objects/TypedArray/slice>`_
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice
 - end\: :kw:`number`  last index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice(begin\: :kw:`number`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a slice of current Uint8ClampedArray using range \[begin, this.length).
|
| **Returns\:** a new Uint8ClampedArray with elements of current Uint8ClampedArray\[begin, this.length)
|
| **Arguments\:**

 - begin\: :kw:`number`  start index to be taken into slice

------


.. rst-class:: doc-code-block

  :kw:`public`
  slice()\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a slice of current Uint8Clamped with all elements.
|
| **Returns\:** a new Uint8ClampedArray with elements of current Uint8ClampedArray
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8ClampedArray satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8ClampedArray satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8ClampedArray satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8ClampedArray satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`, array\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  some(
  |nbsp| fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`
  )\: :kw:`boolean`

| Checks that at least one element of Uint8ClampedArray satisfies the passed function
|
| **Returns\:** true if some element satisfies fn
|
| **Arguments\:**

 - fn\: (element\: :kw:`number`, index\: :kw:`number`) =\> :kw:`boolean`  check function

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort()\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Sorts in-place according to the numeric ordering
|
| **Returns\:** sorted Uint8ClampedArray
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  sort(
  |nbsp| fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`
  )\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Sorts in-place
|
| **Returns\:** sorted Uint8ClampedArray
|
| **Arguments\:**

 - fn\: (a\: :kw:`number`, b\: :kw:`number`) =\> :kw:`number`  comparator

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`, end\: :kw:`number`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a Uint8ClampedArray with the same underlying ArrayBuffer
|
| **Returns\:** new Uint8ClampedArray with the same underlying ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive
 - end\: :kw:`number`  last index, exclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray(begin\: :kw:`number`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a Uint8ClampedArray with the same ArrayBuffer
|
| **Returns\:** new Uint8ClampedArray with the same ArrayBuffer
|
| **Arguments\:**

 - begin\: :kw:`number`  start index, inclusive

------


.. rst-class:: doc-code-block

  :kw:`public`
  subarray()\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a Uint8ClampedArray with the same ArrayBuffer
|
| **Returns\:** new Uint8ClampedArray with the same ArrayBuffer
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`, options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint8ClampedArray to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`
 - options\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString(locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`)\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint8ClampedArray to a string with respect to locale
|
| **Returns\:** string representation
|
| **Arguments\:**

 - locales\: :ref:`object<LsLtLd.LcLoLrLe.UOLbLjLeLcLt>`

------


.. rst-class:: doc-code-block

  :kw:`public`
  toLocaleString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts Uint8ClampedArray to a string with respect to locale
|
| **Returns\:** string representation
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toReversed()\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a reversed copy
|
| **Returns\:** a reversed copy
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  toSorted()\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a sorted copy
|
| **Returns\:** a sorted copy
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Returns a string representation of the Uint8ClampedArray
|
| **Returns\:** a string representation of the Uint8ClampedArray
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  with(index\: :kw:`number`, value\: :kw:`number`)\: :ref:`Uint8ClampedArray<LeLsLcLoLmLpLaLt.UULiLnLtU8UCLlLaLmLpLeLdUALrLrLaLy>`

| Creates a copy with replaced value on index
|
| **Returns\:** an Uint8ClampedArray with replaced value on index
|
| **Arguments\:**

 - index\: :kw:`number`
 - value\: :kw:`number`

------

Properties
----------

.. rst-class:: doc-code-block

  - static BYTES_PER_ELEMENT\: :kw:`int`
  - buffer\: :ref:`ArrayBuffer<LeLsLcLoLmLpLaLt.UALrLrLaLyUBLuLfLfLeLr>`
  - byteLength\: :kw:`number`
  - byteOffset\: :kw:`number`
  - length\: :kw:`number`




.. _LeLsLcLoLmLpLaLt.UWLeLaLkUMLaLp:

WeakMap\<K, V\>
===============



.. rst-class:: doc-code-block

  :kw:`export`

| **Class\:** A WeakMap is a collection of key/value pairs whose keys must be objects or non-registered symbols, with values of any arbitrary JavaScript type, and which does not create strong references to its keys.
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| The WeakMap() constructor creates WeakMap objects.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  delete(k\: K)\: :kw:`boolean`

| The delete() method removes the specified element from a WeakMap object.
|
| **Returns\:** boolean
|
| **Arguments\:**

 - k\: K

------


.. rst-class:: doc-code-block

  :kw:`public`
  get(k\: K)\: V | :kw:`null`

| The get() method returns a specified element from a WeakMap object.
|
| **Returns\:** related value or null
|
| **Arguments\:**

 - k\: K

------


.. rst-class:: doc-code-block

  :kw:`public`
  has(k\: K)\: :kw:`boolean`

| The has() method returns a boolean indicating whether an element with the specified key exists in the WeakMap object or not.
|
| **Returns\:** true if k is in set
|
| **Arguments\:**

 - k\: K

------


.. rst-class:: doc-code-block

  :kw:`public`
  set(k\: K, v\: V)\: :ref:`WeakMap<LeLsLcLoLmLpLaLt.UWLeLaLkUMLaLp>`\<K, V\>

| The set() method adds a new element with a specified key and value to a WeakMap object.
|
| **Returns\:** updatesWeakMap
|
| **Arguments\:**

 - k\: K
 - v\: V

------


.. _LeLsLcLoLmLpLaLt.UWLeLaLkUSLeLt:

WeakSet\<K\>
============



.. rst-class:: doc-code-block

  :kw:`export`

| **Class\:** A WeakSet is a collection of garbage-collectable values. A value in the WeakSet may only occur once. It is unique in the WeakSet's collection.
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  add(v\: K)\: :ref:`WeakSet<LeLsLcLoLmLpLaLt.UWLeLaLkUSLeLt>`\<K\>

| The add() method appends a new object to the end of a WeakSet object.
|
| **Returns\:** new WeakSet with v
|
| **Arguments\:**

 - v\: K

------


.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| The WeakSet() constructor creates WeakSet objects.
|

------


.. rst-class:: doc-code-block

  :kw:`public`
  delete(v\: K)\: :kw:`boolean`

| The delete() method removes the specified element from a WeakSet object.
|
| **Returns\:** boolean
|
| **Arguments\:**

 - v\: K

------


.. rst-class:: doc-code-block

  :kw:`public`
  has(v\: K)\: :kw:`boolean`

| The has() method returns a boolean indicating whether an object exists in a WeakSet or not.
|
| **Returns\:** true if set has v
|
| **Arguments\:**

 - v\: K


.. _LeLsLcLoLmLpLaLt.UWLeLaLkUKLeLy:

WeakKey
=======

.. rst-class:: doc-code-block

  :kw:`export`

| Represents WeakKey
|

Methods
-------



.. rst-class:: doc-code-block

  :kw:`public`
  constructor()\: :kw:`void`

| Constructs a new WeakKey
|

------


.. rst-class:: doc-code-block

  :kw:`public` :kw:`override`
  toString()\: :ref:`string<LsLtLd.LcLoLrLe.USLtLrLiLnLg>`

| Converts string representation
|

------
