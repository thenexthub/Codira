..
    Copyright (c) 2021-2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Lexical Elements:

Lexical Elements
################

.. meta:
    frontend_status: Done

This chapter discusses the lexical structure of the |LANG| programming language.

|

.. _Unicode Characters:

Use of Unicode Characters
*************************

.. meta:
    frontend_status: Done

The |LANG| programming language uses characters of the Unicode Character
set [1]_ as its terminal symbols. It uses the Unicode UTF-16 encoding to
represent text in sequences of 16-bit code units.

The term *Unicode code point* is used in this specification only where such
representation is relevant to refer the reader to Unicode Character set and
UTF-16 encoding. Where such representation is irrelevant to the discussion,
the generic term *character* is used.

.. index::
   terminal symbol
   character
   Unicode character
   Unicode code point

|

.. _Lexical Input Elements:

Lexical Input Elements
**********************

.. meta:
    frontend_status: Done

The language has the following types of *lexical input elements*:

-  :ref:`White Spaces`,
-  :ref:`Line Separators`,
-  :ref:`Tokens`, and
-  :ref:`Comments`.

.. index::
   white space
   line separator
   token
   comment
   lexical input

|

.. _White Spaces:

White Spaces
************

.. meta:
    frontend_status: Done

*White spaces* are lexical input elements that separate tokens from one another.
White spaces include the following:

- Space (U+0020),

- Horizontal tabulation (U+0009),

- Vertical tabulation (U+000B),

- Form feed (U+000C),

- No-break space (U+00A0), and

- Zero-width no-break space (U+FEFF).

White spaces improve source code readability and help avoiding ambiguities.
White spaces are ignored by the syntactic grammar (see :ref:`Grammar Summary`).
White spaces never occur within a single token, but can occur within a comment.

.. index::
   lexical input
   source code
   white space
   syntactic grammar
   comment
   token
   space
   horizontal tabulation
   form feed
   no-break space
   zero-width no-break space

|

.. _Line Separators:

Line Separators
***************

.. meta:
    frontend_status: Done

*Line separators* are lexical input elements that separate tokens from one
another and divide sequences of Unicode input characters into lines.
Line separators include the following:

- Newline character (U+000A or ASCII <LF>),

- Carriage return character (U+000D or ASCII <CR>),

- Line separator character (U+2028 or ASCII <LS>), and

- Paragraph separator character (U+2029 or ASCII <PS>).

Line separators improve source code readability. Any sequence of line
separators is considered a single separator.

.. index::
   lexical input
   newline character
   carriage return character
   line separator character
   paragraph separator character

Line separators are often treated as white spaces, except where line
separators have special meanings. See :ref:`Semicolons` for more details.

|

.. _Tokens:

Tokens
******

.. meta:
    frontend_status: Done

Tokens form the vocabulary of the language. There are four classes of tokens:

-  :ref:`Identifiers`,
-  :ref:`Keywords`,
-  :ref:`Operators and Punctuators`, and
-  :ref:`Literals`.

*Token* is the only lexical input element that can act as a terminal symbol
of the syntactic grammar (see :ref:`Grammar Summary`). In the process of
tokenization, the next token is always the longest sequence of characters that
form a valid token. Tokens are separated by white spaces (see
:ref:`White spaces`), operators, or punctuators (see
:ref:`Operators and Punctuators`). White spaces are ignored by the syntactic
grammar (see :ref:`Grammar Summary`).

.. index::
   line separator
   lexical input element
   Unicode input character
   token
   tokenization
   white space
   source code
   identifier
   keyword
   operator
   punctuator
   literal
   terminal symbol
   syntactic grammar

|

.. _Identifiers:

Identifiers
***********

.. meta:
    frontend_status: Done

*Identifier* is a sequence of one or more valid Unicode characters. The
Unicode grammar of identifiers is based on character properties
specified by the Unicode Standard.

The first character in an identifier must be ``'$'``, ``'_'``, or any Unicode
code point with the Unicode property 'ID_Start'[2]_. Other characters
must be Unicode code points with the Unicode property or one of the following
characters:

-  ``'$'`` (\\U+0024),
-  ``'``Zero-Width Non-Joiner``'`` (``<ZWNJ>``, \\U+200C), or
-  ``'``Zero-Width Joiner``'`` (``<ZWJ>``, \\U+200D).

.. index::
   identifier
   Unicode Standard
   identifier
   Unicode code point
   Unicode character
   zero-width joiner
   zero-width non-joiner

.. code-block:: abnf

    Identifier:
      IdentifierStart IdentifierPart*
      ;

    IdentifierStart:
      UnicodeIDStart
      | '$'
      | '_'
      | '\\' EscapeSequence
      ;

    IdentifierPart:
      UnicodeIDContinue
      | '$'
      | ZWNJ
      | ZWJ
      | '\\' EscapeSequence
      ;

    ZWJ:
     '\u200C'
    ;
    ZWNJ:
     '\u200D'
    ;

    UnicodeIDStart
      : Letter
      | ['$']
      | '\\' UnicodeEscapeSequence;

    UnicodeIDContinue
      : UnicodeIDStart
      | UnicodeDigit
      | '\u200C'
      | '\u200D';

    UnicodeEscapeSequence:
      'u' HexDigit HexDigit HexDigit HexDigit
      | 'u' '{' HexDigit HexDigit+ '}'
      ;

    Letter
      : UNICODE_CLASS_LU
      | UNICODE_CLASS_LL
      | UNICODE_CLASS_LT
      | UNICODE_CLASS_LM
      | UNICODE_CLASS_LO
      ;
    UnicodeDigit
      : UNICODE_CLASS_ND
      ;

See :ref:`Grammar Summary` for the Unicode character categories *UNICODE_CLASS_LU*,
*UNICODE_CLASS_LL*, *UNICODE_CLASS_LT*, *UNICODE_CLASS_LM*, *UNICODE_CLASS_LO*,
and *UNICODE_CLASS_ND*.

|

.. _Keywords:

Keywords
********

.. meta:
    frontend_status: Done

*Keywords* are reserved words with meanings permanently predefined in |LANG|.
Keywords are case-sensitive, and their exact spelling is presented in the
four tables below. The kinds of keywords are discussed below.

1. The following *hard keywords* are reserved in any context, and
cannot be used as identifiers:

.. index::
   keyword
   reserved word
   hard keyword
   soft keyword
   identifier
   context

+--------------------+-------------------+------------------+------------------+
|                    |                   |                  |                  |
+====================+===================+==================+==================+
|   ``abstract``     |   ``enum``        |   ``let``        |   ``this``       |
+--------------------+-------------------+------------------+------------------+
|   ``as``           |   ``export``      |   ``native``     |   ``throw``      |
+--------------------+-------------------+------------------+------------------+
|   ``async``        |   ``extends``     |   ``new``        |   ``true``       |
+--------------------+-------------------+------------------+------------------+
|   ``await``        |   ``false``       |   ``null``       |   ``try``        |
+--------------------+-------------------+------------------+------------------+
|   ``break``        |   ``final``       |   ``overload``   |   ``typeof``     |
+--------------------+-------------------+------------------+------------------+
|   ``case``         |   ``for``         |   ``override``   |   ``undefined``  |
+--------------------+-------------------+------------------+------------------+
|   ``class``        |   ``function``    |   ``private``    |   ``while``      |
+--------------------+-------------------+------------------+------------------+
|   ``const``        |   ``if``          |   ``protected``  |                  |
+--------------------+-------------------+------------------+------------------+
|   ``constructor``  |   ``implements``  |   ``public``     |                  |
+--------------------+-------------------+------------------+------------------+
|   ``continue``     |   ``import``      |   ``return``     |                  |
+--------------------+-------------------+------------------+------------------+
|   ``default``      |   ``in``          |   ``static``     |                  |
+--------------------+-------------------+------------------+------------------+
|   ``do``           |   ``instanceof``  |   ``switch``     |                  |
+--------------------+-------------------+------------------+------------------+
|   ``else``         |   ``interface``   |   ``super``      |                  |
+--------------------+-------------------+------------------+------------------+


.. +--------------------+-------------------+------------------+------------------+
   |                    |                   |                  |                  |
   +====================+===================+==================+==================+
   |   ``abstract``     |   ``enum``        |   ``let``        |   ``super``      |
   +--------------------+-------------------+------------------+------------------+
   |   ``as``           |   ``export``      |   ``native``     |   ``this``       |
   +--------------------+-------------------+------------------+------------------+
   |   ``async``        |   ``extends``     |   ``new``        |   ``throw``      |
   +--------------------+-------------------+------------------+------------------+
   |   ``await``        |   ``false``       |   ``null``       |   ``true``       |
   +--------------------+-------------------+------------------+------------------+
   |   ``break``        |   ``final``       |   ``overload``   |   ``try``        |
   +--------------------+-------------------+------------------+------------------+
   |   ``case``         |   ``for``         |   ``override``   |   ``typeof``     |
   +--------------------+-------------------+------------------+------------------+
   |   ``class``        |   ``function``    |   ``private``    |   ``undefined``  |
   +--------------------+-------------------+------------------+------------------+
   |   ``const``        |   ``if``          |   ``protected``  |   ``while``      |
   +--------------------+-------------------+------------------+------------------+
   |   ``constructor``  |   ``implements``  |   ``public``     |                  |
   +--------------------+-------------------+------------------+------------------+
   |   ``continue``     |   ``import``      |   ``return``     |                  |
   +--------------------+-------------------+------------------+------------------+
   |   ``default``      |   ``in``          |   ``sealed``     |                  |
   +--------------------+-------------------+------------------+------------------+
   |   ``do``           |   ``instanceof``  |   ``static``     |                  |
   +--------------------+-------------------+------------------+------------------+
   |   ``else``         |   ``interface``   |   ``switch``     |                  |
   +--------------------+-------------------+------------------+------------------+


|

2. Names and aliases of predefined types are *hard keywords*. They cannot be
used as identifiers:

+---------------+---------------+
| Primary Name  | Alias         |
+===============+===============+
| ``Any``       |               |
+---------------+---------------+
| ``bigint``    | ``BigInt``    |
+---------------+---------------+
| ``boolean``   | ``Boolean``   |
+---------------+---------------+
| ``byte``      | ``Byte``      |
+---------------+---------------+
| ``char``      | ``Char``      |
+---------------+---------------+
| ``double``    | ``Double``    |
+---------------+---------------+
| ``float``     | ``Float``     |
+---------------+---------------+
| ``int``       | ``Int``       |
+---------------+---------------+
| ``long``      | ``Long``      |
+---------------+---------------+
| ``number``    | ``Number``    |
+---------------+---------------+
| ``Object``    | ``object``    |
+---------------+---------------+
| ``short``     | ``Short``     |
+---------------+---------------+
| ``string``    | ``String``    |
+---------------+---------------+
| ``void``      |               |
+---------------+---------------+

3. The following *soft keywords* have special meaning in certain contexts but
are valid identifiers elsewhere:

.. index::
   keyword
   soft keyword
   identifier

+--------------------+--------------------+
|                    |                    |
+====================+====================+
|      ``catch``     |     ``namespace``  |
+--------------------+--------------------+
|      ``declare``   |     ``of``         |
+--------------------+--------------------+
|      ``finally``   |     ``out``        |
+--------------------+--------------------+
|      ``from``      |    ``readonly``    |
+--------------------+--------------------+
|      ``get``       |    ``set``         |
+--------------------+--------------------+
|      ``keyof``     |    ``type``        |
+--------------------+--------------------+

4. The following identifiers are also treated as *soft keywords* reserved for
the future use or currently used in |TS|:

.. index::
   identifier
   soft keyword

+---------------+---------------+---------------+---------------+----------------+
|               |               |               |               |                |
+===============+===============+===============+===============+================+
|    ``is``     |   ``memo``    |   ``struct``  |    ``var``    |  ``yield``     |
+---------------+---------------+---------------+---------------+----------------+


|

.. _Operators and Punctuators:

Operators and Punctuators
*************************

.. meta:
    frontend_status: Done

*Operators* are tokens that denote various actions to be performed on values:
addition, subtraction, comparison, and other. The keywords ``instanceof`` and
``typeof`` also act as operators.

*Punctuators* are tokens that separate, complete, or otherwise organize program
elements and parts: commas, semicolons, parentheses, square brackets, etc.

The following character sequences represent operators and punctuators:

.. index::
   operator
   token
   value
   addition
   subtraction
   comparison
   punctuator
   semicolon
   parentheses
   comma
   square bracket
   keyword

+----------+----------+----------+----------+----------+----------+----------+
+----------+----------+----------+----------+----------+----------+----------+
|  ``+``   |  ``&``   |  ``+=``  | ``|=``   | ``&=``   |  ``<``   |  ``?.``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``-``   |  ``|``   |  ``-=``  | ``^=``   | ``&&``   |  ``>``   |  ``!``   |
+----------+----------+----------+----------+----------+----------+----------+
|  ``*``   |  ``^``   |  ``*=``  | ``<<=``  | ``||``   |  ``===`` |  ``<=``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``/``   |  ``>>``  |  ``/=``  | ``>>=``  | ``++``   |  ``==``  |  ``>=``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``%``   |  ``<<``  |  ``%=``  | ``>>>=`` | ``--``   |  ``=``   |  ``...`` |
+----------+----------+----------+----------+----------+----------+----------+
|  ``(``   |  ``)``   |  ``[``   | ``]``    | ``{``    |  ``}``   |  ``??``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``,``   |  ``;``   |  ``.``   | ``:``    | ``!=``   |  ``!==`` |  ``**``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``**=`` |  ``&&=`` |  ``||=`` | ``??=``  |          |          |          |
+----------+----------+----------+----------+----------+----------+----------+

|

.. _Literals:

Literals
********

.. meta:
    frontend_status: Done

*Literals* are values of certain types (see :ref:`Predefined Types` and
:ref:`Literal Types`).

.. code-block:: abnf

    Literal:
      IntegerLiteral
      | FloatLiteral
      | BigIntLiteral
      | BooleanLiteral
      | StringLiteral
      | MultilineStringLiteral
      | RegExpLiteral
      | NullLiteral
      | UndefinedLiteral
      | CharLiteral
      ;

Each literal is described in detail below. The experimental ``char literal``
is discussed in :ref:`Character Literals`.

.. index::
   literal
   char

|

.. _Numeric Literals:

Numeric Literals
================

.. meta:
    frontend_status: Done

*Numeric literals* include integer and floating-point literals.

|

.. _Integer Literals:

Integer Literals
================

.. meta:
    frontend_status: Done

Integer literals represent numbers that have neither a decimal point nor
an exponential part. Integer literals can be written with radices 16
(hexadecimal), 10 (decimal), 8 (octal), and 2 (binary) as follows:

.. index::
   integer
   literal
   hexadecimal
   decimal
   octal
   binary
   radix

.. code-block:: abnf

    IntegerLiteral:
      DecimalIntegerLiteral
      | HexIntegerLiteral
      | OctalIntegerLiteral
      | BinaryIntegerLiteral
      ;

    DecimalIntegerLiteral:
      '0'
      | DecimalDigitNotZero ('_'? DecimalDigit)*
      ;

    DecimalDigit:
      [0-9]
      ;

    DecimalDigitNotZero:
      [1-9]
      ;

    HexIntegerLiteral:
      '0' [xX]  ( HexDigit
      | HexDigit (HexDigit | '_')* HexDigit
      )
      ;

    HexDigit:
      [0-9a-fA-F]
      ;

    OctalIntegerLiteral:
      '0' [oO] ( OctalDigit
      | OctalDigit (OctalDigit | '_')* OctalDigit )
      ;

    OctalDigit:
      [0-7]
      ;

    BinaryIntegerLiteral:
      '0' [bB] ( BinaryDigit
      | BinaryDigit (BinaryDigit | '_')* BinaryDigit )
      ;

    BinaryDigit:
      [0-1]
      ;

Integral literals with different radices are represented by the examples below:

.. code-block:: typescript
   :linenos:

    153 // decimal literal
    1_153 // decimal literal
    0xBAD3 // hex literal
    0xBAD_3 // hex literal
    0o777 // octal literal
    0b101 // binary literal

The underscore character ``'_'`` between successive
digits can be used to improve readability.
Underscore characters in such positions do not change the values of literals.
However, the underscore character must be neither the very first nor the very
last symbol of an integer literal.

.. index::
   prefix
   value
   literal
   integer
   underscore character

Type of integer literal is determined by using
:ref:`Type Inference for Numeric Literals` if its context allows inferring type.
Otherwise, the type is determined as follows:

- ``int`` if the literal value can be represented
  by a non-negative 32-bit number, i.e., the value is in the
  range 0..max(int); or

- ``long`` otherwise.

A :index:`compile-time error` occurs if an integer literal value is too
large for the values of type ``long``. The concept is represented by the
examples below:

.. code-block:: typescript
   :linenos:

    // literals of type int:
    0
    1
    0x7F
    0x7FFF_FFFF // max(int)

    // literals of type long:
    0x8000_0000
    0x7FFF_FFFF_1
    9223372036854775807 // max(long)

    // compile-time error as value is too large:
    9223372036854775808 // max(long) + 1
    0xFFFF_FFFF_FFFF_FFFF_0

.. index::
   integer literal
   int
   long

|

.. _Floating-Point Literals:

Floating-Point Literals
=======================

.. meta:
    frontend_status: Done

*Floating-point literals* represent decimal numbers and consist of a
whole-number part, a decimal point, a fraction part, an exponent, and
a ``float`` type suffix as follows:

.. code-block:: abnf

    FloatLiteral:
        DecimalIntegerLiteral '.' FractionalPart? ExponentPart? FloatTypeSuffix?
        | '.' FractionalPart ExponentPart? FloatTypeSuffix?
        | DecimalIntegerLiteral ExponentPart? FloatTypeSuffix
        ;

    ExponentPart:
        [eE] [+-]? DecimalIntegerLiteral
        ;

    FractionalPart:
        DecimalDigit
        | DecimalDigit (DecimalDigit | '_')* DecimalDigit
        ;
    FloatTypeSuffix:
        'f'
        ;

The concept is represented by the examples below:

.. code-block:: typescript
   :linenos:

    3.14
    3.14f
    3.141_592
    .5
    1234f
    1e10
    1e10f

The underscore character ``'_'`` between successive
digits can be used to improve readability.
Underscore characters in such positions do not change the values of literals.
However, the underscore character must be neither the very first nor the very
last symbol of a literal.

Floating-point literals are of floating-point types that match literals as
follows:

- ``float`` if *float type suffix* is present; or
- ``float`` or ``double`` that is inferred using
  :ref:`Type Inference for Numeric Literals`
  if its context allows to infer type; or
- ``double`` otherwise (type ``number`` is an alias to ``double``).
    
A :index:`compile-time error` occurs if a floating-point literal is
too large for its type:

.. code-block:: typescript
   :linenos:

    // compile-time error as value is too large for type float:
    3.4e39f

    // compile-time error as value is too large for type double:
    1.7e309

.. index::
   floating-point literal
   compile-time error
   prefix
   underscore character
   implicit conversion
   constant declaration
   decimal number
   radix
   readability

|

.. _Bigint Literals:

Bigint Literals
===============

.. meta:
    frontend_status: Partly
    todo: hex, octal, binary literals

*Bigint literals* represent integer numbers with an unlimited number of digits.

*Bigint literals* are always of type ``bigint`` (see :ref:`Type bigint`).

A ``bigint`` literal is an *integer literal* followed by the symbol ``'n'``:

.. code-block:: abnf

    BigIntLiteral: IntegerLiteral 'n';

The concept is represented by the examples below:

.. code-block:: typescript

    153n // bigint literal
    1_153n // bigint literal
    -153n // negative bigint literal
    0xBAD_3n // bigint literal in hexadecimal notation

The underscore character ``'_'`` between successive digits can be used to
improve readability. Underscore characters in such positions do not change
the values of literals. However, the underscore character must be neither
the very first nor the very last symbol of a ``bigint`` literal.

Strings that represent numbers or any integer value can be converted to
``bigint`` by using built-in functions as follows:

.. code-block-meta:
    skip

.. code-block:: typescript

    BigInt(other: string): bigint
    BigInt(other: long): bigint

.. index::
   integer
   bigint literal
   underscore character
   readability
   string
   number
   integer value

Two methods allow taking *bitsCount* lower bits of a ``bigint`` number and
return them as a result. Signed and unsigned versions are both possible as
follows:

.. code-block:: typescript

    asIntN(bitsCount: long, bigIntToCut: bigint): bigint
    asUintN(bitsCount: long, bigIntToCut: bigint): bigint

.. index::
   decimal
   radix

|

.. _Boolean Literals:

Boolean Literals
================

.. meta:
    frontend_status: Done

The two *boolean literal* values are represented by the keywords ``true`` and
``false``.

.. code-block:: abnf

    BooleanLiteral:
        'true' | 'false'
        ;

*Boolean literals* are of the ``boolean`` type.

.. index::
   keyword
   Boolean literal
   literal value
   literal

|

.. _String Literals:

String Literals
===============

.. meta:
    frontend_status: Done
    todo: "" sample is invalid: SyntaxError: Newline is not allowed in strings

*String literals* comprize zero or more characters enclosed between
single or double quotes. A *multiline string* literal (see
:ref:`Multiline String Literal`) is a special form of a string literal.

*String literals* are of the literal type that corresponds to the literal.
If an operator is applied to the literal, then the literal type is replaced
for ``string`` (see :ref:`Type string`).

.. index::
   string literal
   multiline string
   predefined reference type

.. code-block:: abnf

    StringLiteral:
        '"' DoubleQuoteCharacter* '"'
        | '\'' SingleQuoteCharacter* '\''
        ;

    DoubleQuoteCharacter:
        ~["\\\r\n]
        | '\\' EscapeSequence
        ;

    SingleQuoteCharacter:
        ~['\\\r\n]
        | '\\' EscapeSequence
        ;

    EscapeSequence:
        ['"bfnrtv0\\]
        | 'x' HexDigit HexDigit
        | 'u' HexDigit HexDigit HexDigit HexDigit
        | 'u' '{' HexDigit+ '}'
        | ~[1-9xu\r\n]
        ;

Characters in *string literals* normally represent themselves. However,
certain non-graphic characters can be represented by explicit specifications
or Unicode codes. Such constructs are called *escape sequences*.

Escape sequences can represent graphic characters within a *string literal*,
e.g., single quotes (``'``), double quotes (``"``), backslashes (``\``),
and more. An escape sequence always starts with the backslash character
``'\'``, followed by one of the following characters:

.. index::
   string literal
   escape sequence
   backslash
   single quote
   double quotes

-  ``"`` (double quote, U+0022),

.. ” "

-  ``'`` (neutral single quote, U+0027),

.. ’ U+2019

-  ``b`` (backspace, U+0008),

-  ``f`` (form feed, U+000c),

-  ``n`` (linefeed, U+000a),

-  ``r`` (carriage return, U+000d),

-  ``t`` (horizontal tab, U+0009),

-  ``v`` (vertical tab, U+000b),

-  ``\`` (backslash, U+005c),

-  ``x`` and two hexadecimal digits (like ``7F``),

-  ``u`` and four hexadecimal digits (forming a fixed Unicode escape
   sequence like ``\u005c``),

-  ``u{`` and at least one hexadecimal digit followed by ``}`` (forming
   a bounded Unicode escape sequence like ``\u{5c}``), and

-  any single character except digits from '1' to '9', and characters ``'x'``,
   ``'u'``, ``'CR'``, and ``'LF'``.

.. index::
   string literal
   escape sequence
   backslash
   horizontal tab
   form feed
   backspace
   vertical tab
   hexadecimal
   Unicode escape sequence

The examples are provided below:

.. code-block:: typescript
   :linenos:

    let s1 = 'Hello, world!'
    let s2 = "Hello, world!"
    let s3 = "\\"
    let s4 = ""
    let s5 = "don’t worry, be happy"
    let s6 = 'don\'t worry, be happy'
    let s7 = 'don\u0027t worry, be happy'

|

.. _Multiline String Literal:

Multiline String Literal
========================

.. meta:
    frontend_status: Done

*Multiline strings* can contain arbitrary text enclosed between backtick
characters ``'`` \` ``'``. Multiline strings can contain any character but
the backlash ``'\'``. Multiline strings can contain newline characters:

.. index::
   string literal
   multiline string literal
   multiline string
   string interpolation
   multiline string
   backtick
   escape
   newline
   character

.. code-block:: abnf

    MultilineStringLiteral:
        '`' (BacktickCharacter)* '`'
        ;

    BacktickCharacter:
        ~['\\\r\n]
        | '\\' EscapeSequence
        | LineContinuation
        ;

     LineContinuation:
        '\\' [\r\n\u2028\u2029]+
        ;

The grammar of *embeddedExpression* is described in
:ref:`String Interpolation Expressions`.

A multiline string is represented in the example below:

.. code-block:: typescript
   :linenos:

    let sentence = `This is an example of
                    a multiline string,
                    which should be enclosed in
                    backticks`

*MultilineString* literals are of literal types corresponding to literals.
If an operator is applied to a literal, then the literal type is replaced for
``string`` (see :ref:`Type String`).

.. index::
   multiline string
   operator
   literal
   literal type

|

.. _Regex Literal:

Regex Literal
=============

.. meta:
    frontend_status: None

*Regex literals* can contain the mandatory regex part and optional flags:

.. index::
   regex literal
   optional flag

.. code-block:: abnf

    RegexLiteral:
        '/' RegexCharSequence '$'? '/' RegExFlags?
        ;

    RegexCharSequence:
        (
            RegexCharacter
            |RegexSpecialForms
            |'(' RegexSpecialForms ')'
            |'(' '?<' Identifier '>' RegexSpecialForms ')'
            |'(' '?:' RegexSpecialForms ')'
        )+
        ;

    RegexCharacter:
        ~["'\\\r\n] 
        ('*'|'+'|'?'|('{' DecimalIntegerLiteral (',' DecimalIntegerLiteral? )? '}'))?
        ;

    RegexSpecialForms:   
        CharacterClass ('(' '?='|'?!' CharacterClass ')')? 
        ('(' '?<='|'?<!' CharacterClass ')') CharacterClass
        ;

    CharacterClass: 
        '[' '^'? '\b'? (RegexCharacter | (RegexCharacter '-' RegexCharacter) '\B'?)+ '\b'? ']'
        | '.'
        | '\'
        ('d' | 'D' | 'w' | 'W' | 's' | 'S' | 't' | 'r' | 'n' | 'v' | 'f' | '0' 
         | 'c' ['A'-'Z'] | 'x' DecimalDigit DecimalDigit 
         | DecimalIntegerLiteral | 'k<' Identifier '>'
        )
        | 'u' HexDigit HexDigit HexDigit HexDigit 
        | 'u{' HexDigit HexDigit HexDigit HexDigit HexDigit? '}'
        | '[\b]' 
        | (RegexCharacter '|' RegexCharacter)
        ;

    RegExFlags:
        'g'? 'i'? 'm'? 's'? 'u'? 'v'? 'y'? 
        ;

Regex flags can be put in any order. However, a duplication of a regex flag
causes a :index:`compile-time error`.

*Regex literals* are represented in the example below:

.. code-block:: typescript
   :linenos:

    let regex1 = /abc/ 
    let regex2 = /ab+c/gi

*Regex literal* is semantically equivalent to the creation of an object of
*RegExp* type that is a part of the :ref:`Standard Library`.

.. code-block:: typescript
   :linenos:

    let regex1 = /abc/ 
    let regex2 = new RegExp ("abc")

The semantics of *regex literals* supported by |LANG| and of regular expressions
in |JS| are the same.

|

.. _Null Literal:

``Null`` Literal
================

.. meta:
    frontend_status: Done

*Null literal* is the only literal of type ``null`` (see :ref:`Type null`) to
denote a reference without pointing at any entity. The null literal is
represented by the keyword ``null``:

.. code-block:: abnf

    NullLiteral:
        'null'
        ;

The value is typically used for types like ``T | null``
(see :ref:`Nullish Types`).

.. index::
   null literal
   null reference
   nullish type
   type null

|

.. _Undefined Literal:

``Undefined`` Literal
=====================

.. meta:
    frontend_status: Done

*Undefined literal* is the only literal of types ``void`` and ``undefined``
(see :ref:`Types void or undefined`) to denote a reference with a value that is
not defined. The undefined literal is represented by the keyword ``undefined``:

.. code-block:: abnf

    UndefinedLiteral:
        'undefined'
        ;

.. index::
   undefined literal
   type undefined
   type void
   keyword

|

.. _Comments:

Comments
********

.. meta:
    frontend_status: Done

*Comment* is a piece of text added in the stream to document and compliment
the source code. Comments are insignificant for the syntactic grammar (see
:ref:`Grammar Summary`).

*Line comments* begin with the sequence of characters ``'//'`` as in the
example below, and end with the line separator character. Any character
or sequence of characters between them is allowed but ignored.

.. code-block:: typescript
   :linenos:

    // This is a line comment

*Multiline comments* begin with the sequence of characters ``'\*'`` as
in the example below, and end with the first subsequent sequence of characters
``'*/'``. Any character or sequence of characters between them is allowed but
ignored.

.. code-block:: typescript
   :linenos:

    /*
        This is a multiline comment
    */

Comments cannot be nested.

.. index::
   comment
   syntactic grammar
   multiline comment

|

.. _Semicolons:

Semicolons
**********

.. meta:
    frontend_status: Done

Declarations and statements are usually terminated by a line separator (see
:ref:`Line Separators`). A semicolon must be used in some cases to separate
syntax productions written in one line, or to avoid ambiguity.

.. index::
   declaration
   statement
   line separator
   syntax production
   semicolon

.. code-block:: typescript
   :linenos:

    function foo(x: number): number {
        x++;
        x *= x;
        return x
    }

    let i = 1
    i-i++ // one expression
    i;-i++ // two expressions

-------------

.. [1]
   Unicode Standard Version 15.0.0,
   https://www.unicode.org/versions/Unicode15.0.0/

.. [2]
   https://unicode.org/reports/tr31/

.. raw:: pdf

   PageBreak
