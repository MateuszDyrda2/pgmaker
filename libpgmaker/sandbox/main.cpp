#include <stdexcept>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <libpgmaker/textureGL.h>
#include <libpgmaker/timeline.h>
#include <libpgmaker/video_reader.h>

static glm::ivec2 windowSize{ 1080, 720 };
static bool isPaused     = true;
static bool changePaused = false;

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
        if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            isPaused ^= 1;
            changePaused = true;
        }
    });

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to load glad\n";
        return -1;
    }

    // ###############################################################
    // 								SHADERS
    // ###############################################################
    const char* vertexShaderCode   = R"(
		#version 330 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aTexCoord;

		out vec2 TexCoord;
		uniform mat4 viewProj;

		void main()
		{
			gl_Position = viewProj * vec4(aPos, 0.0, 1.0);
			TexCoord = aTexCoord;
		}
	)";
    const char* fragmentShaderCode = R"(
		#version 330 core
		out vec4 FragColor;

		in vec2 TexCoord;

		uniform sampler2D ourTexture;

		void main()
		{
			FragColor = texture(ourTexture, TexCoord);
		}
	)";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);
    // ###############################################################
    // ###############################################################
    // ###############################################################
    // 					VERTEX ARRAY / VBO / EBO
    // ###############################################################
    float vertices[] = {
        0.5f, 0.5f, 1.f, 1.f,
        0.5f, -0.5f, 1.f, 0.f,
        -0.5f, -0.5f, 0.f, 0.f,
        -0.5f, 0.5f, 0.f, 1.f
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // ###############################################################
    // ###############################################################
    // 				Read one frame
    // ###############################################################

    video_reader reader;
    auto vid = reader.load_file("/home/matzix/shared/PGMaker/libpgmaker/data/1232.mp4");
    // auto tn  = vid->get_thumbnail(vid->get_state().width / 4, vid->get_state().height / 4);

    {
        textureGL tex(vid->get_info().width, vid->get_info().height);
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

            glm::mat4 viewProj = glm::ortho(float(windowSize.x) * -0.5f, float(windowSize.x * 0.5f),
                                            float(windowSize.y) * 0.5f, float(windowSize.y) * -0.5f);
            viewProj           = glm::scale(viewProj,
                                            glm::vec3(float(vid->get_info().width), float(vid->get_info().height), 1.f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewProj"), 1, GL_FALSE, glm::value_ptr(viewProj));

            frame* fr = nullptr;
            if(changePaused)
            {
                changePaused = false;
                tl.set_paused(isPaused);
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
                tex.update_data(fr->data.get());
            }
            tex.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            tex.unbind();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}
