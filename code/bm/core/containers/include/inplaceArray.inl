/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

template < class T, uint32_t N >
INLINE InplaceArray<T,N>::InplaceArray()
    : Array<T>(BaseArrayBuffer(m_storage, N, false))
{}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>::InplaceArray(const Array<T>& other)
	: Array<T>(BaseArrayBuffer(m_storage, N, false))
{
	auto data  = allocateUninitialized(other.size());
	std::uninitialized_copy_n(other.typedData(), other.size(), data);
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>::InplaceArray(Array<T>&& other)
	: Array<T>(BaseArrayBuffer(m_storage, N, false))
{
	*this = std::move(other);
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>::InplaceArray(const InplaceArray<T,N>& other)
	: Array<T>(BaseArrayBuffer(m_storage, N, false))
{
	auto data = this->allocateUninitialized(other.size());
	std::uninitialized_copy_n(other.typedData(), other.size(), data);
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>::InplaceArray(InplaceArray<T, N>&& other)
	: Array<T>(BaseArrayBuffer(m_storage, N, false))
{
	*this = std::move(other);
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>::InplaceArray(const T* ptr, uint32_t size)
	: Array<T>(BaseArrayBuffer(m_storage, N, false))
{
	auto data = this->allocateUninitialized(size);
	std::uninitialized_copy_n(ptr, size, data);
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>::InplaceArray(std::initializer_list<T> values)
	: Array<T>(BaseArrayBuffer(m_storage, N, false))
{
	reserve(values.size());

	for (auto& val : values)
		emplaceBack(std::move(val));
}

//--

template < class T, uint32_t N >
INLINE InplaceArray<T, N>& InplaceArray<T, N>::operator=(const Array<T>& other)
{
	Array<T>::operator=(other);
	return *this;
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>& InplaceArray<T, N>::operator=(Array<T>&& other)
{
	Array<T>::operator=(std::move(other));
	return *this;
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>& InplaceArray<T, N>::operator=(const InplaceArray<T, N>& other)
{
	Array<T>::operator=(other);
	return *this;		
}

template < class T, uint32_t N >
INLINE InplaceArray<T, N>& InplaceArray<T, N>::operator=(InplaceArray<T, N>&& other)
{
	Array<T>::operator=(std::move(other));
	return *this;
}

//--

template < class T, uint32_t N >
INLINE bool InplaceArray<T, N>::inplace() const
{
	return data() == &m_storage[0];
}

template < class T, uint32_t N >
INLINE void InplaceArray<T, N>::clear()
{
	Array<T>::clear();
	m_buffer.replaceAndFree(m_storage, N, false);
}

//--

END_INFERNO_NAMESPACE()
