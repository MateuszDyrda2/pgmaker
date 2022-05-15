#include <stdexcept>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <libpgmaker/preview.h>
#include <libpgmaker/timeline.h>
#include <libpgmaker/video_reader.h>

static glm::ivec2 windowSize{ 1080, 720 };
static bool isPaused     = true;
static bool changePaused = false;
static bool jumpBack     = false;

using namespace libpgmaker;
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

    GLFWwindow* window = glfwCreateWindow(windowSize.x, windowSize.y, "sandbox", NULL, NULL);
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
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int, int action, int) {
        switch(key)
        {
        case GLFW_KEY_SPACE:
        {
            if(action == GLFW_PRESS)
            {
                isPaused ^= 1;
                changePaused = true;
            }
        }
        break;
        case GLFW_KEY_H:
        {
            if(action == GLFW_PRESS)
            {
                jumpBack = true;
            }
        }
        break;
        default:
            break;
        }
    });

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to load glad\n";
        return -1;
    }
    video_reader reader;
    auto vid = reader.load_file("/home/matzix/shared/PGMaker/libpgmaker/data/1232.mp4");
    // auto tn  = vid->get_thumbnail(vid->get_state().width / 4, vid->get_state().height / 4);

    {
        preview prev({ .width = windowSize.x, .height = windowSize.y });
        timeline tl({ .size = { 1920, 1080 }, .framerate = 30 });
        auto ch = tl.add_channel();
        try
        {
            ch->add_clip(vid, std::chrono::milliseconds(0));
        }
        catch(const std::runtime_error& err)
        {
            std::cerr << err.what();
            return -1;
        }
        // ###############################################################
        using namespace std::chrono;
        auto lastFrame = high_resolution_clock::now();
        while(!glfwWindowShouldClose(window))
        {
            static bool isFirst = true;
            glViewport(0, 0, windowSize.x, windowSize.y);
            glClearColor(0.5f, 0.f, 0.5f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            prev.resize({ .width = (unsigned)windowSize.x, .height = (unsigned)windowSize.y });

            frame* fr = nullptr;
            if(changePaused)
            {
                changePaused = false;
                tl.set_paused(isPaused);
            }
            if(jumpBack)
            {
                jumpBack = false;
                tl.jump2(std::chrono::milliseconds(1000));
            }
            if(high_resolution_clock::now() - lastFrame > microseconds(1661))
            {
                lastFrame = high_resolution_clock::now();
                try
                {
                    fr = tl.get_frame();
                }
                catch(const std::runtime_error& err)
                {
                    std::cerr << err.what();
                    return -1;
                }
            }
            if(fr)
            {
                prev.update(fr);
            }
            prev.draw();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}
