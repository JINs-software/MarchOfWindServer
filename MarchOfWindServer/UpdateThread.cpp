#include "UpdateThread.h"

using namespace std;

UINT __stdcall UpdateThread::UpdateThreadFunc(void* arg)
{
	UpdateThread* updateThread = reinterpret_cast<UpdateThread*>(arg);

	clock_t timeStamp = clock();
	while (true) {
		// 1. 오브젝트 삭제 요청 수행
		{
			lock_guard<mutex> lockGuard(updateThread->m_ReadyToDestroyObjectsMtx);

			for (auto iter = updateThread->m_GameObjects.begin(); iter != updateThread->m_GameObjects.end();) {
				GameObject* gameObject = *iter;
				if (updateThread->m_ReadyToDestroyObjects.find(gameObject) != updateThread->m_ReadyToDestroyObjects.end()) {
					gameObject->OnDestroy();

					delete gameObject;

					updateThread->m_ReadyToDestroyObjects.erase(gameObject);
					iter = updateThread->m_GameObjects.erase(iter);
				}
				else {
					iter++;
				}
			}

			if (updateThread->m_ReadyToDestroyObjects.size() > 0) {
				DebugBreak();
			}
		}

		// 2. 오브젝트 등록 요청 수행
		{
			lock_guard<mutex> lockGuard(updateThread->m_ReadyToRegistObjectsMtx);

			for (auto gameObject : updateThread->m_ReadyToRegistObjects) {
				updateThread->m_GameObjects.push_back(gameObject);
			}

			updateThread->m_ReadyToRegistObjects.clear();
		}

		// 3. Update
		float deltaTime = (clock() - timeStamp) / static_cast<float>(CLOCKS_PER_SEC);
		for (auto gameObject : updateThread->m_GameObjects) {
			gameObject->OnUpdate(deltaTime);
		}
	}

	return 0;
}
