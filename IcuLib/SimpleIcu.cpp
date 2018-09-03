#include "SimpleIcu.h"
#include "unicode/unistr.h"
#include "unicode/ustring.h"

namespace icux {
    
    std::string toUtf8(const wchar_t* utf16Str, int32_t length)
    {
        int32_t bufferSize = length*4+1;
        int32_t resultSize = 0;
        UErrorCode errorCode = UErrorCode::U_ZERO_ERROR;
        const UChar* src = (const UChar*)utf16Str;
        std::unique_ptr<char> buffer(new char[bufferSize]);
        u_strToUTF8(buffer.get(), bufferSize, &resultSize, src, length, &errorCode);
        return std::string(buffer.get(), resultSize);
    }

    std::string toUtf8(const std::string &utf8Str) // Simply returns the same string
    {
        return utf8Str;
    }

    std::string toUtf8(const std::wstring &utf16Str)
    {
        return toUtf8(utf16Str.c_str(), utf16Str.size());
    }

    std::wstring toUtf16(const char* utf8Str, int32_t length)
    {
        int32_t bufferSize = length*2+1;
        int32_t resultSize = 0;
        UErrorCode errorCode = UErrorCode::U_ZERO_ERROR;
        std::unique_ptr<UChar> buffer(new UChar[bufferSize]);
        u_strFromUTF8(buffer.get(), bufferSize, &resultSize, utf8Str, length, &errorCode);
        const wchar_t* result = (wchar_t*)buffer.get();
        return std::wstring(result, resultSize);
    }

    std::wstring toUtf16(const std::string &utf8Str)
    {
        return toUtf16(utf8Str.c_str(), utf8Str.size());
    }

    std::wstring toUtf16(const std::wstring &utf16Str) // Simply returns the same string
    {
        return utf16Str;
    }

    filestring toFilestring(const char* utf8Str, int32_t length)
    {
        #ifdef UTF16_FILESYSTEM
            return (filestring)toUtf16(utf8Str, length);
        #else
            return filestring(utf8Str, length);
        #endif
    }

    filestring toFilestring(const std::string &utf8Str)
    {
        #ifdef UTF16_FILESYSTEM
            return toUtf16(utf8Str);
        #else
            return filestring(utf8Str);
        #endif
    }

    uistring toUistring(const char* utf8Str, int32_t length)
    {
        #ifdef UTF16_UI
            return toUtf16(utf8Str, length);
        #else
            return uistring(utf8Str);
        #endif
    }

    uistring toUistring(const std::string &utf8Str)
    {
        #ifdef UTF16_UI
            return toUtf16(utf8Str);
        #else
            return uistring(utf8Str);
        #endif
    }

#ifdef UTF16_UI
    uistring toUistring(const filestring &filestring)
    {
        #ifdef UTF16_FILESYSTEM
            #ifdef UTF16_UI
                return uistring(filestring);
            #else
                return toUtf8(filestring);
            #endif
        #else
            #ifdef UTF16_UI
                return toUtf16(filestring);
            #else
                return uistring(filestring);
            #endif
        #endif
    }
#endif

}
