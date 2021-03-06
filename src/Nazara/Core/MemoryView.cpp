// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/MemoryView.hpp>
#include <algorithm>
#include <cstring>
#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	MemoryView::MemoryView(void* ptr, UInt64 size) :
	Stream(StreamOption_None, OpenMode_ReadWrite),
	m_ptr(reinterpret_cast<UInt8*>(ptr)), 
	m_pos(0),
	m_size(size)
	{
	}

	MemoryView::MemoryView(const void* ptr, UInt64 size) :
	Stream(StreamOption_None, OpenMode_ReadOnly),
	m_ptr(reinterpret_cast<UInt8*>(const_cast<void*>(ptr))), //< Okay, right, const_cast is bad, but this pointer is still read-only
	m_pos(0),
	m_size(size)
	{
	}

	bool MemoryView::EndOfStream() const
	{
		return m_pos >= m_size;
	}

	UInt64 MemoryView::GetCursorPos() const
	{
		return m_pos;
	}

	UInt64 MemoryView::GetSize() const
	{
		return m_size;
	}

	bool MemoryView::SetCursorPos(UInt64 offset)
	{
		m_pos = std::min(offset, m_size);

		return true;
	}

	void MemoryView::FlushStream()
	{
		// Nothing to do
	}

	std::size_t MemoryView::ReadBlock(void* buffer, std::size_t size)
	{
		std::size_t readSize = std::min<std::size_t>(size, static_cast<std::size_t>(m_size - m_pos));

		if (buffer)
			std::memcpy(buffer, &m_ptr[m_pos], readSize);

		m_pos += readSize;
		return readSize;
	}

	std::size_t MemoryView::WriteBlock(const void* buffer, std::size_t size)
	{
		std::size_t endPos = static_cast<std::size_t>(m_pos + size);
		if (endPos > m_size)
			size = m_size - m_pos;

		std::memcpy(&m_ptr[m_pos], buffer, size);

		m_pos += size;
		return size;
	}
}
