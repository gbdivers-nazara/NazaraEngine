// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/Error.hpp>
#include <Nazara/Utility/Debug.hpp>

namespace Nz
{
	template <typename T>
	ScopedXCB<T>::ScopedXCB(T* pointer) :
	m_pointer(pointer)
	{
	}

	template <typename T>
	ScopedXCB<T>::~ScopedXCB()
	{
		std::free(m_pointer);
	}

	template <typename T>
	T* ScopedXCB<T>::operator ->() const
	{
		return m_pointer;
	}

	template <typename T>
	T** ScopedXCB<T>::operator &()
	{
		return &m_pointer;
	}

	template <typename T>
	ScopedXCB<T>::operator bool() const
	{
		return m_pointer != nullptr;
	}

	template <typename T>
	T* ScopedXCB<T>::get() const
	{
		return m_pointer;
	}
}

#include <Nazara/Utility/DebugOff.hpp>
