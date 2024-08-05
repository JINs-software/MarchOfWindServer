#pragma once

/*
* HP
* SPEED
* ATTACK_DAMAGE
* ATTACK_DIST
* ATTACK_RATE
*/

struct UnitData {
	float Radius;
	int HP;
	float Speed;
	
	int AttackDamage;
	float AttackDistance;
	float AttackRate;	// 3: 0.3초에 한 번 공격
						// 2: 0.5초에 한 번 공격
						// 1: 1초에 한 번 공격
						// 0.5: 2초에 한 번 공격
						// 0.3: 3.3에 한 번 공격

	float AttackDelay;

	UnitData(float radius, int hp, float speed, int attackDamage, float attackDistance, float attackRate, float attackDelay)
		: Radius(radius), HP(hp), Speed(speed), AttackDamage(attackDamage), AttackDistance(attackDistance), AttackRate(attackRate), AttackDelay(attackDelay)
	{}
};

const UnitData UnitDatas[] = {
	// 반지름,	체력,	속도,	공격력,	공격거리,	공격빈도, 공격 딜레이
	{1.5f,		100,	20,		5,		30,			1,			0.1f},			// marine
	{1.5f,		100,	20,		6,		10,			1,			0.1f},			// firebat
	{0.f,		1000,	23,		10,		50,			0.5f,		0},		// tank
	{0.f,		800,	22,		8,		40,			1,			0},			// robocob
	{4.0f,		80,		20,		4,		7,			2,			0.1f},			// zergling
	{4.5f,		120,	21,		7,		35,			1,			0.5f},			// hydra
	{7.5f,		1200,	23,		10,		10,			0.5f,		0},		// golem
	{10.0f,		700,	22,		8,		50,			1,			0}			// tarantula
};