#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{Vector3::UnitZ};
		//Vector3 forward{ 0.266f, -0.453f, 0.860f }; //test the camera rotation
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f}; //c
		float totalYaw{0.f};  //a

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			Matrix ONB{};
			right = { Vector3::Cross(Vector3::UnitY, forward).Normalized() };
			up = { Vector3::Cross(forward, right).Normalized() };

			ONB =
			{
				{right.x, right.y, right.z, 0},
				{up.x, up.y, up.z, 0},
				{forward.x, forward.y, forward.z, 0},
				{origin.x,origin.y,origin.z, 1}
			};
			
			return ONB;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += forward * deltaTime * 5;
			}
			if (pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin -= forward * deltaTime * 5;
			}
			if (pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += right * deltaTime * 5;
			}
			if (pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin -= right * deltaTime * 5;
			}


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouseState == SDL_BUTTON_RMASK)
			{
				SDL_SetRelativeMouseMode(SDL_TRUE);
				totalPitch -= mouseY * deltaTime;
				totalYaw += mouseX * deltaTime;
			}
			else SDL_SetRelativeMouseMode(SDL_FALSE);

			const Matrix rotation{ Matrix::CreateRotation( totalPitch, totalYaw, 0) };
			forward = rotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
		}
	};
}
