//External includes
#include "SDL.h"
#include "SDL_surface.h"

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
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
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
			const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

			rayDirection = cameraToWorld.TransformVector(rayDirection);

			Ray viewRay{ camera.origin, rayDirection };

			ColorRGB finalColor{ };

			HitRecord closestHit{};
			
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				for (int i = 0; i < lights.size(); i++)
				{
					Vector3 lightDir = LightUtils::GetDirectionToLight(lights[i], closestHit.origin + (closestHit.normal * 0.05f));
					Ray lightRay{ closestHit.origin + (closestHit.normal * 0.05f),lightDir };
					const float lightrayMagnitude{ lightDir.Normalize() };
					lightRay.max = lightrayMagnitude;

					float normalLightAngle{ Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(lights[i], closestHit.origin + (closestHit.normal * 0.05f)).Normalized()) };
					if (normalLightAngle < 0)
						continue;
					if (pScene->DoesHit(lightRay))
					{
						finalColor *= 0.5f;
						continue;
					}
					//Radiance * BRDF * ObservedArea
					finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit, lightDir, -viewRay.direction) * normalLightAngle;
				}
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
