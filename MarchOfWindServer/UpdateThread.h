#pragma once

#include <list>
#include <set>
#include <mutex>

#include <Windows.h>	
#include<process.h>

class GameObject {
public:

	virtual void OnStart() {}

	virtual void OnUpdate(float deltaTime) {}

	virtual void OnDestroy() {}
};

class UpdateThread
{
private:
	std::list<GameObject*> m_GameObjects;

	std::set<GameObject*> m_ReadyToRegistObjects;
	std::mutex m_ReadyToRegistObjectsMtx;

	std::set<GameObject*> m_ReadyToDestroyObjects;
	std::mutex m_ReadyToDestroyObjectsMtx;

private:
	bool m_Exit = false;

public:
	void StartUpdateThread() {
		_beginthreadex(NULL, 0, UpdateThreadFunc, this, 0, NULL);
	}
	void StopUpdateThread() {
		m_Exit = true;
	}

	void RegistGameObject(GameObject* gameObject) {
		std::lock_guard<std::mutex> lockGuard(m_ReadyToRegistObjectsMtx);
		m_ReadyToDestroyObjects.insert(gameObject);
	}
	void DestroyGameObject(GameObject* gameObject) {
		std::lock_guard<std::mutex> lockGuard(m_ReadyToDestroyObjectsMtx);
		m_ReadyToDestroyObjects.insert(gameObject);
	}


	static UINT __stdcall UpdateThreadFunc(void* arg);
};

