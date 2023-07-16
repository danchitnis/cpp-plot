#include <GL/glew.h>
#include <cmath>
#include <glfwpp/glfwpp.h>
#include <iostream>
#include <string>


void updateVertices(float* vertices, int lineSize)
{
    for(int i = 0; i < lineSize * 2; i += 2)
    {
        vertices[i] = (float)(i / 2) / lineSize;
        vertices[i + 1] = (float)cos(i / (float)lineSize * 1 * 2 * 3.14159265358979323846);
    }
}

int main()
{
    const int lineSize = 100;
    float vertices[lineSize * 2] = {0.0f, 0.0f, 0.1f, 0.1f, 0.2f, 0.2f, 0.3f, 0.3f, 0.4f, 0.4f, 0.5f, 0.5f,
                                     0.6f, 0.6f, 0.7f, 0.7f, 0.8f, 0.8f, 0.9f, 0.9f};

    std::cout << "Hello, GLFWPP!" << std::endl;

    [[maybe_unused]] glfw::GlfwLibrary library = glfw::init();

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
    glfw::Window wnd(800, 600, "Line Example");

    glEnable(GL_DEBUG_OUTPUT);

    GLenum error = glGetError();


    if(wnd == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        // glfw::terminate();
        return -1;
    }

    glfw::makeContextCurrent(wnd);

    glfw::swapInterval(1);

    std::cout << "GLFW version: " << glfw::getVersionString() << std::endl;

    std::cout << "GLFW version: " << glGetString(GL_VERSION) << std::endl;
    if(glewInit() != GLEW_OK)
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
    const GLchar* vertexShaderSourcePtr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }


    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentShaderSourcePtr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
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
    if(!success2)
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


    updateVertices((float*)vertices, lineSize);

    // print vertices
    for(int i = 0; i < sizeof(vertices) / sizeof(float); i += 2)
    {
        std::cout << vertices[i] << ", " << vertices[i + 1] << std::endl;
    }


    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindVertexArray(VAO);

    
    const auto bufferSize = sizeof(vertices);
    std::cout << "Buffer size: " << bufferSize << std::endl;
    glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);

    std::cout << "Here!" << std::endl;


    glDrawArrays(GL_LINE_STRIP, 0, lineSize);
    
    error = glGetError();
    while(error != GL_NO_ERROR)
    {
        const GLubyte* errorMessage = gluErrorString(error);
        std::cerr << "OpenGL error: " << error << " (" << errorMessage << ")" << std::endl;
        error = glGetError();
    }


    while(!wnd.shouldClose())
    {
        double time = glfw::getTime();
        glClear(GL_COLOR_BUFFER_BIT);
        // glClearColor((sin(time) + 1.0) / 2.0, (cos(time) + 1.0) / 2.0, (-sin(time) + 1.0) / 2.0, 0.0);
        // glClear(GL_COLOR_BUFFER_BIT);

        // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glDrawArrays(GL_LINE_STRIP, 0, lineSize);


        glfw::pollEvents();
        wnd.swapBuffers();
    }
}
