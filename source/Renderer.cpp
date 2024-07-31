//Definitions
#define MULTITHREADING

//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Multithreading
#include <ppl.h>
#include <future>

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include  <iostream>

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	m_AspectRatio = { float(m_Width) / float(m_Height) };
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	static const float FOV{ tanf(TO_RADIANS * (camera.fovAngle / 2)) };
	const unsigned int nrPixels{ static_cast<unsigned int>(m_Width * m_Height) };

	camera.CalculateCameraToWorld();
#ifdef MULTITHREADING
	//Multithreading
	concurrency::parallel_for(0u, nrPixels,
		[=, this](int i)
		{
			RenderPixel(pScene, camera, materials, lights, FOV, i);
		});
#else

	for (int i{0}; i < nrPixels; ++i)
		RenderPixel(pScene, camera, materials, lights, FOV, i);
#endif


	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, const Camera& camera, const std::vector<Material*>& materials, const std::vector<Light>& lights, const float& FOV, unsigned int pixelIndex) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	//Raster to NDC
	const float NDCx{ (px + 0.5f) / m_Width };
	const float NDCy{ (py + 0.5f) / m_Height };

	//NDC to Screen
	const float ScreenX{ 2 * NDCx - 1 };
	const float ScreenY{ 1 - 2 * NDCy };

	//Screen To Cam
	const float CamX{ ScreenX * (float(m_Width) / float(m_Height)) * FOV };
	const float CamY{ ScreenY * FOV };

	Vector3 rayDirection{ CamX, CamY, 1 };

	rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();

	const Ray viewRay{ camera.origin, rayDirection };

	ColorRGB finalColor{};

	HitRecord closestHit{};

	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		for (size_t i{ 0 }; i < lights.size(); ++i)
		{
			Vector3 lightDir = LightUtils::GetDirectionToLight(lights[i], closestHit.origin + (closestHit.normal * 0.001f));
			const float lightrayMagnitude{ lightDir.Magnitude() };
			lightDir.Normalize();

			const float observedArea = Vector3::Dot(closestHit.normal, lightDir);
			if (observedArea < 0)
				continue;

			if (m_ShadowsEnabled)
			{
				Ray lightRay{ closestHit.origin + (closestHit.normal * 0.1f), lightDir };
				lightRay.max = lightrayMagnitude;
				if (pScene->DoesHit(lightRay))
				{
					continue;
				}
			}

			switch (m_CurrentLightMode)
			{
			case LightingMode::ObservedArea:
			{
				finalColor += ColorRGB{ 1,1,1 } *observedArea;
			}
			break;
			case LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin);
				break;
			case LightingMode::BRDF:
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightDir, viewRay.direction);
				break;
			case LightingMode::Combined:
			{
				finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin) * observedArea * materials[closestHit.materialIndex]->Shade(closestHit, lightDir, viewRay.direction);
			}
			break;
			}
		}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}


void Renderer::ToggleLightMode()
{
	switch (m_CurrentLightMode)
	{
	case LightingMode::ObservedArea:
		m_CurrentLightMode = LightingMode::Radiance;
		break;
	case LightingMode::Radiance:
		m_CurrentLightMode = LightingMode::BRDF;
		break;
	case LightingMode::BRDF:
		m_CurrentLightMode = LightingMode::Combined;
		break;
	case LightingMode::Combined:
		m_CurrentLightMode = LightingMode::ObservedArea;
		break;
	}
}

void Renderer::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}

