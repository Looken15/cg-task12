// ����������� ����������� � ��������� (����� ������� �����������)

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

// ���������� � ����������������� ID

// ID ��������� ���������
GLuint shaderProgram;

// ID �������� ������
GLint attribVertex;

// ID �������� ���������� ���������
GLint attribTex;

// ID �������� ��������
GLint unifTexture;

// ID �������� ���� ��������
GLint unifAngle;

// ID ������ ������
GLuint vertexVBO;

// ID ��������
GLint textureHandle;

// SFML ��������
sf::Texture textureData;


GLuint VAO;
GLuint IBO;

float objectRotation[3] = { 0.0f, 0.0f, 0.0f };

// �������
struct Vertex
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct Tex
{
	GLfloat x;
	GLfloat y;
};

struct Normal
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct Color
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat a;
};

struct Index
{
	GLushort x;
	GLushort y;
	GLushort z;
};

struct Surface
{
	Index vert[3];
};



// ������ ��� ������ ������, � �� �����, ����� ������� ��� �������� -
// ����� ��������� ������� �� �����, ����� ��������� ����� � ���������,
// � ��� ����� ����� �������, ��� ������ ������������ �������

// � ���������, ���� ������ �� ��������� �������� �����, ��� ��� ���� ��� �����,
// ��������, ����� ��������� �������������, ����� ���� ������� ������ '\n'

const char* VertexShaderSource = R"(
    #version 330 core

    uniform vec3 angle;

	in vec3 vertCoord;
	in vec2 vertTex;

	out vec2 tCoord;

	void main() {
		tCoord = vertTex;
		// ����������� ���� ��������
        float x_angle = angle[0];
        float y_angle = angle[1];
		float z_angle = angle[2];
        
        // ������������ �������
        vec3 position = vertCoord * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        ) * mat3(
			cos(z_angle), -sin(z_angle), 0,
			sin(z_angle), cos(z_angle), 0,
			0, 0, 1
		) * mat3(
			0.1, 0, 0,
			0, 0.1, 0,
			0, 0, 0.1
		);
		gl_Position = vec4(position, 1.0);
	}
)";

const char* FragShaderSource = R"(
	#version 330 core

	in vec2 tCoord;
	uniform sampler2D textureData;

	out vec4 color;

	void main() {
		color = texture(textureData, tCoord);
	}
)";

//color = texture(textureData, tCoord);

void Init();
void Draw();
void Release();


int main() {
	sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);

	window.setActive(true);

	glewInit();
	glEnable(GL_DEPTH_TEST);

	Init();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::Resized) {
				glViewport(0, 0, event.size.width, event.size.height);
			}
			// ��������� ������� ������
			else if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case (sf::Keyboard::W): objectRotation[0] += 0.1; break;
				case (sf::Keyboard::S): objectRotation[0] -= 0.1; break;
				case (sf::Keyboard::A): objectRotation[1] += 0.1; break;
				case (sf::Keyboard::D): objectRotation[1] -= 0.1; break;
				case (sf::Keyboard::Q): objectRotation[2] += 0.1; break;
				case (sf::Keyboard::E): objectRotation[2] -= 0.1; break;
				default: break;
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw();

		window.display();
	}

	Release();
	return 0;
}


// �������� ������ OpenGL, ���� ���� �� ����� � ������� ��� ������
void checkOpenGLerror() {
	GLenum errCode;
	// ���� ������ ����� �������� ���
	// https://www.khronos.org/opengl/wiki/OpenGL_Error
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error!: " << errCode << std::endl;
}

// ������� ������ ���� �������
void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	int charsWritten = 0;
	char* infoLog;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		infoLog = new char[infologLen];
		if (infoLog == NULL)
		{
			std::cout << "ERROR: Could not allocate InfoLog buffer" << std::endl;
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		std::cout << "InfoLog: " << infoLog << "\n\n\n";
		delete[] infoLog;
	}
}

Index slash_string_to_index(std::string s)
{
	std::vector<std::string> split;
	std::stringstream ss(s);
	std::string str;
	while (std::getline(ss, str, '/'))
	{
		split.push_back(str);
	}
	Index i = { std::stoi(split[0]), std::stoi(split[1]), std::stoi(split[2]) };
	return i;
}

Index surface_to_index(Surface s)
{
	std::vector<Index> v;
	Index i = { s.vert[0].x, s.vert[1].x, s.vert[2].x };
	return i;
}

void InitializeVBO(std::vector<Vertex>& v, std::vector<Tex>& t, std::vector<Normal>& n, std::vector<Surface>& s, std::string filename)
{
	std::ifstream infile(filename);
	std::string line;

	//�������� �� ������� �����
	while (std::getline(infile, line))
	{
		if (line[0] == 'v' || line[0] == 'f')
		{
			//��������� ������ ������ �� �������� � ������ split
			std::vector<std::string> split;
			std::stringstream ss(line);
			std::string str;
			while (std::getline(ss, str, ' '))
			{
				split.push_back(str);
			}

			//��������� ������ ������
			if (split[0] == "v")
			{
				Vertex vert = { std::stof(split[1]), std::stof(split[2]), std::stof(split[3]) };
				v.push_back(vert);
			}

			//��������� ������ ���������� ���������
			else if (split[0] == "vt")
			{
				Tex tex = { std::stof(split[1]), std::stof(split[2]) };
				t.push_back(tex);
			}

			//��������� ������ ��������
			else if (split[0] == "vn")
			{
				Normal norm = { std::stof(split[1]), std::stof(split[2]), std::stof(split[3]) };
				n.push_back(norm);
			}

			//��������� ������ �������� � ������������
			else if (split[0] == "f")
			{
				Surface surf =
				{
					slash_string_to_index(split[1]),
					slash_string_to_index(split[2]),
					slash_string_to_index(split[3]),
				};
				s.push_back(surf);
			}
		}
	}
}

std::vector<Vertex> v;
std::vector<Tex> t;
std::vector<Normal> n;
std::vector<Surface> s;

void InitVBO()
{
	InitializeVBO(v, t, n, s, "teapot.obj");

	std::vector<float> pos_tex;
	for (int i = 0; i < s.size(); ++i)
	{
		pos_tex.push_back(v[s[i].vert[0].x - 1].x);
		pos_tex.push_back(v[s[i].vert[0].x - 1].y);
		pos_tex.push_back(v[s[i].vert[0].x - 1].z);
		pos_tex.push_back(t[s[i].vert[0].y - 1].x);
		pos_tex.push_back(t[s[i].vert[0].y - 1].y);

		pos_tex.push_back(v[s[i].vert[1].x - 1].x);
		pos_tex.push_back(v[s[i].vert[1].x - 1].y);
		pos_tex.push_back(v[s[i].vert[1].x - 1].z);
		pos_tex.push_back(t[s[i].vert[1].y - 1].x);
		pos_tex.push_back(t[s[i].vert[1].y - 1].y);

		pos_tex.push_back(v[s[i].vert[2].x - 1].x);
		pos_tex.push_back(v[s[i].vert[2].x - 1].y);
		pos_tex.push_back(v[s[i].vert[2].x - 1].z);
		pos_tex.push_back(t[s[i].vert[2].y - 1].x);
		pos_tex.push_back(t[s[i].vert[2].y - 1].y);
	}

	glGenBuffers(1, &vertexVBO);
	//glGenBuffers(1, &IBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTex);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, pos_tex.size() * sizeof(GLfloat), pos_tex.data(), GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(Index), index.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(attribTex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	checkOpenGLerror();
}




void InitShader() {
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &VertexShaderSource, NULL);
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	ShaderLog(vShader);

	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &FragShaderSource, NULL);
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	ShaderLog(fShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);

	glLinkProgram(shaderProgram);
	int link_status;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_status);
	if (!link_status)
	{
		std::cout << "error attach shaders \n";
		return;
	}

	attribVertex = glGetAttribLocation(shaderProgram, "vertCoord");
	if (attribVertex == -1)
	{
		std::cout << "could not bind attrib vertCoord" << std::endl;
		return;
	}

	attribTex = glGetAttribLocation(shaderProgram, "vertTex");
	if (attribVertex == -1)
	{
		std::cout << "could not bind attrib vertTex" << std::endl;
		return;
	}

	unifTexture = glGetUniformLocation(shaderProgram, "textureData");
	if (unifTexture == -1)
	{
		std::cout << "could not bind uniform textureData" << std::endl;
		return;
	}

	unifAngle = glGetUniformLocation(shaderProgram, "angle");
	if (unifAngle == -1)
	{
		std::cout << "could not bind uniform angle" << std::endl;
		return;
	}
	checkOpenGLerror();
}

void InitTexture()
{
	const char* filename = "123.jpg";
	// ��������� �������� �� �����
	if (!textureData.loadFromFile(filename))
	{
		// �� ����� ��������� ��������
		return;
	}
	// ������ �������� openGL ���������� ��������
	textureHandle = textureData.getNativeHandle();
}

void Init() {
	InitShader();
	InitVBO();
	InitTexture();
}


void Draw() {
	glUseProgram(shaderProgram);
	glUniform3fv(unifAngle, 1, objectRotation);

	// ���������� ���������� ���� 0, ������ ����� �� �����������, �� ���������
	// � ��� ����������� GL_TEXTURE0, ��� ����� ��� ������������� ���������� �������
	glActiveTexture(GL_TEXTURE0);
	// ������ SFML �� opengl �������� glBindTexture
	sf::Texture::bind(&textureData);
	// � uniform ������� ���������� ������ ����������� ����� (��� GL_TEXTURE0 - 0, ��� GL_TEXTURE1 - 1 � ��)
	glUniform1i(unifTexture, 0);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, s.size() * 3);
	glBindVertexArray(0);

	glUseProgram(0);
	checkOpenGLerror();
}


void ReleaseShader() {
	glUseProgram(0);
	glDeleteProgram(shaderProgram);
}

void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &IBO);
	glDeleteBuffers(1, &vertexVBO);
}

void Release() {
	ReleaseShader();
	ReleaseVBO();
}
