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
	float AttackRate;	// 3: 0.3�ʿ� �� �� ����
						// 2: 0.5�ʿ� �� �� ����
						// 1: 1�ʿ� �� �� ����
						// 0.5: 2�ʿ� �� �� ����
						// 0.3: 3.3�� �� �� ����

	UnitData(float radius, int hp, float speed, int attackDamage, float attackDistance, float attackRate)
		: Radius(radius), HP(hp), Speed(speed), AttackDamage(attackDamage), AttackDistance(attackDistance), AttackRate(attackRate)
	{}
};

const UnitData UnitDatas[] = {
	// ������,	ü��,	�ӵ�,	���ݷ�,	���ݰŸ�,	���ݺ�
	{1.5f,		100,	20,		5,		30,			1},			// marine
	{1.5f,		100,	20,		6,		15,			1},			// firebat
	{0.f,		1000,	23,		10,		50,			0.5f},		// tank
	{0.f,		800,	22,		8,		40,			1},			// robocob
	{4.0f,		80,		20,		4,		10,			2},			// zergling
	{4.5f,		120,	21,		7,		45,			1},			// hydra
	{7.5f,		1200,	23,		10,		10,			0.5f},		// golem
	{10.0f,		700,	22,		8,		50,			1}			// tarantula
};