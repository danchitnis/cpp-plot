#include <GL/glew.h>
#include <cmath>
#include <glfwpp/glfwpp.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

const float squareSize = 0.001f;
const int maxSquareNum = 30000000;
const int newDataNum = 1000;

std::vector<float> squarePositions(maxSquareNum * 2);
const std::vector<char> squareIndices = {0, 1, 2, 2, 1, 3};
std::vector<char> colors(maxSquareNum * 3);

int headIndex = 0;

std::chrono::high_resolution_clock timer;
std::chrono::nanoseconds elapsed(0);
int fps = 0;

void initPos(std::vector<float> &pos)
{
    // set all to zero
    for (size_t i = 0; i < pos.size(); i++)
    {
        pos[i] = 0.0f;
    }
}

void initColors(std::vector<char> &colors)
{
    for (size_t i = 0; i < colors.size(); i++)
    {
        colors[i] = std::rand() % 255;
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

    glfw::Window wnd(1200, 800, "Scatter Plot");

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

        uniform float uSize;
        uniform vec2 uOffset;
        uniform mat2 uScale;

        out vec3 vColor;
        
        void main()
        {
            vec2 squareVertices[4] = vec2[4](vec2(-1.0, 1.0), vec2(1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, -1.0));
            vec2 pos = uSize * squareVertices[gl_VertexID] + aPos;
            gl_Position = vec4((uScale * pos) + uOffset, 0.0, 1.0);

            vColor = aColor;
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

    const auto EBO = []()
    {
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        return EBO;
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

    glUseProgram(shaderProgram);

    // setup elements buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, squareIndices.size() * sizeof(char), squareIndices.data(), GL_STATIC_DRAW);

    // Position
    initPos(squarePositions);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, squarePositions.size() * sizeof(float), squarePositions.data(), GL_DYNAMIC_DRAW);
    auto positionAttribute = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(positionAttribute, 1);
    glEnableVertexAttribArray(positionAttribute);

    // Colors
    initColors(colors);
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(char), colors.data(), GL_DYNAMIC_DRAW);
    auto colorAttribute = glGetAttribLocation(shaderProgram, "aColor");
    glVertexAttribPointer(colorAttribute, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(colorAttribute, 1);
    glEnableVertexAttribArray(colorAttribute);

    // Uniforms

    const auto uSize = glGetUniformLocation(shaderProgram, "uSize");
    const auto uOffset = glGetUniformLocation(shaderProgram, "uOffset");
    const auto uScale = glGetUniformLocation(shaderProgram, "uScale");

    glUniform1f(uSize, squareSize);
    glUniform2f(uOffset, 0.0f, 0.0f);

    const auto wndSize = wnd.getSize();
    std::cout << "Scale: " << std::get<0>(wndSize) << ", " << std::get<1>(wndSize) << std::endl;
    const float aspectRatio = (float)std::get<1>(wndSize) / (float)std::get<0>(wndSize);
    std::cout << "Aspect ratio: " << aspectRatio << std::endl;
    glUniformMatrix2fv(uScale, 1, GL_FALSE, new float[4]{aspectRatio, 0.0f, 0.0f, 1.0f});

    std::cout << "Here!" << std::endl;

    error = glGetError();
    while (error != GL_NO_ERROR)
    {
        const GLubyte *errorMessage = gluErrorString(error);
        std::cerr << "OpenGL error: " << error << " (" << errorMessage << ")" << std::endl;
        error = glGetError();
    }

    glfwSetWindowSizeCallback(wnd, onResize);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    while (!wnd.shouldClose())
    {
        auto start = timer.now();

        float time = glfw::getTime();
        glClear(GL_COLOR_BUFFER_BIT);

        // std::vector<float> pos = {(1 / aspectRatio) * (2 * std::rand() / (float)RAND_MAX - 1), 2 * std::rand() / (float)RAND_MAX - 1};

        std::vector<float> pos(newDataNum * 2);

        for (size_t i = 0; i < pos.size(); i += 2)
        {
            pos[i] = (1 / aspectRatio) * (2 * std::rand() / (float)RAND_MAX - 1);
            pos[i + 1] = 2 * std::rand() / (float)RAND_MAX - 1;
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, headIndex * 2 * sizeof(float), pos.size() * sizeof(float), pos.data());
        glEnableVertexAttribArray(positionAttribute);

        headIndex = (headIndex + pos.size() / 2) % maxSquareNum;

        // std::cout << "Head index: " << headIndex << " " << pos[0] << ", " << pos[1] << std::endl;

        glDrawElementsInstanced(GL_TRIANGLES, squareIndices.size(), GL_UNSIGNED_BYTE, 0, maxSquareNum);

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