#ifndef STRAITX_LIST_HPP
#define STRAITX_LIST_HPP

#include "core/type_traits.hpp"
#include "core/templates.hpp"
#include "core/types.hpp"
#include "core/move.hpp"
#include "core/noncopyable.hpp"
#include "core/assert.hpp"
#include "core/allocators/allocator.hpp"
#include "core/span.hpp"
#include "core/algorithm.hpp"
#include <initializer_list>

//TODO: 
// [ ] Optimize for Types without copy or mouse ctors
// [ ] Get rid of NonCopyable include XD

template<typename Type, typename GeneralAllocator = DefaultGeneralAllocator>
class List: private GeneralAllocator, public NonCopyable{
public:
    static_assert(!IsConst<Type>::Value && !IsVolatile<Type>::Value, "Type can't be const or volatile");

    using Iterator = Type *;
    using ConstIterator = const Type *;
private:
    Type *m_Elements = nullptr;
    size_t m_Size = 0;
    size_t m_Capacity = 0;
public:
    List() = default;

    List(ConstSpan<Type> span){
        Reserve(span.Size());

        for(const Type &element: span)
            Add(element);
    }

    List(Span<Type> span):
        List(ConstSpan<Type>(span.Pointer(), span.Size())) 
    {}

    template <typename EntryType>
    List(std::initializer_list<EntryType> list){
        Reserve(list.size());
        
        for(const EntryType &element: list)
            Emplace(element);
    }

    List(List<Type> &&other) {
        *this = Move(other);
    }

    ~List(){
        Free();
    }

    List &operator=(List<Type> &&other) {
        Free();
        Swap(other);
        return *this;
    }

    void Add(const Type &element){
        Emplace(element);
    }

    void Add(Type &&element){
        Emplace(Move(element));
    }

    template<typename...ArgsType>
    void Emplace(ArgsType&&...args){
        if(m_Size == m_Capacity)
            Reserve(m_Size * 2 + (m_Size == 0));

        new(&Data()[m_Size++]) Type{Forward<ArgsType>(args)...};
    }
    
    template<typename OtherGeneralAllocator>
    void Add(const List<Type, OtherGeneralAllocator>& other) {
        Reserve(Size() + other.Size());
        for(const auto &element: other)
            Add(element);
    }

    void RemoveLast(){
        SX_CORE_ASSERT(m_Size, "Can't remove last element from empty List");

        m_Elements[--m_Size].~Type();
    }

    void UnorderedRemove(size_t index){
        SX_CORE_ASSERT(IsValidIndex(index), "Index is out of range");

        At(index) = Move(Last());
        RemoveLast();
    }

    bool UnorderedRemove(const Type &type){
        ConstIterator it = Find(type);
        if (it != end())
            return (UnorderedRemove(it), true);
        else
            return false;
    }

    bool Contains(const Type& value)const {
        return Find(value) != end();
    }

    ConstIterator Find(const Type& value)const{
        ConstIterator it = begin();
        for (; it != end(); ++it)
            if (*it == value)
                return it;
        return it;
    }
    
    template<typename Predicate>
    ConstIterator FindByPredicate(Predicate predicate) {
        ConstIterator it = begin();
        for (; it != end(); ++it)
            if (predicate(*it))
                return it;
        return it;
    }

    void UnorderedRemove(ConstIterator iterator){
        SX_CORE_ASSERT(iterator >= begin() && iterator < end(), "iterator is out of range");
        UnorderedRemove(iterator - begin());
    }

    void Reserve(size_t capacity){
        if(capacity <= m_Capacity)return;

        Type *new_elements = (Type*)GeneralAllocator::Alloc(capacity * sizeof(Type));

        for(size_t i = 0; i<Size(); i++){
            new(&new_elements[i]) Type(Move(m_Elements[i]));
            m_Elements[i].~Type();
        }

        GeneralAllocator::Free(m_Elements);
        m_Elements = new_elements;
        m_Capacity = capacity;
    }

    void Swap(List<Type> &other) {
        ::Swap(m_Elements, other.m_Elements);
        ::Swap(m_Size, other.m_Size);
        ::Swap(m_Capacity, other.m_Capacity);
    }

    void Clear(){
        if(IsTriviallyDestructable<Type>::Value) {
            m_Size = 0;
        }else{
            while(Size())
                RemoveLast();
        }
    }

    void Free(){
        //Size should be zero
        Clear();
        GeneralAllocator::Free(m_Elements);
        m_Elements = nullptr;
        m_Capacity = 0;
    }

    bool IsValidIndex(size_t index)const {
        return index < m_Size;
    }

    Type &At(size_t index) {
        SX_CORE_ASSERT(IsValidIndex(index), "Invalid Index");
        return m_Elements[index];
    }
    const Type &At(size_t index)const{
        SX_CORE_ASSERT(IsValidIndex(index), "Invalid Index");
        return m_Elements[index];
    }

    Type &operator[](size_t index){
        return At(index);
    }

    const Type &operator[](size_t index)const{
        return At(index);
    }

    operator Span<Type>(){
        return {Data(), Size()};
    }

    operator ConstSpan<Type>()const{
        return {Data(), Size()};
    }

    Type *Data(){
        return m_Elements;
    }

    const Type *Data()const{
        return m_Elements;
    }

    size_t Size()const{
        return m_Size;
    }

    size_t Capacity()const{
        return m_Capacity;
    }

    Type &First(){
        return At(0);
    }

    const Type &First()const{
        return At(0);
    }

    Type &Last(){
        return At(Size() - 1);
    }

    const Type &Last()const{
        return At(Size() - 1);
    }

    Iterator begin(){
        return Data();
    }

    Iterator end(){
        return Data() + Size();
    }

    ConstIterator begin()const{
        return Data();
    }

    ConstIterator end()const{
        return Data() + Size();
    }
private:
    template<typename _Type = Type, typename = typename EnableIf<IsMoveConstructible<_Type>::Value>::Type>
    static void MoveElseCopyCtorImpl(Type *dst, Type *src, void *) {
        new(dst) Type(Move(*src));
    }

    template<typename _Type = Type, typename = typename EnableIf<!IsMoveConstructible<_Type>::Value>::Type>
    static void MoveElseCopyCtorImpl(Type *dst, Type *src, ...) {
        new(dst) Type(*src);
    }
    static void MoveElseCopyCtor(Type *dst, Type *src) {
        MoveElseCopyCtorImpl(dst, src, nullptr);
    }
};

#endif//STRAITX_LIST_HPP