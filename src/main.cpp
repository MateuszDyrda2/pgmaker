#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3_loader.h>
#include <imgui_internal.h>

#include <libpgmaker/timeline.h>
#include <libpgmaker/video_reader.h>

#include <nfd.h>

#include <iostream>
#include <utility>
#include <vector>

using namespace libpgmaker;
static void create_main_menu(std::vector<std::shared_ptr<video>>& videos, video_reader& reader);
static void create_main_dockspace(timeline& tl, std::vector<std::shared_ptr<video>>& videos);
static void initialize_layout(ImGuiID dockspaceId);
static void create_videos(std::vector<std::shared_ptr<video>>& videos);
static void create_properties();
static void create_timeline(timeline& tl);
static void create_playback(timeline& tl);
static void create_node_editor();

static std::pair<int, int> windowSize{ 1920, 1080 };
static std::pair<std::uint32_t, std::uint32_t> textureSize{ 1080, 720 };
static unsigned int texture;

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
    video_reader reader;
    // auto vid = reader.load_file("/home/matzix/shared/PGMaker/libpgmaker/data/1232.mp4");
    auto ch = tl.add_channel();
    // ch->add_clip(vid, std::chrono::milliseconds(0));
    /////////////////////texture///////////
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 textureSize.first, textureSize.second, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    ////////////////////////////////////////
    std::vector<std::shared_ptr<video>> videos;

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ////
        create_main_menu(videos, reader);
        ////
        create_main_dockspace(tl, videos);
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
void create_main_menu(std::vector<std::shared_ptr<video>>& videos, video_reader& reader)
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("OpenVideo"))
            {
                nfdchar_t* outPath;
                auto result = NFD_OpenDialog(&outPath, NULL, 0, NULL);
                if(result == NFD_OKAY)
                {
                    printf("%s\n", outPath);
                    videos.push_back(reader.load_file(outPath));
                    NFD_FreePath(outPath);
                }
            };
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void create_main_dockspace(timeline& tl, std::vector<std::shared_ptr<video>>& videos)
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
            create_timeline(tl);
            create_videos(videos);
            create_properties();
            create_node_editor();
            create_playback(tl);
        }
    }
    ImGui::End();
}
void create_videos(std::vector<std::shared_ptr<video>>& videos)
{
    ImGui::Begin("Videos");
    {
        for(std::size_t i = 0; i < videos.size(); ++i)
        {
            if((i % 3) != 0)
            {
                ImGui::SameLine(0.f);
            }
            if(ImGui::ImageButton(reinterpret_cast<ImTextureID>(videos[i]->get_texture()),
                                  ImVec2(60.f, 60.f)))
            {
            }
            if(ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("demo", &videos[i], sizeof(&videos[i]));
                ImGui::EndDragDropSource();
            }
        }
    }
    ImGui::End();
}
void create_properties()
{
    ImGui::Begin("Properties");
    ImGui::Text("hello from properties");
    ImGui::End();
}
void create_timeline(timeline& tl)
{
    ImGui::Begin("Timeline");
    {
        auto regSize = ImGui::GetWindowSize().x / 2.f;
        ImGui::SetCursorPos(ImVec2(regSize - 70.f, 20.f));
        if(ImGui::Button("<<", ImVec2(40.f, 40.f)))
        {
            tl.jump2(tl.get_timestamp() - std::chrono::milliseconds(3000));
        }
        ImGui::SetCursorPos(ImVec2(regSize - 20.f, 20.f));
        if(ImGui::Button(">", ImVec2(40.f, 40.f)))
        {
            tl.set_paused(!tl.is_paused());
        }
        ImGui::SetCursorPos(ImVec2(regSize + 30.f, 20.f));
        if(ImGui::Button(">>", ImVec2(40.f, 40.f)))
        {
            tl.jump2(tl.get_timestamp() + std::chrono::milliseconds(3000));
        }
        {
            auto& io                        = ImGui::GetIO();
            auto drawList                   = ImGui::GetWindowDrawList();
            auto canvasSize                 = ImGui::GetContentRegionAvail();
            auto canvasPos                  = ImGui::GetCursorScreenPos();
            static constexpr auto textWidth = 100.f;
            float xmin                      = canvasPos.x + textWidth;
            float xmax                      = canvasPos.x + canvasSize.x;
            float channelHeight = 60, channelMargin = 10;

            std::size_t index  = 0;
            const auto timemax = 20000;
            for(const auto& ch : tl.get_channels())
            {
                ImGui::InvisibleButton("canvas", ImVec2(canvasSize.x - canvasPos.x, channelHeight));
                float spacing = index * (channelHeight + channelMargin);
                drawList->AddRectFilled(
                    { xmin, canvasPos.y },
                    { xmax, canvasPos.y + channelHeight },
                    0xFF3D3837, 0);
                drawList->AddText(canvasPos, 0xFFFFFFFF, "Channel 1");
                for(const auto& cl : ch->get_clips())
                {
                    auto starts   = cl->get_starts_at().count();
                    auto ends     = cl->get_duration().count();
                    auto stDiv    = starts / double(timemax);
                    auto edDiv    = ends / double(timemax);
                    float clipXMin = xmin + stDiv * (xmax - xmin),
                         clipXMax = xmin + edDiv * (xmax - xmin);
                    drawList->AddRectFilled(
                        { float(clipXMin), canvasPos.y },
                        { float(clipXMax), canvasPos.y + channelHeight },
                        0xFF0000AA, 0);
                }
                ++index;
            }
            if(ImGui::IsItemActive())
            {
                auto mousePos = io.MouseClickedPos[0];
                if(mousePos.x >= xmin && mousePos.x <= xmax)
                {
                    auto timePos = (mousePos.x - xmin) / (xmax - xmin) * timemax;
                    tl.jump2(std::chrono::milliseconds(std::int64_t(timePos)));
                }
            }
            const auto ts = tl.get_timestamp().count();
            float xlinepos = xmin + (ts / double(timemax) * (xmax - xmin));
            drawList->AddLine(
                { float(xlinepos), canvasPos.y },
                { float(xlinepos), canvasPos.y + canvasSize.y },
                0xFFFF0000,
                3.f);
        }
        if(ImGui::BeginDragDropTarget())
        {
            if(const auto payload = ImGui::AcceptDragDropPayload("demo"))
            {
                tl.get_channel(0)->append_clip(
                    *static_cast<std::shared_ptr<video>*>(payload->Data));
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}
void create_playback(timeline& tl)
{
    auto documentFlags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    if(ImGui::Begin("Playback", NULL, documentFlags))
    {
        // ImGui::BeginChild("VideoRender");

        ////////////////////////
        auto pos  = ImGui::GetWindowPos();
        auto size = ImGui::GetWindowSize();

        glBindTexture(GL_TEXTURE_2D, texture);
        auto frame = tl.get_frame();
        if(frame)
        {
            if(frame->size != textureSize)
            {
                textureSize = frame->size;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.first, textureSize.second,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            }
            else
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                                textureSize.first, textureSize.second, GL_RGBA,
                                GL_UNSIGNED_BYTE, frame->data.data());
            }
        }
        ImGui::Image(reinterpret_cast<ImTextureID>(texture), size);
        ////////////////////////
        // ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
void create_node_editor()
{
    if(ImGui::Begin("NodeEditor"))
    {
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
