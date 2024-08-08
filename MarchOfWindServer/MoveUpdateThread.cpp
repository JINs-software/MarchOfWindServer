#include "MoveUpdateThread.h"

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

	//CGAL::Bbox_2 delBox = circle.bbox();
	//for (int x = delBox.xmin(); x <= delBox.xmax(); x++) {
	//	for (int z = delBox.ymin(); z <= delBox.ymax(); z++) {
	//		Point_2 p(x, z);
	//		if (circle.has_on_boundary(p)) {
	//			if (colliders.find(make_pair(x, z)) == colliders.end()) {
	//				DebugBreak();
	//			}
	//			m_UnitColliderCountMap[z][x] -= 1;
	//			colliders.erase(make_pair(x, z));
	//		}
	//	}
	//}
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

/*
void MoveUpdateThread::ResetCollder(float x, float z, float radius, bool draw) {

	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;


	//pair<int, int> quadrantXZ_1 = make_pair(preciseX + preciseRadius, preciseZ + preciseRadius);
	//pair<int, int> quadrantXZ_2 = make_pair(preciseX - preciseRadius, preciseZ + preciseRadius);
	//pair<int, int> quadrantXZ_3 = make_pair(preciseX - preciseRadius, preciseZ - preciseRadius);
	//pair<int, int> quadrantXZ_4 = make_pair(preciseX + preciseRadius, preciseZ - preciseRadius);
	//
	//int diff = (2 - sqrt(2)) * preciseRadius;
	//
	//vector<Point_2> points = {
	//	Point_2(quadrantXZ_1.first, quadrantXZ_1.second - diff), Point_2(quadrantXZ_1.first - diff, quadrantXZ_1.second),
	//	Point_2(quadrantXZ_2.first + diff, quadrantXZ_2.second), Point_2(quadrantXZ_2.first, quadrantXZ_2.second - diff),
	//	Point_2(quadrantXZ_3.first, quadrantXZ_3.second + diff), Point_2(quadrantXZ_3.first + diff, quadrantXZ_3.second),
	//	Point_2(quadrantXZ_4.first - diff, quadrantXZ_4.second), Point_2(quadrantXZ_4.first, quadrantXZ_4.second + diff),
	//};

	int diff = preciseRadius / sqrt(2);

	vector<Point_2> points = {
		Point_2(preciseX + diff, preciseZ + diff), Point_2(preciseX, preciseZ + preciseRadius),
		Point_2(preciseX - diff, preciseZ + diff), Point_2(preciseX - preciseRadius, preciseZ),
		Point_2(preciseX - diff, preciseZ - diff), Point_2(preciseX, preciseZ - preciseRadius),
		Point_2(preciseX + diff, preciseZ - diff), Point_2(preciseX + preciseRadius, preciseZ)
	};

	Polygon_2 polygon(points.begin(), points.end());

	if (!polygon.is_simple() || !polygon.is_counterclockwise_oriented()) {
		DebugBreak();
	}

	CGAL::Bbox_2 bbox = polygon.bbox();

	//AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
	for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
		for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
			Point_2 p(x, z);
			if (polygon.has_on_boundary(p)) {
				if (draw) {
					m_UnitColliderCountMap[z][x] += 1;
				}
				else {
					//if (m_UnitColliderCountMap[z][x] <= 0) {
					//	DebugBreak();
					//}
					m_UnitColliderCountMap[z][x] -= 1;
				}
			}
		}
	}
	//ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
}

bool MoveUpdateThread::MoveCollider_new(float x, float z, float radius, float nx, float nz) {
	// 기존 팔각형 꼭지점 좌표
	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;

	// 새로운 팔각형 꼭지점 좌표
	int preciseNX = nx * PRECISION;
	int preciseNZ = nz * PRECISION;

	//int diff = (2 - sqrt(2)) * preciseRadius;
	int diff = preciseRadius / sqrt(2);

	if (preciseX == preciseNX && preciseZ == preciseNZ) {
		return true;
	}

	vector<Point_2> points = {
		Point_2(preciseX + diff, preciseZ + diff), Point_2(preciseX, preciseZ + preciseRadius),
		Point_2(preciseX - diff, preciseZ + diff), Point_2(preciseX - preciseRadius, preciseZ),
		Point_2(preciseX - diff, preciseZ - diff), Point_2(preciseX, preciseZ - preciseRadius),
		Point_2(preciseX + diff, preciseZ - diff), Point_2(preciseX + preciseRadius, preciseZ)
	};

	vector<Point_2> pointsNew = {
		Point_2(preciseNX + diff, preciseNZ + diff), Point_2(preciseNX, preciseNZ + preciseRadius),
		Point_2(preciseNX - diff, preciseNZ + diff), Point_2(preciseNX - preciseRadius, preciseNZ),
		Point_2(preciseNX - diff, preciseNZ - diff), Point_2(preciseNX, preciseNZ - preciseRadius),
		Point_2(preciseNX + diff, preciseNZ - diff), Point_2(preciseNX + preciseRadius, preciseNZ)
	};

	Polygon_2 polygon(points.begin(), points.end());
	Polygon_2 polygonNew(pointsNew.begin(), pointsNew.end());

	if (!polygon.is_simple() || !polygon.is_counterclockwise_oriented() || !polygonNew.is_simple() || !polygonNew.is_counterclockwise_oriented()) {	// 자가 교차 폴리곤이 있는지 검사
		DebugBreak();
	}

	if (CGAL::do_intersect(polygon, polygonNew)) {
		CGAL::Bbox_2 bbox = polygonNew.bbox();
		for (int y = bbox.ymin() + 1; y < bbox.ymax(); y++) {
			for (int x = bbox.xmin() + 1; x < bbox.xmax(); x++) {
				Point_2 p(x, y);
				if (polygonNew.has_on_bounded_side(p) || !polygon.has_on_boundary(p) || !polygon.has_on_bounded_side(p)) {
					if (m_UnitColliderCountMap[y][x] > 0) {
						return false;
					}
				}
			}
		}
	}
	else {
		CGAL::Bbox_2 bbox = polygonNew.bbox();
		for (int y = bbox.ymin() + 1; y < bbox.ymax(); y++) {
			for (int x = bbox.xmin() + 1; x < bbox.xmax(); x++) {
				Point_2 p(x, y);
				if (polygonNew.has_on_bounded_side(p)) {
					if (m_UnitColliderCountMap[y][x] > 0) {
						return false;
					}
				}
			}
		}
	}

	CGAL::Bbox_2 drawBox = polygonNew.bbox();
	for (int x = drawBox.xmin(); x <= drawBox.xmax(); x++) {
		for (int z = drawBox.ymin(); z <= drawBox.ymax(); z++) {
			Point_2 p(x, z);
			if (polygonNew.has_on_boundary(p)) {
				m_UnitColliderCountMap[z][x] += 1;
			}
		}
	}
	CGAL::Bbox_2 delBox = polygon.bbox();
	for (int x = delBox.xmin(); x <= delBox.xmax(); x++) {
		for (int z = delBox.ymin(); z <= delBox.ymax(); z++) {
			Point_2 p(x, z);
			if (polygon.has_on_boundary(p)) {
				m_UnitColliderCountMap[z][x] -= 1;
			}
		}
	}

	return true;
}



bool MoveUpdateThread::MoveCollider(float x, float z, float radius, float nx, float nz) {
	// 기존 팔각형 꼭지점 좌표
	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;

	// 새로운 팔각형 꼭지점 좌표
	int preciseNX = nx * PRECISION;
	int preciseNZ = nz * PRECISION;

	//int diff = (2 - sqrt(2)) * preciseRadius;
	int diff = preciseRadius / sqrt(2);

	if (preciseX == preciseNX && preciseZ == preciseNZ) {
		return true;
	}

	//pair<int, int> quadrantXZ_1 = make_pair(preciseX + preciseRadius, preciseZ + preciseRadius);
	//pair<int, int> quadrantXZ_2 = make_pair(preciseX - preciseRadius, preciseZ + preciseRadius);
	//pair<int, int> quadrantXZ_3 = make_pair(preciseX - preciseRadius, preciseZ - preciseRadius);
	//pair<int, int> quadrantXZ_4 = make_pair(preciseX + preciseRadius, preciseZ - preciseRadius);
	//
	//
	//vector<Point_2> points = {
	//	Point_2(quadrantXZ_1.first, quadrantXZ_1.second - diff), Point_2(quadrantXZ_1.first - diff, quadrantXZ_1.second),
	//	Point_2(quadrantXZ_2.first + diff, quadrantXZ_2.second), Point_2(quadrantXZ_2.first, quadrantXZ_2.second - diff),
	//	Point_2(quadrantXZ_3.first, quadrantXZ_3.second + diff), Point_2(quadrantXZ_3.first + diff, quadrantXZ_3.second),
	//	Point_2(quadrantXZ_4.first - diff, quadrantXZ_4.second), Point_2(quadrantXZ_4.first, quadrantXZ_4.second + diff),
	//};
	//
	//pair<int, int> quadrantNXZ_1 = make_pair(preciseNX + preciseRadius, preciseNZ + preciseRadius);
	//pair<int, int> quadrantNXZ_2 = make_pair(preciseNX - preciseRadius, preciseNZ + preciseRadius);
	//pair<int, int> quadrantNXZ_3 = make_pair(preciseNX - preciseRadius, preciseNZ - preciseRadius);
	//pair<int, int> quadrantNXZ_4 = make_pair(preciseNX + preciseRadius, preciseNZ - preciseRadius);
	//
	//vector<Point_2> pointsNew = {
	//	Point_2(quadrantNXZ_1.first, quadrantNXZ_1.second - diff), Point_2(quadrantNXZ_1.first - diff, quadrantNXZ_1.second),
	//	Point_2(quadrantNXZ_2.first + diff, quadrantNXZ_2.second), Point_2(quadrantNXZ_2.first, quadrantNXZ_2.second - diff),
	//	Point_2(quadrantNXZ_3.first, quadrantNXZ_3.second + diff), Point_2(quadrantNXZ_3.first + diff, quadrantNXZ_3.second),
	//	Point_2(quadrantNXZ_4.first - diff, quadrantNXZ_4.second), Point_2(quadrantNXZ_4.first, quadrantNXZ_4.second + diff),
	//};

	vector<Point_2> points = {
		Point_2(preciseX + diff, preciseZ + diff), Point_2(preciseX, preciseZ + preciseRadius),
		Point_2(preciseX - diff, preciseZ + diff), Point_2(preciseX - preciseRadius, preciseZ),
		Point_2(preciseX - diff, preciseZ - diff), Point_2(preciseX, preciseZ - preciseRadius),
		Point_2(preciseX + diff, preciseZ - diff), Point_2(preciseX + preciseRadius, preciseZ)
	};

	vector<Point_2> pointsNew = {
		Point_2(preciseNX + diff, preciseNZ + diff), Point_2(preciseNX, preciseNZ + preciseRadius),
		Point_2(preciseNX - diff, preciseNZ + diff), Point_2(preciseNX - preciseRadius, preciseNZ),
		Point_2(preciseNX - diff, preciseNZ - diff), Point_2(preciseNX, preciseNZ - preciseRadius),
		Point_2(preciseNX + diff, preciseNZ - diff), Point_2(preciseNX + preciseRadius, preciseNZ)
	};

	Polygon_2 polygon(points.begin(), points.end());
	Polygon_2 polygonNew(pointsNew.begin(), pointsNew.end());

	if (!polygon.is_simple() || !polygon.is_counterclockwise_oriented() || !polygonNew.is_simple() || !polygonNew.is_counterclockwise_oriented()) {	// 자가 교차 폴리곤이 있는지 검사
		DebugBreak();
	}

	std::list<Polygon_with_holes_2> intersections;
	CGAL::intersection(polygon, polygonNew, std::back_inserter(intersections));

	if (intersections.size() == 0) {

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 조건 추가
		// => 중심 좌표가 약간 다른 상태에서 거의 겹친 상태, 이 경우 아래 newBox에 포함된 좌표가 이미 표시된 좌표일 수 있음. 
		// (+ 표시되는 선분의 좌표는 정수이기에 정확한 일직선이 아님)
		int distance = sqrt(pow(preciseX - preciseNX, 2) + pow(preciseZ - preciseNZ, 2));
		if (distance < preciseRadius * 2) {
			cout << "distance < preciseRadius * 2" << endl;
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		else {

			CGAL::Bbox_2 newBbox = polygonNew.bbox();
			for (int x = newBbox.xmin(); x <= newBbox.xmax(); x++) {
				for (int z = newBbox.ymin(); z <= newBbox.ymax(); z++) {
					Point_2 p(x, z);
					if (polygonNew.has_on_bounded_side(p)) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							return false;
						}
					}
				}
			}
		}

		CGAL::Bbox_2 drawBox = polygonNew.bbox();
		for (int x = drawBox.xmin(); x <= drawBox.xmax(); x++) {
			for (int z = drawBox.ymin(); z <= drawBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygonNew.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] += 1;
				}
			}
		}
		CGAL::Bbox_2 delBox = polygon.bbox();
		for (int x = delBox.xmin(); x <= delBox.xmax(); x++) {
			for (int z = delBox.ymin(); z <= delBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygon.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] -= 1;
				}
			}
		}

		//ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

		return true;
	}
	else {
		// 두 개의 새로운 도형을 위한 벡터입니다.
		std::vector<Polygon_2> forwardPolygon;

		// polygonA와 교차 영역의 차이를 계산하여 polygonA의 겹치지 않는 영역을 구합니다.
		for (const auto& intersection : intersections) {
			// 교차 폴리곤의 유효성 검사
			// is_unbounded: 교차 폴리곤이 무한한지(즉, 경계가 없는지)를 확인
			// outer_boundary: Polygon_with_holes_2 객체에서 외곽 경계를 나타내는 Polygon_2 객체를 반환
			if (intersection.is_unbounded() || intersection.outer_boundary().is_empty()) {
				continue;
			}

			std::list<Polygon_with_holes_2> result;
			CGAL::difference(polygonNew, intersection, std::back_inserter(result));
			for (const auto& res : result) {
				forwardPolygon.push_back(res.outer_boundary());
			}
		}

		// 장애물 체크
		for (const auto& poly : forwardPolygon) {
			CGAL::Bbox_2 bbox = poly.bbox();
			for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
				for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
					Point_2 p(x, z);
					//if (poly.has_on_bounded_side(p)) {
					// => 조건 추가, 기존 팔각형과 다르게 팔각형의 테구리 각도가 45도가 아니기에 기존 테두리를 포함할 수 있다.
					if(poly.has_on_bounded_side(p) && polygon.has_on_boundary(p) == false) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							return false;
						}
					}
				}
			}
		}


		//AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
		//std::vector<Polygon_2> backwardPolygon;
		//
		//for (const auto& intersection : intersections) {
		//	std::list<Polygon_with_holes_2> result;
		//	CGAL::difference(polygon, intersection, std::back_inserter(result));
		//	for (const auto& res : result) {
		//		backwardPolygon.push_back(res.outer_boundary());
		//	}
		//}
		//
		//for (const auto& poly : forwardPolygon) {
		//	CGAL::Bbox_2 bbox = poly.bbox();
		//	for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
		//		for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
		//			Point_2 p(x, z);
		//			if (poly.has_on_bounded_side(p)) {
		//				m_UnitColliderCountMap[z][x] += 1;
		//			}
		//		}
		//	}
		//}
		//
		//for (const auto& poly : backwardPolygon) {
		//	CGAL::Bbox_2 bbox = poly.bbox();
		//	for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
		//		for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
		//			Point_2 p(x, z);
		//			if (poly.has_on_bounded_side(p)) {
		//				//if (m_UnitColliderCountMap[z][x] <= 0) {
		//				//	DebugBreak();
		//				//}
		//				m_UnitColliderCountMap[z][x] -= 1;
		//			}
		//		}
		//	}
		//}

		CGAL::Bbox_2 drawBox = polygonNew.bbox();
		for (int x = drawBox.xmin(); x <= drawBox.xmax(); x++) {
			for (int z = drawBox.ymin(); z <= drawBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygonNew.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] += 1;
				}
			}
		}
		CGAL::Bbox_2 delBox = polygon.bbox();
		for (int x = delBox.xmin(); x <= delBox.xmax(); x++) {
			for (int z = delBox.ymin(); z <= delBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygon.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] -= 1;
				}
			}
		}

		//ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

		return true;// 두 개의 새로운 도형을 위한 벡터입니다.
	}
}
*/