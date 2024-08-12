#pragma once

#include <list>
#include <set>
#include <mutex>

#include <Windows.h>	
#include<process.h>

#include "LockFreeQueue.h"
#include "JBuffer.h"

class GameObject {
public:
	virtual ~GameObject() {}

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

	LockFreeQueue<std::pair<int, JBuffer*>> m_SendReqMessageQueue;

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
		m_ReadyToRegistObjects.insert(gameObject);
	}
	void DestroyGameObject(GameObject* gameObject) {
		std::lock_guard<std::mutex> lockGuard(m_ReadyToDestroyObjectsMtx);
		m_ReadyToDestroyObjects.insert(gameObject);
	}

	bool GetSendReqMessage(std::pair<int, JBuffer*>& request) {
		if (m_SendReqMessageQueue.GetSize() == 0) {
			return false;
		}
		else {
			m_SendReqMessageQueue.Dequeue(request, true);
			return true;
		}
	}

protected:
	void PushSendReqMessage(int id, JBuffer* sendMsg) {
		m_SendReqMessageQueue.Enqueue({ id, sendMsg });
	}

private:
	static UINT __stdcall UpdateThreadFunc(void* arg);
};

