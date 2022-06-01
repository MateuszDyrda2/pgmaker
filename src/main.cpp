#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include "cmain_menu.h"
#include "cnode_editor.h"
#include "cplayback.h"
#include "cproperties.h"
#include "ctimeline.h"
#include "cvideos.h"

#include <nfd.h>

#include <iostream>
#include <utility>
#include <vector>

using namespace libpgmaker;
static void create_main_dockspace(cvideos& videoWindow,
                                  cproperties& propertyWindow,
                                  ctimeline& timelineWindow,
                                  cplayback& playbackWindow,
                                  cnode_editor& nodeEditorWindow);
static void initialize_layout(ImGuiID dockspaceId);

static std::pair<int, int> windowSize{ 1920, 1080 };
static video* selectedVideo = nullptr;

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

    timeline tl(project_settings{});
    auto ch = tl.add_channel();
    std::vector<std::shared_ptr<video>> videos;

    cmain_menu menu(videos, tl);
    cvideos videoWindow(videos);
    cproperties propertyWindow;
    ctimeline timelineWindow(tl);
    cplayback playbackWindow(tl);
    cnode_editor nodeEditorWindow;

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ////
        menu.draw();
        ////
        create_main_dockspace(videoWindow, propertyWindow, timelineWindow, playbackWindow, nodeEditorWindow);
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
void create_main_dockspace(cvideos& videoWindow,
                           cproperties& propertyWindow,
                           ctimeline& timelineWindow,
                           cplayback& playbackWindow,
                           cnode_editor& nodeEditorWindow)
{
    ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags windowFlags      = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ////
    const auto vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ////
    if(dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
    {
        windowFlags |= ImGuiWindowFlags_NoBackground;
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

    ImGui::Begin("MainDockSpace", nullptr, windowFlags);
    {
        ImGui::PopStyleVar(3);
        auto dockspaceId = ImGui::GetID("MyDockspace");
        if(ImGui::DockBuilderGetNode(dockspaceId) == NULL)
        {
            initialize_layout(dockspaceId);
        }
        else
        {
            ImGui::DockSpace(dockspaceId, ImVec2(0, 0), dockspaceFlags);

            timelineWindow.draw();
            videoWindow.draw();
            propertyWindow.draw();
            nodeEditorWindow.draw();
            playbackWindow.draw();
        }
    }
    ImGui::End();
}
void initialize_layout(ImGuiID dockspaceId)
{
    ImGui::DockBuilderGetNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

    auto dockMainId  = dockspaceId;
    auto dockIdDown  = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.2f, NULL, &dockMainId);
    auto dockIdLeft  = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, NULL, &dockMainId);
    auto dockIdRight = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.25f, NULL, &dockMainId);

    ImGui::DockBuilderDockWindow("Timeline", dockIdDown);
    ImGui::DockBuilderDockWindow("Videos", dockIdLeft);
    ImGui::DockBuilderDockWindow("Properties", dockIdRight);
    ImGui::DockBuilderDockWindow("Playback", dockMainId);
    ImGui::DockBuilderDockWindow("NodeEditor", dockMainId);
    ImGui::DockBuilderFinish(dockspaceId);
}
