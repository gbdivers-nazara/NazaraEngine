// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Utility/Buffer.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Core/Error.hpp>
#include <Nazara/Core/ErrorFlags.hpp>
#include <Nazara/Utility/AbstractBuffer.hpp>
#include <Nazara/Utility/BufferMapper.hpp>
#include <Nazara/Utility/Config.hpp>
#include <Nazara/Utility/SoftwareBuffer.hpp>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <Nazara/Utility/Debug.hpp>

namespace Nz
{
	namespace
	{
		AbstractBuffer* SoftwareBufferFactory(Buffer* parent, BufferType type)
		{
			return new SoftwareBuffer(parent, type);
		}
	}

	Buffer::Buffer(BufferType type) :
	m_type(type),
	m_impl(nullptr),
	m_size(0)
	{
	}

	Buffer::Buffer(BufferType type, unsigned int size, UInt32 storage, BufferUsage usage) :
	m_type(type),
	m_impl(nullptr)
	{
		ErrorFlags flags(ErrorFlag_ThrowException, true);
		Create(size, storage, usage);
	}

	Buffer::~Buffer()
	{
		OnBufferRelease(this);

		Destroy();
	}

	bool Buffer::CopyContent(const Buffer& buffer)
	{
		#if NAZARA_UTILITY_SAFE
		if (!m_impl)
		{
			NazaraError("Buffer must be valid");
			return false;
		}

		if (!buffer.IsValid())
		{
			NazaraError("Source buffer must be valid");
			return false;
		}
		#endif

		BufferMapper<Buffer> mapper(buffer, BufferAccess_ReadOnly);
		return Fill(mapper.GetPointer(), 0, buffer.GetSize());
	}

	bool Buffer::Create(unsigned int size, UInt32 storage, BufferUsage usage)
	{
		Destroy();

		// Notre buffer est-il supporté ?
		if (!IsStorageSupported(storage))
		{
			NazaraError("Buffer storage not supported");
			return false;
		}

		std::unique_ptr<AbstractBuffer> impl(s_bufferFactories[storage](this, m_type));
		if (!impl->Create(size, usage))
		{
			NazaraError("Failed to create buffer");
			return false;
		}

		m_impl = impl.release();
		m_size = size;
		m_storage = storage;
		m_usage = usage;

		return true; // Si on arrive ici c'est que tout s'est bien passé.
	}

	void Buffer::Destroy()
	{
		if (m_impl)
		{
			OnBufferDestroy(this);

			m_impl->Destroy();
			delete m_impl;
			m_impl = nullptr;
		}
	}

	bool Buffer::Fill(const void* data, unsigned int offset, unsigned int size, bool forceDiscard)
	{
		#if NAZARA_UTILITY_SAFE
		if (!m_impl)
		{
			NazaraError("Buffer not valid");
			return false;
		}

		if (offset+size > m_size)
		{
			NazaraError("Exceeding buffer size (" + String::Number(offset+size) + " > " + String::Number(m_size) + ')');
			return false;
		}
		#endif

		return m_impl->Fill(data, offset, (size == 0) ? m_size-offset : size, forceDiscard);
	}

	AbstractBuffer* Buffer::GetImpl() const
	{
		return m_impl;
	}

	unsigned int Buffer::GetSize() const
	{
		return m_size;
	}

	UInt32 Buffer::GetStorage() const
	{
		return m_storage;
	}

	BufferType Buffer::GetType() const
	{
		return m_type;
	}

	BufferUsage Buffer::GetUsage() const
	{
		return m_usage;
	}

	bool Buffer::IsHardware() const
	{
		return m_storage & DataStorage_Hardware;
	}

	bool Buffer::IsValid() const
	{
		return m_impl != nullptr;
	}

	void* Buffer::Map(BufferAccess access, unsigned int offset, unsigned int size)
	{
		#if NAZARA_UTILITY_SAFE
		if (!m_impl)
		{
			NazaraError("Buffer not valid");
			return nullptr;
		}

		if (offset+size > m_size)
		{
			NazaraError("Exceeding buffer size");
			return nullptr;
		}
		#endif

		return m_impl->Map(access, offset, (size == 0) ? m_size-offset : size);
	}

	void* Buffer::Map(BufferAccess access, unsigned int offset, unsigned int size) const
	{
		#if NAZARA_UTILITY_SAFE
		if (!m_impl)
		{
			NazaraError("Buffer not valid");
			return nullptr;
		}

		if (access != BufferAccess_ReadOnly)
		{
			NazaraError("Buffer access must be read-only when used const");
			return nullptr;
		}

		if (offset+size > m_size)
		{
			NazaraError("Exceeding buffer size");
			return nullptr;
		}
		#endif

		return m_impl->Map(access, offset, (size == 0) ? m_size-offset : size);
	}

	bool Buffer::SetStorage(UInt32 storage)
	{
		#if NAZARA_UTILITY_SAFE
		if (!m_impl)
		{
			NazaraError("Buffer not valid");
			return false;
		}
		#endif

		if (m_storage == storage)
			return true;

		if (!IsStorageSupported(storage))
		{
			NazaraError("Storage not supported");
			return false;
		}

		void* ptr = m_impl->Map(BufferAccess_ReadOnly, 0, m_size);
		if (!ptr)
		{
			NazaraError("Failed to map buffer");
			return false;
		}

		CallOnExit unmapMyImpl([this]()
		{
			m_impl->Unmap();
		});

		std::unique_ptr<AbstractBuffer> impl(s_bufferFactories[storage](this, m_type));
		if (!impl->Create(m_size, m_usage))
		{
			NazaraError("Failed to create buffer");
			return false;
		}

		CallOnExit destroyImpl([&impl]()
		{
			impl->Destroy();
		});

		if (!impl->Fill(ptr, 0, m_size))
		{
			NazaraError("Failed to fill buffer");
			return false;
		}

		destroyImpl.Reset();

		unmapMyImpl.CallAndReset();
		m_impl->Destroy();
		delete m_impl;

		m_impl = impl.release();
		m_storage = storage;

		return true;
	}

	void Buffer::Unmap() const
	{
		#if NAZARA_UTILITY_SAFE
		if (!m_impl)
		{
			NazaraError("Buffer not valid");
			return;
		}
		#endif

		if (!m_impl->Unmap())
			NazaraWarning("Failed to unmap buffer (it's content may be undefined)"); ///TODO: Unexpected ?
	}

	bool Buffer::IsStorageSupported(UInt32 storage)
	{
		return s_bufferFactories[storage] != nullptr;
	}

	void Buffer::SetBufferFactory(UInt32 storage, BufferFactory func)
	{
		s_bufferFactories[storage] = func;
	}

	bool Buffer::Initialize()
	{
		s_bufferFactories[DataStorage_Software] = SoftwareBufferFactory;

		return true;
	}

	void Buffer::Uninitialize()
	{
		std::memset(s_bufferFactories, 0, (DataStorage_Max+1)*sizeof(Buffer::BufferFactory));
	}

	Buffer::BufferFactory Buffer::s_bufferFactories[DataStorage_Max+1] = {nullptr};
}
