// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Graphics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace Nz
{
	inline void ForwardRenderTechnique::SendLightUniforms(const Shader* shader, const LightUniforms& uniforms, unsigned int index, unsigned int uniformOffset) const
	{
		if (index < m_lights.size())
		{
			const LightIndex& lightIndex = m_lights[index];

			shader->SendInteger(uniforms.locations.type + uniformOffset, lightIndex.type); //< Sends the light type

			switch (lightIndex.type)
			{
				case LightType_Directional:
				{
					const auto& light = m_renderQueue.directionalLights[lightIndex.index];

					shader->SendColor(uniforms.locations.color + uniformOffset, light.color);
					shader->SendVector(uniforms.locations.factors + uniformOffset, Vector2f(light.ambientFactor, light.diffuseFactor));
					shader->SendVector(uniforms.locations.parameters1 + uniformOffset, Vector4f(light.direction));
					break;
				}

				case LightType_Point:
				{
					const auto& light = m_renderQueue.pointLights[lightIndex.index];

					shader->SendColor(uniforms.locations.color + uniformOffset, light.color);
					shader->SendVector(uniforms.locations.factors + uniformOffset, Vector2f(light.ambientFactor, light.diffuseFactor));
					shader->SendVector(uniforms.locations.parameters1 + uniformOffset, Vector4f(light.position, light.attenuation));
					shader->SendVector(uniforms.locations.parameters2 + uniformOffset, Vector4f(0.f, 0.f, 0.f, light.invRadius));
					break;
				}

				case LightType_Spot:
				{
					const auto& light = m_renderQueue.spotLights[lightIndex.index];

					shader->SendColor(uniforms.locations.color + uniformOffset, light.color);
					shader->SendVector(uniforms.locations.factors + uniformOffset, Vector2f(light.ambientFactor, light.diffuseFactor));
					shader->SendVector(uniforms.locations.parameters1 + uniformOffset, Vector4f(light.position, light.attenuation));
					shader->SendVector(uniforms.locations.parameters2 + uniformOffset, Vector4f(light.direction, light.invRadius));
					shader->SendVector(uniforms.locations.parameters3 + uniformOffset, Vector2f(light.innerAngleCosine, light.outerAngleCosine));
					break;
				}
			}
		}
		else
			shader->SendInteger(uniforms.locations.type + uniformOffset, -1); //< Disable the light in the shader
	}

	inline float ForwardRenderTechnique::ComputeDirectionalLightScore(const Spheref& object, const AbstractRenderQueue::DirectionalLight& light)
	{
		NazaraUnused(object);
		NazaraUnused(light);

		///TODO: Compute a score depending on the light luminosity
		return 0.f;
	}

	inline float ForwardRenderTechnique::ComputePointLightScore(const Spheref& object, const AbstractRenderQueue::PointLight& light)
	{
		///TODO: Compute a score depending on the light luminosity
		return object.SquaredDistance(light.position);
	}

	inline float ForwardRenderTechnique::ComputeSpotLightScore(const Spheref& object, const AbstractRenderQueue::SpotLight& light)
	{
		///TODO: Compute a score depending on the light luminosity and spot direction
		return object.SquaredDistance(light.position);
	}

	inline bool ForwardRenderTechnique::IsDirectionalLightSuitable(const Spheref& object, const AbstractRenderQueue::DirectionalLight& light)
	{
		NazaraUnused(object);
		NazaraUnused(light);

		// Directional light are always suitables
		return true;
	}

	inline bool ForwardRenderTechnique::IsPointLightSuitable(const Spheref& object, const AbstractRenderQueue::PointLight& light)
	{
		// If the object is too far away from this point light, there is not way it could light it
		return object.SquaredDistance(light.position) <= light.radius * light.radius;
	}

	inline bool ForwardRenderTechnique::IsSpotLightSuitable(const Spheref& object, const AbstractRenderQueue::SpotLight& light)
	{
		///TODO: Exclude spot lights based on their direction and outer angle?
		return object.SquaredDistance(light.position) <= light.radius * light.radius;
	}
}
