#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"
#include <imgui.h>

CMyApp::CMyApp() { }

CMyApp::~CMyApp() { }

void CMyApp::SetupDebugCallback()
{
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_programID = glCreateProgram();
	AssembleProgram(m_programID, "Shaders/Vert_PosTex.vert", "Shaders/Frag_Lighting.frag");
}

void CMyApp::CleanShaders()
{
	glDeleteProgram(m_programID);
}

void CMyApp::InitGeometry()
{
	MeshObject<VertexPosTex> quadCPU;

	quadCPU.vertexArray = {
		{ glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f) },
		{ glm::vec3(1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 1.0f) }
	};

	quadCPU.indexArray = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &quadVaoID);
	glBindVertexArray(quadVaoID);

	glGenBuffers(1, &quadVboID);
	glBindBuffer(GL_ARRAY_BUFFER, quadVboID);

	glBufferData(GL_ARRAY_BUFFER, quadCPU.vertexArray.size() * sizeof(VertexPosTex), quadCPU.vertexArray.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &quadIboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadCPU.indexArray.size() * sizeof(GLuint), quadCPU.indexArray.data(), GL_STATIC_DRAW);

	quadCount = static_cast<GLsizei>(quadCPU.indexArray.size());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPosTex), reinterpret_cast<const void*>(offsetof(VertexPosTex, position)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPosTex), reinterpret_cast<const void*>(offsetof(VertexPosTex, texcoord)));

	glBindVertexArray(0);
}

void CMyApp::CleanGeometry()
{
	glDeleteBuffers(1, &quadVaoID);
	glDeleteBuffers(1, &quadVboID);
	glDeleteBuffers(1, &quadIboID);
}

void CMyApp::InitTextures()
{
	glGenTextures(1, &m_textureID);
	SetupTextureSampling(GL_TEXTURE_2D, m_textureID);
}

void CMyApp::CleanTextures()
{
	glDeleteTextures(1, &m_textureID);
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();
	InitTextures();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	m_camera.SetView(glm::vec3(0.0, 7.0, 7.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	m_cameraManipulator.SetCamera( &m_camera );

	metaBalls.push_back(glm::vec4(1,0,0,1));
	metaBalls.push_back(glm::vec4(-1,0,0,1));
	metaBalls.push_back(glm::vec4(10, 0, 0, 1));
	metaBalls.push_back(glm::vec4(-10, 0, 0, 1));

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanTextures();
	CleanGeometry();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
	m_cameraManipulator.Update( updateInfo.DeltaTimeInSec );

	//metaBalls[0].a += sin(updateInfo.DeltaTimeInSec) / 2.0f + 0.5f;
	//metaBalls[1].w += sin(updateInfo.DeltaTimeInSec) / 2.0f + 0.5f;
}

void CMyApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(quadVaoID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glUseProgram(m_programID);

	glUniform3fv(ul("eye"), 1, glm::value_ptr(m_camera.GetEye()));
	glUniform1f(ul("aspect"), m_camera.GetAspect());

	glUniformMatrix4fv(ul("inverseProjection"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetProj())));
	glUniformMatrix4fv(ul("inverseView"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewMatrix())));

	glUniform4fv(ul("lightPos1"), 1, glm::value_ptr(m_lightPos1));
	glUniform4fv(ul("lightPos2"), 1, glm::value_ptr(m_lightPos2));

	glUniform3fv(ul("La"), 1, glm::value_ptr(m_La));
	glUniform3fv(ul("Ld"), 1, glm::value_ptr(m_Ld));
	glUniform3fv(ul("Ls"), 1, glm::value_ptr(m_Ls));

	glUniform1f(ul("lightConstantAttenuation"), m_lightConstantAttenuation);
	glUniform1f(ul("lightLinearAttenuation"), m_lightLinearAttenuation);
	glUniform1f(ul("lightQuadraticAttenuation"), m_lightQuadraticAttenuation);

	glUniform3fv(ul("Ka"), 1, glm::value_ptr(m_Ka));
	glUniform3fv(ul("Kd"), 1, glm::value_ptr(m_Kd));
	glUniform3fv(ul("Ks"), 1, glm::value_ptr(m_Ks));

	glUniform1f(ul("Shininess"), m_Shininess);

	glUniform1f(ul("threshold"), THRESHOLD);
	glUniform4fv(ul("metaBalls"), metaBalls.size(), glm::value_ptr(metaBalls[0]));

	glDisable(GL_CULL_FACE);

	glDrawElements(GL_TRIANGLES, quadCount, GL_UNSIGNED_INT, nullptr);

	glEnable(GL_CULL_FACE);

	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void CMyApp::RenderGUI()
{
	if ( ImGui::Begin( "Lighting settings" ) )
	{
		ImGui::SeparatorText("Settings");
		
		ImGui::InputFloat("Shininess", &m_Shininess, 0.1f, 1.0f, "%.1f");
		static float Kaf = 1.0f;
		static float Kdf = 1.0f;
		static float Ksf = 1.0f;
		if ( ImGui::SliderFloat( "Ka", &Kaf, 0.0f, 1.0f ) )
		{
			m_Ka = glm::vec3( Kaf );
		}
		if ( ImGui::SliderFloat( "Kd", &Kdf, 0.0f, 1.0f ) )
		{
			m_Kd = glm::vec3( Kdf );
		}
		if ( ImGui::SliderFloat( "Ks", &Ksf, 0.0f, 1.0f ) )
		{
			m_Ks = glm::vec3( Ksf );
		}

		ImGui::SliderFloat( "Constant Attenuation", &m_lightConstantAttenuation, 0.001f, 2.0f );
		ImGui::SliderFloat( "Linear Attenuation", &m_lightLinearAttenuation, 0.001f, 2.0f );
		ImGui::SliderFloat( "Quadratic Attenuation", &m_lightQuadraticAttenuation, 0.001f, 2.0f );	
	}

	ImGui::End();
}

GLint CMyApp::ul( const char* uniformName ) noexcept
{
	GLuint programID = 0;
	glGetIntegerv( GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>( &programID ) );
	return glGetUniformLocation( programID, uniformName );
}

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{	
	if ( key.repeat == 0 )
	{
		if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL )
		{
			CleanShaders();
			InitShaders();
		}
		if ( key.keysym.sym == SDLK_F1 )
		{
			GLint polygonModeFrontAndBack[ 2 ] = {};
			glGetIntegerv( GL_POLYGON_MODE, polygonModeFrontAndBack );
			GLenum polygonMode = ( polygonModeFrontAndBack[ 0 ] != GL_FILL ? GL_FILL : GL_LINE );
			glPolygonMode( GL_FRONT_AND_BACK, polygonMode );
		}
	}
	m_cameraManipulator.KeyboardDown( key );
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_cameraManipulator.KeyboardUp( key );
}

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_cameraManipulator.MouseMove( mouse );
}

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse) { }

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse) { }

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_cameraManipulator.MouseWheel( wheel );
}

void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.SetAspect(static_cast<float>(_w) / _h);
}

void CMyApp::OtherEvent( const SDL_Event& ev ) { }