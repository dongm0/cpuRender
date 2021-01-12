#include "rasterizer.h"
#include "boxjudge.h"

#include <bitset>

static bool inTriangle(int i, int j, std::array<Eigen::Vector4f, 3> &triangle) {
  float posx = i + 0.5, posy = j + 0.5;
  Eigen::Vector3f v0 = {triangle[0][0], triangle[0][1], 1.};
  Eigen::Vector3f v1 = {triangle[1][0], triangle[1][1], 1.};
  Eigen::Vector3f v2 = {triangle[2][0], triangle[2][1], 1.};
  Eigen::Vector3f v;
  v << posx, posy, 1.;

  auto v00 = v - v0;
  auto v01 = v1 - v0;
  auto v02 = v2 - v0;

  float dot00 = v00.dot(v00);
  float dot01 = v00.dot(v01);
  float dot02 = v00.dot(v02);
  float dot11 = v01.dot(v01);
  float dot12 = v01.dot(v02);

  float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

  float u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
  if (u < 0 or u > 1) {
    return false;
  }
  float u1 = (dot00 * dot12 - dot01 * dot02) * inverDeno;
  if (u1 < 0 or u1 > 1) {
    return false;
  }

  return u + u1 <= 1;
}

static float interpolationX(float x1, float x2, float y1, float y2, float y) {
  // (y-y1)/(x-x1) = (y2-y1)/(x2-x1)
  return (y - y1) * (x2 - x1) / (y2 - y1) + x1;
}

static float interpolationY(float x1, float x2, float y1, float y2, float x) {
  // (y-y1)/(x-x1) = (y2-y1)/(x2-x1)
  return (y2 - y1) * (x - x1) / (x2 - x1) + y1;
}

static std::array<float, 4>
calculateBaryCenterPara(std::array<Eigen::Vector4f, 3> &triangle) {
  std::array<float, 4> res;
  float x0 = triangle[0][0], x1 = triangle[1][0], x2 = triangle[2][0];
  float y0 = triangle[0][1], y1 = triangle[1][1], y2 = triangle[2][1];
  res[0] = (x1 - x2) / ((y0 - y2) * (x1 - x2) - (x0 - x2) * (y1 - y2));
  res[1] = (y1 - y2) / ((y0 - y2) * (x1 - x2) - (x0 - x2) * (y1 - y2));
  res[2] = (x0 - x2) / ((y1 - y2) * (x0 - x2) - (x1 - x2) * (y0 - y2));
  res[3] = (y0 - y2) / ((y1 - y2) * (x0 - x2) - (x1 - x2) * (y0 - y2));
  return res;
}

static std::tuple<float, float, float>
baryCenterCoord(int i, int j, std::array<Eigen::Vector4f, 3> &triangle,
                std::array<float, 4> &para) {
  float x = i + 0.5, y = j + 0.5;
  float c1 = (y - triangle[2][1]) * para[0] - (x - triangle[2][0]) * para[1];
  float c2 = (y - triangle[2][1]) * para[2] - (x - triangle[2][0]) * para[3];
  float c3 = 1 - c1 - c2;
  return {c1, c2, c3};
}

static bool triangleDirection(float x1, float x2, float x3, float y1, float y2,
                              float y3) {
  Eigen::Vector3f v1(x2 - x1, y2 - y1, 0), v2(x3 - x2, y3 - y2, 0);
  return v1.cross(v2).z() > 0;
}

void rst::SoftRasterizer::Draw(std::vector<Triangle> tri_list) {
  float f1 = (50 - 0.1) / 2.0;
  float f2 = (50 + 0.1) / 2.0;
  for (const auto &t : tri_list) {
    Triangle newtri = t;
    newtri.v[0] = m_mv * newtri.v[0];
    newtri.v[1] = m_mv * newtri.v[1];
    newtri.v[2] = m_mv * newtri.v[2];
    newtri.normal[0] = m_mvInvT * newtri.normal[0];
    newtri.normal[1] = m_mvInvT * newtri.normal[1];
    newtri.normal[2] = m_mvInvT * newtri.normal[2];

    for (auto &p : newtri.v) {
      p.x() /= p.w();
      p.y() /= p.w();
      p.z() /= p.w();
      p.w() = 1;
    }

    std::array<Eigen::Vector4f, 3> proj_pos;
    proj_pos[0] = m_projection * newtri.v[0];
    proj_pos[1] = m_projection * newtri.v[1];
    proj_pos[2] = m_projection * newtri.v[2];
    for (auto &p : proj_pos) {
      p.x() /= p.w();
      p.y() /= p.w();
      p.z() /= p.w();
      p.w() = 1;
    }

    // VERTEX SHADER -> MVP -> Clipping -> /.W -> VIEWPORT -> DRAWLINE/DRAWTRI
    // -> FRAGSHADER

    for (auto &p : proj_pos) {
      p.x() = (p.x() + 1) * 0.5 * m_width;
      p.y() = (p.y() + 1) * 0.5 * m_height;
    }
    if (m_cull != 0) {
      if (m_cull == 1) {
        if (triangleDirection(proj_pos[0].x(), proj_pos[1].x(), proj_pos[2].x(),
                              proj_pos[0].y(), proj_pos[1].y(),
                              proj_pos[2].y())) {
          rasterizeTriangle(newtri, proj_pos);
        }
      } else if (m_cull == 2) {
        if (!triangleDirection(proj_pos[0].x(), proj_pos[1].x(),
                               proj_pos[2].x(), proj_pos[0].y(),
                               proj_pos[1].y(), proj_pos[2].y())) {
          rasterizeTriangle(newtri, proj_pos);
        }
      }
    } else {
      rasterizeTriangle(newtri, proj_pos);
    }
  }
}

void rst::SoftRasterizer::rasterizeLine(Eigen::Vector4f &st,
                                        Eigen::Vector4f &ed) {}

void rst::SoftRasterizer::rasterizeTriangleWithClip(
    Triangle &t, std::array<Eigen::Vector4f, 3> &proj_pos) {
  std::array<std::bitset<9>, 3> vert_status;
  for (int i = 0; i < 3; ++i) {
    if (proj_pos[i].x() > 1) {
      vert_status[i][1] = 1;
    } else if (proj_pos[i].x() < -1) {
      vert_status[i][2] = 1;
    } else {
      vert_status[i][0] = 1;
    }

    if (proj_pos[i].y() > 1) {
      vert_status[i][4] = 1;
    } else if (proj_pos[i].y() < -1) {
      vert_status[i][5] = 1;
    } else {
      vert_status[i][3] = 1;
    }

    if (proj_pos[i].z() > 1) {
      vert_status[i][7] = 1;
    } else if (proj_pos[i].z() < -1) {
      vert_status[i][8] = 1;
    } else {
      vert_status[i][6] = 1;
    }
  }
  if (vert_status[0][0] and vert_status[0][3] and vert_status[0][6] and
      vert_status[1][0] and vert_status[1][3] and vert_status[1][6] and
      vert_status[2][0] and vert_status[2][3] and vert_status[2][6]) {
    rasterizeTriangle(t, proj_pos);
    return;
  }
  if (vert_status[0][1] and vert_status[1][1] and vert_status[2][1]) {
    return;
  }
  if (vert_status[0][2] and vert_status[1][2] and vert_status[2][2]) {
    return;
  }
  if (vert_status[0][4] and vert_status[1][4] and vert_status[2][4]) {
    return;
  }
  if (vert_status[0][5] and vert_status[1][5] and vert_status[2][5]) {
    return;
  }
  if (vert_status[0][7] and vert_status[1][7] and vert_status[2][7]) {
    return;
  }
  if (vert_status[0][8] and vert_status[1][8] and vert_status[2][8]) {
    return;
  }
  // only clip znear face
  int num_out = 0;
  std::vector<int> vert_num;
  for (int i = 0; i < 3; ++i) {
    if (vert_status[i][7]) {
      num_out += 1;
    }
  }
  if (num_out == 1) {
    //  p1----------p0
    //  \ nt1       /
    //   \     nt2 /
    //    np2-----np1
    //     \     /
    //      \   /
    //       \ /
    //        p2
    int _s0, _s1, _s2;
    for (int i = 0; i < 3; ++i) {
      if (vert_status[(i + 2) % 3][7]) {
        _s0 = i, _s1 = (i + 1) % 3, _s2 = (i + 2) % 3;
        break;
      }
    }
    mPoint p0{t.v[_s0].x(), t.v[_s0].y(), t.v[_s0].z()};
    mPoint p1{t.v[_s1].x(), t.v[_s1].y(), t.v[_s1].z()};
    mPoint p2{t.v[_s2].x(), t.v[_s2].y(), t.v[_s2].z()};
    auto np1 = rayCrossZFace(p0, p2, 1);
    auto np2 = rayCrossZFace(p1, p2, 1);
    float para1 = (np1.x - p0.x) / (p2.x - p0.x);
    float para2 = (np2.x - p1.x) / (p2.x - p1.x);
    Triangle nt1 = t, nt2 = t;
    nt1.setVertex(_s2, {np2.x, np2.y, np2.z});
    nt1.color[_s2] = t.color[_s1] * para2 + t.color[_s2] * (1 - para2);
    nt1.normal[_s2] = t.normal[_s1] * para2 + t.normal[_s2] * (1 - para2);
    nt1.color[_s2] = t.color[_s1] * para2 + t.color[_s2] * (1 - para2);
    nt1.uv[_s2] = t.uv[_s1] * para2 + t.uv[_s2] * (1 - para2);

    nt2.setVertex(_s1, {np2.x, np2.y, np2.z});
    nt2.color[_s1] = t.color[_s1] * para2 + t.color[_s2] * (1 - para2);
    nt2.normal[_s1] = t.normal[_s1] * para2 + t.normal[_s2] * (1 - para2);
    nt2.color[_s1] = t.color[_s1] * para2 + t.color[_s2] * (1 - para2);
    nt2.uv[_s1] = t.uv[_s1] * para2 + t.uv[_s2] * (1 - para2);

    nt2.setVertex(_s2, {np1.x, np1.y, np1.z});
    nt2.color[_s2] = t.color[_s0] * para1 + t.color[_s2] * (1 - para1);
    nt2.normal[_s2] = t.normal[_s0] * para1 + t.normal[_s2] * (1 - para1);
    nt2.color[_s2] = t.color[_s0] * para1 + t.color[_s2] * (1 - para1);
    nt2.uv[_s2] = t.uv[_s0] * para1 + t.uv[_s2] * (1 - para1);

    std::array<Eigen::Vector4f, 3> proj_pos1{
        proj_pos[_s0], proj_pos[_s1], {np2.x, np2.y, np2.z, 1.f}};
    std::array<Eigen::Vector4f, 3> proj_pos2{
        proj_pos[_s0], {np2.x, np2.y, np2.z, 1.f}, {np1.x, np1.y, np1.z, 1.f}};

    rasterizeTriangle(nt1, proj_pos1);
    rasterizeTriangle(nt2, proj_pos2);
  } else if (num_out == 2) {
    //       p0
    //      / \
    //     /   \
    //    np1--np2
    //   /       \
    //  /         \
    // p1----------p2
    int _s0, _s1, _s2;
    for (int i = 0; i < 3; ++i) {
      if (vert_status[(i + 1) % 3][7] and vert_status[(i + 2) % 3][7]) {
        _s0 = i, _s1 = (i + 1) % 3, _s2 = (i + 2) % 3;
        break;
      }
    }
    mPoint p0{t.v[_s0].x(), t.v[_s0].y(), t.v[_s0].z()};
    mPoint p1{t.v[_s1].x(), t.v[_s1].y(), t.v[_s1].z()};
    mPoint p2{t.v[_s2].x(), t.v[_s2].y(), t.v[_s2].z()};
    auto np1 = rayCrossZFace(p0, p1, 1);
    auto np2 = rayCrossZFace(p0, p2, 1);
    float para1 = (np1.x - p0.x) / (p1.x - p0.x);
    float para2 = (np2.x - p0.x) / (p2.x - p0.x);

    t.setVertex(_s1, {np1.x, np1.y, np1.z});
    t.color[_s1] = t.color[_s0] * para1 + t.color[_s1] * (1 - para1);
    t.normal[_s1] = t.normal[_s0] * para1 + t.normal[_s1] * (1 - para1);
    t.color[_s1] = t.color[_s0] * para1 + t.color[_s1] * (1 - para1);
    t.uv[_s1] = t.uv[_s0] * para1 + t.uv[_s1] * (1 - para1);

    t.setVertex(_s2, {np2.x, np2.y, np2.z});
    t.color[_s2] = t.color[_s0] * para2 + t.color[_s2] * (1 - para2);
    t.normal[_s2] = t.normal[_s0] * para2 + t.normal[_s2] * (1 - para2);
    t.color[_s2] = t.color[_s0] * para2 + t.color[_s2] * (1 - para2);
    t.uv[_s2] = t.uv[_s0] * para2 + t.uv[_s2] * (1 - para2);

    std::array<Eigen::Vector4f, 3> proj_pos_new{
        proj_pos[_s0], {np1.x, np1.y, np1.z, 1.f}, {np2.x, np2.y, np2.z, 1.f}};

    rasterizeTriangle(t, proj_pos_new);

  } else {
#ifndef NDEBUG
    throw std::runtime_error("num of vert out of znear face error!");
#endif
  }
}

void rst::SoftRasterizer::rasterizeTriangle(
    const Triangle &t, std::array<Eigen::Vector4f, 3> &proj_pos) {
  int maxypos, midypos, minypos;
  if (proj_pos[0].y() > proj_pos[1].y()) {
    if (proj_pos[0].y() > proj_pos[2].y()) {
      if (proj_pos[1].y() > proj_pos[2].y()) {
        maxypos = 0;
        midypos = 1;
        minypos = 2;
      } else {
        maxypos = 0;
        midypos = 2;
        minypos = 1;
      }
    } else {
      maxypos = 2;
      midypos = 0;
      minypos = 1;
    }
  } else {
    if (proj_pos[1].y() > proj_pos[2].y()) {
      if (proj_pos[0].y() > proj_pos[2].y()) {
        maxypos = 1;
        midypos = 0;
        minypos = 2;
      } else {
        maxypos = 1;
        midypos = 2;
        minypos = 0;
      }
    } else {
      maxypos = 2;
      midypos = 1;
      minypos = 0;
    }
  }
  float x0 = proj_pos[minypos].x(), x1 = proj_pos[midypos].x(),
        x2 = proj_pos[maxypos].x(), y0 = proj_pos[minypos].y(),
        y1 = proj_pos[midypos].y(), y2 = proj_pos[maxypos].y();
  int miny = y0, midy = y1, maxy = y2;
  auto para = calculateBaryCenterPara(proj_pos);
  for (int y = miny; y <= maxy; ++y) {
    if (y >= m_height or y < 0) {
      continue;
    }
    int minx, maxx;
    if (y < midy) {
      std::tie(minx, maxx) =
          std::minmax(interpolationX(x0, x1, y0, y1, y + 0.5f),
                      interpolationX(x0, x2, y0, y2, y + 0.5f));
    } else if (y > midy) {
      std::tie(minx, maxx) =
          std::minmax(interpolationX(x0, x2, y0, y2, y + 0.5f),
                      interpolationX(x1, x2, y1, y2, y + 0.5f));
    } else {
      std::tie(minx, maxx) =
          std::minmax(interpolationX(x0, x2, y0, y2, y + 0.5f), x1);
    }
    for (int x = minx; x <= maxx; ++x) {
      if (x < 0 or x >= m_width) {
        continue;
      }
      auto [b1, b2, b3] = baryCenterCoord(x, y, proj_pos, para);
      if (b1 >= 0 and b1 <= 1 and b2 >= 0 and b2 <= 1 and b3 >= 0 and b3 <= 1) {
        float dp =
            proj_pos[0].z() * b1 + proj_pos[1].z() * b2 + proj_pos[2].z() * b3;
        // add a judge whether zbuffer is used.
        if (dp > m_depthBuffer[y * m_width + x]) {
          m_depthBuffer[y * m_width + x] = dp;
          auto p_pos = t.v[0] * b1 + t.v[1] * b2 + t.v[2] * b3;
          auto p_color = t.color[0] * b1 + t.color[1] * b2 + t.color[2] * b2;
          auto p_normal =
              t.normal[0] * b1 + t.normal[1] * b2 + t.normal[2] * b3;
          auto p_uv = t.uv[0] * b1 + t.uv[1] * b2 + t.uv[2] * b3;
          SoftFragmentShaderPayload payload(p_pos.head<3>(), p_color,
                                            p_normal.head<3>(), p_uv);
          auto pixel = fragmentShader(payload);
          m_frameBuffer[y * m_width + x] = pixel;
        }
      }
    }
  }

  /*

  float minxf, maxxf, minyf, maxyf;

  minxf = std::min({proj_pos[0].x(), proj_pos[1].x(), proj_pos[2].x()});
  maxxf = std::max({proj_pos[0].x(), proj_pos[1].x(), proj_pos[2].x()});
  minyf = std::min({proj_pos[0].y(), proj_pos[1].y(), proj_pos[2].y()});
  maxyf = std::max({proj_pos[0].y(), proj_pos[1].y(), proj_pos[2].y()});
  // std::minmax({ proj_pos[0].x(), proj_pos[1].x(), proj_pos[2].x() });
  int minx = minxf, maxx = maxxf + 1, miny = minyf, maxy = maxyf + 1;
  for (int i = minx; i < maxx; ++i) {
    for (int j = miny; j < maxy; ++j) {
      auto [b1, b2, b3] = baryCenterCoord(i, j, proj_pos);
      if (b1 > 0 and b1 < 1 and b2 > 0 and b2 < 1 and b3 > 0 and b3 < 1) {
        float dp =
            proj_pos[0].z() * b1 + proj_pos[1].z() * b2 + proj_pos[2].z() * b3;
        // add a judge whether zbuffer is used.
        if (dp > m_depthBuffer[j * m_width + i]) {
          m_depthBuffer[j * m_width + i] = dp;
          auto p_pos = t.v[0] * b1 + t.v[1] * b2 + t.v[2] * b3;
          auto p_color = t.color[0] * b1 + t.color[1] * b2 + t.color[2] * b2;
          auto p_normal =
              t.normal[0] * b1 + t.normal[1] * b2 + t.normal[2] * b3;
          auto p_uv = t.uv[0] * b1 + t.uv[1] * b2 + t.uv[2] * b3;
          SoftFragmentShaderPayload payload(p_pos.head<3>(), p_color,
                                            p_normal.head<3>(), p_uv);
          auto pixel = fragmentShader(payload);
          m_frameBuffer[j * m_width + i] = pixel;
        }
      }
    }
  }
  */
}
