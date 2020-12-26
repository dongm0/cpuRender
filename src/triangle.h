// ----
// Author: dongmo
// Email: dong_mo@163.com
// Date: 2020.12.13
// ----

#pragma once

#include <Eigen/Eigen>
#include <array>

#include <algorithm>


using namespace Eigen;
class Triangle {
public:
    Vector4f v[3];
    Vector3f color[3];
    Vector4f normal[3];
    Vector2f uv[3];

    Triangle() { 
        v[0] << 0, 0, 0, 1.;
        v[1] << 0, 0, 0, 1.;
        v[2] << 0, 0, 0, 1.;

        color[0] << 0.0, 0.0, 0.0;
        color[1] << 0.0, 0.0, 0.0;
        color[2] << 0.0, 0.0, 0.0;

        normal[0] << 0, 0, 0, 1.0;
        normal[1] << 0, 0, 0, 1.0;
        normal[2] << 0, 0, 0, 1.0;

        uv[0] << 0, 0;
        uv[1] << 0, 0;
        uv[2] << 0, 0;
    }

    Vector4f a() const { return v[0]; }
    Vector4f b() const { return v[1]; }
    Vector4f c() const { return v[2]; }

    void setVertex(int ind, Vector3f ver) { v[ind] << ver[0], ver[1], ver[2], 1.f; }
    void setNormal(int ind, Vector3f n) { normal[ind] << n[0], n[1], n[2], 1.f; } /*set i-th vertex normal vector*/
    void setColor(int ind, float r, float g, float b) {
        r = std::max(0.f, r), r = std::min(255.f, r);
        g = std::max(0.f, g), r = std::min(255.f, g);
        b = std::max(0.f, b), b = std::min(255.f, b);
        color[ind] = Vector3f(r, g, b);
    }

    void setNormals(const std::array<Vector3f, 3>& normals) {
        setNormal(0, normals[0]);
        setNormal(1, normals[1]);
        setNormal(2, normals[2]);
    }
    void setColors(const std::array<Vector3f, 3>& colors) {color[0] = colors[0], color[1] = colors[1], color[2] = colors[2]}
    void setTexCoord(int ind, Vector2f _uv) {uv[ind] = _uv;} /*set i-th vertex texture coordinate*/
    //std::array<Vector4f, 3> toVector4() const;
};