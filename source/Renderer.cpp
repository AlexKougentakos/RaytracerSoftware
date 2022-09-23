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

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			/*float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;*/

			//Raster to NDC
			float NDCx{ (px + 0.5f) / m_Width };
			float NDCy{ (py + 0.5f) / m_Height };

			//NDC to Screen
			float ScreenX{ 2 * NDCx - 1 };
			float ScreenY{  1 - 2 * NDCy};

			//Screen To Cam
			float CamX { ScreenX * (float(m_Width) / float(m_Height)) };
			float CamY{ ScreenY };

			Vector3 rayDirection{ CamX, CamY, 1 };
			rayDirection.Normalize();
			Ray viewRay{ {0,0,0}, rayDirection };

			ColorRGB finalColor{ };
			
			HitRecord closestHit;
			
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();
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
