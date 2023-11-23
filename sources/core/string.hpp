#ifndef STRAITX_STRING_HPP
#define STRAITX_STRING_HPP

#include <string>
#include "core/types.hpp"
#include "core/move.hpp"
#include "core/noncopyable.hpp"
#include "core/string_view.hpp"
#include "core/os/memory.hpp"
#include "core/unicode.hpp"

class String: public std::string{
private:
    static const char* s_Empty;

    using ImplStringClass = std::string;
public:
    String() = default;

    String(const char *string, size_t size) :
        ImplStringClass(string, size)
    {}

    String(char ch, size_t size) :
        ImplStringClass(ch, size)
    {}

    String(ImplStringClass&& string) {
        Assign(Move(string));
    }

    String(const ImplStringClass& string) {
        Assign(string);
    }

    String(const char *string):
        String(string, Length(string))
    {}

    String(StringView view):
        String(view.Data(), view.Size())
    {}

    String(ConstSpan<char> span):
        String(span.Pointer(), span.Size())
    {}

    String(String&& other)noexcept{
        *this = Move(other);
    }

    String(const String& other):
        String(other.Data(), other.Size())
    {}

    String& operator=(String&& other)noexcept {
        return Assign(Move(other));
    }

    String& operator=(const String& other)noexcept {
        return Assign(other);
    }

    String& operator=(ImplStringClass&& other)noexcept {
        return Assign(Move(other));
    }

    String& operator=(const ImplStringClass& other)noexcept {
        return Assign(other);
    }
    
    template<typename StringType>
    String& Assign(StringType &&other)noexcept {
        ImplStringClass::assign(Forward<StringType>(other));
        return *this;
    }

    StringView View()const {
        return {Data(), Size()};
    }

    void Resize(size_t size) {
        ImplStringClass::resize(size);
    }

    void Append(const String &string) {
        Append(string.View());
    }

    void Append(StringView string) {
        ImplStringClass::append(string.Data(), string.Size());
    }

    void Clear() {
        ImplStringClass::clear();
    }
    
    char* Data(){
        return &ImplStringClass::operator[](0);
    }

    const char* Data()const{
        return ImplStringClass::data();
    }

    size_t Size()const {
        return ImplStringClass::size();
    }

    bool IsEmpty()const {
        return !Size();
    }

    size_t CodeunitsCount()const {
        return Size();
    }

    size_t CodepointsCount()const {
        size_t counter = 0;
        for (u32 ch : *this)
            counter++;
        return counter;
    }

    operator ConstSpan<char>()const {
        return { Data(), Size() };
    }

    UnicodeIterator begin()const {
        return { Data() };
    }

    UnicodeIterator end()const {
        return { Data() + Size() };
    }
    
    //XXX: Do something about this
    static bool Contains(const char *string, const char *internal);

    static bool Contains(const char *string, size_t limit, const char *internal);

    static void ToUpperCase(char *string);

    static void ToLowerCase(char *string);
    // nul character is also a character
    static size_t Length(const char *string);
    //nil or \n characters are included
    static size_t LineLength(const char *string);

    static s32 Compare(const char *first, const char *second);

    static bool Equals(const char *first, const char *second);

    static const char *Find(const char *string, const char *internal);

    static char *Find(char *string, const char *internal);

    static const char *Find(const char *string, size_t limit, const char *internal);

    static char *Find(char *string, size_t limit, const char *internal);

    static const char *FindLast(const char *string, const char *internal);

    static char *FindLast(char *string, const char *internal);

    static const char *IgnoreUntil(const char *string, char ch);

    static char *IgnoreUntil(char *string, char ch);

    static const char *Ignore(const char *string, char ch);

    static char *Ignore(char *string, char ch);
};

SX_INLINE String operator+(const StringView& lvalue, const StringView &rvalue) {
    String sum(lvalue.Size() + rvalue.Size());

    Memory::Copy(lvalue.Data(), sum.Data()                , lvalue.Size());
    Memory::Copy(rvalue.Data(), sum.Data() + lvalue.Size(), rvalue.Size());

    return sum;
}

SX_INLINE String operator+(const char* lvalue, const String& rvalue) {
    return StringView(lvalue) + (const StringView &)rvalue;
}

SX_INLINE String operator+(const String& lvalue, const char *rvalue) {
    return (const StringView&)lvalue + StringView(rvalue);
}

template<>
struct Printer<String> {
	static void Print(const String& value, StringWriter &writer) {
        writer.Write(value.Data(), value.Size());
	}
};

SX_INLINE bool String::Contains(const char *string, const char *internal){
    return Find(string,internal);
}

SX_INLINE bool String::Contains(const char *string, size_t limit, const char *internal){
    return Find(string, limit, internal);
}

SX_INLINE bool String::Equals(const char *first, const char *second){
    return Compare(first, second) == 0;
}

SX_INLINE char *String::Find(char *string, const char *internal){
    return (char*)Find((const char*)string, internal);
}

SX_INLINE char *String::Find(char *string, size_t limit, const char *internal){
    return (char*)Find((const char*)string, limit, internal);
}

SX_INLINE char *String::FindLast(char *string, const char *internal){
    return (char*)FindLast((const char *)string, internal);
}

SX_INLINE char *String::IgnoreUntil(char *string, char ch){
    return (char*)IgnoreUntil((const char*)string, ch);
}

SX_INLINE char *String::Ignore(char *string, char ch){
    return (char *)Ignore((const char *)string, ch);
}

#endif // STRAITX_STRING_HPP