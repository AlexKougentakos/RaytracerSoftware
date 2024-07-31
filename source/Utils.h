#pragma once

#include <string>
#include <vector>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
    namespace GeometryUtils
    {
        // Sphere Hit-Tests
        bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false);
        bool HitTest_Sphere(const Sphere& sphere, const Ray& ray);

        // Plane Hit-Tests
        bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false);
        bool HitTest_Plane(const Plane& plane, const Ray& ray);

        // Triangle Hit-Tests
        bool IsPointOnTheInsideOfEdge(const Vector3& point, const Vector3& v0, const Vector3& v1, const Vector3& normal);
        bool HitTest_Triangle(Vector3 v1, Vector3 v2, Vector3 v3, TriangleCullMode cullMode, unsigned char materialIndex, const Vector3& transformedNormal,
            const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false);
        bool HitTest_Triangle(const Triangle& triangle, const Ray& ray);

        // Triangle Mesh Hit-Tests
        bool HitTest_SlabTest(const TriangleMesh& mesh, const Ray& ray);
        bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false);
        bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray);
    }

    namespace LightUtils
    {
        Vector3 GetDirectionToLight(const Light& light, const Vector3 origin);
        ColorRGB GetRadiance(const Light& light, const Vector3& target);
    }

    namespace Utils
    {
        bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices);
    }
}