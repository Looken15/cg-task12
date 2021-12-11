// Вращающийся треугольник с текстурой (можно вращать стрелочками)

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>


// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint shaderProgram;
// ID атрибута вершин
GLint attribVertex;
// ID атрибута текстурных координат
GLint attribTexture;

GLint attribColor;
// ID юниформа текстуры
GLint unifTexture1;
GLint unifTexture2;
// ID юниформа угла поворота
GLint unifAngle;
// ID буфера ыершин
GLuint vertexVBO;
// ID буфера текстурных координат
GLuint textureVBO;
GLuint colorVBO;
// ID текстуры
GLint textureHandle1;
GLint textureHandle2;
// SFML текстура
sf::Texture textureData1;
sf::Texture textureData2;

float objectRotation[3] = { 0.5f, 0.5f, 0.0f };

// Вершина
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





// Шейдер это просто строка, и не кажно, каким образом она получена -
// можно загружать шейдеры из файла, можно объявлять прямо в программе,
// в том числе таким образом, при помощи специального макроса

// К сожалению, этот макрос не учитывает переводы строк, так что если они нужны,
// например, после дирректив препроцессора, нужно явно ставить символ '\n'

const char* VertexShaderSource = R"(
    #version 330 core

    uniform vec3 angle;

	in vec3 vertCoord;
	in vec2 texureCoord;
	in vec4 vertColor;

	out vec4 OurColor;
	out vec2 tCoord;

	void main() {
	   tCoord = texureCoord;
		OurColor = vertColor;
		// Захардкодим углы поворота
        float x_angle = angle[0];
        float y_angle = angle[1];
		float z_angle = angle[2];
        
        // Поворачиваем вершину
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
			0, 0, 1);
		gl_Position = vec4(position, 1.0);
	}
)";

const char* FragShaderSource = R"(
	#version 330 core

	uniform sampler2D textureData1;
	uniform sampler2D textureData2;
	in vec2 tCoord;
	in vec4 OurColor;

	out vec4 color;

	void main() {
		//color = texture(textureData1, tCoord);
		//color = texture(textureData1, tCoord) * OurColor;
		color = texture(textureData1, tCoord) * texture(textureData2, tCoord);
	}
)";

//color = texture(textureData, tCoord);
//color = texture(textureData, tCoord) * OurColor;
//color = mix(texture(textureData1, tCoord), texture(textureData2, tCoord), 0.3);



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
			// обработка нажатий клавиш
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


void InitVBO()
{
	glGenBuffers(1, &vertexVBO);
	glGenBuffers(1, &textureVBO);
	glGenBuffers(1, &colorVBO);



	// Объявляем вершины треугольника
	Vertex triangle[36] = {
		//правая видимая грань
		{ -0.5, +0.5, -0.5 }, { -0.5, +0.5, +0.5 }, { +0.5, +0.5, +0.5 },
		{ +0.5, +0.5, +0.5 }, { +0.5, +0.5, -0.5 }, { -0.5, +0.5, -0.5 },

		//нижняя видимая грань
		{ -0.5, -0.5, +0.5 }, { -0.5, +0.5, +0.5 }, { +0.5, +0.5, +0.5 },
		{ +0.5, +0.5, +0.5 }, { +0.5, -0.5, +0.5 }, { -0.5, -0.5, +0.5 },

		//правая невидимая грань
		{ +0.5, -0.5, -0.5 }, { +0.5, -0.5, +0.5 }, { +0.5, +0.5, +0.5 },
		{ +0.5, +0.5, +0.5 }, { +0.5, +0.5, -0.5 }, { +0.5, -0.5, -0.5 },

		//верхняя невидимая грань
		{ -0.5, -0.5, -0.5 }, { +0.5, +0.5, -0.5 }, { -0.5, +0.5, -0.5 },
		{ +0.5, +0.5, -0.5 }, { -0.5, -0.5, -0.5 }, { +0.5, -0.5, -0.5 },

		//левая невидимая грань
		{ -0.5, -0.5, -0.5 }, { -0.5, +0.5, +0.5 }, { -0.5, -0.5, +0.5 },
		{ -0.5, +0.5, +0.5 }, { -0.5, -0.5, -0.5 }, { -0.5, +0.5, -0.5 },

		//передняя видимая грань
		{ -0.5, -0.5, -0.5 }, { +0.5, -0.5, +0.5 }, { -0.5, -0.5, +0.5 },
		{ +0.5, -0.5, +0.5 }, { -0.5, -0.5, -0.5 }, { +0.5, -0.5, -0.5 },

	};

	// Объявляем текстурные координаты
	Tex texture[36] = {
		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f },
		{ 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },

		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f },
		{ 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },

		{ 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f },

		{ 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f },
		{ 1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f },

		{ 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f },
		{ 1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f },

		{ 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f },
		{ 1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f },
	};

	float colors[36][4] = {
		//правая видимая грань
		{ 1.0, 1.0, 0.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.75, 1.0, 1.0 },
		{ 0.0, 0.75, 1.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 },

		//нижняя видимая грань
		{ 1.0, 0.07, 0.57, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.75, 1.0, 1.0 },
		{ 0.0, 0.75, 1.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 1.0, 0.07, 0.57, 1.0 },

		//правая невидимая грань
		{ 0.5, 0.0, 0.5, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.75, 1.0, 1.0 },
		{ 0.0, 0.75, 1.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.5, 0.0, 0.5, 1.0 },

		//верхняя невидимая грань
		{ 1.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.5, 0.0, 0.5, 1.0 },

		//левая невидимая грань
		{ 1.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 1.0, 0.07, 0.57, 1.0 },
		{ 1.0, 1.0, 1.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 },

		//передняя видимая грань
		{ 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 1.0, 0.07, 0.57, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.5, 0.0, 0.5, 1.0 },
	};

	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture), texture, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
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

	attribTexture = glGetAttribLocation(shaderProgram, "texureCoord");
	if (attribTexture == -1)
	{
		std::cout << "could not bind attrib texureCoord" << std::endl;
		return;
	}

	attribColor = glGetAttribLocation(shaderProgram, "vertColor");
	if (attribTexture == -1)
	{
		std::cout << "could not bind attrib vertColor" << std::endl;
		return;
	}

	unifTexture1 = glGetUniformLocation(shaderProgram, "textureData1");
	if (unifTexture1 == -1)
	{
		std::cout << "could not bind uniform textureData1" << std::endl;
		return;
	}

	unifTexture2 = glGetUniformLocation(shaderProgram, "textureData2");
	if (unifTexture2 == -1)
	{
		std::cout << "could not bind uniform textureData2" << std::endl;
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
	const char* filename1 = "123.jpg";
	const char* filename2 = "321.jpg";
	// Загружаем текстуру из файла
	if (!textureData1.loadFromFile(filename1))
	{
		// Не вышло загрузить картинку
		return;
	}
	if (!textureData2.loadFromFile(filename2))
	{
		std::cout << "321";
		// Не вышло загрузить картинку
		return;
	}
	// Теперь получаем openGL дескриптор текстуры
	textureHandle1 = textureData1.getNativeHandle();
	textureHandle2 = textureData2.getNativeHandle();
}

void Init() {
	InitShader();
	InitVBO();
	InitTexture();
}


void Draw() {
	// Устанавливаем шейдерную программу текущей
	glUseProgram(shaderProgram);
	// Передаем юниформ в шейдер
	glUniform3fv(unifAngle, 1, objectRotation);

	// Активируем текстурный блок 0, делать этого не обязательно, по умолчанию
	// и так активирован GL_TEXTURE0, это нужно для использования нескольких текстур
	glActiveTexture(GL_TEXTURE0);
	// Обёртка SFML на opengl функцией glBindTexture
	sf::Texture::bind(&textureData1);
	// В uniform кладётся текстурный индекс текстурного блока (для GL_TEXTURE0 - 0, для GL_TEXTURE1 - 1 и тд)
	glUniform1i(unifTexture1, 0);

	glActiveTexture(GL_TEXTURE1);
	sf::Texture::bind(&textureData2);
	glUniform1i(unifTexture2, 1);

	// Подключаем VBO
	glEnableVertexAttribArray(attribVertex);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(attribTexture);
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	glVertexAttribPointer(attribTexture, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(attribColor);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Передаем данные на видеокарту(рисуем)
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// Отключаем массив атрибутов
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribColor);
	// Отключаем шейдерную программу
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
	glDeleteBuffers(1, &vertexVBO);
	glDeleteBuffers(1, &colorVBO);
}

void Release() {
	ReleaseShader();
	ReleaseVBO();
}
