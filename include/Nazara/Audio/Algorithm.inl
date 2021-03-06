// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Audio module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/Error.hpp>
#include <Nazara/Audio/Debug.hpp>

namespace Nz
{
	template<typename T>
	void MixToMono(T* input, T* output, unsigned int channelCount, unsigned int frameCount)
	{
		///DOC: Le buffer d'entrée peut être le même que le buffer de sortie
		// Pour éviter l'overflow, on utilise comme accumulateur un type assez grand, (u)int 64 bits pour les entiers, double pour les flottants
		typedef typename std::conditional<std::is_unsigned<T>::value, UInt64, Int64>::type BiggestInt;
		typedef typename std::conditional<std::is_integral<T>::value, BiggestInt, double>::type Biggest;

		for (unsigned int i = 0; i < frameCount; ++i)
		{
			Biggest acc = Biggest(0);
			for (unsigned int j = 0; j < channelCount; ++j)
				acc += input[i*channelCount + j];

			output[i] = static_cast<T>(acc / channelCount);
		}
	}
}

#include <Nazara/Audio/DebugOff.hpp>
