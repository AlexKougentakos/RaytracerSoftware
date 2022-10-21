#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{	
			Vector3 L{ sphere.origin - ray.origin };
			Vector3 d{ ray.direction.Normalized() };
			float Tca{ Vector3::Dot(L, d) };
			float od2{ Square(Vector3::Reject(L, d).Magnitude()) };

			if (od2 > Square(sphere.radius))
				return false;

			float Thc = SquareRootImp(Square(sphere.radius) - od2);
			//float Thc = std::sqrtf(Square(sphere.radius) - od2);
			float t0 = Tca - Thc;
			float t1 = Tca + Thc;
			float t{ t1 };
			if ((t0 < t1) && (t0 > 0))
			{
				t = t0;
			}

			if (t > ray.min && t < ray.max)
			{
				Vector3 p{ ray.origin + t * d };

				hitRecord.didHit = true;
				hitRecord.origin = p;
				hitRecord.t = t;

				hitRecord.normal = p - sphere.origin;
				hitRecord.normal.Normalize();

				hitRecord.materialIndex = sphere.materialIndex;
			}

			return hitRecord.didHit;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//if (Vector3::Dot(plane.normal, ray.direction) == 0) return false;
			
			float t = ( Vector3::Dot( (plane.origin - ray.origin), plane.normal.Normalized() / Vector3::Dot(ray.direction, plane.normal)) );

			if (t >= ray.min && t <= ray.max)
			{
				Vector3 interPoint{ ray.origin.x + ray.direction.x * t, ray.origin.y + ray.direction.y * t, ray.origin.z + ray.direction.z * t};

				hitRecord.didHit = true;
				hitRecord.origin = interPoint;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.normal = plane.normal.Normalized();
				hitRecord.t = t;

			}

			return hitRecord.didHit;

		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			Vector3 vector{ light.origin - origin };
			
			return vector;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//ColorRGB radiance{ light.color.r * light.intensity, light.color.g * light.intensity,light.color.b * light.intensity };
			//return radiance;

			float irradiance{ light.intensity / Square((light.origin - target).Magnitude()) };
			ColorRGB radiance{ light.color.r * irradiance, light.color.g * irradiance, light.color.b * irradiance };
			return radiance;

		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}