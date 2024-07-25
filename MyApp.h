#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "GLUtils.hpp"
#include "Camera.h"
#include "CameraManipulator.h"

struct SUpdateInfo
{
	float ElapsedTimeInSec = 0.0f;
	float DeltaTimeInSec   = 0.0f;
};

class CMyApp
{
public:
	CMyApp();
	~CMyApp();

	bool Init();
	void Clean();

	void Update( const SUpdateInfo& );
	void Render();
	void RenderGUI();

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);

	void OtherEvent( const SDL_Event& );
protected:
	void SetupDebugCallback();

	Camera m_camera;
	CameraManipulator m_cameraManipulator;

	float m_ElapsedTimeInSec = 0.0f;

	static GLint ul( const char* uniformName ) noexcept;

	GLuint m_programID = 0;

	glm::vec4 m_lightPos1 = glm::vec4(20.0f, 0.0f, 1.0f, 0.0f);
	glm::vec4 m_lightPos2 = glm::vec4(-20.0f, 0.0f, 1.0f, 0.0f);

	glm::vec3 m_La = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 m_Ld = glm::vec3(1.0, 1.0, 1.0);
	glm::vec3 m_Ls = glm::vec3(1.0, 1.0, 1.0);

	float m_lightConstantAttenuation = 1.0;
	float m_lightLinearAttenuation = 0.0;
	float m_lightQuadraticAttenuation = 0.0;

	glm::vec3 m_Ka = glm::vec3(1.0);
	glm::vec3 m_Kd = glm::vec3(1.0);
	glm::vec3 m_Ks = glm::vec3(1.0);

	float m_Shininess = 1.0;

	void InitShaders();
	void CleanShaders();

	const float THRESHOLD = 0.5f;

	std::vector<glm::vec4> metaBalls;

	GLuint quadVaoID = 0;
	GLuint quadVboID = 0;
	GLuint quadIboID = 0;
	GLsizei quadCount = 0;

	void InitGeometry();
	void CleanGeometry();

	GLuint m_textureID = 0;

	void InitTextures();
	void CleanTextures();
};

