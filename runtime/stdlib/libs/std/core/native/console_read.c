/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdlib.h>
#include <unistd.h>
#include "securec.h"

#ifdef WIN32
#include <windows.h>
#include <wchar.h>
#endif  // WIN32

typedef struct StructureString {
    char* buffer;
    size_t length;
} StructureString;

#define INITIAL_BUFFER_SIZE 512
#define WIN_EOF 0x1A
#define STR_END '\0'

/**
 * @brief Reads a line of text from the console or from redirected stdin until newline or EOF.
 *
 * This function reads input from the console, storing it in a dynamically allocated
 * buffer within a `StructureString` object. If the buffer reaches its initial capacity,
 * it is automatically resized to accommodate additional characters.
 * The function stops reading when it encounters a newline character or the end of the file.
 *
 * @return StructureString*
 * - Returns a pointer to a `StructureString` containing the read input.
 * - If an error occurs (e.g., memory allocation failure) or no data is read, it returns NULL.
 */
static StructureString* ConsoleReadLine()
{
    StructureString* str = (StructureString*)malloc(sizeof(StructureString));
    if (str == NULL) {
        return NULL;
    }

    str->buffer = (char*)malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if (str->buffer == NULL) {
        free(str);
        return NULL;
    }

    size_t bufferSize = INITIAL_BUFFER_SIZE;
    str->length = 0;
    (void)memset_s(str->buffer, bufferSize * sizeof(char), 0, bufferSize * sizeof(char));

    // Read characters from the console until EOF or \n is encountered
    int c;
    while ((c = getchar()) != EOF) {
        if (str->length >= bufferSize) {
            size_t newBufferSize = bufferSize * 2;
            if (newBufferSize >= SECUREC_MEM_MAX_LEN) {
                break;
            }

            char* newBuffer = (char*)malloc(newBufferSize * sizeof(char));
            if (newBuffer == NULL) {
                str->length = 0; // Length reset to zero indicates an error
                break;
            }

            (void)memset_s(newBuffer, newBufferSize * sizeof(char), 0, newBufferSize * sizeof(char));
            if (memcpy_s(newBuffer, newBufferSize * sizeof(char), str->buffer, str->length * sizeof(char)) != EOK) {
                str->length = 0;
                free(newBuffer);
                break;
            }
            free(str->buffer);

            str->buffer = newBuffer;
            bufferSize = newBufferSize;
        }

        str->buffer[str->length++] = (char)c;
        if (c == '\n') {
            break;
        }
    }
    // If no data was read or error happend, clean up and return NULL
    if (str->length == 0) {
        free(str->buffer);
        free(str);
        return NULL;
    }

    return str;
}

#ifdef WIN32
static char* Wchar2Char(const wchar_t* wstr, DWORD size, size_t* length)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, size, NULL, 0, NULL, NULL);
    if (len == 0) {
        return NULL;
    }
    *length = len;
    len += 1;
    char* cstr = (char*)malloc(sizeof(char) * len);
    if (cstr == NULL) {
        return NULL;
    }
    (void)memset_s(cstr, len * sizeof(char), 0, len * sizeof(char));

    WideCharToMultiByte(CP_UTF8, 0, wstr, size, cstr, len, NULL, NULL);

    // cstr will be freed on cangjie side
    return cstr;
}

/**
 * @brief Reads wide character input (UTF-16) from the console and converts it to UTF-8.
 *
 * This function reads UTF-16 input from the console using the specified console handle.
 * It supports Windows environments where input is retrieved using `ReadConsoleW`.
 * The read data is stored in a buffer that grows dynamically if needed. Once the data
 * is fully read, it is converted from wide characters (UTF-16) to UTF-8 and stored in
 * a `StructureString` structure.
 *
 * @param hStdin The handle to the console input (stdin).
 *
 * @return StructureString*
 * - Returns a pointer to a `StructureString` containing the converted UTF-8 string.
 * - Returns NULL if an error occurs (e.g., memory allocation failure) or if no data is read.
 */
static StructureString* ConsoleReadWideChars(HANDLE hStdin)
{
    wchar_t* buffer = (wchar_t*)malloc(INITIAL_BUFFER_SIZE * sizeof(wchar_t));
    if (buffer == NULL) {
        return NULL;
    }
    DWORD bufferSize = INITIAL_BUFFER_SIZE;
    (void)memset_s(buffer, bufferSize * sizeof(wchar_t), 0, bufferSize * sizeof(wchar_t));
    DWORD totalReadlen = 0;
    DWORD spareSize = bufferSize;
    DWORD readlen = 0;
    char* pReadStart = buffer;

    while (ReadConsoleW(hStdin, pReadStart, spareSize, &readlen, NULL)) {
        wchar_t* eof_pos = wcschr(buffer, WIN_EOF);
        if (eof_pos != NULL) {
            *eof_pos = STR_END;
            readlen = eof_pos - buffer;
            totalReadlen += readlen;
            break;
        }
        totalReadlen += readlen;
        if (buffer[totalReadlen - 1] == L'\n') {
            break;
        }

        DWORD newBufferSize = bufferSize * 2; // 2: double the buffer

        if (newBufferSize >= SECUREC_MEM_MAX_LEN) {
            break;
        }
        bufferSize = newBufferSize;
        wchar_t* newbuffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
        if (newbuffer == NULL) {
            break;
        }

        (void)memset_s(newbuffer, bufferSize * sizeof(wchar_t), 0, bufferSize * sizeof(wchar_t));
        if (memcpy_s(newbuffer, bufferSize * sizeof(wchar_t), buffer, totalReadlen * sizeof(wchar_t)) != EOK) {
            free(newbuffer);
            break;
        }

        free(buffer);
        buffer = newbuffer;
        pReadStart = buffer + totalReadlen; // totalReadlen always less then buffer size
        spareSize = bufferSize - totalReadlen;
    }

    if (totalReadlen == 0) {
        free(buffer);
        return NULL;
    }

    size_t resultLength = 0;
    char* result = Wchar2Char(buffer, totalReadlen, &resultLength);
    // Free the buffer
    free(buffer);

    StructureString* str = (StructureString*)malloc(sizeof(StructureString));
    if (str == NULL) {
        free(result);
        return NULL;
    }
    str->buffer = result;
    str->length = resultLength;
    return str; // str and str->buffer will be freed by cangjie
}
#endif

#ifdef WIN32
/**
 * @brief Determines the input method for reading from the console or redirected file input on Windows.
 *
 * This function checks if the input handle (`STD_INPUT_HANDLE`) is associated with a console or
 * a character device. If so, it reads input as wide characters (UTF-16) using `ConsoleReadWideChars`.
 * Otherwise, it uses `ConsoleReadLine` to read from a redirected file (or non-console) input.
 *
 * This approach is typically used to handle both console input and file redirection on Windows platforms.
 *
 * @return StructureString*
 * - Returns a pointer to a `StructureString` containing the read input data in UTF-8.
 * - Returns NULL if an error occurs or no data is read.
 */
extern StructureString* CODE_CONSOLE_Readline(void)
{
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (GetFileType(hStdIn) == FILE_TYPE_CHAR) {
        // a character file, typically an LPT device or a console
        return ConsoleReadWideChars(hStdIn);
    } else {
        return ConsoleReadLine();
    }
}

extern void CODE_CONSOLE_ClearstdInerr(void)
{
    // do nothing in windows system
    return;
}

#else
// not WIN32
extern StructureString* CODE_CONSOLE_Readline(void)
{
    return ConsoleReadLine();
}

extern void CODE_CONSOLE_ClearstdInerr(void)
{
    clearerr(stdin);
}
#endif // WIN32
