#pragma once
namespace SDK
{
	class CUserCmd;
	class CBaseEntity;
}

void z180(SDK::CUserCmd* cmd);

class CAntiAim
{
public:
	void do_antiaim(SDK::CUserCmd* cmd);
	void fix_movement(SDK::CUserCmd* cmd);
private:
	void freestanding(SDK::CUserCmd * cmd);
	void legit(SDK::CUserCmd* cmd);
	void sideways(SDK::CUserCmd* cmd);
	void lowerbody(SDK::CUserCmd* cmd);
	void backjitter(SDK::CUserCmd* cmd);
};

extern CAntiAim* antiaim;