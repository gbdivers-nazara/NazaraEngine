// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Graphics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Graphics/DeferredPhongLightingPass.hpp>
#include <Nazara/Graphics/AbstractViewer.hpp>
#include <Nazara/Graphics/DeferredRenderQueue.hpp>
#include <Nazara/Renderer/Renderer.hpp>
#include <Nazara/Renderer/RenderTexture.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <memory>
#include <Nazara/Graphics/Debug.hpp>

namespace Nz
{
	DeferredPhongLightingPass::DeferredPhongLightingPass() :
	m_lightMeshesDrawing(false)
	{
		m_directionalLightShader = ShaderLibrary::Get("DeferredDirectionnalLight");
		m_directionalLightShaderEyePositionLocation = m_directionalLightShader->GetUniformLocation("EyePosition");
		m_directionalLightShaderSceneAmbientLocation = m_directionalLightShader->GetUniformLocation("SceneAmbient");

		m_directionalLightUniforms.ubo = false;
		m_directionalLightUniforms.locations.type = -1; // Type déjà connu
		m_directionalLightUniforms.locations.color = m_directionalLightShader->GetUniformLocation("LightColor");
		m_directionalLightUniforms.locations.factors = m_directionalLightShader->GetUniformLocation("LightFactors");
		m_directionalLightUniforms.locations.parameters1 = m_directionalLightShader->GetUniformLocation("LightDirection");
		m_directionalLightUniforms.locations.parameters2 = -1;
		m_directionalLightUniforms.locations.parameters3 = -1;

		m_pointSpotLightShader = ShaderLibrary::Get("DeferredPointSpotLight");
		m_pointSpotLightShaderDiscardLocation = m_pointSpotLightShader->GetUniformLocation("Discard");
		m_pointSpotLightShaderEyePositionLocation = m_pointSpotLightShader->GetUniformLocation("EyePosition");
		m_pointSpotLightShaderSceneAmbientLocation = m_pointSpotLightShader->GetUniformLocation("SceneAmbient");

		m_pointSpotLightUniforms.ubo = false;
		m_pointSpotLightUniforms.locations.type = m_pointSpotLightShader->GetUniformLocation("LightType");
		m_pointSpotLightUniforms.locations.color = m_pointSpotLightShader->GetUniformLocation("LightColor");
		m_pointSpotLightUniforms.locations.factors = m_pointSpotLightShader->GetUniformLocation("LightFactors");
		m_pointSpotLightUniforms.locations.parameters1 = m_pointSpotLightShader->GetUniformLocation("LightParameters1");
		m_pointSpotLightUniforms.locations.parameters2 = m_pointSpotLightShader->GetUniformLocation("LightParameters2");
		m_pointSpotLightUniforms.locations.parameters3 = m_pointSpotLightShader->GetUniformLocation("LightParameters3");

		m_pointSampler.SetAnisotropyLevel(1);
		m_pointSampler.SetFilterMode(SamplerFilter_Nearest);
		m_pointSampler.SetWrapMode(SamplerWrap_Clamp);

		m_cone = Mesh::New();
		m_cone->CreateStatic();
		m_coneMesh = static_cast<StaticMesh*>(m_cone->BuildSubMesh(Primitive::Cone(1.f, 1.f, 16, Matrix4f::Rotate(EulerAnglesf(90.f, 0.f, 0.f)))));

		m_sphere = Mesh::New();
		m_sphere->CreateStatic();
		m_sphereMesh = static_cast<StaticMesh*>(m_sphere->BuildSubMesh(Primitive::IcoSphere(1.f, 1)));
	}

	DeferredPhongLightingPass::~DeferredPhongLightingPass() = default;

	void DeferredPhongLightingPass::EnableLightMeshesDrawing(bool enable)
	{
		m_lightMeshesDrawing = enable;
	}

	bool DeferredPhongLightingPass::IsLightMeshesDrawingEnabled() const
	{
		return m_lightMeshesDrawing;
	}

	bool DeferredPhongLightingPass::Process(const SceneData& sceneData, unsigned int firstWorkTexture, unsigned secondWorkTexture) const
	{
		NazaraAssert(sceneData.viewer, "Invalid viewer");
		NazaraUnused(secondWorkTexture);

		m_workRTT->SetColorTarget(firstWorkTexture);
		Renderer::SetTarget(m_workRTT);
		Renderer::SetViewport(Recti(0, 0, m_dimensions.x, m_dimensions.y));

		Renderer::SetTexture(0, m_GBuffer[0]);
		Renderer::SetTextureSampler(0, m_pointSampler);

		Renderer::SetTexture(1, m_GBuffer[1]);
		Renderer::SetTextureSampler(1, m_pointSampler);

		Renderer::SetTexture(2, m_GBuffer[2]);
		Renderer::SetTextureSampler(2, m_pointSampler);

		Renderer::SetClearColor(Color::Black);
		Renderer::Clear(RendererBuffer_Color);

		RenderStates lightStates;
		lightStates.dstBlend = BlendFunc_One;
		lightStates.srcBlend = BlendFunc_One;
		lightStates.parameters[RendererParameter_Blend] = true;
		lightStates.parameters[RendererParameter_DepthBuffer] = false;
		lightStates.parameters[RendererParameter_DepthWrite] = false;

		// Directional lights
		if (!m_renderQueue->directionalLights.empty())
		{
			Renderer::SetRenderStates(lightStates);
			Renderer::SetShader(m_directionalLightShader);
			m_directionalLightShader->SendColor(m_directionalLightShaderSceneAmbientLocation, sceneData.ambientColor);
			m_directionalLightShader->SendVector(m_directionalLightShaderEyePositionLocation, sceneData.viewer->GetEyePosition());

			for (auto& light : m_renderQueue->directionalLights)
			{
				m_directionalLightShader->SendColor(m_directionalLightUniforms.locations.color, light.color);
				m_directionalLightShader->SendVector(m_directionalLightUniforms.locations.factors, Vector2f(light.ambientFactor, light.diffuseFactor));
				m_directionalLightShader->SendVector(m_directionalLightUniforms.locations.parameters1, Vector4f(light.direction));

				Renderer::DrawFullscreenQuad();
			}
		}

		// Point lights/Spot lights
		if (!m_renderQueue->pointLights.empty() || !m_renderQueue->spotLights.empty())
		{
			// http://www.altdevblogaday.com/2011/08/08/stencil-buffer-optimisation-for-deferred-lights/
			lightStates.parameters[RendererParameter_StencilTest] = true;
			lightStates.faceCulling = FaceSide_Front;
			lightStates.backFace.stencilMask = 0xFF;
			lightStates.backFace.stencilReference = 0;
			lightStates.backFace.stencilFail = StencilOperation_Keep;
			lightStates.backFace.stencilPass = StencilOperation_Keep;
			lightStates.backFace.stencilZFail = StencilOperation_Invert;
			lightStates.frontFace.stencilMask = 0xFF;
			lightStates.frontFace.stencilReference = 0;
			lightStates.frontFace.stencilFail = StencilOperation_Keep;
			lightStates.frontFace.stencilPass = StencilOperation_Keep;
			lightStates.frontFace.stencilZFail = StencilOperation_Invert;

			Renderer::SetRenderStates(lightStates);

			Renderer::SetShader(m_pointSpotLightShader);
			m_pointSpotLightShader->SendColor(m_pointSpotLightShaderSceneAmbientLocation, sceneData.ambientColor);
			m_pointSpotLightShader->SendVector(m_pointSpotLightShaderEyePositionLocation, sceneData.viewer->GetEyePosition());

			Matrix4f lightMatrix;
			lightMatrix.MakeIdentity();
			if (!m_renderQueue->pointLights.empty())
			{
				const IndexBuffer* indexBuffer = m_sphereMesh->GetIndexBuffer();
				Renderer::SetIndexBuffer(indexBuffer);
				Renderer::SetVertexBuffer(m_sphereMesh->GetVertexBuffer());

				m_pointSpotLightShader->SendInteger(m_pointSpotLightUniforms.locations.type, LightType_Point);
				for (const auto& light : m_renderQueue->pointLights)
				{
					m_pointSpotLightShader->SendColor(m_pointSpotLightUniforms.locations.color, light.color);
					m_pointSpotLightShader->SendVector(m_pointSpotLightUniforms.locations.factors, Vector2f(light.ambientFactor, light.diffuseFactor));
					m_pointSpotLightShader->SendVector(m_pointSpotLightUniforms.locations.parameters1, Vector4f(light.position, light.attenuation));
					m_pointSpotLightShader->SendVector(m_pointSpotLightUniforms.locations.parameters2, Vector4f(0.f, 0.f, 0.f, light.invRadius));

					lightMatrix.SetScale(Vector3f(light.radius * 1.1f)); // Pour corriger les imperfections liées à la sphère
					lightMatrix.SetTranslation(light.position);

					Renderer::SetMatrix(MatrixType_World, lightMatrix);

					// Rendu de la sphère dans le stencil buffer
					Renderer::Enable(RendererParameter_ColorWrite, false);
					Renderer::Enable(RendererParameter_DepthBuffer, true);
					Renderer::Enable(RendererParameter_FaceCulling, false);
					Renderer::SetStencilCompareFunction(RendererComparison_Always);

					m_pointSpotLightShader->SendBoolean(m_pointSpotLightShaderDiscardLocation, true);

					Renderer::DrawIndexedPrimitives(PrimitiveMode_TriangleList, 0, indexBuffer->GetIndexCount());

					// Rendu de la sphère comme zone d'effet
					Renderer::Enable(RendererParameter_ColorWrite, true);
					Renderer::Enable(RendererParameter_DepthBuffer, false);
					Renderer::Enable(RendererParameter_FaceCulling, true);
					Renderer::SetStencilCompareFunction(RendererComparison_NotEqual, FaceSide_Back);
					Renderer::SetStencilPassOperation(StencilOperation_Zero, FaceSide_Back);

					m_pointSpotLightShader->SendBoolean(m_pointSpotLightShaderDiscardLocation, false);

					Renderer::DrawIndexedPrimitives(PrimitiveMode_TriangleList, 0, indexBuffer->GetIndexCount());
				}

				if (m_lightMeshesDrawing)
				{
					Renderer::Enable(RendererParameter_DepthBuffer, true);
					Renderer::Enable(RendererParameter_DepthWrite, true);
					Renderer::Enable(RendererParameter_FaceCulling, false);
					Renderer::Enable(RendererParameter_StencilTest, false);
					Renderer::SetFaceFilling(FaceFilling_Line);

					const Shader* shader = ShaderLibrary::Get("DebugSimple");
					static int colorLocation = shader->GetUniformLocation("Color");

					Renderer::SetShader(shader);
					for (const auto& light : m_renderQueue->pointLights)
					{
						lightMatrix.SetScale(Vector3f(light.radius * 1.1f)); // Pour corriger les imperfections liées à la sphère
						lightMatrix.SetTranslation(light.position);

						Renderer::SetMatrix(MatrixType_World, lightMatrix);

						shader->SendColor(colorLocation, light.color);

						Renderer::DrawIndexedPrimitives(PrimitiveMode_TriangleList, 0, indexBuffer->GetIndexCount());
					}

					Renderer::Enable(RendererParameter_DepthBuffer, false);
					Renderer::Enable(RendererParameter_DepthWrite, false);
					Renderer::Enable(RendererParameter_FaceCulling, true);
					Renderer::Enable(RendererParameter_StencilTest, true);
					Renderer::SetFaceFilling(FaceFilling_Fill);
				}
			}

			if (!m_renderQueue->spotLights.empty())
			{
				const IndexBuffer* indexBuffer = m_coneMesh->GetIndexBuffer();
				Renderer::SetIndexBuffer(indexBuffer);
				Renderer::SetVertexBuffer(m_coneMesh->GetVertexBuffer());

				m_pointSpotLightShader->SendInteger(m_pointSpotLightUniforms.locations.type, LightType_Spot);
				for (const auto& light : m_renderQueue->spotLights)
				{
					m_pointSpotLightShader->SendColor(m_pointSpotLightUniforms.locations.color, light.color);
					m_pointSpotLightShader->SendVector(m_pointSpotLightUniforms.locations.factors, Vector2f(light.ambientFactor, light.diffuseFactor));
					m_pointSpotLightShader->SendVector(m_pointSpotLightUniforms.locations.parameters1, Vector4f(light.position, light.attenuation));
					m_pointSpotLightShader->SendVector(m_pointSpotLightUniforms.locations.parameters2, Vector4f(light.direction, light.invRadius));
					m_pointSpotLightShader->SendVector(m_pointSpotLightUniforms.locations.parameters3, Vector2f(light.innerAngleCosine, light.outerAngleCosine));

					float baseRadius = light.radius * light.outerAngleTangent * 1.1f;
					lightMatrix.MakeTransform(light.position, Quaternionf::RotationBetween(Vector3f::Forward(), light.direction), Vector3f(baseRadius, baseRadius, light.radius));

					Renderer::SetMatrix(MatrixType_World, lightMatrix);

					// Rendu de la sphère dans le stencil buffer
					Renderer::Enable(RendererParameter_ColorWrite, false);
					Renderer::Enable(RendererParameter_DepthBuffer, true);
					Renderer::Enable(RendererParameter_FaceCulling, false);
					Renderer::SetStencilCompareFunction(RendererComparison_Always);

					m_pointSpotLightShader->SendBoolean(m_pointSpotLightShaderDiscardLocation, true);

					Renderer::DrawIndexedPrimitives(PrimitiveMode_TriangleList, 0, indexBuffer->GetIndexCount());

					// Rendu de la sphère comme zone d'effet
					Renderer::Enable(RendererParameter_ColorWrite, true);
					Renderer::Enable(RendererParameter_DepthBuffer, false);
					Renderer::Enable(RendererParameter_FaceCulling, true);
					Renderer::SetFaceCulling(FaceSide_Front);
					Renderer::SetStencilCompareFunction(RendererComparison_NotEqual, FaceSide_Back);
					Renderer::SetStencilPassOperation(StencilOperation_Zero, FaceSide_Back);

					m_pointSpotLightShader->SendBoolean(m_pointSpotLightShaderDiscardLocation, false);

					Renderer::DrawIndexedPrimitives(PrimitiveMode_TriangleList, 0, indexBuffer->GetIndexCount());
				}

				if (m_lightMeshesDrawing)
				{
					Renderer::Enable(RendererParameter_DepthBuffer, true);
					Renderer::Enable(RendererParameter_DepthWrite, true);
					Renderer::Enable(RendererParameter_FaceCulling, false);
					Renderer::Enable(RendererParameter_StencilTest, false);
					Renderer::SetFaceFilling(FaceFilling_Line);

					const Shader* shader = ShaderLibrary::Get("DebugSimple");
					static int colorLocation = shader->GetUniformLocation("Color");

					Renderer::SetShader(shader);
					for (const auto& light : m_renderQueue->spotLights)
					{
						float baseRadius = light.radius * light.outerAngleTangent * 1.1f;
						lightMatrix.MakeTransform(light.position, Quaternionf::RotationBetween(Vector3f::Forward(), light.direction), Vector3f(baseRadius, baseRadius, light.radius));

						Renderer::SetMatrix(MatrixType_World, lightMatrix);

						shader->SendColor(colorLocation, light.color);

						Renderer::DrawIndexedPrimitives(PrimitiveMode_TriangleList, 0, indexBuffer->GetIndexCount());
					}

					Renderer::Enable(RendererParameter_DepthBuffer, false);
					Renderer::Enable(RendererParameter_DepthWrite, false);
					Renderer::Enable(RendererParameter_FaceCulling, true);
					Renderer::Enable(RendererParameter_StencilTest, true);
					Renderer::SetFaceFilling(FaceFilling_Fill);
				}
			}

			Renderer::Enable(RendererParameter_StencilTest, false);
		}

		return true;
	}
}
