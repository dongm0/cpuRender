#include "rasterizer.h"

static bool inTriangle(int i, int j, std::array<Eigen::Vector4f, 3> &triangle) {
    float posx = i + 0.5, posy = j + 0.5;
    Eigen::Vector3f v0 = { triangle[0][0], triangle[0][1], 1. };
    Eigen::Vector3f v1 = { triangle[1][0], triangle[1][1], 1. };
    Eigen::Vector3f v2 = { triangle[2][0], triangle[2][1], 1. };
    Eigen::Vector3f v;
    v << posx, posy, 1.;

    auto c1 = (v0 - v).cross(v1 - v);
    auto c2 = (v1 - v).cross(v2 - v);
    auto c3 = (v2 - v).cross(v1 - v);
    if (c1.dot(c2) < 0 or c2.dot(c3) < 0 or c1.dot(c3) < 0)
        return false;
    return true;
}

static std::tuple<float, float, float> baryCenterCoord(int i, int j, std::array<Eigen::Vector4f, 3>& triangle) {
    float x = i + 0.5, y = j + 0.5;
    float x0 = triangle[0][0], x1 = triangle[1][0], x2 = triangle[2][0];
    float y0 = triangle[0][1], y1 = triangle[1][1], y2 = triangle[2][1];
    float c1 = (x * (y1 - y2) + y * (x2 - x1) + x1 * y2 - x2 * y1) / (x0 * (y1 - y2) + y0 * (x2 - x1) + x1 * y2 - x2 * y1);
    float c2 = (x * (y2 - y0) + y * (x0 - x2) + x2 * y0 - x0 * y2) / (x1 * (y2 - y0) + y1 * (x0 - x2) + x2 * y0 - x0 * y2);
    float c3 = 1 - c1 - c2;
    return { c1, c2, c3 };
}

void rst::SoftRasterizer::Draw(std::vector<Triangle> tri_list) {
    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;
    for (const auto& t : tri_list) {
        Triangle newtri = t;
        newtri.v[0] = m_mv * newtri.v[0];
        newtri.v[1] = m_mv * newtri.v[1];
        newtri.v[2] = m_mv * newtri.v[2];
        newtri.normal[0] = m_mvInvT * newtri.normal[0];
        newtri.normal[1] = m_mvInvT * newtri.normal[1];
        newtri.normal[2] = m_mvInvT * newtri.normal[2];
        //clip
        //clip finish
        for (auto& p : newtri.v) {
            p.x() /= p.w();
            p.y() /= p.w();
            p.z() /= p.w();
        }
        std::array<Eigen::Vector4f, 3> proj_pos;
        proj_pos[0] = m_projection * newtri.v[0];
        proj_pos[1] = m_projection * newtri.v[1];
        proj_pos[2] = m_projection * newtri.v[2];
        for (auto& p : proj_pos) {
            p.x() /= p.w();
            p.y() /= p.w();
            p.z() /= p.w();
        }

        // VERTEX SHADER -> MVP -> Clipping -> /.W -> VIEWPORT -> DRAWLINE/DRAWTRI -> FRAGSHADER
        //for (auto& p : newtri.v) {
            //p.x() = (p.x() + 1) * 0.5 * m_width;
            //p.y() = (p.y() + 1) * 0.5 * m_height;
            //p.z() = p.z() * f1 + f2;
        //}
        for (auto& p : proj_pos) {
            p.x() = (p.x() + 1) * 0.5 * m_width;
            p.y() = (p.y() + 1) * 0.5 * m_height;
            p.z() = p.z() * f1 + f2;
        }
        

        rasterizeTriangle(newtri, proj_pos);
    }
}

void rst::SoftRasterizer::rasterizeLine(Eigen::Vector4f& st, Eigen::Vector4f& ed) {

}


void rst::SoftRasterizer::rasterizeTriangle(const Triangle& t, std::array<Eigen::Vector4f, 3> proj_pos) {
    //simple version first, using bounding box
    float minxf, maxxf, minyf, maxyf;
    minxf = std::min({ proj_pos[0].x(), proj_pos[1].x(), proj_pos[2].x() });
    maxxf = std::max({ proj_pos[0].x(), proj_pos[1].x(), proj_pos[2].x() });
    minyf = std::min({ proj_pos[0].y(), proj_pos[1].y(), proj_pos[2].y() });
    maxyf = std::max({ proj_pos[0].y(), proj_pos[1].y(), proj_pos[2].y() });
    //std::minmax({ proj_pos[0].x(), proj_pos[1].x(), proj_pos[2].x() });
    int minx = minxf, maxx = maxxf + 1, miny = minyf, maxy = maxyf + 1;
    for (int i = minx; i < maxx; ++i) {
        for (int j = miny; j < maxy; ++j) {
            if (inTriangle(i, j, proj_pos)) {
                auto [b1, b2, b3] = baryCenterCoord(i, j, proj_pos);
                float dp = proj_pos[0].z() * b1 + proj_pos[1].z() * b2 + proj_pos[2].z() * b3;
                //add a judge whether zbuffer is used.
                if (dp > m_depthBuffer[i * m_width + j]) {
                    m_depthBuffer[i * m_width + j] = dp;
                    auto p_pos = t.v[0] * b1 + t.v[1] * b2 + t.v[2] * b3;
                    auto p_color = t.color[0] * b1 + t.color[1] * b2 + t.color[2] * b2;
                    auto p_normal = t.normal[0] * b1 + t.normal[1] * b2 + t.normal[3] * b3;
                    auto p_uv = t.uv[0] * b1 + t.uv[1] * b2 + t.uv[2] * b3;
                    SoftFragmentShaderPayload payload(p_pos, p_color, p_normal, p_uv);
                    auto pixel = fragmentShader(payload);
                    m_frameBuffer[i * m_width + j] = pixel;
                }
                SoftFragmentShaderPayload payload();
            }
        }
    }
}
