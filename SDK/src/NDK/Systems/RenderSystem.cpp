// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Development Kit"
// For conditions of distribution and use, see copyright notice in Prerequesites.hpp

#include <NDK/Systems/RenderSystem.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/LightComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace Ndk
{
	RenderSystem::RenderSystem() :
	m_coordinateSystemMatrix(Nz::Matrix4f::Identity()),
	m_coordinateSystemInvalidated(true)
	{
		SetDefaultBackground(Nz::ColorBackground::New());
		SetUpdateRate(0.f);
	}

	void RenderSystem::OnEntityRemoved(Entity* entity)
	{
		m_cameras.Remove(entity);
		m_drawables.Remove(entity);
		m_lights.Remove(entity);
	}

	void RenderSystem::OnEntityValidation(Entity* entity, bool justAdded)
	{
		NazaraUnused(justAdded);

		if (entity->HasComponent<CameraComponent>() && entity->HasComponent<NodeComponent>())
		{
			m_cameras.Insert(entity);
			std::sort(m_cameras.begin(), m_cameras.end(), [](const EntityHandle& handle1, const EntityHandle& handle2)
			{
				return handle1->GetComponent<CameraComponent>().GetLayer() < handle2->GetComponent<CameraComponent>().GetLayer();
			});
		}
		else
			m_cameras.Remove(entity);

		if (entity->HasComponent<GraphicsComponent>() && entity->HasComponent<NodeComponent>())
			m_drawables.Insert(entity);
		else
			m_drawables.Remove(entity);

		if (entity->HasComponent<LightComponent>() && entity->HasComponent<NodeComponent>())
			m_lights.Insert(entity);
		else
			m_lights.Remove(entity);
	}

	void RenderSystem::OnUpdate(float elapsedTime)
	{
		NazaraUnused(elapsedTime);

		// Invalidate every renderable if the coordinate system changed
		if (m_coordinateSystemInvalidated)
		{
			for (const Ndk::EntityHandle& drawable : m_drawables)
			{
				GraphicsComponent& graphicsComponent = drawable->GetComponent<GraphicsComponent>();
				graphicsComponent.InvalidateTransformMatrix();
			}

			m_coordinateSystemInvalidated = false;
		}

		for (const Ndk::EntityHandle& camera : m_cameras)
		{
			CameraComponent& camComponent = camera->GetComponent<CameraComponent>();
			camComponent.ApplyView();

			Nz::AbstractRenderQueue* renderQueue = m_renderTechnique.GetRenderQueue();
			renderQueue->Clear();

			//TODO: Culling
			for (const Ndk::EntityHandle& drawable : m_drawables)
			{
				GraphicsComponent& graphicsComponent = drawable->GetComponent<GraphicsComponent>();
				NodeComponent& drawableNode = drawable->GetComponent<NodeComponent>();

				graphicsComponent.AddToRenderQueue(renderQueue);
			}

			for (const Ndk::EntityHandle& light : m_lights)
			{
				LightComponent& lightComponent = light->GetComponent<LightComponent>();
				NodeComponent& drawableNode = light->GetComponent<NodeComponent>();

				///TODO: Cache somehow?
				lightComponent.AddToRenderQueue(renderQueue, Nz::Matrix4f::ConcatenateAffine(m_coordinateSystemMatrix, drawableNode.GetTransformMatrix()));
			}

			Nz::SceneData sceneData;
			sceneData.ambientColor = Nz::Color(25, 25, 25);
			sceneData.background = m_background;
			sceneData.viewer = &camComponent;

			m_renderTechnique.Draw(sceneData);
		}
	}

	SystemIndex RenderSystem::systemIndex;
}
