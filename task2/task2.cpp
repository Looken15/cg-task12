
// Градиентнйы треугольник

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

// Переменные с индентификаторами ID

// ID шейдерной программы
GLuint shaderProgram;

// ID атрибута вершин
GLint attribVertex;

// ID атрибута текстурных координат
GLint attribTex;

GLint attribNormal;

// ID юниформа текстуры
GLint unifTexture;

// ID юниформа угла поворота
GLint unifAngle;

// ID буфера ыершин
GLuint vertexVBO;

// ID текстуры
GLint textureHandle;

// SFML текстура
sf::Texture textureData;

GLuint VAO;
GLuint IBO;

float objectRotation[3] = { 0.5f, 0.5f, 0.0f };

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

GLint Unif_transform_model;
GLint Unif_transform_viewProjection;
GLint Unif_transform_normal;
GLint Unif_transform_viewPosition;

GLint Unif_material_emission;
GLint Unif_material_ambient;
GLint Unif_material_diffuse;
GLint Unif_material_specular;
GLint Unif_material_shininess;

GLint Unif_light_ambient;
GLint Unif_light_diffuse;
GLint Unif_light_specular;
GLint Unif_light_attenuation;
GLint Unif_light_position;


struct Transform
{
	float model[4][4] =
	{
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0},
	};
	float viewProjection[4][4] =
	{
		/*{0.7071, 0.0, 0.7071, 0.0},
		{0.5, 0.7071, -0.5, 0.0},
		{-0.5, 0.7071, 0.5, 0.0},
		{0.0, 0.0, 0.0, 1.0},*/
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0},
	};
	float normal[3][3] =
	{
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
	};
	float viewPosition[3] = { 0.0, 0.0, -3.0 };
};
Transform transform;

struct Material
{
	float emission[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float ambient[4] = { 0.135f, 0.2225f, 0.1575f, 0.95f };
	float diffuse[4] = { 0.54f, 0.89f, 0.63f, 0.95f };
	float specular[4] = { 0.316228f, 0.316228f, 0.316228f, 0.95f };
	float shininess = 12.8f;
};
Material material;

struct Light
{
	float ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float attenuation[3] = { 1.0, 0.0, 0.0 };
	float position[3] = {-1.2f, -0.5f, -1.0f};
};
Light light;


// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core

	uniform vec3 angle;

    in vec3 coord;
	in vec2 texcoord;
	in vec3 normal;

	uniform struct Transform {
		mat4 model;
		mat4 viewProjection;
		mat3 normal;
		vec3 viewPosition;
	} transform;

	uniform struct PointLight {
		vec3 position;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec3 attenuation;
	} light;

	out Vertex {
		vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
		float distance;
	} Vert;

    void main() {
		float x_angle = angle[0];
        float y_angle = angle[1];
		float z_angle = angle[2];
        
        // Поворачиваем вершину
        mat3 rotate = mat3(
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
		);

		vec3 vertex = coord * rotate;

		vec3 lightDir = light.position - vertex;

		gl_Position = vec4(vertex, 1.0);

		Vert.texcoord = texcoord;
		Vert.normal = normal * rotate;
		Vert.lightDir = lightDir;
		Vert.viewDir = transform.viewPosition - vertex;
		Vert.distance = length(lightDir);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core

    in Vertex {
		vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
		float distance;
	} Vert;

	uniform struct PointLight {
		vec3 position;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec3 attenuation;
	} light;

	uniform struct Material {
		sampler2D texture;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec4 emission;
		float shininess;
	} material;

	out vec4 color;

    void main() {

		vec3 normal = normalize(Vert.normal);
		vec3 lightDir = normalize(Vert.lightDir);
		vec3 viewDir = normalize(Vert.viewDir);
	
		float attenuation = 1.0 / (light.attenuation[0] + light.attenuation[1] * Vert.distance + light.attenuation[2] * Vert.distance * Vert.distance);;

		color = material.emission;
		color += material.ambient * light.ambient * attenuation;

		float Ndot = max(dot(normal, lightDir), 0.0);
		color += material.diffuse * light.diffuse * Ndot * attenuation;

		float RdotVpow = max(pow(dot(reflect(-lightDir, normal), viewDir), material.shininess), 0.0);
		color += material.specular * light.specular * RdotVpow * attenuation;

		color *= texture(material.texture, Vert.texcoord);
    }
)";

//color = texture(material.texture, Vert.texcoord) * material.emission;

//1.0 / (light.attenuation[0] + light.attenuation[1] * Vert.distance + light.attenuation[2] * Vert.distance * Vert.distance);


void Init();
void Draw();
void Release();


int main() {
	sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);

	window.setActive(true);

	// Инициализация glew
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


// Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки
void checkOpenGLerror() {
	GLenum errCode;
	// Коды ошибок можно смотреть тут
	// https://www.khronos.org/opengl/wiki/OpenGL_Error
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error!: " << errCode << std::endl;
}

// Функция печати лога шейдера
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

	//проходим по строкам файла
	while (std::getline(infile, line))
	{
		if (line[0] == 'v' || line[0] == 'f')
		{
			//разбиваем каждую строку по пробелам в вектор split
			std::vector<std::string> split;
			std::stringstream ss(line);
			std::string str;
			while (std::getline(ss, str, ' '))
			{
				split.push_back(str);
			}

			//заполняем вектор вершин
			if (split[0] == "v")
			{
				Vertex vert = { std::stof(split[1]), std::stof(split[2]), std::stof(split[3]) };
				v.push_back(vert);
			}

			//заполняем вектор текстурных координат
			else if (split[0] == "vt")
			{
				Tex tex = { std::stof(split[1]), std::stof(split[2]) };
				t.push_back(tex);
			}

			//заполняем вектор нормалей
			else if (split[0] == "vn")
			{
				Normal norm = { std::stof(split[1]), std::stof(split[2]), std::stof(split[3]) };
				n.push_back(norm);
			}

			//заполняем массив индексов и соответствий
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
		pos_tex.push_back(n[s[i].vert[0].z - 1].x);
		pos_tex.push_back(n[s[i].vert[0].z - 1].y);
		pos_tex.push_back(n[s[i].vert[0].z - 1].z);

		pos_tex.push_back(v[s[i].vert[1].x - 1].x);
		pos_tex.push_back(v[s[i].vert[1].x - 1].y);
		pos_tex.push_back(v[s[i].vert[1].x - 1].z);
		pos_tex.push_back(t[s[i].vert[1].y - 1].x);
		pos_tex.push_back(t[s[i].vert[1].y - 1].y);
		pos_tex.push_back(n[s[i].vert[1].z - 1].x);
		pos_tex.push_back(n[s[i].vert[1].z - 1].y);
		pos_tex.push_back(n[s[i].vert[1].z - 1].z);

		pos_tex.push_back(v[s[i].vert[2].x - 1].x);
		pos_tex.push_back(v[s[i].vert[2].x - 1].y);
		pos_tex.push_back(v[s[i].vert[2].x - 1].z);
		pos_tex.push_back(t[s[i].vert[2].y - 1].x);
		pos_tex.push_back(t[s[i].vert[2].y - 1].y);
		pos_tex.push_back(n[s[i].vert[2].z - 1].x);
		pos_tex.push_back(n[s[i].vert[2].z - 1].y);
		pos_tex.push_back(n[s[i].vert[2].z - 1].z);
	}

	glGenBuffers(1, &vertexVBO);
	//glGenBuffers(1, &IBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTex);
	glEnableVertexAttribArray(attribNormal);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, pos_tex.size() * sizeof(GLfloat), pos_tex.data(), GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(Index), index.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(attribTex, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
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

	attribVertex = glGetAttribLocation(shaderProgram, "coord");
	if (attribVertex == -1)
	{
		std::cout << "could not bind attrib coord" << std::endl;
		return;
	}

	attribTex = glGetAttribLocation(shaderProgram, "texcoord");
	if (attribVertex == -1)
	{
		std::cout << "could not bind attrib texcoord" << std::endl;
		return;
	}

	attribNormal = glGetAttribLocation(shaderProgram, "normal");
	if (attribNormal == -1)
	{
		std::cout << "could not bind attrib normal" << std::endl;
		return;
	}

	Unif_transform_viewPosition = glGetUniformLocation(shaderProgram, "transform.viewPosition");
	if (Unif_transform_viewPosition == -1)
	{
		std::cout << "could not bind uniform transform.viewPosition" << std::endl;
		return;
	}

	unifTexture = glGetUniformLocation(shaderProgram, "material.texture");
	if (unifTexture == -1)
	{
		std::cout << "could not bind uniform material.texture" << std::endl;
		return;
	}

	Unif_material_emission = glGetUniformLocation(shaderProgram, "material.emission");
	if (Unif_material_emission == -1)
	{
		std::cout << "could not bind uniform material.emission" << std::endl;
		return;
	}

	Unif_material_ambient = glGetUniformLocation(shaderProgram, "material.ambient");
	if (Unif_material_ambient == -1)
	{
		std::cout << "could not bind uniform material.ambient" << std::endl;
		return;
	}

	Unif_material_diffuse = glGetUniformLocation(shaderProgram, "material.diffuse");
	if (Unif_material_diffuse == -1)
	{
		std::cout << "could not bind uniform material.diffuse" << std::endl;
		return;
	}

	Unif_material_specular = glGetUniformLocation(shaderProgram, "material.specular");
	if (Unif_material_specular == -1)
	{
		std::cout << "could not bind uniform material.specular" << std::endl;
		return;
	}

	Unif_light_ambient = glGetUniformLocation(shaderProgram, "light.ambient");
	if (Unif_light_ambient == -1)
	{
		std::cout << "could not bind uniform light.ambient" << std::endl;
		return;
	}

	Unif_light_diffuse = glGetUniformLocation(shaderProgram, "light.diffuse");
	if (Unif_light_diffuse == -1)
	{
		std::cout << "could not bind uniform light.diffuse" << std::endl;
		return;
	}

	Unif_light_specular = glGetUniformLocation(shaderProgram, "light.specular");
	if (Unif_light_specular == -1)
	{
		std::cout << "could not bind uniform light.specular" << std::endl;
		return;
	}

	Unif_light_attenuation = glGetUniformLocation(shaderProgram, "light.attenuation");
	if (Unif_light_attenuation == -1)
	{
		std::cout << "could not bind uniform ligtt.attenuation" << std::endl;
		return;
	}

	Unif_light_position = glGetUniformLocation(shaderProgram, "light.position");
	if (Unif_light_position == -1)
	{
		std::cout << "could not bind uniform ligtt.position" << std::endl;
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
	const char* filename = "jade.jpg";
	// Загружаем текстуру из файла
	if (!textureData.loadFromFile(filename))
	{
		// Не вышло загрузить картинку
		return;
	}
	// Теперь получаем openGL дескриптор текстуры
	textureHandle = textureData.getNativeHandle();
}

void Init() {
	InitShader();
	InitVBO();
	InitTexture();
}


void Draw() {
	// Устанавливаем шейдерную программу текущей
	glUseProgram(shaderProgram);

	//material
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&textureData);
	glUniform1i(unifTexture, 0);
	glUniform4fv(Unif_material_emission, 1, material.emission);
	glUniform4fv(Unif_material_ambient, 1, material.ambient);
	glUniform4fv(Unif_material_diffuse, 1, material.diffuse);
	glUniform4fv(Unif_material_specular, 1, material.specular);
	glUniform1f(Unif_material_shininess, material.shininess);

	//light
	glUniform4fv(Unif_light_ambient, 1, light.ambient);
	glUniform4fv(Unif_light_diffuse, 1, light.diffuse);
	glUniform4fv(Unif_light_specular, 1, light.specular);
	glUniform3fv(Unif_light_attenuation, 1, light.attenuation);
	glUniform3fv(Unif_light_position, 1, light.position);

	//transform
	/*glUniformMatrix4fv(Unif_transform_model, 1, GL_TRUE, &transform.model[0][0]);
	glUniformMatrix4fv(Unif_transform_viewProjection, 1, GL_TRUE, &transform.viewProjection[0][0]);
	glUniformMatrix3fv(Unif_transform_normal, 1, GL_TRUE, &transform.normal[0][0]);*/
	glUniform3fv(Unif_transform_viewPosition, 1, transform.viewPosition);

	glUniform3fv(unifAngle, 1, objectRotation);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, s.size() * 3);
	glBindVertexArray(0);

	glUseProgram(0);
	checkOpenGLerror();
	checkOpenGLerror();
}


// Освобождение шейдеров
void ReleaseShader() {
	// Передавая ноль, мы отключаем шейдрную программу
	glUseProgram(0);
	// Удаляем шейдерную программу
	glDeleteProgram(shaderProgram);
}

// Освобождение буфера
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
