#include <GL/glew.h>
#include <cmath>
#include <glfwpp/glfwpp.h>
#include <iostream>
#include <string>
#include <vector>

const int lineNum = 1000;
const int lineSize = 1000;
std::vector<float> vertices(lineNum *lineSize * 2);

void updateVertices(std::vector<float> &vertices, float phase = 0.0f)
{
    for (int i = 0; i < lineNum; i++)
    {
        for (int j = 0; j < lineSize; j++)
        {
            const float x = 2 * (float)j / (float)lineSize - 1.0f;
            const float y = sin(x * 10.0f + phase + (float)i * 0.5f);
            vertices[(i * lineSize + j) * 2] = x;
            vertices[(i * lineSize + j) * 2 + 1] = y;
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
        layout (location = 0) in vec3 aPos;
        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    )";

    const std::string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
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

    const auto VBO = []()
    {
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        return VBO;
    }();

    const auto VAO = []()
    {
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        return VAO;
    }();

    // printVertices(vertices);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindVertexArray(VAO);

    const auto bufferSize = vertices.size() * sizeof(float);
    std::cout << "Buffer size: " << bufferSize << std::endl;

    glEnableVertexAttribArray(0);

    std::cout << "Here!" << std::endl;

    error = glGetError();
    while (error != GL_NO_ERROR)
    {
        const GLubyte *errorMessage = gluErrorString(error);
        std::cerr << "OpenGL error: " << error << " (" << errorMessage << ")" << std::endl;
        error = glGetError();
    }

    glfwSetWindowSizeCallback(wnd, onResize);

    while (!wnd.shouldClose())
    {
        double time = glfw::getTime();
        glClear(GL_COLOR_BUFFER_BIT);
        // glClearColor((sin(time) + 1.0) / 2.0, (cos(time) + 1.0) / 2.0, (-sin(time) + 1.0) / 2.0, 0.0);

        updateVertices(vertices, time);

        glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices.data(), GL_STATIC_DRAW);

        for (int i = 0; i < lineNum; i++)
        {
            glDrawArrays(GL_LINE_STRIP, i * lineSize, lineSize);
        }

        glfw::pollEvents();
        wnd.swapBuffers();
    }
}