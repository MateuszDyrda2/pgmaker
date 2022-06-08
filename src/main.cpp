#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include "command_handler.h"
#include "events.h"
#include "main_application.h"
#include "welcome_screen.h"

#if defined(WIN32)
#    define NFD_FILTER_ITEM nfdu8filteritem_t
#else
#    define NFD_FILTER_ITEM nfdnfilteritem_t
#endif
#include <nfd.hpp>

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
        event_handler::fire_event("WindowResize", width, height);
    });
    event_handler::subscribe("WindowResize", std::function<void(int, int)>([&](int width, int height) {
                                 windowSize = { width, height };
                             }));

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
    ImGui_ImplOpenGL3_Init("#version 430");
    ImGui::StyleColorsDark();

    NFD_Init();
    std::unique_ptr<application_base> application =
        std::make_unique<welcome_screen>();

    bool shouldRun = true;

    command_handler::listen(
        "StartApplication",
        [&](command& c) { application = std::make_unique<main_application>(); });

    command_handler::listen(
        "ExitApplication",
        [&](command& c) { shouldRun = false; });

    command_handler::listen(
        "CreateProject",
        [](command& c) {
            {
                auto path = *reinterpret_cast<std::string*>(c.data);
                try
                {
                    project_manager::create_project(path);
                }
                catch(const std::runtime_error& err)
                {
                    std::cerr << err.what();
                }
            }
        });
    command_handler::listen(
        "LoadProject",
        [](command& c) {
            auto& path = *reinterpret_cast<std::string*>(c.data);
            try
            {
                project_manager::load_project(path);
            }
            catch(const std::runtime_error& err)
            {
                std::cerr << err.what();
            }
        });
    command_handler::listen(
        "OpenVideo",
        [](command& c) {
            auto proj = project_manager::get_current_project();
            if(!proj) return;
            auto& path = *reinterpret_cast<std::string*>(c.data);
            proj->load_video(path);
        });
    command_handler::listen(
        "SaveProject",
        [](command& c) {
            auto proj = project_manager::get_current_project();
            if(!proj) return;
            proj->save();
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