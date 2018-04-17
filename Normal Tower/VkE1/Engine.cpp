#include "Engine.h"

glm::vec3 Camera::globalUp;

Timer Engine::timer;
double Engine::deltaTime;
Camera Engine::camera;

void Engine::Init()
{
	Camera::globalUp = glm::vec3(0.0f, 1.0f, 0.0f);

	timer.SetResolution(Timer::RESOLUTION_NANOSECONDS);
	timer.Play();

	input.activeKeys = { Input::INPUT_KEYS::KEY_ESC,
		Input::INPUT_KEYS::KEY_MOUSE_RIGHT,
		Input::INPUT_KEYS::KEY_SHIFT,
		Input::INPUT_KEYS::KEY_R, Input::INPUT_KEYS::KEY_F, Input::INPUT_KEYS::KEY_ARROW_UP, Input::INPUT_KEYS::KEY_ARROW_DOWN, Input::INPUT_KEYS::KEY_ARROW_LEFT, Input::INPUT_KEYS::KEY_ARROW_RIGHT,
		Input::INPUT_KEYS::KEY_Q, Input::INPUT_KEYS::KEY_W, Input::INPUT_KEYS::KEY_E, Input::INPUT_KEYS::KEY_A, Input::INPUT_KEYS::KEY_S, Input::INPUT_KEYS::KEY_D };

	input.Update();
	input.Update();

	renderer.Init();
	renderer.Load(
	{
		Renderer::ShaderProperties::GetShaderProperties("Shaders/vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main"),
		Renderer::ShaderProperties::GetShaderProperties("Shaders/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main"),
	},
	{
		"Models/Tower.fbx",
	},
	{
		"Images/TowerDiffuse.tga",
		"Images/TowerNormal.tga",
	});
	renderer.Setup();

	camera.Init(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), 3.0f);
}

void Engine::Input()
{
	input.Update();
}
void Engine::Update()
{
	glm::vec3 translation;

	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_W))
		translation.z -= (float)deltaTime;
	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_S))
		translation.z += (float)deltaTime;

	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_D))
		translation.x += (float)deltaTime;
	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_A))
		translation.x -= (float)deltaTime;

	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_E))
		translation.y += (float)deltaTime;
	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_Q))
		translation.y -= (float)deltaTime;

	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_MOUSE_RIGHT))
	{
		translation.x += -input.cursorDelta.x * (float)deltaTime * 10.0f;
		translation.y += input.cursorDelta.y * (float)deltaTime * 10.0f;
	}

	float multiplier = 1.0f;
	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_SHIFT))
		multiplier = 10.0f;

	camera.MoveLocal(translation * camera.speed * multiplier);

	glm::mat4* view = renderer.GetView();
	*view = camera.GetView();

	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_R))
		camera.target.y += (float)deltaTime;
	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_F))
		camera.target.y -= (float)deltaTime;

	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_ARROW_UP))
		camera.target.z += (float)deltaTime;
	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_ARROW_DOWN))
		camera.target.z -= (float)deltaTime;

	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_ARROW_LEFT))
		camera.target.x += (float)deltaTime;
	if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_ARROW_RIGHT))
		camera.target.x -= (float)deltaTime;
}
void Engine::Render()
{
	renderer.Render();
}

void Engine::Loop()
{
	double previousTime = timer.GetTime();
	double currentTime;

	while (!done)
	{
		currentTime = timer.GetTime();
		deltaTime = currentTime - previousTime;
		previousTime = currentTime;

		if (input.CheckKeyDown(Input::INPUT_KEYS::KEY_ESC))
			done = true;

		Input();
		Update();
		Render();
	}
}

void Engine::ShutDown()
{
	renderer.ShutDown();
}
