// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/Core.hpp>
#include <Nazara/Core/Config.hpp>
#include <Nazara/Core/Error.hpp>
#include <Nazara/Core/HardwareInfo.hpp>
#include <Nazara/Core/Log.hpp>
#include <Nazara/Core/PluginManager.hpp>
#include <Nazara/Core/TaskScheduler.hpp>
#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	bool Core::Initialize()
	{
		if (s_moduleReferenceCounter > 0)
		{
			s_moduleReferenceCounter++;
			return true; // Déjà initialisé
		}

		s_moduleReferenceCounter++;

		Log::Initialize();

		NazaraNotice("Initialized: Core");
		return true;
	}

	bool Core::IsInitialized()
	{
		return s_moduleReferenceCounter != 0;
	}

	void Core::Uninitialize()
	{
		if (s_moduleReferenceCounter != 1)
		{
			// Le module est soit encore utilisé, soit pas initialisé
			if (s_moduleReferenceCounter > 1)
				s_moduleReferenceCounter--;

			return;
		}

		// Libération du module
		s_moduleReferenceCounter = 0;

		HardwareInfo::Uninitialize();
		Log::Uninitialize();
		PluginManager::Uninitialize();
		TaskScheduler::Uninitialize();

		NazaraNotice("Uninitialized: Core");
	}

	unsigned int Core::s_moduleReferenceCounter = 0;
}
