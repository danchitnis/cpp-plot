#include <GL/glew.h>
#include <cmath>
#include <glfwpp/glfwpp.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

const int lineNum = 3000;
const int lineSize = 2000;

std::vector<float> vertices(lineNum *lineSize * 2);
std::vector<float> colors(lineNum *lineSize * 3);

std::chrono::high_resolution_clock timer;
std::chrono::nanoseconds elapsed(0);
int fps = 0;

void initVertices(std::vector<float> &vertices)
{
    for (int i = 0; i < lineNum; i++)
    {
        for (int j = 0; j < lineSize; j++)
        {
            const float x = 2 * (float)j / (float)lineSize - 1.0f;
            vertices[(i * lineSize + j) * 2] = x;
            vertices[(i * lineSize + j) * 2 + 1] = 0.0;
        }
    }
}

void initColors(std::vector<float> &colors)
{
    for (int i = 0; i < lineNum; i++)
    {
        const float r = std::rand() / (float)RAND_MAX;
        const float g = std::rand() / (float)RAND_MAX;
        const float b = std::rand() / (float)RAND_MAX;

        for (int j = 0; j < lineSize; j++)
        {
            colors[(i * lineSize + j) * 3] = r;
            colors[(i * lineSize + j) * 3 + 1] = g;
            colors[(i * lineSize + j) * 3 + 2] = b;
        }
    }
}

void updateVertices(std::vector<float> &vertices, float phase = 0.0f)
{
    for (int i = 0; i < lineNum; i++)
    {
        const float y0 = (float)i / (float)lineNum + phase * 0.1f;
        for (int j = 0; j < lineSize; j++)
        {
            float y = y0 + j * 0.1 / (float)lineSize;
            const float yy = y - std::lroundf(y);
            vertices[(i * lineSize + j) * 2 + 1] = 2 * yy;
        }
    }
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
        layout (location = 0) in vec3 aColor;

        out vec3 vColor;
        
        void main()
        {
            vColor = aColor;
            gl_Position = vec4(aPos.x, aPos.y, 0, 1.0);
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
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    auto colorAttribute = glGetAttribLocation(shaderProgram, "aColor");
    std::cout << "Color attribute: " << colorAttribute << std::endl;
    glEnableVertexAttribArray(colorAttribute);
    glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Position
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

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

    while (!wnd.shouldClose())
    {
        auto start = timer.now();

        double time = glfw::getTime();
        glClear(GL_COLOR_BUFFER_BIT);
        // glClearColor((sin(time) + 1.0) / 2.0, (cos(time) + 1.0) / 2.0, (-sin(time) + 1.0) / 2.0, 0.0);

        updateVertices(vertices, time);

        glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices.data(), GL_DYNAMIC_DRAW);

        for (int i = 0; i < lineNum; i++)
        {
            glDrawArrays(GL_LINE_STRIP, i * lineSize, lineSize);
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