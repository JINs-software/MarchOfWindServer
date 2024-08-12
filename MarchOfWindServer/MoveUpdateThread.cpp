#include "MoveUpdateThread.h"
#include "JPSPathFinder.h"

using namespace std;


void MoveUpdateThread::ResetCollder(float x, float z, float radius, bool draw, std::set<std::pair<int, int>>& colliders) {
	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;

	if (draw) {
		Circle_2 circle({ preciseX - preciseRadius, preciseZ }, { preciseX + preciseRadius, preciseZ });
		CGAL::Bbox_2 bbox = circle.bbox();
		for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
			for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
				Point_2 p(x, z);
				if (circle.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] += 1;
					colliders.insert(make_pair(x, z));
				}
			}
		}
	}
	else {
		for (const auto& p : colliders) {
			m_UnitColliderCountMap[p.second][p.first] -= 1;
		}
		colliders.clear();
	}
	
}
bool MoveUpdateThread::MoveCollider(float x, float z, float radius, float nx, float nz, std::set<std::pair<int, int>>& colliders) {
	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;
	int preciseNX = nx * PRECISION;
	int preciseNZ = nz * PRECISION;

	if (preciseX == preciseNX && preciseZ == preciseNZ) {
		return true;
	}

	Circle_2 circle({ preciseX - preciseRadius, preciseZ }, { preciseX + preciseRadius, preciseZ });
	Circle_2 circleNew({ preciseNX - preciseRadius, preciseNZ }, { preciseNX + preciseRadius, preciseNZ });

	if (CGAL::do_intersect(circle, circleNew)) {
		CGAL::Bbox_2 bbox = circleNew.bbox();
		for (int y = bbox.ymin() + 1; y < bbox.ymax(); y++) {
			for (int x = bbox.xmin() + 1; x < bbox.xmax(); x++) {
				Point_2 p(x, y);
				if (circleNew.has_on_bounded_side(p) && !circle.has_on_boundary(p) && !circle.has_on_bounded_side(p)) {
					if (m_UnitColliderCountMap[y][x] > 0) {
						return false;
					}
				}
			}
		}
	}
	else {
		CGAL::Bbox_2 bbox = circleNew.bbox();
		for (int y = bbox.ymin() + 1; y < bbox.ymax(); y++) {
			for (int x = bbox.xmin() + 1; x < bbox.xmax(); x++) {
				Point_2 p(x, y);
				if (circleNew.has_on_bounded_side(p)) {
					if (m_UnitColliderCountMap[y][x] > 0) {
						return false;
					}
				}
			}
		}
	}

	for (const auto& p : colliders) {
		m_UnitColliderCountMap[p.second][p.first] -= 1;
	}
	colliders.clear();


	CGAL::Bbox_2 drawBox = circleNew.bbox();
	for (int x = drawBox.xmin(); x <= drawBox.xmax(); x++) {
		for (int z = drawBox.ymin(); z <= drawBox.ymax(); z++) {
			Point_2 p(x, z);
			if (circleNew.has_on_boundary(p)) {
				//if (colliders.find(make_pair(x, z)) != colliders.end()) {
				//	DebugBreak();
				//}
				m_UnitColliderCountMap[z][x] += 1;
				colliders.insert(make_pair(x, z));
			}
		}
	}

	return true;
}

void MoveUpdateThread::TracePathFindingFunc(const pair<float, float>& position, float radius, float tolerance, const pair<float, float>& dest, vector<pair<float, float>>& resultPath) {
	//m_UnitColliderCountMap[z][x]

	int iPosX = position.first * PRECISION;
	int iPosZ = position.second * PRECISION;
	int iDestX = dest.first * PRECISION;
	int iDestZ = dest.second * PRECISION;
	int iRadius = radius * PRECISION;
	int iTolerance = tolerance * PRECISION;

	Circle_2 radiusCircle({ -iRadius, 0 }, { iRadius, 0 });
	Circle_2 toleranceCircle({ -iTolerance, 0 }, { iTolerance, 0 });
	vector<pair<int, int>> radiusCircleBoundary;
	vector<pair<int, int>> radiusCircleBounded;
	vector<pair<int, int>> toleranceCircleBoundary;
	vector<pair<int, int>> toleranceCircleBounded;

	CGAL::Bbox_2 radiusBbox = radiusCircle.bbox();
	for (int x = radiusBbox.xmin(); x < radiusBbox.xmax(); x++) {
		for (int z = radiusBbox.ymin(); z < radiusBbox.ymax(); z++) {
			Point_2 p(x, z);
			if (radiusCircle.has_on_boundary(p)) {
				radiusCircleBoundary.push_back({x, z});
			}
			else if (radiusCircle.has_on_bounded_side(p)) {
				radiusCircleBounded.push_back({ x, z });
			}
		}
	}
	
	CGAL::Bbox_2 toleranceBbox = toleranceCircle.bbox();
	for (int x = toleranceBbox.xmin(); x < toleranceBbox.xmax(); x++) {
		for (int z = toleranceBbox.ymin(); z < toleranceBbox.ymax(); z++) {
			Point_2 p(x, z);
			if (toleranceCircle.has_on_boundary(p)) {
				toleranceCircleBoundary.push_back({ x, z });
			}
			else if (toleranceCircle.has_on_bounded_side(p)) {
				toleranceCircleBounded.push_back({ x, z });
			}
		}
	}

	JPSPathFinder<int> jpsPathFinder;
	jpsPathFinder.Init(3000, 3000);

	for (int z = 0; z < 3000; z++) {
		jpsPathFinder.SetObstacle(z, 1000);
	}
	for (int x = 0; x < 3000; x++) {
		jpsPathFinder.SetObstacle(1000, x);
	}

	for (int x = 1001; x < 3000; x++) {
		for (int z = 1001; z < 3000; z++) {
			if (m_UnitColliderCountMap[z][x] > 0) {
				for (const auto& p : radiusCircleBoundary) {
					int setX = p.first + x;
					int setZ = p.second + z;

					jpsPathFinder.SetObstacle(setZ, setX);
				}
			}
		}
	}

	for (const auto& p : radiusCircleBoundary) {
		int delX = iPosX + p.first;
		int delZ = iPosZ + p.second;
		jpsPathFinder.UnsetObstacle(delZ, delX);
	}
	for (const auto& p : radiusCircleBounded) {
		int delX = iPosX + p.first;
		int delZ = iPosZ + p.second;
		jpsPathFinder.UnsetObstacle(delZ, delX);
	}

	for (const auto& p : toleranceCircleBoundary) {
		int delX = iDestX + p.first;
		int delZ = iDestZ + p.second;
		jpsPathFinder.UnsetObstacle(delZ, delX);
	}
	for (const auto& p : toleranceCircleBounded) {
		int delX = iDestX + p.first;
		int delZ = iDestZ + p.second;
		jpsPathFinder.UnsetObstacle(delZ, delX);
	}

	vector<PathNode<int>> trackList;
	auto iter = jpsPathFinder.FindPath(iPosX, iPosZ, iDestX, iDestZ, trackList);
	
	// 메시지 생성
	// .....
	//PushSendReqMessage(유닛 아이디, 메시지);
}