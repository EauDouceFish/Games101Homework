#pragma once

#include "Object.hpp"

#include <cstring>

bool rayTriangleIntersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& orig,
                          const Vector3f& dir, float& tnear, float& u, float& v)
{
    // TODO: Implement this function that tests whether the triangle
    // that's specified bt v0, v1 and v2 intersects with the ray (whose
    // origin is *orig* and direction is *dir*)
    // Also don't forget to update tnear, u and v.


    //o + td = (1-b1-b2)P0 + b1P1 + b2P2
    //o + td = P0 + (P1-P0)b1 + (P2-P0)b2
    //o + td = v0 + (v1-v0)b1 + (v2-v0)b2
    //o - v0 = (v1-v0)b1 + (v2-v0)b2 - td
    auto S = orig - v0;
    auto E1 = v1-v0;
    auto E2 = v2-v0;
    //S = E1b1 + E2b2 - td
    //[-d E1 E2]*[[t b1 b2]] = S

    //According to Cramer's law
    //t  = det(S E1 E2) / det(-d E1 E2)
    //b1 = det(-d S E2) / det(-d E1 E2)
    //b2 = det(-d E1 S) / det(-d E1 E2)

    //det(S E1 E2) = (SxE1)*E2, Let SxE1=S2
    auto S2 = crossProduct(S, E1);
    //det(S E1 E2) S2*E2
 
    //det(-d E1 E2) = -d*(E1xE2) = d*(E2xE1) = (dxE2)*E1, Let dxE2 = S1
    auto S1 = crossProduct(dir, E2);
    //det(-d E1 E2) = S1*E1

    //so t = det(S2*E2) / det(S1*E1)
    float t = dotProduct(S2, E2)/dotProduct(S1, E1);


    //Same method to calculate b1, b2:
    //for b1:
    //det(-d S E2) = -d*(SxE2) = d*(E2xS) = (dxE2)*S = S1*S
    //for b2:
    //det(-d E1 S) = -d*(E1xS) = d*(SxE1) = d*S2
    float b1 = dotProduct(S1, S)/dotProduct(S1, E1);
    float b2 = dotProduct(dir, S2)/dotProduct(S1, E1);

    if(t>=0 && b1>=0 && b2>=0 && (1-b1-b2)>=0)
    {
        tnear = t;
        u = b1;
        v = b2;
        return true;//intersect with the triangle
    }

    return false;
}


//mesh Triangle receive a series of triangles as the object inputs
class MeshTriangle : public Object
{
public:
    MeshTriangle(const Vector3f* verts, const uint32_t* vertsIndex, const uint32_t& numTris, const Vector2f* st)
    {
        uint32_t maxIndex = 0;
        for (uint32_t i = 0; i < numTris * 3; ++i)
            if (vertsIndex[i] > maxIndex)
                maxIndex = vertsIndex[i];
        maxIndex += 1;
        vertices = std::unique_ptr<Vector3f[]>(new Vector3f[maxIndex]);
        memcpy(vertices.get(), verts, sizeof(Vector3f) * maxIndex);
        vertexIndex = std::unique_ptr<uint32_t[]>(new uint32_t[numTris * 3]) ;
        memcpy(vertexIndex.get(), vertsIndex, sizeof(uint32_t) * numTris * 3);
        numTriangles = numTris;
        stCoordinates = std::unique_ptr<Vector2f[]>(new Vector2f[maxIndex]);
        memcpy(stCoordinates.get(), st, sizeof(Vector2f) * maxIndex);
    }

    bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear, uint32_t& index,
                   Vector2f& uv) const override
    {
        bool intersect = false;
        for (uint32_t k = 0; k < numTriangles; ++k)
        {
            const Vector3f& v0 = vertices[vertexIndex[k * 3]];
            const Vector3f& v1 = vertices[vertexIndex[k * 3 + 1]];
            const Vector3f& v2 = vertices[vertexIndex[k * 3 + 2]];
            float t, u, v;
            if (rayTriangleIntersect(v0, v1, v2, orig, dir, t, u, v) && t < tnear)
            {
                tnear = t;
                uv.x = u;
                uv.y = v;
                index = k;
                intersect |= true;
            }
        }

        return intersect;
    }

    void getSurfaceProperties(const Vector3f&, const Vector3f&, const uint32_t& index, const Vector2f& uv, Vector3f& N,
                              Vector2f& st) const override
    {
        const Vector3f& v0 = vertices[vertexIndex[index * 3]];
        const Vector3f& v1 = vertices[vertexIndex[index * 3 + 1]];
        const Vector3f& v2 = vertices[vertexIndex[index * 3 + 2]];
        Vector3f e0 = normalize(v1 - v0);
        Vector3f e1 = normalize(v2 - v1);
        N = normalize(crossProduct(e0, e1));
        const Vector2f& st0 = stCoordinates[vertexIndex[index * 3]];
        const Vector2f& st1 = stCoordinates[vertexIndex[index * 3 + 1]];
        const Vector2f& st2 = stCoordinates[vertexIndex[index * 3 + 2]];
        st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
    }

    Vector3f evalDiffuseColor(const Vector2f& st) const override
    {
        float scale = 5;
        float pattern = (fmodf(st.x * scale, 1) > 0.5) ^ (fmodf(st.y * scale, 1) > 0.5);
        return lerp(Vector3f(0.815, 0.235, 0.031), Vector3f(0.937, 0.937, 0.231), pattern);
    }

    std::unique_ptr<Vector3f[]> vertices;
    uint32_t numTriangles;
    std::unique_ptr<uint32_t[]> vertexIndex;
    std::unique_ptr<Vector2f[]> stCoordinates;
};
