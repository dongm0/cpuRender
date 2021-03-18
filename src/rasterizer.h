// --------------------------
// Author: dongmo
// Email: dong_mo@163.com
// Date: 2020.12.13
// --------------------------

#pragma once

#include <Eigen/Eigen>

// stl
#include <functional>
#include <optional>
#include <vector>

#include "triangle.h"

namespace rst {
struct SoftFragmentShaderPayload {
  Eigen::Vector3f view_pos;
  Eigen::Vector3f color;
  Eigen::Vector3f normal;
  Eigen::Vector2f uv;

  // std::optional<Texture*> texture;

  SoftFragmentShaderPayload(const Eigen::Vector3f &v, const Eigen::Vector3f &c,
                            const Eigen::Vector3f &n, const Eigen::Vector2f &tc)
      : view_pos(v), color(c), normal(n), uv(tc) {}
};

struct SoftVertexShaderPayload {
  Eigen::Vector3f position;

  SoftVertexShaderPayload(const Eigen::Vector3f &p) : position(p) {}
};

class SoftRasterizer {
public:
  SoftRasterizer(int w, int h) : m_width(w), m_height(h) {
    m_frameBuffer.resize(w * h);
    m_depthBuffer.resize(w * h);
  }

  void SetModelMatrix(const Eigen::Matrix4f &m) { m_model = m; }
  void SetViewMatrix(const Eigen::Matrix4f &v) { m_view = v; }
  void SetProjectionMatrix(const Eigen::Matrix4f &p) { m_projection = p; }

  void SetFragmentShader(
      std::function<Eigen::Vector3f(SoftFragmentShaderPayload)> fs) {
    fragmentShader = fs;
  }
  void
  SetVertexShader(std::function<Eigen::Vector3f(SoftVertexShaderPayload)> vs) {
    vertexShader = vs;
  }

  void Draw(std::vector<Triangle> tri_list);
  std::vector<Eigen::Vector3f> &GetFrameBuffer() { return m_frameBuffer; }

  int GetWidth() { return m_width; }
  int GetHeight() { return m_height; }

  void ClearBuffer() {
    m_depthBuffer.assign(m_width * m_height, -10e8);
    m_frameBuffer.assign(m_width * m_height, {0, 0, 0});
  }

  void UpdateTransform() {
    m_mvp = m_projection * m_view * m_model;
    m_mv = m_view * m_model;
    m_mvInvT = m_mv.inverse().transpose();
  }

private:
  int m_width;
  int m_height;

  Eigen::Matrix4f m_model;
  Eigen::Matrix4f m_view;
  Eigen::Matrix4f m_projection;

  Eigen::Matrix4f m_mvp;
  Eigen::Matrix4f m_mv;
  Eigen::Matrix4f m_mvInvT;

  std::function<Eigen::Vector3f(SoftFragmentShaderPayload)> fragmentShader;
  std::function<Eigen::Vector3f(SoftVertexShaderPayload)> vertexShader;

  std::vector<Eigen::Vector3f> m_frameBuffer;
  std::vector<float> m_depthBuffer;

  // 0: no cull 1: z forward 2: z backward
  const uint8_t m_cull = 1;

private:
  void rasterizeTriangleWithClip(Triangle &t,
                                 std::array<Eigen::Vector4f, 3> &proj_pos);
  void rasterizeTriangle(const Triangle &t,
                         std::array<Eigen::Vector4f, 3> &proj_pos);
  void rasterizeLine(Eigen::Vector4f &st, Eigen::Vector4f &ed);
};

inline Eigen::Matrix4f
GetModelMatrix(Eigen::Vector3f base, Eigen::Quaternionf rotation, float scale) {
  Eigen::Matrix4f res;
  return res;
}
inline Eigen::Matrix4f GetModelMatrix(Eigen::Vector3f base, float scale) {
  Eigen::Matrix4f res;
  res << scale, 0, 0, base[0], 0, scale, 0, base[1], 0, 0, scale, base[2], 0, 0,
      0, 1;
  return res;
}
inline Eigen::Matrix4f GetViewMatrix(Eigen::Vector3f base,
                                     Eigen::Vector3f lookat,
                                     Eigen::Vector3f head) {
  Eigen::Matrix4f res;
  return res;
}
inline Eigen::Matrix4f GetViewMatrix(Eigen::Vector3f base) {
  Eigen::Matrix4f res;
  res << 1, 0, 0, -base[0], 0, 1, 0, -base[1], 0, 0, 1, -base[2], 0, 0, 0, 1;
  return res;
}
inline Eigen::Matrix4f GetProjMatrix(float fovy, float asp_ratio, float zn,
                                     float zf) {
  Eigen::Matrix4f move, squeeze, proj, res;
  move << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, -(zn + zf) / 2, 0, 0, 0, 1;
  float alpha = fovy * atan(1) * 4 / 180;
  float h = (-zn) * tan(alpha / 2) * 2;
  float w = h / asp_ratio;
  squeeze << 2 / w, 0, 0, 0, 0, 2 / h, 0, 0, 0, 0, 2 / (-zf + zn), 0, 0, 0, 0,
      1;
  proj << zn, 0, 0, 0, 0, zn, 0, 0, 0, 0, zn + zf, -zn * zf, 0, 0, 1, 0;
  res = squeeze * move * proj;
  return res;
}
} // namespace rst