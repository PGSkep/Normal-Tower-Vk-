#include <stdint.h>
#include <vector>
#include <iostream>
#include <Windows.h>

#include "Timer.h"
#include "Engine.h"
#include "Renderer.h"

struct Projectile
{
	glm::mat4 transform;
	float speed;
	float damage;
	float travel;

	static Projectile GetProjectile(glm::mat4 _transform, float _speed, float _damage, float _travel = 0.0f)
	{
		Projectile projectile;

		projectile.transform = _transform;
		projectile.speed = _speed;
		projectile.damage = _damage;
		projectile.travel = _travel;

		return projectile;
	}
};
struct Tower
{
	glm::mat4 transform;
	float baseFireRate;
	float fireRateDelta;
	float radius;

	static Tower GetTower(glm::mat4 _transform, float _baseFireRate, float _fireRateDelta, float _radius)
	{
		Tower tower;

		tower.transform = _transform;
		tower.baseFireRate = _baseFireRate;
		tower.fireRateDelta = _fireRateDelta;
		tower.radius = _radius;

		return tower;
	}
};
struct Enemy
{
	glm::mat4 transform;
	float speed;
	float hitPoints;
	uint16_t pathIndex;

	Enemy(glm::mat4 _transform, float _speed, float _hitPoints, uint16_t _pathIndex = 0)
	{
		transform = _transform;
		speed = _speed;
		hitPoints = _hitPoints;
		pathIndex = _pathIndex;
	}
};
struct Object
{
	std::vector<void(*)(void*)> functions;
	void* data;

	static Object GetObject(std::vector<void(*)(void*)> _functions, void* _data)
	{
		Object object;

		object.functions = _functions;
		object.data = _data;

		return object;
	}
};

std::vector<glm::vec3> path
{
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(10.0f, 0.0f,  0.0f),
	glm::vec3(10.0f, 10.0f, 0.0f),
	glm::vec3(15.0f, 15.0f, 0.0f),
};

std::vector<Object> object;

double deltaTime = 0.0f;

#define SPAWN_RATE 1.0

void EnemyMove(void* _data)
{
	glm::mat4 newTransform = glm::translate(glm::mat4(), glm::vec3(((Enemy*)_data)->transform[3][0], ((Enemy*)_data)->transform[3][1], ((Enemy*)_data)->transform[3][2]));

	glm::mat4 rotation = glm::lookAt(glm::vec3(((Enemy*)_data)->transform[3][0], ((Enemy*)_data)->transform[3][1], ((Enemy*)_data)->transform[3][2]), path[((Enemy*)_data)->pathIndex + 1], glm::vec3(0.0f, 1.0f, 0.0f));

	//newTransform = newTransform * rotation;

	newTransform = glm::rotate(newTransform, 3.1415f/4, glm::vec3(0, 1, 0));
	newTransform = glm::translate(newTransform, glm::vec3(0.0f, 0.0f, ((Enemy*)_data)->speed * deltaTime));

	((Enemy*)_data)->transform = newTransform;
}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);

	std::cout << "Controls: QWEASDRF.\n";

	//Timer globalTimer;
	//globalTimer.SetResolution(Timer::RESOLUTION_NANOSECONDS);
	//globalTimer.Play();
	//
	//Timer spawnTimer;
	//spawnTimer.SetResolution(Timer::RESOLUTION_NANOSECONDS);
	//spawnTimer.Play();

	Engine engine;
	engine.Init();
	engine.Loop();

	while (false)
	{

		// Spawn enemy
		//if (spawnTimer.GetTime() > SPAWN_RATE)
		//{
		//	spawnTimer.SetTime(spawnTimer.GetTime() - SPAWN_RATE);
		//	object.push_back(Object::GetObjectA({EnemyMove}, new Enemy(glm::translate(glm::mat4(), path[0]), 0.2f, 10, 0)));
		//}

		// Update all objects
		//for (size_t i = 0; i != object.size(); ++i)
		//{
		//	for (size_t j = 0; j != object[i].functions.size(); ++j)
		//	{
		//		object[i].functions[j](object[i].data);
		//	}
		//}

		

		// print
		//system("cls");
		//
		//if(object.size() > 0)
		//	std::cout << ((Enemy*)object[0].data)->transform[3][0] << '\n' << ((Enemy*)object[0].data)->transform[3][1] << '\n' << ((Enemy*)object[0].data)->transform[3][2] << '\n';
		//
		//deltaTime = globalTimer.GetTime() - startTime;
		//startTime = globalTimer.GetTime();
	}

	//for (size_t i = 0; i != object.size(); ++i)
	//{
	//	if(object[i].data != nullptr)
	//		delete object[i].data;
	//}

	engine.ShutDown();

	return 0;
}