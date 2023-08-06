#include <GL/glew.h>
#include <cmath>
#include <glfwpp/glfwpp.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

const int lineNum = 3;
const int rollBufferSize = 2000;

const int lineSize = rollBufferSize + 2;

struct RollData
{
    float dataX = 1;
    int dataIndex = 0;
    float shift = 0;
    std::vector<float> lastDataX;
    std::vector<float> lastDataY;
};

std::vector<float> vertices(static_cast<size_t>(lineNum) * (rollBufferSize + 2) * 2);
std::vector<char> colors(static_cast<size_t>(lineNum) * (rollBufferSize + 2) * 3);

std::chrono::high_resolution_clock timer;
std::chrono::nanoseconds elapsed(0);
int fps = 0;

void initVertices(std::vector<float> &vertices)
{
    for (size_t i = 0; i < lineNum; i++)
    {
        for (size_t j = 0; j < lineSize; j++)
        {
            const float x = -1.0 + 2.0 * j / (lineSize - 1);
            vertices[(i * lineSize + j) * 2] = x;
            vertices[(i * lineSize + j) * 2 + 1] = 0.0;
        }
    }
}

void initColors(std::vector<char> &colors)
{
    for (size_t i = 0; i < lineNum; i++)
    {
        const float r = std::rand() / (float)RAND_MAX;
        const float g = std::rand() / (float)RAND_MAX;
        const float b = std::rand() / (float)RAND_MAX;

        for (size_t j = 0; j < lineSize; j++)
        {
            colors[(i * lineSize + j) * 3] = r * 255.0f;
            colors[(i * lineSize + j) * 3 + 1] = g * 255.0f;
            colors[(i * lineSize + j) * 3 + 2] = b * 255.0f;
        }
    }
}

void updateVertices(std::vector<float> &ys, RollData &rollData, GLint shiftUniform)
{
    const int bfSize = rollBufferSize + 2;
    rollData.shift = rollData.shift + 2.0 / rollBufferSize;
    rollData.dataX = rollData.dataX + 2.0 / rollBufferSize;

    glUniform1f(shiftUniform, rollData.shift);

    for (size_t i = 0; i < lineNum; i++)
    {
        std::vector<float> data = {rollData.dataX, ys[i]};
        glBufferSubData(GL_ARRAY_BUFFER, (i * bfSize + rollData.dataIndex) * 2 * sizeof(float), data.size() * sizeof(float), data.data());
    }

    if (rollData.dataIndex == rollBufferSize - 1)
    {
        for (size_t i = 0; i < lineNum; i++)
        {
            rollData.lastDataX[i] = rollData.dataX;
            rollData.lastDataY[i] = ys[i];
        }
    }

    if (rollData.dataIndex == 0 && rollData.lastDataX[0] != 0)
    {
        for (size_t i = 0; i < lineNum; i++)
        {
            std::vector<float> data = {rollData.lastDataX[i], rollData.lastDataY[i], rollData.dataX, ys[i]};
            glBufferSubData(GL_ARRAY_BUFFER, (i * bfSize + rollBufferSize) * 2 * sizeof(float), data.size() * sizeof(float), data.data());
        }
    }

    rollData.dataIndex = (rollData.dataIndex + 1) % rollBufferSize;
}

void printVertices(const std::vector<float> &vertices)
{
    for (std::vector<float>::size_type i = 0; i < vertices.size(); i += 2)
    {
        std::cout << vertices[i] << ", " << vertices[i + 1] << std::endl;
    }
}

void printColors(const std::vector<float> &colors)
{
    for (std::vector<float>::size_type i = 0; i < colors.size(); i += 3)
    {
        std::cout << colors[i] << ", " << colors[i + 1] << ", " << colors[i + 2] << std::endl;
    }
}

void onResize([[maybe_unused]] GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{

    std::cout << "Hello, GLFWPP!" << std::endl;

    glfw::GlfwLibrary library = glfw::init();

    glfw::WindowHints hints;
    hints.clientApi = glfw::ClientApi::OpenGl;
    hints.contextVersionMajor = 4;
    hints.contextVersionMinor = 6;
    hints.apply();
    // Or with C++20:
    // glfw::WindowHints{
    //        .clientApi = glfw::ClientApi::OpenGl,
    //        .contextVersionMajor = 4,
    //        .contextVersionMinor = 6}
    //        .apply();
    glfw::Window wnd(1200, 800, "Line Example");

    glEnable(GL_DEBUG_OUTPUT);

    GLenum error = glGetError();

    if (wnd == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        // glfw::terminate();
        return -1;
    }

    glfw::makeContextCurrent(wnd);

    glfw::swapInterval(1);

    std::cout << "GLFW version: " << glfw::getVersionString() << std::endl;

    std::cout << "GLFW version: " << glGetString(GL_VERSION) << std::endl;
    if (glewInit() != GLEW_OK)
    {
        throw std::runtime_error("Could not initialize GLEW");
    }

    const std::string vertexShaderSource = R"(
        #version 330 core
        layout (location = 1) in vec2 aPos;
        layout (location = 2) in vec3 aColor;

        uniform float uShift;
        out vec3 vColor;
        
        void main()
        {
            vColor = aColor;
            vec2 shiftedPosition = aPos - vec2(uShift, 0);
            gl_Position = vec4(shiftedPosition, 0, 1);
            vColor = aColor/ vec3(255.0, 255.0, 255.0);
        }
    )";

    const std::string fragmentShaderSource = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(vColor, 0.7);
        }
    )";

    const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourcePtr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderSourcePtr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    const auto shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for link errors
    int success2;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success2);
    if (!success2)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    [[maybe_unused]] const auto VAO = []()
    {
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        return VAO;
    }();

    const auto VBO = []()
    {
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        return VBO;
    }();

    const auto CBO = []()
    {
        GLuint CBO;
        glGenBuffers(1, &CBO);
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        return CBO;
    }();

    // printVertices(vertices);

    initColors(colors);

    // printColors(colors);

    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(char), colors.data(), GL_STATIC_DRAW);
    auto colorAttribute = glGetAttribLocation(shaderProgram, "aColor");
    glEnableVertexAttribArray(colorAttribute);
    glVertexAttribPointer(colorAttribute, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, (void *)0);

    // Position
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    auto positionAttribute = glGetAttribLocation(shaderProgram, "aPos");
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // uniforms
    GLint shiftUniform = glGetUniformLocation(shaderProgram, "uShift");

    // setup

    glUseProgram(shaderProgram);

    const auto bufferSize = vertices.size() * sizeof(float);
    std::cout << "Buffer size: " << bufferSize << std::endl;

    std::cout << "Here!" << std::endl;

    error = glGetError();
    while (error != GL_NO_ERROR)
    {
        const GLubyte *errorMessage = gluErrorString(error);
        std::cerr << "OpenGL error: " << error << " (" << errorMessage << ")" << std::endl;
        error = glGetError();
    }

    glfwSetWindowSizeCallback(wnd, onResize);

    initVertices(vertices);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices.data(), GL_DYNAMIC_DRAW);

    RollData rollData;

    rollData.lastDataX.resize(lineNum, 0.0f);
    rollData.lastDataY.resize(lineNum, 0.0f);

    const int bfSize = rollBufferSize + 2;

    std::vector<float> ys(lineNum);

    while (!wnd.shouldClose())
    {
        auto start = timer.now();

        // double time = glfw::getTime();
        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < lineNum; i++)
        {
            const float a = ys[i] + 0.01 * (i + 1) / lineNum;
            ys[i] = a - std::lroundf(a);
        }

        updateVertices(ys, rollData, shiftUniform);

        for (size_t i = 0; i < lineNum; i++)
        {
            glDrawArrays(GL_LINE_STRIP, i * bfSize, rollData.dataIndex);
            glDrawArrays(GL_LINE_STRIP, i * bfSize + rollData.dataIndex, rollBufferSize - rollData.dataIndex);
            glDrawArrays(GL_LINE_STRIP, i * bfSize + rollBufferSize, 2);
        }

        glfw::pollEvents();
        wnd.swapBuffers();

        auto end = timer.now();

        elapsed += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        fps++;

        if (elapsed.count() > 1e9)
        {
            std::cout << "FPS: " << fps << std::endl;
            elapsed = std::chrono::nanoseconds(0);
            fps = 0;
        }
    }
}