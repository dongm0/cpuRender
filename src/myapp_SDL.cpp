#include "myapp.h"

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "Glfw error"+std::to_string(error)+
                 ": "+std::string(description) << std::endl;
}

void MyApp::initAll() {
    glfwSetErrorCallback(glfw_error_callback);

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
}

void MyApp::cleanUp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void MyApp::mainLoop() {
    //预定变量
    ControlPointArray2D arr;
    bool edit = false;
    int it = 0;
    float l = 0.12;
    int me = 0;
    int chosepoint = -1;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {            
            ImGui::SetNextWindowSize(ImVec2(900,1000), ImGuiCond_Appearing);
            ImGui::Begin("main");

            if (ImPlot::BeginPlot("", "", "", ImVec2(800, 800))) {
                if (!edit) {
                    if (ImPlot::IsPlotHovered() and ImGui::IsMouseClicked(0) and !arr.closed()) {
                        ImPlotPoint pt = ImPlot::GetPlotMousePos();
                        arr.pushBack({pt.x, pt.y});
                    }
                    else if (ImPlot::IsPlotHovered() and ImGui::IsMouseClicked(1)) {
                        arr.makeClose();
                    }
                }
                else {
                    if (ImPlot::IsPlotHovered() and ImGui::IsMouseClicked(0) and chosepoint==-1) {
                        ImPlotPoint pt = ImPlot::GetPlotMousePos();
                        chosepoint = arr.getClosePoint({pt.x, pt.y});
                    }
                    else if (ImPlot::IsPlotHovered() and ImGui::IsMouseClicked(0) and chosepoint != -1) {
                        chosepoint = -1;
                    }
                    else if (chosepoint != -1) {
                        ImPlotPoint pt = ImPlot::GetPlotMousePos();
                        arr.setPos(chosepoint, {pt.x, pt.y});
                    }
                }
            }
            {
                auto cp = arr.getCtrlPoint();
                int kkk = cp.size();
                auto dp = arr.getDrawPoint();

                if (cp.size() > 0) {
                    ImPlot::PlotLine("control points", &cp[0].x, &cp[0].y, cp.size(), 0, 2*sizeof(double));
                    if (arr.closed()) {
                        double l1[4] = {cp.back().x, cp.back().y, cp[0].x, cp[0].y};
                        ImPlot::PlotLine("control points", &l1[0], &l1[1], 2, 0, 2*sizeof(double));
                    }
                }
                if (dp.size() > 0) {
                    ImPlot::PlotLine("curve", &dp[0].x, &dp[0].y, dp.size(), 0, 2*sizeof(double));
                    if (arr.closed()) {
                        double l1[4] = {dp.back().x, dp.back().y, dp[0].x, dp[0].y};
                        ImPlot::PlotLine("curve", &l1[0], &l1[1], 2, 0, 2*sizeof(double));
                    }
                }

                if (edit) {
                    ImPlot::PlotScatter("", &cp[0].x, &cp[0].y, cp.size(), 0, 2*sizeof(double));
                }

            }
            ImPlot::EndPlot();

            ImGui::Checkbox("edit ctrl points", &edit);
            ImGui::SameLine();
            if (ImGui::Button("clean"))
                arr.clear();
            ImGui::SliderInt("curve iteration times", &it, 0, 12);
            arr.setItTime(it);
            ImGui::SliderFloat("interpolation para", &l, 0, 0.6f);
            arr.setInterpolationPara(l);
            ImGui::RadioButton("2nd B-spline", &me, 0);ImGui::SameLine();
            ImGui::RadioButton("3rd B-spline", &me, 1);ImGui::SameLine();
            ImGui::RadioButton("Intrtpolation", &me, 2);
            arr.setSegmentation(Segmentation(me));

            ImGui::End();
        }

        ImGui::Render();
        int _width, _height;
        glfwGetFramebufferSize(window, &_width, &_height);
        glViewport(0, 0, _width, _height);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}