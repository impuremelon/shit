#include "../includes.h"
#include "../UTILS/mopvar.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include "../SDK/CBaseEntity.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CBaseAnimState.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/CTrace.h"
#include "../SDK/CBaseWeapon.h"
#include "../FEATURES/Aimbot.h"
#include "../FEATURES/AntiAim.h"
#include "../FEATURES/AutoWall.h"

float randnum(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

float get_curtime(SDK::CUserCmd* ucmd) {
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return 0;

	int g_tick = 0;
	SDK::CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * mopvar::Globals->interval_per_tick;
	return curtime;
}

float anim_velocity(SDK::CBaseEntity* LocalPlayer)
{
	if (LocalPlayer->GetAnimState() == nullptr)
		return false;

	int vel = LocalPlayer->GetAnimState()->speed_2d;
	return vel;
}

bool first_supress(const float yaw_to_break, SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return false;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto animstate = local_player->GetAnimState();
	if (!animstate)
		return false;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return false;

	if (SETTINGS::settings.delta_val < 120)
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22 - TICKS_TO_TIME(1);

	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1;
		return true;
	}

	return false;
}

bool second_supress(const float yaw_to_break, SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return false;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto animstate = local_player->GetAnimState();
	if (!animstate)
		return false;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return false;

	if (SETTINGS::settings.delta_val < 120)
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22 + TICKS_TO_TIME(1);


	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1;
		return true;
	}

	return false;
}

float GetBestHeadAngle(SDK::CUserCmd *cmd, float yaw)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return false;
	SDK::CTraceWorldOnly filter;
	SDK::trace_t tr;
	SDK::Ray_t ray;
	SDK::Ray_t ray1, ray2, ray3;
	SDK::IEngine;
	float Back, Right, Left;
	Vector src3D, dst3D, forward, right, up;

	cmd->viewangles.x = 0;

	filter.pSkip1 = local_player;
	src3D = local_player->GetEyeAngles();
	dst3D = src3D + (forward * 384);

	ray1.Init(src3D, dst3D);
	Back = (tr.end - tr.start).Length();
	
	ray2.Init(src3D + right * 35, dst3D + right * 35);
	Right = (tr.end - tr.start).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);
	Left = (tr.end - tr.start).Length();

	if (Left > Right)
	{
		return (yaw - 90);
	}

	else if (Right > Left)
	{
		return (yaw + 90);
	}

	else if (Back > Right || Back > Left)
	{
		return (yaw - 180);
	}
}

bool next_lby_update(const float yaw_to_break, SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return false;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto animstate = local_player->GetAnimState();
	if (!animstate)
		return false;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22f;

	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1f;
		return true;
	}

	return false;
}

void CAntiAim::do_antiaim(SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	if (local_player->GetHealth() <= 0)
		return;

	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(mopvar::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));

	if ( local_player->GetMoveType() == MOVETYPE_LADDER || local_player->GetMoveType() == MOVETYPE_NOCLIP )
	return;

	if (!weapon)
		return;

	if (cmd->buttons & IN_USE)
		return;

	if (cmd->buttons & IN_ATTACK && aimbot->can_shoot(cmd))
		return;

	if (weapon->get_full_info()->WeaponType == 9)
		return;

	if (!SETTINGS::settings.aa_bool)
		return;

	float temp = cmd->viewangles.y;

	//float delta = abs(MATH::NormalizeYaw(GLOBAL::real_angles.y - local_player->GetLowerBodyYaw()));
	//std::cout << std::to_string(delta) << std::endl;



	switch (SETTINGS::settings.aa_type % 6)
	{
	case 0:
		break;
	case 1:

		if (local_player->GetAnimState()->m_bInHitGroundAnimation)
		{
			if (local_player->GetAnimState()->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
			{

				cmd->viewangles.x = 0.0;
				freestanding(cmd);
				return;
			}
		}
		freestanding(cmd);
		cmd->viewangles.x = 89.000000;
		if (!(local_player->GetFlags() & FL_ONGROUND))
			z180(cmd);
		break;
	case 2:
		if (local_player->GetAnimState()->m_bInHitGroundAnimation)
		{
			if (local_player->GetAnimState()->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
			{
				cmd->viewangles.x = 0.0;
				sideways(cmd);

				return;
			}
		}

		cmd->viewangles.x = 89.000000;
		sideways(cmd);
		if (!(local_player->GetFlags() & FL_ONGROUND))
			z180(cmd);
		break;
	case 3:
		if (local_player->GetAnimState()->m_bInHitGroundAnimation)
		{
			if (local_player->GetAnimState()->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
			{
				cmd->viewangles.x = 0.0;
				backjitter(cmd);
				return;
			}
		}
		cmd->viewangles.x = 89.000000;
		backjitter(cmd);
		if (!(local_player->GetFlags() & FL_ONGROUND))
			z180(cmd);
		break;
	case 4:
		if (local_player->GetAnimState()->m_bInHitGroundAnimation)
		{
			if (local_player->GetAnimState()->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
			{
				cmd->viewangles.x = 0.0;
				lowerbody(cmd);
				return;
			}
		}
		cmd->viewangles.x = 89.000000;
		lowerbody(cmd);
		if (!(local_player->GetFlags() & FL_ONGROUND))
			z180(cmd);
		break;
	case 5:
		legit(cmd);
		break;
	}
}

void CAntiAim::fix_movement(SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	Vector real_viewangles;
	mopvar::Engine->GetViewAngles(real_viewangles);

	Vector vecMove(cmd->forwardmove, cmd->sidemove, cmd->upmove);
	float speed = sqrt(vecMove.x * vecMove.x + vecMove.y * vecMove.y);

	Vector angMove;
	MATH::VectorAngles(vecMove, angMove);

	float yaw = DEG2RAD(cmd->viewangles.y - real_viewangles.y + angMove.y);

	cmd->forwardmove = cos(yaw) * speed;
	cmd->sidemove = sin(yaw) * speed;

	cmd->viewangles = MATH::NormalizeAngle(cmd->viewangles);
}

#define	MASK_ALL				(0xFFFFFFFF)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 			/**< everything that is normally solid */
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 	/**< everything that blocks player movement */
#define	MASK_NPCSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) /**< blocks npc movement */
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME) 							/**< water physics in these contents */
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE) 							/**< everything that blocks line of sight for AI, lighting, etc */
#define MASK_OPAQUE_AND_NPCS	(MASK_OPAQUE|CONTENTS_MONSTER)										/**< everything that blocks line of sight for AI, lighting, etc, but with monsters added. */
#define	MASK_VISIBLE			(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE) 								/**< everything that blocks line of sight for players */
#define MASK_VISIBLE_AND_NPCS	(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE) 							/**< everything that blocks line of sight for players, but with monsters added. */
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX) 	/**< bullets see these as solid */
#define MASK_SHOT_HULL			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE) 	/**< non-raycasted weapons see this as solid (includes grates) */
#define MASK_SHOT_PORTAL		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW) 							/**< hits solids (not grates) and passes through everything else */
#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS) // non-raycasted weapons see this as solid (includes grates)
#define MASK_SOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE) 					/**< everything normally solid, except monsters (world+brush only) */
#define MASK_PLAYERSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for player movement, except monsters (world+brush only) */
#define MASK_NPCSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for npc movement, except monsters (world+brush only) */
#define MASK_NPCWORLDSTATIC		(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 					/**< just the world, used for route rebuilding */
#define MASK_SPLITAREAPORTAL	(CONTENTS_WATER|CONTENTS_SLIME) 									/**< These are things that can split areaportals */

float fov_player(Vector ViewOffSet, Vector View, SDK::CBaseEntity* entity, int hitbox)
{
	// Anything past 180 degrees is just going to wrap around
	CONST FLOAT MaxDegrees = 180.0f;

	// Get local angles
	Vector Angles = View;

	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	//Vector Origin(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	MATH::AngleVectors(Angles, &Forward);

	Vector AimPos = aimbot->get_hitbox_pos(entity, hitbox); //pvs fix disabled

	MATH::VectorSubtract(AimPos, Origin, Delta);
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	MATH::NormalizeNum(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (MaxDegrees / M_PI));
}


int closest_to_crosshair()
{
	int index = -1;
	float lowest_fov = INT_MAX;

	SDK::CBaseEntity* local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return -1;

	Vector local_position = local_player->GetVecOrigin() + local_player->GetViewOffset();

	Vector angles;
	mopvar::Engine->GetViewAngles(angles);

	for (int i = 1; i <= mopvar::Globals->maxclients; i++)
	{
		SDK::CBaseEntity *entity = mopvar::ClientEntityList->GetClientEntity(i);

		if (!entity || !entity->IsAlive() || entity->GetTeam() == local_player->GetTeam() || entity->GetIsDormant() || entity == local_player)
			continue;

		float fov = fov_player(local_position, angles, entity, 0);

		if (fov < lowest_fov)
		{
			lowest_fov = fov;
			index = i;
		}
	}

	return index;
}


void CAntiAim::freestanding(SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	static float last_real;
	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;

	auto leyepos = local_player->GetVecOrigin() + local_player->GetViewOffset();
	auto headpos = aimbot->get_hitbox_pos(local_player, 0);
	auto origin = local_player->GetAbsOrigin();

	auto checkWallThickness = [&](SDK::CBaseEntity* pPlayer, Vector newhead) -> float
	{
		Vector endpos1, endpos2;
		Vector eyepos = pPlayer->GetVecOrigin() + pPlayer->GetViewOffset();

		SDK::Ray_t ray;
		ray.Init(newhead, eyepos);

		SDK::CTraceFilterSkipTwoEntities filter(pPlayer, local_player);

		SDK::trace_t trace1, trace2;
		mopvar::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.end;
		else
			return 0.f;

		ray.Init(eyepos, newhead);
		mopvar::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);

		if (trace2.DidHit())
			endpos2 = trace2.end;

		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;
	};

	int index = closest_to_crosshair();

	static SDK::CBaseEntity * entity;

	if (index != -1)
		entity = mopvar::ClientEntityList->GetClientEntity(index);

	float step = (2 * M_PI) / 18.f; // One PI = half a circle ( for stacker cause low iq :sunglasses: ), 28

	float radius = fabs(Vector(headpos - origin).Length2D());

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
		{
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);

			float totalthickness = 0.f;

			no_active = false;

			totalthickness += checkWallThickness(entity, newhead);

			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				besthead = newhead;
			}
		}
	}

	if (GLOBAL::should_send_packet)
		cmd->viewangles.y += randnum(-180, 180);
	else
	{
		if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd))
		{
			cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
		}
		else
		{
			if (no_active)
				cmd->viewangles.y += 180.f;
			else
				cmd->viewangles.y = RAD2DEG(bestrotation);

			last_real = cmd->viewangles.y;
		}
	}
}

void CAntiAim::legit(SDK::CUserCmd* cmd)
{
	if (GLOBAL::should_send_packet)
		cmd->viewangles.y += 0;
	else
		cmd->viewangles.y += 90;
}

void CAntiAim::sideways(SDK::CUserCmd* cmd)
{
	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += 90;
		else
			cmd->viewangles.y -= 90;
	}
	else
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y -= 90;
		else
			cmd->viewangles.y += 90;
	}
}

void CAntiAim::backjitter(SDK::CUserCmd* cmd)
{
	if (GLOBAL::should_send_packet)
		cmd->viewangles.y += randnum(-180, 180);
	else
		cmd->viewangles.y += 180 + ((rand() % 15) - (15 * 0.5f));
}

void z180(SDK::CUserCmd *pCmd)
{
	static int Ticks;
	pCmd->viewangles.y += Ticks;

	Ticks += 4;

	if (Ticks > 95)
		Ticks = 15;
}


void CAntiAim::lowerbody(SDK::CUserCmd* cmd)
{
	static float last_real;

	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet)

			cmd->viewangles.y += randnum(-180 + ((rand() % 15) - (15 * 0.5f)), 180 + ((rand() % 15) - (15 * 0.5f)));
		else
		{
			//if (first_supress(cmd->viewangles.y + 110, cmd))
				//cmd->viewangles.y = last_real + 110;
			if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd)) //else if
				cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
			//else if (second_supress(cmd->viewangles.y - 110, cmd))
			//	cmd->viewangles.y = last_real - 110;
			else
			{
				cmd->viewangles.y -= 90;
				last_real = cmd->viewangles.y;
			}
		}
	}
	else
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += randnum(-180, 180);
		else
		{
			if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
				cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
			else
			{
				cmd->viewangles.y += 90;
				last_real = cmd->viewangles.y;
			}
		}
	}
}

CAntiAim* antiaim = new CAntiAim();