#include <libpgmaker/effect.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include <glad/glad.h>

void GLAPIENTRY
message_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei lenght,
    const GLchar* message,
    const void* userParams)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

namespace libpgmaker {
void glsl_effect::prepare(int width, int height)
{
    this->width  = width;
    this->height = height;
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);

    glGenTextures(4, inTexs);
    for(int i = 0; i < 4; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, inTexs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, 0);
    }
    glGenTextures(4, outTexs);
    for(int i = 0; i < 4; ++i)
    {
        glActiveTexture(GL_TEXTURE4 + i);
        glBindTexture(GL_TEXTURE_2D, outTexs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    const char* shaderSource = get_shader();
    int computeShader        = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &shaderSource, nullptr);
    glCompileShader(computeShader);
    GLint ret = 0;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &ret);
    if(ret == GL_FALSE)
    {
        GLint maxLenght = 0;
        glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLenght);
        std::vector<GLchar> errorLog(maxLenght);
        glGetShaderInfoLog(computeShader, maxLenght, &maxLenght, &errorLog[0]);
        printf("%s", errorLog.data());
        glDeleteShader(computeShader);
        return;
    }
    programID = glCreateProgram();
    glAttachShader(programID, computeShader);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &ret);
    if(ret == GL_FALSE)
    {
        GLint maxLenght = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLenght);
        std::vector<GLchar> errorLog(maxLenght);
        glGetProgramInfoLog(programID, maxLenght, &maxLenght, &errorLog[0]);
        printf("%s", errorLog.data());
        glDeleteProgram(programID);
    }
    glDeleteShader(computeShader);
    glUseProgram(programID);
    glBindTextures(0, 4, inTexs);
    glUniform1i(0, 0);
    glUniform1i(1, 1);
    glUniform1i(2, 2);
    glUniform1i(3, 3);
    glBindImageTextures(0, 4, inTexs);

    glBindTextures(4, 4, outTexs);
    glUniform1i(4, 4);
    glUniform1i(5, 5);
    glUniform1i(6, 6);
    glUniform1i(7, 7);
    glBindImageTextures(4, 4, outTexs);

    set_uniforms();
}
void glsl_effect::process(AVFrame* inFrame, AVFrame* outFrame)
{
    glUseProgram(programID);
    for(int i = 0; i < 4; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, inTexs[i]);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_FLOAT, inFrame->data[i]);
    }
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    for(int i = 0; i < 4; ++i)
    {
        glActiveTexture(GL_TEXTURE4 + i);
        glGetTexImage(
            GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, outFrame->data[i]);
    }
}
void glsl_effect::cleanup()
{
    glDeleteProgram(programID);
    glDeleteTextures(4, inTexs);
    glDeleteTextures(4, outTexs);
}
const char* grayscale::get_shader() const
{
    return R"(
		#version 450 core
		layout (local_size_x = 1, local_size_y = 1) in;

		layout (location = 0, binding = 0, r32f) uniform readonly image2D inB;
		layout (location = 1, binding = 1, r32f) uniform readonly image2D inG;
		layout (location = 2, binding = 2, r32f) uniform readonly image2D inR;
		layout (location = 3, binding = 3, r32f) uniform readonly image2D inA;
		layout (location = 4, binding = 4, r32f) uniform writeonly image2D outB;
		layout (location = 5, binding = 5, r32f) uniform writeonly image2D outG;
		layout (location = 6, binding = 6, r32f) uniform writeonly image2D outR;
		layout (location = 7, binding = 7, r32f) uniform writeonly image2D outA;

		void main()
		{
			ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
			vec4 pixelB = imageLoad(inB, coords);
			vec4 pixelG = imageLoad(inG, coords);
			vec4 pixelR = imageLoad(inR, coords);
			vec4 pixelA = imageLoad(inA, coords);
			float Y = 0.299 * pixelR.x + 0.587 * pixelG.x + 0.114 * pixelB.x;
			pixelB = vec4(Y, pixelB.yzw);
			pixelG = vec4(Y, pixelG.yzw);
			pixelR = vec4(Y, pixelR.yzw);
			imageStore(outB, coords, pixelB);
			imageStore(outG, coords, pixelG);
			imageStore(outR, coords, pixelR);
			imageStore(outA, coords, pixelA);
		}
	)";
}

void pass_through::prepare(int width, int height)
{
    this->width  = width;
    this->height = height;
}
void pass_through::process(AVFrame* inFrame, AVFrame* outFrame)
{
    std::copy_n(inFrame->data[0], width * 4 * height, outFrame->data[0]);
    std::copy_n(inFrame->data[1], width * 4 * height, outFrame->data[1]);
    std::copy_n(inFrame->data[2], width * 4 * height, outFrame->data[2]);
    std::copy_n(inFrame->data[3], width * 4 * height, outFrame->data[3]);
}
void pass_through::cleanup()
{
}
} // namespace libpgmaker
