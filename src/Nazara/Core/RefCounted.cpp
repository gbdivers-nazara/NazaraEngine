// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/RefCounted.hpp>
#include <Nazara/Core/Config.hpp>
#include <Nazara/Core/Error.hpp>

#if NAZARA_CORE_THREADSAFE && NAZARA_THREADSAFETY_REFCOUNTED
	#include <Nazara/Core/ThreadSafety.hpp>
#else
	#include <Nazara/Core/ThreadSafetyOff.hpp>
#endif

#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	RefCounted::RefCounted(bool persistent) :
	m_persistent(persistent),
	m_referenceCount(0)
	{
	}

	RefCounted::~RefCounted()
	{
		#if NAZARA_CORE_SAFE
		if (m_referenceCount > 0)
			NazaraWarning("Resource destroyed while still referenced " + String::Number(m_referenceCount) + " time(s)");
		#endif
	}

	void RefCounted::AddReference() const
	{
		m_referenceCount++;
	}

	unsigned int RefCounted::GetReferenceCount() const
	{
		return m_referenceCount;
	}

	bool RefCounted::IsPersistent() const
	{
		return m_persistent;
	}

	bool RefCounted::RemoveReference() const
	{
		#if NAZARA_CORE_SAFE
		if (m_referenceCount == 0)
		{
			NazaraError("Impossible to remove reference (Ref. counter is already 0)");
			return false;
		}
		#endif

		if (--m_referenceCount == 0 && !m_persistent)
		{
			delete this; // Suicide

			return true;
		}
		else
			return false;
	}

	bool RefCounted::SetPersistent(bool persistent, bool checkReferenceCount)
	{
		m_persistent = persistent;

		if (checkReferenceCount && !persistent && m_referenceCount == 0)
		{
			delete this;

			return true;
		}
		else
			return false;
	}
}
