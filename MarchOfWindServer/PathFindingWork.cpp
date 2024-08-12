#include "PathFindingWork.h"

void PathFindingWorkerPool::AddPathFindingWorkToPool(const PathFindingJob& work)
{
	lock_guard<mutex> lockGuard(m_JobQueueMtx);
	m_PathFindingJobQueue.push(work);
}

void PathFindingWorkerPool::PathFindingWorkerFunc(LPVOID pParam)
{
	PathFindingJob pathFindingJob;
	PathFindingWorkerPool* workerPool = reinterpret_cast<PathFindingWorkerPool*>(pParam);

	DWORD thrdID = GetCurrentThreadId();
	HANDLE event = workerPool->m_ThredIdToEventHndMap[thrdID];

	while (true) {
		if (workerPool->GetWorkFromPool(pathFindingJob)) {
			WaitForSingleObject(event, INFINITE);
			continue;
		}

		PathFindingParams params = pathFindingJob.params;
		vector<pair<float, float>> resultPath;
		pathFindingJob.pathFindingFunc(params.position, params.radius, params.tolerance, params.destination, resultPath);
	}
}

bool PathFindingWorkerPool::GetWorkFromPool(PathFindingJob& job)
{
	bool ret = false;

	lock_guard<mutex> lockGuard(m_JobQueueMtx);
	if (!m_PathFindingJobQueue.empty()) {
		job = m_PathFindingJobQueue.front();
		m_PathFindingJobQueue.pop();
		ret = true;
	}

	return ret;
}
