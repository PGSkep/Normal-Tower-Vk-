#ifndef	ENGINE_H
#define ENGINE_H

#include "Timer.h"

#include "Input.h"
#include "Renderer.h"

class Camera
{
public:
	static glm::vec3 globalUp;

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;

	float speed;

	glm::mat4 view;

	void UpdateFront()
	{
		front = glm::normalize(position - target);
	}
	void UpdateRight()
	{
		right = glm::normalize(glm::cross(globalUp, front));
	}
	void UpdateUp()
	{
		up = glm::cross(front, right);
	}

	void Init(glm::vec3 _position, glm::vec3 _target, float _speed)
	{
		position = _position;
		target = _target;

		speed = _speed;

		UpdateFront();
		UpdateRight();
		UpdateUp();
	}

	void MoveLocal(glm::vec3 _translation)
	{
		position += front * _translation.z;
		position += right * _translation.x;
		position += up * _translation.y;

		UpdateFront();
		UpdateRight();
		UpdateUp();
	}
	void MoveGlobal(glm::vec3 _translation)
	{
		position += _translation;

		UpdateFront();
		UpdateRight();
		UpdateUp();
	}

	glm::mat4 GetView()
	{
		return glm::lookAt(position, target, globalUp);
	}
};

class Engine
{
public:
	static Timer timer;
	static double deltaTime;

	bool done = false;

	Input input;
	Renderer renderer;

	static Camera camera;

	void Init();

	void Input();
	void Update();
	void Render();

	void Loop();

	void ShutDown();
};

#endif