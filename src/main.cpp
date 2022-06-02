#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include "command_handler.h"
#include "main_application.h"
#include "welcome_screen.h"

#include <nfd.h>

#include <iostream>
#include <utility>
#include <vector>

static std::pair<int, int> windowSize{ 1920, 1080 };
int main()
{
    if(!glfwInit())
    {
        std::cerr << "Couldn't initialize glfw\n";
        return -1;
    }
    glfwSetErrorCallback([](int error, const char* desc) {
        std::cerr << error << ": " << desc << '\n';
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowSize.first, windowSize.second, "PGMaker", NULL, NULL);
    if(!window)
    {
        std::cerr << "Couldn't initialize window\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        windowSize = { width, height };
    });

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Couldn't load glad";
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    NFD_Init();
    std::unique_ptr<application_base> application =
        std::make_unique<welcome_screen>();

    bool shouldRun = true;
    command_handler::listen(
        "StartApplication",
        [&]() { application = std::make_unique<main_application>(); });

    command_handler::listen(
        "ExitApplication",
        [&]() { shouldRun = false; });

    command_handler::listen(
        "CreateProject",
        []() {
            nfdchar_t* outPath;
            nfdnfilteritem_t a = { "PGMaker projects", "pgproj" };
            auto result        = NFD_SaveDialog(&outPath, &a, 1, NULL, "NewProject.pgproj");
            if(result == NFD_OKAY)
            {
                try
                {
                    project_manager::create_project(outPath);
                }
                catch(const std::runtime_error& err)
                {
                    std::cerr << err.what();
                }

                NFD_FreePath(outPath);
            }
        });
    command_handler::listen(
        "LoadProject",
        []() {
            nfdchar_t* outPath;
            nfdnfilteritem_t a = { "PGMaker projects", "pgproj" };
            auto result        = NFD_OpenDialog(&outPath, &a, 1, 0);
            if(result == NFD_OKAY)
            {
                try
                {
                    project_manager::load_project(outPath);
                }
                catch(const std::runtime_error& err)
                {
                    std::cerr << err.what();
                }
                NFD_FreePath(outPath);
            }
        });
    command_handler::listen(
        "OpenVideo",
        []() {
            auto proj = project_manager::get_current_project();
            if(!proj) return;
            nfdchar_t* outPath;
            auto result = NFD_OpenDialog(&outPath, NULL, 0, NULL);
            if(result == NFD_OKAY)
            {
                proj->load_video(outPath);
                NFD_FreePath(outPath);
            }
        });

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        command_handler::update();
        if(!shouldRun) break;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ////
        application->update();
        ////
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    NFD_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    return 0;
}