// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_PARAMETERLIST_HPP
#define NAZARA_PARAMETERLIST_HPP

#include <Nazara/Prerequesites.hpp>
#include <Nazara/Core/String.hpp>
#include <atomic>
#include <unordered_map>

namespace Nz
{
	class NAZARA_CORE_API ParameterList
	{
		public:
			using Destructor = void (*)(void* value);

			ParameterList() = default;
			ParameterList(const ParameterList& list);
			ParameterList(ParameterList&&) = default;
			~ParameterList();

			void Clear();

			bool GetBooleanParameter(const String& name, bool* value) const;
			bool GetFloatParameter(const String& name, float* value) const;
			bool GetIntegerParameter(const String& name, int* value) const;
			bool GetParameterType(const String& name, ParameterType* type) const;
			bool GetPointerParameter(const String& name, void** value) const;
			bool GetStringParameter(const String& name, String* value) const;
			bool GetUserdataParameter(const String& name, void** value) const;

			bool HasParameter(const String& name) const;

			void RemoveParameter(const String& name);

			void SetParameter(const String& name);
			void SetParameter(const String& name, const String& value);
			void SetParameter(const String& name, const char* value);
			void SetParameter(const String& name, void* value);
			void SetParameter(const String& name, void* value, Destructor destructor);
			void SetParameter(const String& name, bool value);
			void SetParameter(const String& name, float value);
			void SetParameter(const String& name, int value);

			ParameterList& operator=(const ParameterList& list);
			ParameterList& operator=(ParameterList&&) = default;

		private:
			struct Parameter
			{
				struct UserdataValue
				{
					UserdataValue(Destructor Destructor, void* value) :
					counter(1),
					destructor(Destructor),
					ptr(value)
					{
					}

					std::atomic_uint counter;
					Destructor destructor;
					void* ptr;
				};

				ParameterType type;
				union Value
				{
					// On définit un constructeur/destructeur vide, permettant de mettre des classes dans l'union
					Value() {}
					Value(const Value&) {} // Placeholder
					~Value() {}

					bool boolVal;
					float floatVal;
					int intVal;
					void* ptrVal;
					String stringVal;
					UserdataValue* userdataVal;
				};

				Value value;
			};

			using ParameterMap = std::unordered_map<String, Parameter>;

			void DestroyValue(Parameter& parameter);

			ParameterMap m_parameters;
	};
}

#endif // NAZARA_PARAMETERLIST_HPP
