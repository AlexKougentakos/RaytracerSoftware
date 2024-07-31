#pragma once

#include <cstdint>

#include "Camera.h"
#include "Material.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;
		void ToggleShadows();
		void ToggleLightMode();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		void RenderPixel(Scene* pScene, const Camera& camera, const std::vector<Material*>& materials,
			const std::vector<Light>& lights, const float& FOV, unsigned int pixelIndex) const;

		enum class LightingMode
		{
			ObservedArea, //Lambert Cosine
			Radiance, //Incident Radiance
			BRDF, //Scattering of light
			Combined, //ObservedArea * Radiance * BRDF
		};

		LightingMode m_CurrentLightMode{ LightingMode::Combined };

		bool m_ShadowsEnabled{ false };

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};

		float m_Cx{}, m_Cy{};
	};
}
