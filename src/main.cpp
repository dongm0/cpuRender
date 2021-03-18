#include <SDL.h>

#include "OBJ_Loader.h"
#include "rasterizer.h"
#include "triangle.h"

SDL_Window *gWindow = nullptr;
SDL_Renderer *gRenderer = nullptr;

const int WINDOW_WIDTH = 700;
const int WINDOW_HEIGHT = 700;

bool init_sdl(int _w, int _h, const char *title) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to init SDL. SDL error:" << std::endl;
    std::cerr << SDL_GetError() << std::endl;
    return false;
  } else {
    gWindow =
        SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, _w, _h, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
      std::cerr << "Failed to create SDL window. SDL error:" << std::endl;
      std::cerr << SDL_GetError() << std::endl;
      return false;
    } else {
      gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
      if (gRenderer == nullptr) {
        std::cerr << "Failed to create SDL renderer. SDL error:" << std::endl;
        std::cerr << SDL_GetError() << std::endl;
        return false;
      } else {
        SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
      }
    }
  }
  return true;
}

void close_sdl() {
  SDL_DestroyWindow(gWindow);
  SDL_DestroyRenderer(gRenderer);
  gWindow = nullptr;
  gRenderer = nullptr;
  SDL_Quit();
}

static Eigen::Vector3f
normal_fragment_shader(rst::SoftFragmentShaderPayload payload) {
  Eigen::Vector3f return_color = (payload.normal.head<3>().normalized() +
                                  Eigen::Vector3f(1.0f, 1.0f, 1.0f)) /
                                 2.f;
  Eigen::Vector3f result;
  result << return_color.x() * 255, return_color.y() * 255,
      return_color.z() * 255;
  return result;
}

uint32_t lasttick = 0;
uint32_t curtick = 0;
float fps = 0, dt = 0;

int main(int argc, char **argv) {
#ifndef NDEBUG
  if (argc != 2) {
    return 1;
  }
#endif
  if (auto ret = init_sdl(WINDOW_WIDTH, WINDOW_HEIGHT, "test"); !ret) {
    return 1;
  }
  objl::Loader loader;
  loader.LoadFile(argc == 2 ? argv[1] : "./model/face.obj");
  std::vector<Triangle> triList;
  for (auto &mesh : loader.LoadedMeshes) {
    for (int i = 0; i < mesh.Vertices.size(); i += 3) {
      Triangle t;
      for (int j = 0; j < 3; ++j) {
        t.setVertex(j, {mesh.Vertices[i + j].Position.X,
                        mesh.Vertices[i + j].Position.Y,
                        mesh.Vertices[i + j].Position.Z});
        t.setNormal(j, {mesh.Vertices[i + j].Normal.X,
                        mesh.Vertices[i + j].Normal.Y,
                        mesh.Vertices[i + j].Normal.Z});
        t.setTexCoord(j, {mesh.Vertices[i + j].TextureCoordinate.X,
                          mesh.Vertices[i + j].TextureCoordinate.Y});
      }
      triList.push_back(t);
    }
  }

  rst::SoftRasterizer r(WINDOW_WIDTH, WINDOW_HEIGHT);
  r.SetModelMatrix(rst::GetModelMatrix({0, 0, 0}, 1.));
  r.SetViewMatrix(rst::GetViewMatrix({0, 0, 5}));
  r.SetProjectionMatrix(
      rst::GetProjMatrix(90, WINDOW_HEIGHT * 1.0 / WINDOW_WIDTH, -0.1, -50));
  r.SetFragmentShader(normal_fragment_shader);

  r.UpdateTransform();

  bool quit = false;
  SDL_Event e;
  while (!quit) {
    curtick = SDL_GetTicks();
    dt = (curtick - lasttick) * 1.f / 1000;
    if (dt > 0) {
      fps = 1.f / dt;
      SDL_Log("fps: %f", fps);
    }
    lasttick = curtick;
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);
    r.ClearBuffer();
    r.Draw(triList);

    auto color = r.GetFrameBuffer();
    for (int i = 0; i < r.GetHeight(); ++i) {
      int row = WINDOW_HEIGHT - i - 1;
      for (int j = 0; j < r.GetWidth(); ++j) {
        SDL_SetRenderDrawColor(gRenderer, color[row * r.GetWidth() + j][0],
                               color[row * r.GetWidth() + j][1],
                               color[row * r.GetWidth() + j][2], 255);
        SDL_RenderDrawPoint(gRenderer, j, i);
      }
    }
    SDL_RenderPresent(gRenderer);
  }

  close_sdl();
  return 0;
}
