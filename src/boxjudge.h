#pragma once

#include <stdexcept>

using REAL = float;

struct mPoint {
  REAL x, y, z;
};

struct mBox {
  REAL xl, xs, yl, ys, zl, zs;
  mBox(REAL x1, REAL x2, REAL y1, REAL y2, REAL z1, REAL z2)
      : xl(x1), xs(x2), yl(y1), ys(y2), zl(z1), zs(z2) {}
  mBox() : mBox(1, -1, 1, -1, 1, -1) {}
};

// const mBox defaultBox{{0, 0, 0}, 2, 2, 2};

bool pointInBox(const mPoint &_p, const mBox &_b = mBox()) {
  return (_p.x <= _b.xl and _p.x >= _b.xs and _p.y <= _b.yl and
          _p.y >= _b.ys and _p.z <= _b.zl and _p.z >= _b.zs);
}

static inline mPoint rayCrossXFace(const mPoint &_st, const mPoint &_ed,
                                   REAL _x) {
  REAL y = (_x - _st.x) * (_ed.y - _st.y) / (_ed.x - _st.x) + _st.y;
  REAL z = (_x - _st.x) * (_ed.z - _st.z) / (_ed.x - _st.x) + _st.z;
  return {_x, y, z};
}
static inline mPoint rayCrossYFace(const mPoint &_st, const mPoint &_ed,
                                   REAL _y) {
  REAL x = (_y - _st.y) * (_ed.x - _st.x) / (_ed.y - _st.y) + _st.x;
  REAL z = (_y - _st.y) * (_ed.z - _st.z) / (_ed.y - _st.y) + _st.z;
  return {x, _y, z};
}
static inline mPoint rayCrossZFace(const mPoint &_st, const mPoint &_ed,
                                   REAL _z) {
  REAL x = (_z - _st.z) * (_ed.x - _st.x) / (_ed.z - _st.z) + _st.x;
  REAL y = (_z - _st.z) * (_ed.y - _st.y) / (_ed.z - _st.z) + _st.y;
  return {x, y, _z};
}

mPoint rayCrossBox(const mPoint &_st, const mPoint &_ed,
                   const mBox &_b = mBox()) {
#ifdef BOXJUDGE_SAFE
  if (!pointInBox(_st, _b) or pointInBox(_ed, _b)) {
    throw std::runtime_error("Invalid ray when cross box!");
  }
#endif
  if (_ed.x > _st.x) {
    if (auto [_, _y, _z] = rayCrossXFace(_st, _ed, _b.xl);
        _y >= _b.ys and _y <= _b.yl and _z >= _b.zs and _z <= _b.zl) {
      return {_, _y, _z};
    }
  } else if (_ed.x < _st.x) {
    if (auto [_, _y, _z] = rayCrossXFace(_st, _ed, _b.xs);
        _y >= _b.ys and _y <= _b.yl and _z >= _b.zs and _z <= _b.zl) {
      return {_, _y, _z};
    }
  }

  if (_ed.y > _st.y) {
    if (auto [_x, _, _z] = rayCrossYFace(_st, _ed, _b.yl);
        _x >= _b.xs and _x <= _b.xl and _z >= _b.zs and _z <= _b.zl) {
      return {_x, _, _z};
    }
  } else if (_ed.y < _st.y) {
    if (auto [_x, _, _z] = rayCrossYFace(_st, _ed, _b.ys);
        _x >= _b.xs and _x <= _b.xl and _z >= _b.zs and _z <= _b.zl) {
      return {_x, _, _z};
    }
  }

  if (_ed.z > _st.z) {
    if (auto [_x, _y, _] = rayCrossZFace(_st, _ed, _b.zl);
        _x >= _b.xs and _x <= _b.xl and _y >= _b.ys and _y <= _b.yl) {
      return {_x, _y, _};
    }
  } else if (_ed.z < _st.z) {
    if (auto [_x, _y, _] = rayCrossZFace(_st, _ed, _b.zs);
        _x >= _b.xs and _x <= _b.xl and _y >= _b.ys and _y <= _b.zl) {
      return {_x, _y, _};
    }
  }

#ifdef BOXJUDGE_SAFE
  throw std::runtime_error("Invalid ray when cross box!");
#endif
  return {0, 0, 0};
}