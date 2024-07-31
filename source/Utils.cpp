#include "Utils.h"
#include <cassert>
#include <fstream>

namespace dae
{
    namespace GeometryUtils
    {
        bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord)
        {
            Vector3 L{ sphere.origin - ray.origin };
            Vector3 d{ ray.direction.Normalized() };
            float Tca{ Vector3::Dot(L, d) };
            float od2{ Square(Vector3::Reject(L, d).Magnitude()) };

            if (od2 > Square(sphere.radius))
                return false;

            float Thc = SquareRootImp(Square(sphere.radius) - od2);
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

        bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
        {
            HitRecord temp{};
            return HitTest_Sphere(sphere, ray, temp, true);
        }

        bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord)
        {
            float t = (Vector3::Dot((plane.origin - ray.origin), plane.normal.Normalized() / Vector3::Dot(ray.direction, plane.normal)));

            if (t >= ray.min && t <= ray.max)
            {
                Vector3 interPoint{ ray.origin.x + ray.direction.x * t, ray.origin.y + ray.direction.y * t, ray.origin.z + ray.direction.z * t };

                hitRecord.didHit = true;
                hitRecord.origin = interPoint;
                hitRecord.materialIndex = plane.materialIndex;
                hitRecord.normal = plane.normal.Normalized();
                hitRecord.t = t;
            }

            return hitRecord.didHit;
        }

        bool HitTest_Plane(const Plane& plane, const Ray& ray)
        {
            HitRecord temp{};
            return HitTest_Plane(plane, ray, temp, true);
        }

        bool IsPointOnTheInsideOfEdge(const Vector3& point, const Vector3& v0, const Vector3& v1, const Vector3& normal)
        {
            const Vector3 edge{ v1 - v0 };
            const Vector3 pointToVertexOne{ point - v0 };
            const Vector3 cross{ Vector3::Cross(edge, pointToVertexOne) };
            return Vector3::Dot(cross, normal) > 0;
        }

        bool DidHit_MollerTrombore(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Ray& ray, unsigned char materialIndex, const Vector3& transformedNormal, HitRecord& hitRecord)
        {
	        Vector3 edge1{}, edge2{}, h{}, s{}, q{};
	        float a{}, f{}, u{}, v{};
	        hitRecord.didHit = false;

	        edge1 = { v1 - v0 };
	        edge2 = { v2 - v0 };

	        h = Vector3::Cross(ray.direction, edge2);
	        a = Vector3::Dot(edge1, h);

	        if (AreEqual(a, 0))
		        return false;

	        f = 1.f / a;
	        s = ray.origin - v0;
	        u = f * Vector3::Dot(s, h);
	        if (u < 0.0f || u > 1.0f)
		        return false;
	        q = Vector3::Cross(s, edge1);
	        v = f * Vector3::Dot(ray.direction, q);
	        if (v < 0.0f || u + v > 1.0f)
		        return false;

	        float t{ f * Vector3::Dot(edge2, q) };
	        if (t > 0.f)
	        {
		        hitRecord.origin = ray.origin + ray.direction * t;
		        hitRecord.t = t;
		        hitRecord.didHit = true;
		        hitRecord.materialIndex = materialIndex;
		        hitRecord.normal = transformedNormal;
	        }

	        return hitRecord.didHit;
        }

        bool DidHit(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord)
        {
	        const Vector3 a{ triangle.v1 - triangle.v0 };
	        const Vector3 b{ triangle.v2 - triangle.v0 };

	        const Vector3 planeNormal = Vector3::Cross(a, b).Normalized();

	        if (AreEqual(Vector3::Dot(planeNormal, ray.direction), 0))
		        return false;

	        const Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3 };
	        const Vector3 L{ center - ray.origin };
	        const float t{ Vector3::Dot(L, planeNormal) / Vector3::Dot(ray.direction, planeNormal) };

	        if (t < ray.min || t > ray.max)
		        return false;

	        const Vector3 p{ ray.origin + t * ray.direction };

	        if (!(IsPointOnTheInsideOfEdge(p, triangle.v0, triangle.v1, planeNormal)
		        && IsPointOnTheInsideOfEdge(p, triangle.v1, triangle.v2, planeNormal)
		        && IsPointOnTheInsideOfEdge(p, triangle.v2, triangle.v0, planeNormal)))
		        return false;

	        hitRecord.normal = planeNormal;
	        hitRecord.didHit = true;
	        hitRecord.materialIndex = triangle.materialIndex;
	        hitRecord.origin = p;
	        hitRecord.t = t;

	        return hitRecord.didHit;
        }

        bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
        {
            HitRecord temp{};
            return HitTest_Triangle(triangle.v0, triangle.v1, triangle.v2, triangle.cullMode, triangle.materialIndex, triangle.normal, ray, temp, true);
        }

        bool HitTest_SlabTest(const TriangleMesh& mesh, const Ray& ray)
        {
            const float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
            const float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

            float tmin = std::min(tx1, tx2);
            float tmax = std::max(tx1, tx2);

            const float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
            const float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

            tmin = std::min(tmin, std::min(ty1, ty2));
            tmax = std::max(tmax, std::max(ty1, ty2));

            const float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
            const float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

            tmin = std::min(tmin, std::min(tz1, tz2));
            tmax = std::max(tmax, std::max(tz1, tz2));

            return tmax > 0 && tmax >= tmin;
        }

        bool HitTest_Triangle(Vector3 v1, Vector3 v2, Vector3 v3, TriangleCullMode cullMode, unsigned char materialIndex, const Vector3& transformedNormal,
            const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord)
        {
            hitRecord.didHit = false;

            if (cullMode == TriangleCullMode::BackFaceCulling
                && Vector3::Dot(transformedNormal, ray.direction) > 0.f)
                return false;
            if (cullMode == TriangleCullMode::FrontFaceCulling
                && Vector3::Dot(transformedNormal, ray.direction) < 0.f)
                return false;

            return DidHit_MollerTrombore(v1, v2, v3, ray, materialIndex, transformedNormal, hitRecord);
            //return DidHit(triangle, ray, hitRecord);
        }

        bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord)
        {
            if (!HitTest_SlabTest(mesh, ray))
                return false;

            HitRecord hit{};
            hitRecord.t = FLT_MAX;
            for (int i{ 0 }; i < (mesh.indices.size() / 3); ++i)
            {
                Triangle temp{};
                temp = { mesh.transformedPositions[mesh.indices[i * 3]], mesh.transformedPositions[mesh.indices[i * 3 + 1]], mesh.transformedPositions[mesh.indices[i * 3 + 2]] };
                temp.cullMode = mesh.cullMode;
                temp.materialIndex = mesh.materialIndex;
                temp.normal = mesh.transformedNormals[i];

                const bool didHitTriangle{ HitTest_Triangle(
                    mesh.transformedPositions[mesh.indices[i * 3]], 
                    mesh.transformedPositions[mesh.indices[i * 3 + 1]], 
                    mesh.transformedPositions[mesh.indices[i * 3 + 2]],
                    mesh.cullMode,
                    mesh.materialIndex,
                    mesh.transformedNormals[i],
                    ray, hit, ignoreHitRecord) };

                if (didHitTriangle)
                    if (hit.t < hitRecord.t) hitRecord = hit;
            }

            return hitRecord.didHit;
        }

        bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
        {
            HitRecord temp{};
            return HitTest_TriangleMesh(mesh, ray, temp, true);
        }
    }

    namespace LightUtils
    {
        Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
        {
            return light.origin - origin;
        }

        ColorRGB GetRadiance(const Light& light, const Vector3& target)
        {
            float irradiance{ light.intensity / Square((light.origin - target).Magnitude()) };
            return ColorRGB{ light.color.r * irradiance, light.color.g * irradiance, light.color.b * irradiance };
        }
    }

    namespace Utils
    {
        bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
        {
            std::ifstream file(filename);
            if (!file)
                return false;

            std::string sCommand;
            while (!file.eof())
            {
                file >> sCommand;
                if (sCommand == "#")
                {
                    // Ignore Comment
                }
                else if (sCommand == "v")
                {
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
                file.ignore(1000, '\n');

                if (file.eof())
                    break;
            }

            for (uint64_t index = 0; index < indices.size(); index += 3)
            {
                uint32_t i0 = indices[index];
                uint32_t i1 = indices[index + 1];
                uint32_t i2 = indices[index + 2];

                Vector3 edgeV0V1 = positions[i1] - positions[i0];
                Vector3 edgeV0V2 = positions[i2] - positions[i0];
                Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

                normal.Normalize();

                normals.push_back(normal);
            }

            return true;
        }
    }
}
