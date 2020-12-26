#include "myapp_SDL.h"
#include "rasterizer.h"
#include "triangle.h"
#include "OBJ_Loader.h"

MyApp_Error MyApp::MyApp_SDL_Init(unsigned int _w, unsigned int _h, const char *_title) {
    MyApp_Error _err = MyApp_success;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "MyApp: Failed to initialize SDL." << std::endl;
        std::cerr << SDL_GetError() << std::endl;
        _err = MyApp_fail;
    }
    else {
        _window = SDL_CreateWindow(_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _w, _h, SDL_WINDOW_SHOWN);
        if (_window == nullptr) {
            std::cerr << "MyApp: Failed to create SDL window." << std::endl;
            std::cerr << SDL_GetError() << std::endl;
            _err = MyApp_fail;
        }
        else {
            SDL_SetRenderDrawColor(_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }
    return _err;
}

MyApp_Error MyApp::MyApp_SDL_Exit() {
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    _window = nullptr;
    _renderer = nullptr;

    SDL_Quit();
}



MyApp_Error MyApp::MyApp_Init() {
    MyApp_Error _err = MyApp_success;
    if (MyApp_SDL_Init(MYAPP_WINDOW_WIDTH_DEFAULT, MYAPP_WINDOW_WIDTH_DEFAULT, "MyApp: Run") == MyApp_fail) {
        _err = MyApp_fail;
    }
    return _err;
    /*
    glfwInit();

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    window = glfwCreateWindow(1200, 1200, "Hw5 by dongmo", NULL, NULL);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    gladLoadGL();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    */
}

MyApp_Error MyApp::cleanUp() {
    MyApp_Error _err = MyApp_success;
    if (MyApp_SDL_Exit() == MyApp_fail) {
        _err = MyApp_fail;
        return _err;
    }
    return _err;
    /*
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    */
}

void MyApp::mainSDLLoop() {
    objl::Loader loader;
    loader.LoadFile("");
    std::vector<Triangle> triList;
    for (auto mesh : loader.LoadedMeshes) {
        for (int i = 0; i < mesh.Vertices.size(); i += 3) {
            Triangle t;
            for (int j = 0; j < 3; ++j) {
                t.setVertex(j, { mesh.Vertices[i + j].Position.X, mesh.Vertices[i + j].Position.Y, mesh.Vertices[i + j].Position.Z });
                t.setNormal(j, { mesh.Vertices[i + j].Normal.X, mesh.Vertices[i + j].Normal.Y, mesh.Vertices[i + j].Normal.Z });
                t.setVertex(j, { mesh.Vertices[i + j].TextureCoordinate.X, mesh.Vertices[i + j].TextureCoordinate.Y });
            }
        }
    }
    Matrix4f model, view, proj;
    model = rst::GetModelMatrix({ 0, 0, 0 }, 1);
    view = rst::GetViewMatrix({ 0, 0, 5 });
    proj = rst::GetProjMatrix(45, 1, -0.1, -50);

    rst::SoftRasterizer r(700, 700);
    r.SetModelMatrix(model);
    r.SetProjectionMatrix(proj);
    r.SetViewMatrix(view);

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        r.Draw(triList);
        for (int i = 0; i < r.GetHeight(); ++i) {
            for (int j = 0; j < r.GetWidth(); ++j) {
                auto color = r.GetFrameBuffer();
                SDL_SetRenderDrawColor(_renderer, color[i * r.GetWidth() + j][0], color[i * r.GetWidth() + j][1], color[i * r.GetWidth() + j][2], 1.f);
                SDL_RenderDrawPoint(_renderer, i, j);
            }
        }
    }
}