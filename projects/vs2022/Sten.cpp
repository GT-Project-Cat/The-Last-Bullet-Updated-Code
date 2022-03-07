#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "soundent.h"//���������� ����� �����
#include "gamerules.h"

enum sten_e//��������� ��������
{
	STEN_LONGIDLE = 0,//������� �����������
	STEN_IDLE1,//�����������
	STEN_LAUNCH,//������� �����������
	STEN_RELOAD,//�����������
	STEN_DEPLOY,//������ � ����
	STEN_FIRE1,//�������
	STEN_FIRE2,//�������
	STEN_FIRE3,//�������
};

LINK_ENTITY_TO_CLASS(weapon_sten, CStenGun);

void CStenGun::Spawn()
{
	pev->classname = MAKE_STRING("weapon_sten"); // ����� ��� ������ ������� ���� ����� ����� ������� � ���
	Precache();
	SET_MODEL(ENT(pev), "models/w_sten.mdl"); // ��� ������ ���� ������ ������ ������
	m_iId = WEAPON_STEN;

	m_iDefaultAmmo = 50;

	FallInit();
}

void CStenGun::Precache(void)
{
	// ��� �� ��������� ����� � ������ ��� ������ ������
	PRECACHE_MODEL("models/v_sten.mdl");
	PRECACHE_MODEL("models/w_sten.mdl");
	PRECACHE_MODEL("models/p_sten.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/sten/stenshoot.wav");
	PRECACHE_SOUND("weapons/sten/stenshoot2.wav");


	// this is to hook your client-side event
	m_usStenFire = PRECACHE_EVENT(1, "events/sten.sc");
}

int CStenGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "mp5";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 2; // ���� � ���� ( ������� �����, ��� ���� ����� �� �������� �������� 2 �� � ���� ����� ����� 3 ����, �.� � ���� ���������� ����������� �� ����)
	p->iPosition = 3; // ������� � ����� ( �� �� ����� ��� � � ����� - ���� ��� ���� ������ 4 ������� , �� ������ 3 � ����)
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_STEN;
	p->iWeight = MP5_WEIGHT; //
	p->pszAmmo2 = NULL; //
	p->iMaxAmmo2 = -1;

	return 1;
}


int CStenGun::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}


BOOL CStenGun::Deploy()//��������� ������
{
	return DefaultDeploy("models/v_sten.mdl"/* ������ � ����� ������*/, "models/p_sten.mdl"/*������ � ����� ����������*/, STEN_DEPLOY/*��������      ��������*/, "mp5"/*������� �������� ������*/);
}



void CStenGun::WeaponIdle(void) //�����������
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	int iAnim; //��������� ����� ��������� ������� ����� ������ ����� ��������� ��������
	switch (RANDOM_LONG(0, 1))//��������� ����� �� 0 �� 1 ��� ������ ��������
	{
	case 0:  //���� 0 �� ������� �����������
		iAnim = STEN_LONGIDLE;
		break;

	default:
	case 1: //���� �� ����, � ���-�� ������ �� ������ �����������
		iAnim = STEN_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim); //��������� ��� �������� ������� ���������� � ����������

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15); // ����� ������� ����� ��������� ������� �����������
}

void CStenGun::PrimaryAttack()//��������� �����
{
	//�� �������� ��� �����
	if (m_pPlayer->pev->waterlevel == 3)//���� ����� ��� �����
	{
		PlayEmptySound();//������ ���� ������������� ��������
		m_flNextPrimaryAttack = 0.15;//����� ��������� �������
		return;//������� �� ���� �������
	}

	if (m_iClip <= 0)//���� �������� 0 ��� ������ 0 ��
	{
		PlayEmptySound();//������ ���� ������������� ��������
		m_flNextPrimaryAttack = 0.15;//����� ��������� �������
		return;//������� �� ���� �������
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; //������������� ��������� ������
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH; //������������� ������� ������� ������

	m_iClip--; //��������� ���������� �������� �� 1 


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// ������������ �������� �������� �������
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	//����������� ������� vecSrc ��������� �����
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);//����������� ������� vecAiming ����� ��������� ����
	Vector vecDir;
	//������ ��������
	vecDir = m_pPlayer->FireBulletsPlayer(1/*���������� ���� ������������*/, vecSrc/*������ �������� ����(��������� �����) */, vecAiming/*���� �������� ����*/, VECTOR_CONE_1DEGREES/*��������� ����� ��������*/, 8192/*���������*/, BULLET_PLAYER_MP5/*��� ���� (�����������)*/, 2/* ���������� ������������ ����*/, 0, m_pPlayer->pev, m_pPlayer->random_seed);


	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usStenFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0); //�������� ����� �� ������

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV ������ ������� ��� ��� ��������
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;//����� ��������� �������

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

}

void CStenGun::Reload(void)//�����������
{
	DefaultReload(100/*������������ ������ ������*/, STEN_RELOAD/*�������� �����������*/, 4 /*����� �����������*/);
}





