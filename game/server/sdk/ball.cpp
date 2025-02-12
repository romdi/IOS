//============================================================================//
//
// Ball.cpp	by Mark	Gornall, Jan 2007.
//
//=============================================================================//

#include "cbase.h"
#include "baseanimating.h"
#include "player_pickup.h"
#include "props_shared.h"
#include "props.h"
#include "sdk_player.h"
#include "sdk_gamerules.h"
#include "in_buttons.h"
#include "nav_mesh.h"
#include "game.h"
#include "ball.h"
#include "team.h"
#include "triggers.h"
#include "ios_mapentities.h"
#include "sdk_shareddefs.h"
#include "ios_replaymanager.h"
#include "movevars_shared.h"
#include "ios_replaymanager.h"

extern ConVar sv_player_mass;

ConVar

	// Physics

	sv_ball_mass											("sv_ball_mass",											"5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_damping											("sv_ball_damping",											"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_rotdamping										("sv_ball_rotdamping",										"0.66",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_rotinertialimit									("sv_ball_rotinertialimit",									"2",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_dragcoeff										("sv_ball_dragcoeff",										"1",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_inertia											("sv_ball_inertia",											"1",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_drag_enabled									("sv_ball_drag_enabled",									"1",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_goalpostfriction								("sv_ball_goalpostfriction",								"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_goalnetfriction_min								("sv_ball_goalnetfriction_min",								"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_goalnetfriction_max								("sv_ball_goalnetfriction_max",								"0.9",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	
	sv_ball_spin											("sv_ball_spin",											"6000",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_spin_mincoeff									("sv_ball_spin_mincoeff",									"0.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_defaultspin										("sv_ball_defaultspin",										"150",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_magnus_coeff									("sv_ball_magnus_coeff",									"100",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_sandground_enabled								("sv_ball_sandground_enabled",								"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_sandground_maxshotstrength						("sv_ball_sandground_maxshotstrength",						"400",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_sandground_rollfriction							("sv_ball_sandground_rollfriction",							"20",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_sandground_bouncefriction						("sv_ball_sandground_bouncefriction",						"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),



	// Distances

	sv_ball_maxplayerfinddist								("sv_ball_maxplayerfinddist",								"200",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	
	sv_ball_standing_reach									("sv_ball_standing_reach",									"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	
	sv_ball_slidesidereach_ball								("sv_ball_slidesidereach_ball",								"30",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_slideforwardreach_ball							("sv_ball_slideforwardreach_ball",							"50",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_slidebackwardreach_ball							("sv_ball_slidebackwardreach_ball",							"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_slidezstart										("sv_ball_slidezstart",										"-20",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY), 
	sv_ball_slidezend										("sv_ball_slidezend",										"25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY), 

	sv_ball_standing_tackle_sidereach						("sv_ball_standing_tackle_sidereach",						"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_standing_tackle_forwardreach					("sv_ball_standing_tackle_forwardreach",					"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_standing_tackle_backwardreach					("sv_ball_standing_tackle_backwardreach",					"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_standing_tackle_zstart							("sv_ball_standing_tackle_zstart",							"-20",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY), 
	sv_ball_standing_tackle_zend							("sv_ball_standing_tackle_zend",							"25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	
	sv_ball_keeper_standing_reach							("sv_ball_keeper_standing_reach",							"50",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	
	sv_ball_keeper_forwarddive_shortsidereach				("sv_ball_keeper_forwarddive_shortsidereach",				"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_forwarddive_longsidereach				("sv_ball_keeper_forwarddive_longsidereach",				"70",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_forwarddive_longsidereach_opposite		("sv_ball_keeper_forwarddive_longsidereach_opposite",		"50",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_forwarddive_zstart						("sv_ball_keeper_forwarddive_zstart",						"-20",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_forwarddive_zend							("sv_ball_keeper_forwarddive_zend",							"50",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keeper_backwarddive_shortsidereach				("sv_ball_keeper_backwarddive_shortsidereach",				"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_backwarddive_longsidereach				("sv_ball_keeper_backwarddive_longsidereach",				"70",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_backwarddive_longsidereach_opposite		("sv_ball_keeper_backwarddive_longsidereach_opposite",		"50",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_backwarddive_zstart						("sv_ball_keeper_backwarddive_zstart",						"25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_backwarddive_zend						("sv_ball_keeper_backwarddive_zend",						"100",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	
	sv_ball_keeper_sidedive_shortsidereach					("sv_ball_keeper_sidedive_shortsidereach",					"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_sidedive_longsidereach					("sv_ball_keeper_sidedive_longsidereach",					"60",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_sidedive_longsidereach_opposite			("sv_ball_keeper_sidedive_longsidereach_opposite",			"50",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_sidedive_zstart							("sv_ball_keeper_sidedive_zstart",							"-30",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_sidedive_zend							("sv_ball_keeper_sidedive_zend",							"90",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_sidedive_catchcenteroffset_side			("sv_ball_keeper_sidedive_catchcenteroffset_side",			"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_sidedive_catchcenteroffset_z				("sv_ball_keeper_sidedive_catchcenteroffset_z",				"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_bodypos_feet_start								("sv_ball_bodypos_feet_start",								"-15",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bodypos_hip_start								("sv_ball_bodypos_hip_start",								"20",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bodypos_head_start								("sv_ball_bodypos_head_start",								"55",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bodypos_head_end								("sv_ball_bodypos_head_end",								"90",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bodypos_keeperarms_end							("sv_ball_bodypos_keeperarms_end",							"105",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_bodypos_keeperhands								("sv_ball_bodypos_keeperhands",								"40",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_bodypos_collision_start							("sv_ball_bodypos_collision_start",							"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bodypos_collision_end							("sv_ball_bodypos_collision_end",							"80",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_bodypos_deflection_start						("sv_ball_bodypos_deflection_start",						"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bodypos_deflection_end							("sv_ball_bodypos_deflection_end",							"90",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),



	// Shots

	sv_ball_bestshotangle									("sv_ball_bestshotangle",									"-25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_pitchdown_exponent								("sv_ball_pitchdown_exponent",								"2",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_pitchdown_fixedcoeff							("sv_ball_pitchdown_fixedcoeff",							"0.3",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_pitchup_exponent								("sv_ball_pitchup_exponent",								"1",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_pitchup_fixedcoeff								("sv_ball_pitchup_fixedcoeff",								"0.75",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_dynamicshotdelay_mindelay						("sv_ball_dynamicshotdelay_mindelay",						"0.2",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_dynamicshotdelay_maxdelay						("sv_ball_dynamicshotdelay_maxdelay",						"1.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_dynamicshotdelay_minshotstrength				("sv_ball_dynamicshotdelay_minshotstrength",				"300",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_dynamicshotdelay_maxshotstrength				("sv_ball_dynamicshotdelay_maxshotstrength",				"1500",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_shotdelay_global_coeff							("sv_ball_shotdelay_global_coeff",							"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_groundshot_minangle								("sv_ball_groundshot_minangle",								"-7",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_normalshot_strength								("sv_ball_normalshot_strength",								"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_normalshot_minpostdelay							("sv_ball_normalshot_minpostdelay",							"0.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_chargedshot_minstrength							("sv_ball_chargedshot_minstrength",							"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_chargedshot_maxstrength							("sv_ball_chargedshot_maxstrength",							"1500",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_chargedshot_minpostdelay						("sv_ball_chargedshot_minpostdelay",						"0.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_normalheader_strength							("sv_ball_normalheader_strength",							"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_chargedheader_minstrength						("sv_ball_chargedheader_minstrength",						"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_chargedheader_maxstrength						("sv_ball_chargedheader_maxstrength",						"1000",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_slide_strength									("sv_ball_slide_strength",									"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_slide_pitchangle								("sv_ball_slide_pitchangle",								"-15",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_slide_minpostdelay								("sv_ball_slide_minpostdelay",								"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_standing_tackle_strength						("sv_ball_standing_tackle_strength",						"700",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_standing_tackle_pitchangle						("sv_ball_standing_tackle_pitchangle",						"-10",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_standing_tackle_minpostdelay					("sv_ball_standing_tackle_minpostdelay",					"0.1",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keeper_forwarddive_catchcoeff					("sv_ball_keeper_forwarddive_catchcoeff",					"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keepershot_minangle								("sv_ball_keepershot_minangle",								"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keeperthrow_minstrength							("sv_ball_keeperthrow_minstrength",							"100",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeperthrow_maxstrength							("sv_ball_keeperthrow_maxstrength",							"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeperthrow_minpostdelay						("sv_ball_keeperthrow_minpostdelay",						"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeperthrow_minangle							("sv_ball_keeperthrow_minangle",							"90",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keeperpunch_minstrength							("sv_ball_keeperpunch_minstrength",							"900",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeperpunch_sidespeedcoeff						("sv_ball_keeperpunch_sidespeedcoeff",						"0.1",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeperpunch_minpostdelay						("sv_ball_keeperpunch_minpostdelay",						"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keeper_backwarddive_catchcoeff					("sv_ball_keeper_backwarddive_catchcoeff",					"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keeper_backwarddive_punchupangle				("sv_ball_keeper_backwarddive_punchupangle",				"45",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keepercatchdelay_sidedive_global_coeff			("sv_ball_keepercatchdelay_sidedive_global_coeff",			"0.75",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keepercatchdelay_forwarddive_global_coeff		("sv_ball_keepercatchdelay_forwarddive_global_coeff",		"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keepercatchdelay_backwarddive_global_coeff		("sv_ball_keepercatchdelay_backwarddive_global_coeff",		"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keepercatchdelay_standing_global_coeff			("sv_ball_keepercatchdelay_standing_global_coeff",			"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_keepercatchdelay_poscoeffmin					("sv_ball_keepercatchdelay_poscoeffmin",					"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keepercatch_minpostdelay						("sv_ball_keepercatch_minpostdelay",						"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_volleyshot_minangle								("sv_ball_volleyshot_minangle",								"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_volleyshot_minpostdelay							("sv_ball_volleyshot_minpostdelay",							"0.25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_header_spincoeff								("sv_ball_header_spincoeff",								"1",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_header_minangle									("sv_ball_header_minangle",									"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_header_maxangle									("sv_ball_header_maxangle",									"-35",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_header_minpostdelay								("sv_ball_header_minpostdelay",								"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_chestdrop_strength								("sv_ball_chestdrop_strength",								"100",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_chestdrop_angle									("sv_ball_chestdrop_angle",									"90",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_chestdrop_minpostdelay							("sv_ball_chestdrop_minpostdelay",							"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_rainbowflick_angle								("sv_ball_rainbowflick_angle",								"-90",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_rainbowflick_minstrength						("sv_ball_rainbowflick_minstrength",						"200",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_rainbowflick_maxstrength						("sv_ball_rainbowflick_maxstrength",						"500",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_rainbowflick_spincoeff							("sv_ball_rainbowflick_spincoeff",							"0.33",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_rainbowflick_minpostdelay						("sv_ball_rainbowflick_minpostdelay",						"0.75",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_divingheader_minstrength						("sv_ball_divingheader_minstrength",						"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_divingheader_maxstrength						("sv_ball_divingheader_maxstrength",						"1100",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_divingheader_minangle							("sv_ball_divingheader_minangle",							"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_divingheader_maxangle							("sv_ball_divingheader_maxangle",							"-25",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_bicycleshot_minstrength							("sv_ball_bicycleshot_minstrength",							"800",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bicycleshot_maxstrength							("sv_ball_bicycleshot_maxstrength",							"1100",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_bicycleshot_minpostdelay						("sv_ball_bicycleshot_minpostdelay",						"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_lift_angle										("sv_ball_lift_angle",										"-90",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_lift_minstrength								("sv_ball_lift_minstrength",								"100",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_lift_maxstrength								("sv_ball_lift_maxstrength",								"400",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_lift_minpostdelay								("sv_ball_lift_minpostdelay",								"0.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_roll_strength									("sv_ball_roll_strength",									"250",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_roll_minpostdelay								("sv_ball_roll_minpostdelay",								"0.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_heelshot_minangle								("sv_ball_heelshot_minangle",								"0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_heelshot_strengthcoeff							("sv_ball_heelshot_strengthcoeff",							"0.75",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_heelshot_minpostdelay							("sv_ball_heelshot_minpostdelay",							"0.75",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_heelshot_minstrength							("sv_ball_heelshot_minstrength",							"300",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),



	// Collisions

	sv_ball_deflectionradius								("sv_ball_deflectionradius",								"30",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_collisionradius									("sv_ball_collisionradius",									"15",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_collision_passive_mass							("sv_ball_collision_passive_mass",							"5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_collision_passive_coeff							("sv_ball_collision_passive_coeff",							"0.6",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_collision_deflection_mass						("sv_ball_collision_deflection_mass",						"5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_collision_deflection_coeff						("sv_ball_collision_deflection_coeff",						"0.6",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_collision_dribbling_mass						("sv_ball_collision_dribbling_mass",						"75",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_collision_dribbling_coeff						("sv_ball_collision_dribbling_coeff",						"1.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_collision_selfhit_mass							("sv_ball_collision_selfhit_mass",							"225",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_collision_selfhit_coeff							("sv_ball_collision_selfhit_coeff",							"1.0",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_keeperdeflectioncoeff							("sv_ball_keeperdeflectioncoeff",							"0.5",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),



	// Misc

	sv_ball_animation_minstrength_strongshot				("sv_ball_animation_minstrength_strongshot",				"600",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_animation_minstrength_weakshot					("sv_ball_animation_minstrength_weakshot",					"400",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_animation_minstrength_dribbleshot				("sv_ball_animation_minstrength_dribbleshot",				"200",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),

	sv_ball_highlightsdelay_intermissions					("sv_ball_highlightsdelay_intermissions",					"3",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY),
	sv_ball_highlightsdelay_cooldown						("sv_ball_highlightsdelay_cooldown",						"3",		FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);

	

//==========================================================
//	
//	
//==========================================================
BEGIN_DATADESC(	CBall )
	DEFINE_THINKFUNC( Think	),
	DEFINE_USEFUNC(	Use	),
	//DEFINE_ENTITYFUNC( BallTouch ),
END_DATADESC()

//==========================================================
//	
//	
//==========================================================
IMPLEMENT_SERVERCLASS_ST( CBall, DT_Ball )
	SendPropInt( SENDINFO( m_iPhysicsMode ), 2,	SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_fMass ),	0, SPROP_NOSCALE ),
	SendPropEHandle(SENDINFO(m_pHoldingPlayer)),
	SendPropInt(SENDINFO(m_eBallState)),
	SendPropString(SENDINFO(m_szSkinName)),

	//ios1.1
    //SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
END_SEND_TABLE()

const objectparams_t g_IOSPhysDefaultObjectParams =
{
	NULL,
	1.0, //mass
	1.0, // inertia
	0.1f, // damping
	0.1f, // rotdamping
	0.05f, // rotIntertiaLimit
	"DEFAULT",
	NULL,// game data
	1.f, // volume (leave 0 if you don't have one or call physcollision->CollideVolume() to compute it)
	1.0f, // drag coefficient
	true,// enable collisions?
};

CBall::CBall()
{
	m_eNextState = BALL_STATE_NONE;
	m_pPl = NULL;
	m_pHoldingPlayer = NULL;
	m_bHasQueuedState = false;
	m_bHitThePost = false;
	m_nInPenBoxOfTeam = TEAM_NONE;
	memset(m_szSkinName.GetForModify(), 0, sizeof(m_szSkinName));
}

CBall::~CBall()
{
}

bool CBall::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	return false;
}

//==========================================================
//	
//	
//==========================================================
void CBall::Spawn (void)
{
	//RomD: Don't fade the ball
	SetFadeDistance(-1, 0);
	DisableAutoFade();

	PrecacheModel(BALL_MODEL);
	SetModel(BALL_MODEL);

	CreateVPhysics();

	SetThink(&CBall::Think);
	SetNextThink(gpGlobals->curtime);

	m_nBody = 0;

	int ballSkinIndex = g_IOSRand.RandomInt(0, CBallInfo::m_BallInfo.Count() - 1);
	Q_strncpy(m_szSkinName.GetForModify(), CBallInfo::m_BallInfo[ballSkinIndex]->m_szFolderName, sizeof(m_szSkinName));

	m_pPhys->SetPosition(GetLocalOrigin(), GetLocalAngles(), true);
	m_pPhys->SetVelocityInstantaneous(&vec3_origin, &vec3_origin);

	PrecacheScriptSound("Ball.Kicknormal");
	PrecacheScriptSound("Ball.Kickhard");
	PrecacheScriptSound("Ball.Touch");
	PrecacheScriptSound("Ball.Post");
	PrecacheScriptSound("Ball.Net");
	PrecacheScriptSound("Ball.Whistle");

	State_Transition(BALL_STATE_NORMAL);
}

bool CBall::CreateVPhysics()
{	
	if (m_pPhys)
	{
		VPhysicsDestroyObject();
		m_pPhys = NULL;
	}

	objectparams_t params =	g_IOSPhysDefaultObjectParams;
	params.pGameData = static_cast<void	*>(this);
	params.damping = sv_ball_damping.GetFloat();
	params.mass = sv_ball_mass.GetFloat();
	params.dragCoefficient = sv_ball_dragcoeff.GetFloat();
	params.inertia = sv_ball_inertia.GetFloat();
	params.rotdamping = sv_ball_rotdamping.GetFloat();
	params.rotInertiaLimit = sv_ball_rotinertialimit.GetFloat();
	int	nMaterialIndex = physprops->GetSurfaceIndex(mp_weather.GetInt() == 0 ? "dryball" : (mp_weather.GetInt() == 1 ? "wetball" : "icyball"));
	m_pPhys = physenv->CreateSphereObject( BALL_PHYS_RADIUS, nMaterialIndex, GetAbsOrigin(), GetAbsAngles(), &params, false );
	if (!m_pPhys)
		return false;

	VPhysicsSetObject( m_pPhys );
	
	SetSolid( SOLID_VPHYSICS );
	SetSolidFlags( FSOLID_NOT_STANDABLE	);
	UTIL_SetSize(this, -Vector(BALL_PHYS_RADIUS, BALL_PHYS_RADIUS, BALL_PHYS_RADIUS), Vector(BALL_PHYS_RADIUS, BALL_PHYS_RADIUS, BALL_PHYS_RADIUS));

	SetMoveType( MOVETYPE_VPHYSICS );

	PhysSetGameFlags(m_pPhys, FVPHYSICS_NO_PLAYER_PICKUP);

	m_pPhys->SetMass(sv_ball_mass.GetFloat());//0.05f	);
	m_fMass	= m_pPhys->GetMass();
	//m_pPhys->EnableGravity(	sv_ball_enable_gravity.GetFloat() );
	m_pPhys->EnableGravity(true);
	//m_pPhys->EnableDrag( sv_ball_enable_drag.GetFloat() );
	m_pPhys->EnableDrag(sv_ball_drag_enabled.GetBool());
	//SetElasticity(sv_ball_elasticity.GetFloat());
	//SetGravity(sv_ball_gravity.GetFloat());
	//float drag = sv_ball_dragcoefficient.GetFloat();
	//float angdrag = sv_ball_angdragcoefficient.GetFloat();
	//m_pPhys->SetDragCoefficient(&drag, &angdrag);
	float flDamping	= sv_ball_damping.GetFloat(); //0.0f
	float flAngDamping = sv_ball_rotdamping.GetFloat(); //2.5f
	m_pPhys->SetDamping( &flDamping, &flAngDamping );
	//float drag = 0;
	//m_pPhys->SetDragCoefficient(&drag, &drag);
	//m_pPhys->SetInertia(sv_ball_inertia.GetFloat());
	//VPhysicsGetObject()->SetInertia( Vector( 0.0023225760f,	0.0023225760f, 0.0023225760f ) );
	SetPhysicsMode(PHYSICS_MULTIPLAYER_SOLID);
	//SetPhysicsMode(PHYSICS_MULTIPLAYER_AUTODETECT);
	EnablePlayerCollisions(true);
	m_pPhys->SetBuoyancyRatio(0.5f);
	m_pPhys->Wake();

	return true;
}

void CBall::VPhysicsUpdate(IPhysicsObject *pPhysics)
{
	Vector vel, worldAngImp;
	AngularImpulse angImp;
	m_pPhys->GetVelocity(&vel, &angImp);
	VectorRotate(angImp, EntityToWorldTransform(), worldAngImp);
	Vector magnusVel = worldAngImp.Cross(vel);

	if (vel.Length() >= 0.1f)
	{
		vel += magnusVel * 1e-6 * sv_ball_magnus_coeff.GetFloat() * gpGlobals->frametime;

		if (sv_ball_sandground_enabled.GetBool() && vel.z < 0.1f && m_pPhys->GetContactPoint(NULL, NULL))
		{
			//vel *= 1 - min(1.0f, sv_ball_sandground_rollfriction.GetFloat() * gpGlobals->frametime);
			angImp *= 1 - min(1.0f, sv_ball_sandground_rollfriction.GetFloat() * gpGlobals->frametime);
		}

		m_pPhys->SetVelocity(&vel, &angImp);
	}

	BaseClass::VPhysicsUpdate(pPhysics);
}

void CBall::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
	float speed = pEvent->collisionSpeed;
	int surfaceProps = pEvent->surfaceProps[!index];

	if (surfaceProps == SURFACEPROPS_NET && speed >= 100.0f)
		EmitSound("Ball.Net");
	else if (surfaceProps == SURFACEPROPS_POST && speed >= 100.0f)
		EmitSound("Ball.Post");
	else if (speed >= 100.0f)
		EmitSound("Ball.Touch");

	Vector vel;
	AngularImpulse angImp;
	m_pPhys->GetVelocity(&vel, &angImp);

	if (vel.Length() >= 0.1f)
	{
		if (surfaceProps == SURFACEPROPS_POST)
		{
			vel *= 1 - sv_ball_goalpostfriction.GetFloat();
		}
		else if (surfaceProps == SURFACEPROPS_NET)
		{
			Vector dir = pEvent->preVelocity[index];
			dir.NormalizeInPlace();
			Vector normal;
			pEvent->pInternalData->GetSurfaceNormal(normal);
			float netFriction = Lerp(abs(dir.Dot(normal)), sv_ball_goalnetfriction_min.GetFloat(), sv_ball_goalnetfriction_max.GetFloat());
			vel *= 1 - netFriction;
			angImp *= 1 - netFriction;
		}
		else if (sv_ball_sandground_enabled.GetBool() && surfaceProps == 0) // 0 == ground
		{
			vel *= 1 - sv_ball_sandground_bouncefriction.GetFloat();
		}

		m_pPhys->SetVelocity(&vel, &angImp);
	}
}

CSDKPlayer *CBall::FindNearestPlayer(int team /*= TEAM_NONE*/, int posFlags /*= FL_POS_OUTFIELD*/, bool checkIfShooting /*= false*/, int ignoredPlayerBits /*= 0*/, float radius /*= -1*/)
{
	CSDKPlayer *pNearest = NULL;
	float shortestDist = FLT_MAX;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!CSDKPlayer::IsOnField(pPlayer))
			continue;

		if (ignoredPlayerBits & (1 << (pPlayer->entindex() - 1)))
			continue;

		if (!(posFlags & FL_POS_ANY))
		{
			int posType = (int)pPlayer->GetTeam()->GetFormation()->positions[pPlayer->GetTeamPosIndex()]->type;

			if ((posFlags & FL_POS_OUTFIELD) != 0 && !((1 << posType) & (g_nPosDefense + g_nPosMidfield + g_nPosAttack)))
				continue;

			if ((posFlags & FL_POS_KEEPER) != 0 && !((1 << posType) & g_nPosKeeper))
				continue;

			if ((posFlags & FL_POS_DEFENDER) != 0 && !((1 << posType) & g_nPosDefense))
				continue;

			if ((posFlags & FL_POS_MIDFIELDER) != 0 && !((1 << posType) & g_nPosMidfield))
				continue;

			if ((posFlags & FL_POS_ATTACKER) != 0 && !((1 << posType) & g_nPosAttack))
				continue;

			if ((posFlags & FL_POS_LEFT) != 0 && !((1 << posType) & g_nPosLeft))
				continue;

			if ((posFlags & FL_POS_CENTER) != 0 && !((1 << posType) & g_nPosCenter))
				continue;

			if ((posFlags & FL_POS_RIGHT) != 0 && !((1 << posType) & g_nPosRight))
				continue;
		}

		if (team != TEAM_NONE && pPlayer->GetTeamNumber() != team)
			continue;

		if (checkIfShooting && (!pPlayer->IsShooting() || !pPlayer->CanShoot()) && !pPlayer->IsKeeperDiving())
			continue;

		if (radius != -1 && (pPlayer->GetLocalOrigin() - m_vPos).Length2DSqr() > Sqr(radius))
			continue;

		Vector dir = m_vPos - pPlayer->GetLocalOrigin();
		float dist = dir.Length2D();

		if (dist < shortestDist)
		{
			shortestDist = dist;
			pNearest = pPlayer;	
		}
	}

	//// If we didn't find a player of a certain team, just look for any other player
	//if (!pNearest && team != TEAM_NONE)
	//	pNearest = FindNearestPlayer();

	return pNearest;
}

void CBall::SetPos(const Vector &pos, bool addBallRadiusZOffset/* = true*/, bool freeze/* = true*/)
{
	m_vPos = pos;

	// The ball origin is right in the center of the ball, not at the bottom.
	// When setting the ball position it's often easier to just assume the origin is at the bottom of the ball.
	// By default this method compensates for the origin difference automatically.
	if (addBallRadiusZOffset)
		m_vPos.z += BALL_PHYS_RADIUS;

	m_vVel = vec3_origin;
	m_vRot = vec3_origin;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocityInstantaneous(&vec3_origin, &vec3_origin);
	m_pPhys->SetPosition(m_vPos, m_aAng, true);

	if (freeze)
		m_pPhys->EnableMotion(false);
}

void CBall::SetAng(const QAngle &ang)
{
	m_aAng = ang;
	m_pPhys->SetPosition(m_vPos, m_aAng, false);
}

void CBall::SetVel(Vector vel, float spinCoeff, int spinFlags, body_part_t bodyPart, bool markOffsidePlayers, float minPostDelay, bool resetShotCharging)
{
	Vector oldVel = m_vVel;

	m_vVel = vel;

	if (sv_ball_sandground_enabled.GetBool())
	{
		if (m_pPhys->GetContactPoint(NULL, NULL))
		{
			float speed = VectorNormalize(m_vVel);
			m_vVel *= min(speed, sv_ball_sandground_maxshotstrength.GetInt());
		}
	}

	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocity(&m_vVel, &m_vRot);

	if (spinCoeff != -1)
		SetRot(CalcSpin(spinCoeff, spinFlags));

	float dynamicDelay = RemapValClamped(m_vVel.Length(), sv_ball_dynamicshotdelay_minshotstrength.GetInt(), sv_ball_dynamicshotdelay_maxshotstrength.GetInt(), sv_ball_dynamicshotdelay_mindelay.GetFloat(), sv_ball_dynamicshotdelay_maxdelay.GetFloat());
	
	m_flGlobalLastShot = gpGlobals->curtime;
	m_flGlobalDynamicShotDelay = dynamicDelay;

	m_pPl->m_flNextShot = gpGlobals->curtime + max(dynamicDelay, minPostDelay);

	if (resetShotCharging)
		m_pPl->ResetShotCharging();

	Touched(true, bodyPart, oldVel);
}

void CBall::SetRot(AngularImpulse rot)
{
	m_vRot = rot;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocity(&m_vVel, &m_vRot);
}

CBallStateInfo *CBall::State_LookupInfo(ball_state_t state)
{
	static CBallStateInfo ballStateInfos[] =
	{
		{ BALL_STATE_STATIC,		"BALL_STATE_STATIC",		&CBall::State_STATIC_Enter,			&CBall::State_STATIC_Think,			&CBall::State_STATIC_Leave },
		{ BALL_STATE_NORMAL,		"BALL_STATE_NORMAL",		&CBall::State_NORMAL_Enter,			&CBall::State_NORMAL_Think,			&CBall::State_NORMAL_Leave },
		{ BALL_STATE_KICKOFF,		"BALL_STATE_KICKOFF",		&CBall::State_KICKOFF_Enter,		&CBall::State_KICKOFF_Think,		&CBall::State_KICKOFF_Leave },
		{ BALL_STATE_THROWIN,		"BALL_STATE_THROWIN",		&CBall::State_THROWIN_Enter,		&CBall::State_THROWIN_Think,		&CBall::State_THROWIN_Leave },
		{ BALL_STATE_GOALKICK,		"BALL_STATE_GOALKICK",		&CBall::State_GOALKICK_Enter,		&CBall::State_GOALKICK_Think,		&CBall::State_GOALKICK_Leave },
		{ BALL_STATE_CORNER,		"BALL_STATE_CORNER",		&CBall::State_CORNER_Enter,			&CBall::State_CORNER_Think,			&CBall::State_CORNER_Leave },
		{ BALL_STATE_GOAL,			"BALL_STATE_GOAL",			&CBall::State_GOAL_Enter,			&CBall::State_GOAL_Think,			&CBall::State_GOAL_Leave },
		{ BALL_STATE_FREEKICK,		"BALL_STATE_FREEKICK",		&CBall::State_FREEKICK_Enter,		&CBall::State_FREEKICK_Think,		&CBall::State_FREEKICK_Leave },
		{ BALL_STATE_PENALTY,		"BALL_STATE_PENALTY",		&CBall::State_PENALTY_Enter,		&CBall::State_PENALTY_Think,		&CBall::State_PENALTY_Leave },
		{ BALL_STATE_KEEPERHANDS,	"BALL_STATE_KEEPERHANDS",	&CBall::State_KEEPERHANDS_Enter,	&CBall::State_KEEPERHANDS_Think,	&CBall::State_KEEPERHANDS_Leave },
	};

	for (int i = 0; i < ARRAYSIZE(ballStateInfos); i++)
	{
		if (ballStateInfos[i].m_eBallState == state)
			return &ballStateInfos[i];
	}

	return NULL;
}

bool CBall::CanReachBallStandingXY()
{
	return m_vPlDirToBall.Length2D() <= sv_ball_standing_reach.GetFloat();
}

bool CBall::GetCollisionPoint(bool isDeflection, Vector &collisionPoint)
{
	Vector dirToBall = m_vPos - m_vPlPos;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);
	float zDist = dirToBall.z;
	float xyDist = dirToBall.Length2D();

	if (m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_SLIDE_TACKLE
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_DIVING_HEADER
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_FORWARD
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_RIGHT_FORWARD
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_RIGHT
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_RIGHT_BACKWARD
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_BACKWARD
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_LEFT_BACKWARD
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_LEFT
		|| m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_LEFT_FORWARD)
	{
		float padding = isDeflection ? 0 : -15;
		Vector plDir;

		switch (m_pPl->m_Shared.GetAction())
		{
		default:
			plDir = m_vPlForward2D;
			break;
		case PLAYERANIMEVENT_KEEPER_DIVE_LEFT_FORWARD:
		case PLAYERANIMEVENT_KEEPER_DIVE_LEFT:
		case PLAYERANIMEVENT_KEEPER_DIVE_LEFT_BACKWARD:
			plDir = -m_vPlRight;
			break;
		case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT_FORWARD:
		case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT:
		case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT_BACKWARD:
			plDir = m_vPlRight;
			break;
		}

		Vector plOffsetPos = Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + BALL_PHYS_RADIUS);
		float dot = (m_vPos - plOffsetPos).Dot(plDir);
		Vector closestPoint = plOffsetPos + plDir * dot;
		Vector closestDir = m_vPos - closestPoint;
		float plDistToClosestPoint = (closestPoint - plOffsetPos).Length();

		if ((dot >= 0 && plDistToClosestPoint <= sv_ball_slideforwardreach_ball.GetFloat() + padding
			|| dot < 0 && plDistToClosestPoint <= sv_ball_slidebackwardreach_ball.GetFloat() + padding)
			&& closestDir.Length() <= sv_ball_slidesidereach_ball.GetFloat() + padding)
		{
			if (dot >= 0 && plDistToClosestPoint > (sv_ball_slideforwardreach_ball.GetFloat() + padding) - (sv_ball_slidesidereach_ball.GetFloat() + padding))
			{
				collisionPoint = plOffsetPos + plDir * ((sv_ball_slideforwardreach_ball.GetFloat() + padding) - (sv_ball_slidesidereach_ball.GetFloat() + padding));
				return true;
			}
			else if (dot < 0 && plDistToClosestPoint > (sv_ball_slidebackwardreach_ball.GetFloat() + padding) - (sv_ball_slidesidereach_ball.GetFloat() + padding))
			{
				collisionPoint = plOffsetPos - plDir * ((sv_ball_slidebackwardreach_ball.GetFloat() + padding) - (sv_ball_slidesidereach_ball.GetFloat() + padding));
				return true;
			}
			else
			{
				collisionPoint = closestPoint;
				return true;
			}
		}
	}
	else
	{
		if (isDeflection)
		{
			if (zDist >= sv_ball_bodypos_deflection_start.GetFloat() && zDist < sv_ball_bodypos_deflection_end.GetFloat() && xyDist <= sv_ball_deflectionradius.GetFloat())
			{
				if (zDist > sv_ball_bodypos_deflection_end.GetFloat() - sv_ball_deflectionradius.GetFloat())
				{
					collisionPoint = Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + sv_ball_bodypos_deflection_end.GetFloat() - sv_ball_deflectionradius.GetFloat());
					return true;
				}
				else if (zDist < sv_ball_bodypos_deflection_start.GetFloat() + sv_ball_deflectionradius.GetFloat())
				{
					collisionPoint = Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + sv_ball_bodypos_deflection_start.GetFloat() + sv_ball_deflectionradius.GetFloat());
					return true;
				}
				else
				{
					collisionPoint = Vector(m_vPlPos.x, m_vPlPos.y, m_vPos.z);
					return true;
				}
			}
		}
		else
		{
			if (zDist >= sv_ball_bodypos_collision_start.GetFloat() && zDist < sv_ball_bodypos_collision_end.GetFloat() && xyDist <= sv_ball_collisionradius.GetFloat())
			{
				if (zDist > sv_ball_bodypos_collision_end.GetFloat() - sv_ball_collisionradius.GetFloat())
				{
					collisionPoint = Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + sv_ball_bodypos_collision_end.GetFloat() - sv_ball_collisionradius.GetFloat());
					return true;
				}
				else if (zDist < sv_ball_bodypos_collision_start.GetFloat() + sv_ball_collisionradius.GetFloat())
				{
					collisionPoint = Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + sv_ball_bodypos_collision_start.GetFloat() + sv_ball_collisionradius.GetFloat());
					return true;
				}
				else
				{
					collisionPoint = Vector(m_vPlPos.x, m_vPlPos.y, m_vPos.z);
					return true;
				}
			}
		}
	}

	return false;
}

bool CBall::CheckPlayerInteraction()
{
	bool actionPerformed = CheckBodyPartAction();
	
	if (!actionPerformed)
		CheckCollision();

	return actionPerformed;
}

bool CBall::CheckBodyPartAction()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);
	float zDist = dirToBall.z;

	if ((m_pPl->IsNormalshooting() || m_pPl->IsKeeperDiving())
		&& m_pPl->CanShoot()
		&& m_pPl->GetTeamPosType() == POS_GK
		&& m_nInPenBoxOfTeam == m_pPl->GetTeamNumber()
		&& !m_pPl->m_pHoldingBall
		&& m_pPl->m_Shared.m_nInPenBoxOfTeam == m_pPl->GetTeamNumber())
	{
		if (IsLegallyCatchableByKeeper())
			return CheckKeeperCatch();
	}

	if (m_pPl->IsKeeperDiving() ||
		!m_pPl->CanShoot() ||
		gpGlobals->curtime < m_flGlobalLastShot + m_flGlobalDynamicShotDelay * sv_ball_shotdelay_global_coeff.GetFloat())
	{
		return false;
	}

	if (m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_SLIDE_TACKLE)
		return DoSlideAction();

	if (m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_STANDING_TACKLE)
		return DoStandingTackle();

	if (m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_DIVING_HEADER)
		return DoDivingHeader();

	if (m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_BICYCLE_KICK)
		return DoBicycleKick();

	if (!m_pPl->IsShooting())
		return false;

	if (zDist >= sv_ball_bodypos_feet_start.GetFloat()
		&& zDist < sv_ball_bodypos_hip_start.GetFloat()
		&& CanReachBallStandingXY())
	{
		return DoGroundHeightAction(true);
	}

	if (zDist >= sv_ball_bodypos_hip_start.GetFloat() && zDist < sv_ball_bodypos_head_start.GetFloat() && CanReachBallStandingXY())
		return DoHipHeightAction();

	if (zDist >= sv_ball_bodypos_head_start.GetFloat() && zDist < sv_ball_bodypos_head_end.GetFloat() && CanReachBallStandingXY())
		return DoHeadHeightAction();

	return false;
}

// http://gamedev.stackexchange.com/a/15936
bool CBall::CheckCollision()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);
	bool collide = false;
	float collisionCoeff;
	float ballMass;
	Vector collisionPoint;

	if (m_pPl->IsShooting() && m_pPl->CanShoot() && GetCollisionPoint(true, collisionPoint))
	{
		collide = true;
		collisionCoeff = sv_ball_collision_deflection_coeff.GetFloat();
		ballMass = sv_ball_collision_deflection_mass.GetFloat();
	}
	else if (GetCollisionPoint(false, collisionPoint))
	{
		collide = true;

		BallTouchInfo *pLastShot = LastInfo(true);

		if (pLastShot && pLastShot->m_pPl == m_pPl)
		{
			if (DotProduct2D(m_vVel.AsVector2D(), m_vPlVel.AsVector2D()) >= 0
				&& DotProduct2D(m_vVel.AsVector2D(), dirToBall.AsVector2D()) >= 0
				&& DotProduct2D(m_vPlVel.AsVector2D(), dirToBall.AsVector2D()) >= 0)
			{
				collisionCoeff = sv_ball_collision_dribbling_coeff.GetFloat();
				ballMass = sv_ball_collision_dribbling_mass.GetFloat();
			}
			else
			{
				collisionCoeff = sv_ball_collision_selfhit_coeff.GetFloat();
				ballMass = sv_ball_collision_selfhit_mass.GetFloat();
			}
		}
		else
		{
			collisionCoeff = sv_ball_collision_passive_coeff.GetFloat();
			ballMass = sv_ball_collision_passive_mass.GetFloat();
		}
	}

	if (!collide)
		return false;

	Vector dirToBallNormal = m_vPos - collisionPoint;
	dirToBallNormal.NormalizeInPlace();
	float dotballVel = m_vVel.Dot(dirToBallNormal);
	float dotPlayerVel = m_vPlVel.Dot(dirToBallNormal);
	float indicator = dotballVel - dotPlayerVel;

	// If indicator is bigger or equal to 0, the ball is either moving away from the player or going the same speed, so there's no need to apply additional velocity
	if (indicator >= 0)
		return false;

	float optimizedP = 2 * indicator / (ballMass + sv_player_mass.GetFloat());
	Vector vel = m_vVel - optimizedP * sv_player_mass.GetFloat() * dirToBallNormal;
	vel *= collisionCoeff;

	if (m_vVel.Length() > 900)
		m_pPl->EmitSound("Player.Oomph");

	Touched(false, BODY_PART_UNKNOWN, m_vVel);

	m_vVel = vel;

	m_pPhys->SetVelocity(&m_vVel, &m_vRot);

	return true;
}

bool CBall::DoSlideAction()
{
	if (!m_pPl->IsNormalshooting())
		return false;

	Vector dirToBall = m_vPos - m_vPlPos;
	float zDist = dirToBall.z;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);

	bool canShootBall = zDist < sv_ball_slidezend.GetFloat()
		&& zDist >= sv_ball_slidezstart.GetFloat()
		&& localDirToBall.x >= -sv_ball_slidebackwardreach_ball.GetFloat()
		&& localDirToBall.x <= sv_ball_slideforwardreach_ball.GetFloat()
		&& abs(localDirToBall.y) <= sv_ball_slidesidereach_ball.GetFloat();
	
	if (!canShootBall)
		return false;

	Vector forward;
	AngleVectors(QAngle(sv_ball_slide_pitchangle.GetFloat(), m_aPlAng[YAW], 0), &forward, NULL, NULL);

	Vector vel = forward * GetNormalshotStrength(GetPitchCoeff(), sv_ball_slide_strength.GetInt());

	SetVel(vel, 0, FL_SPIN_FORCE_NONE, BODY_PART_FEET, true, sv_ball_slide_minpostdelay.GetFloat(), true);

	if (!SDKGameRules()->IsIntermissionState() && State_Get() == BALL_STATE_NORMAL && !HasQueuedState())
		m_pPl->GetData()->AddSlidingTackleCompleted();

	return true;
}

bool CBall::DoStandingTackle()
{
	if (gpGlobals->curtime > m_pPl->m_Shared.GetActionStartTime() + mp_standing_tackle_move_duration.GetFloat())
		return false;

	Vector dirToBall = m_vPos - m_vPlPos;
	float zDist = dirToBall.z;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);

	bool canShootBall = zDist < sv_ball_standing_tackle_zend.GetFloat()
		&& zDist >= sv_ball_standing_tackle_zstart.GetFloat()
		&& localDirToBall.x >= -sv_ball_standing_tackle_backwardreach.GetFloat()
		&& localDirToBall.x <= sv_ball_standing_tackle_forwardreach.GetFloat()
		&& abs(localDirToBall.y) <= sv_ball_standing_tackle_sidereach.GetFloat();

	if (!canShootBall)
		return false;

	Vector forward;
	AngleVectors(QAngle(sv_ball_standing_tackle_pitchangle.GetFloat(), m_aPlAng[YAW], 0), &forward, NULL, NULL);

	Vector vel = forward * GetNormalshotStrength(1.0f, sv_ball_standing_tackle_strength.GetInt());

	SetVel(vel, 0, FL_SPIN_FORCE_NONE, BODY_PART_FEET, true, sv_ball_standing_tackle_minpostdelay.GetFloat(), true);

	return true;
}


bool CBall::DoDivingHeader()
{
	if (!m_pPl->IsChargedshooting())
		return false;

	Vector dirToBall = m_vPos - m_vPlPos;
	float zDist = dirToBall.z;

	if (zDist < sv_ball_bodypos_hip_start.GetFloat() || zDist >= sv_ball_bodypos_head_start.GetFloat() || !CanReachBallStandingXY())
		return false;

	if (gpGlobals->curtime > m_pPl->m_Shared.GetActionStartTime() + mp_divingheader_move_duration.GetFloat())
		return false;

	Vector forward;
	QAngle headerAngle = m_aPlAng;

	headerAngle[PITCH] = clamp(headerAngle[PITCH], sv_ball_divingheader_maxangle.GetFloat(), sv_ball_divingheader_minangle.GetFloat());
	AngleVectors(headerAngle, &forward);

	Vector vel = forward * GetChargedshotStrength(1.0f, sv_ball_divingheader_minstrength.GetInt(), sv_ball_divingheader_maxstrength.GetInt());
	EmitSound("Ball.Kickhard");

	SetVel(vel, sv_ball_header_spincoeff.GetFloat(), FL_SPIN_PERMIT_SIDE, BODY_PART_HEAD, true, sv_ball_header_minpostdelay.GetFloat(), true);

	return true;
}

bool CBall::DoBicycleKick()
{
	if (!m_pPl->IsChargedshooting())
		return false;

	Vector dirToBall = m_vPos - m_vPlPos;
	float zDist = dirToBall.z;

	if (zDist < sv_ball_bodypos_head_start.GetFloat() || zDist >= sv_ball_bodypos_head_end.GetFloat() || !CanReachBallStandingXY())
		return false;

	if (gpGlobals->curtime > m_pPl->m_Shared.GetActionStartTime() + mp_bicycleshot_move_duration.GetFloat())
		return false;

	QAngle ang = m_aPlAng;
	ang[YAW] += 180;

	Vector dir;
	AngleVectors(ang, &dir);

	Vector vel = dir * GetChargedshotStrength(GetPitchCoeff(), sv_ball_bicycleshot_minstrength.GetInt(), sv_ball_bicycleshot_maxstrength.GetInt());
	EmitSound("Ball.Kickhard");

	SetVel(vel, 0, FL_SPIN_FORCE_NONE, BODY_PART_FEET, true, sv_ball_bicycleshot_minpostdelay.GetFloat(), true);

	return true;
}

bool CBall::CheckKeeperCatch()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);

	bool canReach = false;
	static float sqrt2 = sqrt(2.0f);
	float distXY = localDirToBall.Length2D();
	float diveTypeCatchCoeff = 1.0f;
	float ballPosCatchCoeff = 1.0f;

	switch (m_pPl->m_Shared.GetAction())
	{
	case PLAYERANIMEVENT_KEEPER_DIVE_LEFT_FORWARD:
	case PLAYERANIMEVENT_KEEPER_DIVE_LEFT:
	case PLAYERANIMEVENT_KEEPER_DIVE_LEFT_BACKWARD:
		{
			canReach = (localDirToBall.z < sv_ball_keeper_sidedive_zend.GetFloat()
				&& localDirToBall.z >= sv_ball_keeper_sidedive_zstart.GetFloat()
				&& abs(localDirToBall.x) <= sv_ball_keeper_sidedive_shortsidereach.GetFloat()
				&& localDirToBall.y >= -sv_ball_keeper_sidedive_longsidereach_opposite.GetFloat()
				&& localDirToBall.y <= sv_ball_keeper_sidedive_longsidereach.GetFloat());

			if (canReach)
			{
				float distY = localDirToBall.y - sv_ball_keeper_sidedive_catchcenteroffset_side.GetFloat(); 
				float maxYReach = (distY >= 0 ? sv_ball_keeper_sidedive_longsidereach.GetFloat() : -sv_ball_keeper_sidedive_longsidereach_opposite.GetFloat()) - sv_ball_keeper_sidedive_catchcenteroffset_side.GetFloat();

				ballPosCatchCoeff = Square(min(1, abs(distY) / maxYReach));
				diveTypeCatchCoeff = sv_ball_keepercatchdelay_sidedive_global_coeff.GetFloat();
			}
		}
		break;
	case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT_FORWARD:
	case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT:
	case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT_BACKWARD:
		{
			canReach = (localDirToBall.z < sv_ball_keeper_sidedive_zend.GetFloat()
				&& localDirToBall.z >= sv_ball_keeper_sidedive_zstart.GetFloat()
				&& abs(localDirToBall.x) <= sv_ball_keeper_sidedive_shortsidereach.GetFloat()
				&& localDirToBall.y <= sv_ball_keeper_sidedive_longsidereach_opposite.GetFloat()
				&& localDirToBall.y >= -sv_ball_keeper_sidedive_longsidereach.GetFloat());

			if (canReach)
			{
				float distY = localDirToBall.y - -sv_ball_keeper_sidedive_catchcenteroffset_side.GetFloat(); 
				float maxYReach = (distY >= 0 ? sv_ball_keeper_sidedive_longsidereach_opposite.GetFloat() : -sv_ball_keeper_sidedive_longsidereach.GetFloat()) - -sv_ball_keeper_sidedive_catchcenteroffset_side.GetFloat();

				ballPosCatchCoeff = Square(min(1, abs(distY) / maxYReach));
				diveTypeCatchCoeff = sv_ball_keepercatchdelay_sidedive_global_coeff.GetFloat();
			}
		}
		break;
	case PLAYERANIMEVENT_KEEPER_DIVE_FORWARD:
		canReach = (localDirToBall.z < sv_ball_keeper_forwarddive_zend.GetFloat()
			&& localDirToBall.z >= sv_ball_keeper_forwarddive_zstart.GetFloat()
			&& localDirToBall.x >= -sv_ball_keeper_forwarddive_longsidereach_opposite.GetFloat()
			&& localDirToBall.x <= sv_ball_keeper_forwarddive_longsidereach.GetFloat()
			&& abs(localDirToBall.y) <= sv_ball_keeper_forwarddive_shortsidereach.GetFloat());

		if (canReach)
		{
			ballPosCatchCoeff = sv_ball_keeper_forwarddive_catchcoeff.GetFloat();
			diveTypeCatchCoeff = sv_ball_keepercatchdelay_forwarddive_global_coeff.GetFloat();
		}
		break;
	case PLAYERANIMEVENT_KEEPER_DIVE_BACKWARD:
		canReach = (localDirToBall.z < sv_ball_keeper_backwarddive_zend.GetFloat()
			&& localDirToBall.z >= sv_ball_keeper_backwarddive_zstart.GetFloat()
			&& localDirToBall.x >= -sv_ball_keeper_backwarddive_longsidereach.GetFloat()
			&& localDirToBall.x <= sv_ball_keeper_backwarddive_longsidereach_opposite.GetFloat()
			&& abs(localDirToBall.y) <= sv_ball_keeper_backwarddive_shortsidereach.GetFloat());

		if (canReach)
		{
			ballPosCatchCoeff = sv_ball_keeper_backwarddive_catchcoeff.GetFloat();
			diveTypeCatchCoeff = sv_ball_keepercatchdelay_backwarddive_global_coeff.GetFloat();
		}
		break;
	case PLAYERANIMEVENT_KEEPER_JUMP:
	default: // Keeper standing
		float maxReachXY = sv_ball_keeper_standing_reach.GetFloat();

		canReach = (localDirToBall.z < sv_ball_bodypos_keeperarms_end.GetFloat()
			&& localDirToBall.z >= sv_ball_bodypos_feet_start.GetFloat()
			&& distXY <= maxReachXY);

		if (canReach)
		{
			float distY = localDirToBall.y; 
			float maxYReach = distY >= 0 ? maxReachXY : -maxReachXY;

			ballPosCatchCoeff = Square(min(1, abs(distY) / maxYReach));
			diveTypeCatchCoeff = sv_ball_keepercatchdelay_standing_global_coeff.GetFloat();
		}
		break;
	}

	if (!canReach)
		return false;
	
	ballPosCatchCoeff = clamp(ballPosCatchCoeff * (1 - sv_ball_keepercatchdelay_poscoeffmin.GetFloat()) + sv_ball_keepercatchdelay_poscoeffmin.GetFloat(), 0.0f, 1.0f);

	float nextCatch = m_flGlobalLastShot + m_flGlobalDynamicShotDelay * diveTypeCatchCoeff * ballPosCatchCoeff;

	// Check if ball should be punched away
	if (m_bHasQueuedState || gpGlobals->curtime < nextCatch || m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_BACKWARD)
	{
		Vector punchDir;

		QAngle angDiff = m_aPlCamAng - m_aPlAng;

		if (m_pPl->m_Shared.GetAction() == PLAYERANIMEVENT_KEEPER_DIVE_BACKWARD)
		{
			QAngle ang = m_aPlCamAng;
			ang[YAW] += 180;
			ang[PITCH] = -sv_ball_keeper_backwarddive_punchupangle.GetInt();
			AngleVectors(ang, &punchDir);
		}
		else
		{
			// Punch into the player camera direction
			QAngle ang = m_aPlCamAng;

			// Add the player speed to the punch yaw direction, so new players who don't move the camera don't punch straight ahead every time
			float extraYaw = -ZeroSign(m_vPlVel2D.Dot(m_vPlRight)) * m_vPlSideVel2D.Length() * sv_ball_keeperpunch_sidespeedcoeff.GetFloat();

			// Make the extra yaw weaker the closer to 90� the model to free cam yaw difference is
			extraYaw *= 1.0f - clamp(abs(AngleDiff(m_aPlAng[YAW], m_aPlCamAng[YAW])) / 90.0f, 0.0f, 1.0f);

			ang[YAW] += extraYaw;

			AngleVectors(ang, &punchDir);
		}

		Vector vel = punchDir * max(m_vVel.Length2D(), sv_ball_keeperpunch_minstrength.GetFloat()) * sv_ball_keeperdeflectioncoeff.GetFloat();

		SetVel(vel, 0, FL_SPIN_FORCE_NONE, BODY_PART_KEEPERPUNCH, false, sv_ball_keeperpunch_minpostdelay.GetFloat(), true);
	}
	// Catch ball instead of punching
	else
	{
		SetVel(vec3_origin, 0, FL_SPIN_FORCE_NONE, BODY_PART_KEEPERCATCH, false, sv_ball_keepercatch_minpostdelay.GetFloat(), true);
		State_Transition(BALL_STATE_KEEPERHANDS);
	}

	return true;
}

float CBall::GetPitchCoeff()
{
	//return pow(cos((m_aPlAng[PITCH] - sv_ball_bestshotangle.GetInt()) / (PITCH_LIMIT - sv_ball_bestshotangle.GetInt()) * M_PI / 2), 2);
	// plot 0.5 + (cos(x/89 * pi/2) * 0.5), x=-89..89

	float bestAng = sv_ball_bestshotangle.GetFloat();
	float pitch = m_aPlAng[PITCH];

	float coeff;

	if (pitch <= bestAng)
	{
		float upCoeff = sv_ball_pitchup_fixedcoeff.GetFloat();
		double upExp = sv_ball_pitchup_exponent.GetFloat();
		coeff = upCoeff + (1 - upCoeff) * pow(cos((pitch - bestAng) / (-mp_pitchup.GetFloat() - bestAng) * M_PI / 2), upExp);		
	}
	else
	{
		float downCoeff = sv_ball_pitchdown_fixedcoeff.GetFloat();
		double downExp = sv_ball_pitchdown_exponent.GetFloat();
		coeff = downCoeff + (1 - downCoeff) * pow(cos((pitch - bestAng) / (mp_pitchdown.GetFloat() - bestAng) * M_PI / 2), downExp);
	}

	//DevMsg("coeff: %.2f\n", coeff);

	return coeff;
}

float CBall::GetNormalshotStrength(float coeff, int strength)
{
	return coeff * strength;
}

float CBall::GetChargedshotStrength(float coeff, int minStrength, int maxStrength)
{
	float shotStrength = minStrength + (maxStrength - minStrength) * m_pPl->GetChargedShotStrength();

	return coeff * shotStrength;
}

bool CBall::DoGroundHeightAction(bool markOffsidePlayers)
{
	int spinFlags = FL_SPIN_FORCE_NONE;
	float spinCoeff = 1.0f;
	float shotStrength = 0;
	QAngle shotAngle = m_aPlAng;
	float minPostDelay = 0.0f;
	bool addPlayerSpeed = false;

	if (m_pPl->DoSkillMove())
	{
		if (!m_pPl->GetGroundEntity() || m_pPl->IsChargedshooting())
			return false;

		if (m_vPlLocalDirToBall.x >= 0)
		{
			if ((m_pPl->m_nButtons & IN_MOVELEFT) || (m_pPl->m_nButtons & IN_MOVERIGHT) || (m_pPl->m_nButtons & IN_BACK))
			{
				// Ball roll
				minPostDelay = sv_ball_roll_minpostdelay.GetFloat();
				shotAngle = m_aPlAng;

				if (m_pPl->m_nButtons & IN_MOVELEFT)
				{
					shotAngle = QAngle(0, m_aPlAng[YAW] + 90, 0);
					shotStrength = sv_ball_roll_strength.GetInt();
				}
				else if (m_pPl->m_nButtons & IN_MOVERIGHT)
				{
					shotAngle = QAngle(0, m_aPlAng[YAW] - 90, 0);
					shotStrength = sv_ball_roll_strength.GetInt();
				}
				else
				{
					shotAngle = QAngle(0, m_aPlAng[YAW] + 180, 0);
					shotStrength = sv_ball_roll_strength.GetInt();
				}
			}
			else
			{
				// Ball lift
				shotAngle[PITCH] = sv_ball_lift_angle.GetInt();
				shotStrength = Lerp(GetPitchCoeff(), sv_ball_lift_minstrength.GetInt(), sv_ball_lift_maxstrength.GetInt());
				spinFlags = FL_SPIN_FORCE_NONE;
				spinCoeff = 0;
				minPostDelay = sv_ball_lift_minpostdelay.GetFloat();
				addPlayerSpeed = true;
				EmitSound("Ball.Touch");
			}
		}
		else
		{
			if ((m_pPl->m_nButtons & IN_BACK) || (m_pPl->m_nButtons & IN_MOVELEFT) || (m_pPl->m_nButtons & IN_MOVERIGHT))
			{
				if (LastPl(false) == m_pPl)
					return false;

				// Heel shot
				if (m_pPl->m_nButtons & IN_BACK)
				{
					if (m_pPl->m_nButtons & IN_MOVELEFT)
						shotAngle[YAW] += 135;
					else if (m_pPl->m_nButtons & IN_MOVERIGHT)
						shotAngle[YAW] -= 135;
					else
						shotAngle[YAW] += 180;
				}
				else
				{
					if (m_pPl->m_nButtons & IN_MOVELEFT)
						shotAngle[YAW] += 90;
					else if (m_pPl->m_nButtons & IN_MOVERIGHT)
						shotAngle[YAW] -= 90;
				}

				shotAngle[PITCH] = min(sv_ball_heelshot_minangle.GetFloat(), shotAngle[PITCH]);
				shotStrength = max(sv_ball_heelshot_minstrength.GetInt(), m_vVel.Length2D() * sv_ball_heelshot_strengthcoeff.GetFloat());
				minPostDelay = sv_ball_heelshot_minpostdelay.GetFloat();
				m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEEL_KICK);
				EmitSound("Ball.Kicknormal");
			}
			else
			{
				// Rainbow flick
				shotAngle = m_aPlAng;
				shotAngle[PITCH] = sv_ball_rainbowflick_angle.GetInt();
				shotStrength = Lerp(GetPitchCoeff(), sv_ball_rainbowflick_minstrength.GetInt(), sv_ball_rainbowflick_maxstrength.GetInt());
				spinFlags = FL_SPIN_FORCE_TOP;
				spinCoeff = sv_ball_rainbowflick_spincoeff.GetFloat();
				minPostDelay = sv_ball_rainbowflick_minpostdelay.GetFloat();
				addPlayerSpeed = true;
				m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_RAINBOW_FLICK);
				EmitSound("Ball.Kicknormal");
			}
		}
	}
	else
	{
		// Dribble shot
		spinFlags = FL_SPIN_PERMIT_SIDE;

		if (m_pPl->IsNormalshooting())
		{
			shotStrength = GetNormalshotStrength(GetPitchCoeff(), sv_ball_normalshot_strength.GetInt());
			minPostDelay = sv_ball_normalshot_minpostdelay.GetFloat();
		}
		else
		{
			shotStrength = GetChargedshotStrength(GetPitchCoeff(), sv_ball_chargedshot_minstrength.GetInt(), sv_ball_chargedshot_maxstrength.GetInt());
			minPostDelay = sv_ball_chargedshot_minpostdelay.GetFloat();
		}

		shotAngle[PITCH] = min(sv_ball_groundshot_minangle.GetFloat(), shotAngle[PITCH]);

		Vector shotDir;
		AngleVectors(shotAngle, &shotDir);
		Vector vel = shotDir * shotStrength;

		float relativeShotStrength = (1 - clamp(m_vPlVel2D.Dot(vel) / vel.Dot(vel), 0.0f, 1.0f)) * shotStrength;

		if (relativeShotStrength >= sv_ball_animation_minstrength_strongshot.GetInt())
		{
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KICK_STRONG);
			EmitSound("Ball.Kickhard");
		}
		else if (relativeShotStrength >= sv_ball_animation_minstrength_weakshot.GetInt())
		{
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KICK_WEAK);
			EmitSound("Ball.Kicknormal");
		}
		else if (relativeShotStrength >= sv_ball_animation_minstrength_dribbleshot.GetInt())
		{
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KICK_DRIBBLE);
			EmitSound("Ball.Touch");
		}
	}

	Vector shotDir;
	AngleVectors(shotAngle, &shotDir);
	Vector vel = shotDir * shotStrength;

	if (addPlayerSpeed)
		vel += m_vPlForwardVel2D;

	SetVel(vel, spinCoeff, spinFlags, BODY_PART_FEET, markOffsidePlayers, minPostDelay, true);

	return true;
}

bool CBall::DoHipHeightAction()
{
	if (m_pPl->DoSkillMove())
		return false;

	float spinCoeff = 1.0f;
	int spinFlags = FL_SPIN_PERMIT_SIDE;
	QAngle shotAngle = m_aPlAng;

	float shotStrength;

	if (m_pPl->IsNormalshooting())
		shotStrength = GetNormalshotStrength(GetPitchCoeff(), sv_ball_normalshot_strength.GetInt());
	else
		shotStrength = GetChargedshotStrength(GetPitchCoeff(), sv_ball_chargedshot_minstrength.GetInt(), sv_ball_chargedshot_maxstrength.GetInt());

	shotAngle[PITCH] = min(sv_ball_volleyshot_minangle.GetFloat(), shotAngle[PITCH]);

	Vector shotDir;
	AngleVectors(shotAngle, &shotDir);

	Vector vel = shotDir * shotStrength;

	if (vel.Length() > 700)
	{
		if (vel.Length() > 1000)
			EmitSound("Ball.Kickhard");
		else
			EmitSound("Ball.Kicknormal");

		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_VOLLEY);
	}

	SetVel(vel, spinCoeff, spinFlags, BODY_PART_FEET, true, sv_ball_volleyshot_minpostdelay.GetFloat(), true);

	return true;
}

bool CBall::DoHeadHeightAction()
{
	if (m_pPl->DoSkillMove())
		return false;

	Vector vel, forward;
	QAngle headerAngle = m_aPlAng;

	// Normal header
	if (m_pPl->IsNormalshooting())
	{
		headerAngle[PITCH] = clamp(headerAngle[PITCH], sv_ball_header_maxangle.GetFloat(), sv_ball_header_minangle.GetFloat());
		AngleVectors(headerAngle, &forward);

		vel = forward * GetNormalshotStrength(GetPitchCoeff(), sv_ball_normalheader_strength.GetInt());

		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEADER_WEAK);
		EmitSound("Ball.Kickhard");
	}
	// Charged header
	else
	{
		headerAngle[PITCH] = clamp(headerAngle[PITCH], sv_ball_header_maxangle.GetFloat(), sv_ball_header_minangle.GetFloat());
		AngleVectors(headerAngle, &forward);

		vel = forward * GetChargedshotStrength(GetPitchCoeff(), sv_ball_chargedheader_minstrength.GetInt(), sv_ball_chargedheader_maxstrength.GetInt());

		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEADER_STRONG);
		EmitSound("Ball.Kickhard");
	}

	SetVel(vel, sv_ball_header_spincoeff.GetFloat(), FL_SPIN_PERMIT_SIDE, BODY_PART_HEAD, true, sv_ball_header_minpostdelay.GetFloat(), true);

	return true;
}

AngularImpulse CBall::CalcSpin(float coeff, int spinFlags)
{	
	Vector worldRot;

	if (spinFlags & FL_SPIN_RETAIN_SIDE)
	{
		VectorRotate(m_vRot, EntityToWorldTransform(), worldRot);
		worldRot.x = 0;
		worldRot.y = 0;
		worldRot *= coeff;
	}
	else
	{
		Vector sideRot = vec3_origin;
		float sideSpin = 0;

		if (spinFlags & FL_SPIN_PERMIT_SIDE)
		{
			sideSpin = sv_ball_spin.GetInt() * coeff * GetPitchCoeff();

			int sidemoveSign = m_pPl->GetSidemoveSign();

			if (sidemoveSign == -1) 
				sideRot = Vector(0, 0, m_pPl->IsReverseSideCurl() ? -1 : 1);
			else if (sidemoveSign == 1) 
				sideRot = Vector(0, 0, m_pPl->IsReverseSideCurl() ? 1 : -1);
		}

		Vector backRot = m_vPlRight;
		float backSpin = 0;

		Vector topRot = -m_vPlRight;
		float topSpin = 0;

		if (spinFlags & FL_SPIN_FORCE_BACK)
			backSpin = sv_ball_spin.GetInt() * coeff;

		if (spinFlags & FL_SPIN_FORCE_TOP)
			topSpin = sv_ball_spin.GetInt() * coeff;

		worldRot = sideRot * sideSpin + backRot * backSpin + topRot * topSpin;
	}

	AngularImpulse randRot = vec3_origin;

	for (int i = 0; i < 3; i++)
	{
		// Add some weak random rotation to all three axes, since a ball which only rotates in one or no axis looks unnatural
		randRot[i] = g_IOSRand.RandomInt(0, 1) == 1 ? 1 : -1;
	}

	worldRot += randRot * sv_ball_defaultspin.GetInt();

	AngularImpulse localRot;

	VectorIRotate(worldRot, EntityToWorldTransform(), localRot);

	return localRot;
}

void CBall::Think( void	)
{
	SetNextThink(gpGlobals->curtime);

	State_Think();
}

void CBall::UpdateCarrier()
{
	if (CSDKPlayer::IsOnField(m_pPl))
	{
		m_vPlPos = m_pPl->GetLocalOrigin();

		m_vPlVel = m_pPl->GetLocalVelocity();
		m_vPlVel2D = Vector(m_vPlVel.x, m_vPlVel.y, 0);

		m_aPlAng = m_pPl->EyeAngles();
		m_aPlAng[PITCH] = clamp(m_aPlAng[PITCH], -mp_pitchup.GetFloat(), mp_pitchdown.GetFloat());
		AngleVectors(m_aPlAng, &m_vPlForward, &m_vPlRight, &m_vPlUp);
		
		m_aPlCamAng = m_pPl->m_aCamViewAngles;
		m_aPlCamAng[PITCH] = clamp(m_aPlCamAng[PITCH], -mp_pitchup.GetFloat(), mp_pitchdown.GetFloat());
		
		m_vPlForward2D = m_vPlForward;
		m_vPlForward2D.z = 0;
		m_vPlForward2D.NormalizeInPlace();

		m_vPlForwardVel2D = m_vPlForward2D * max(0, m_vPlVel2D.Dot(m_vPlForward2D));
		m_vPlBackVel2D = m_vPlForward2D * min(0, m_vPlVel2D.Dot(m_vPlForward2D));
		m_vPlSideVel2D = -m_vPlRight * m_vPlVel2D.Dot(m_vPlRight);
		
		m_vPlDirToBall = m_vPos - m_vPlPos;
		VectorIRotate(m_vPlDirToBall, m_pPl->EntityToWorldTransform(), m_vPlLocalDirToBall);
	}
}

int CBall::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CBall::Reset()
{
	ReloadSettings();
	m_pPl = NULL;
	m_bHasQueuedState = false;
	RemoveEffects(EF_NODRAW);
	EnablePlayerCollisions(true);
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pHoldingPlayer = NULL;
	m_bHitThePost = false;
	m_nInPenBoxOfTeam = TEAM_NONE;
}

void CBall::ReloadSettings()
{
	CreateVPhysics();
}

void CBall::EnablePlayerCollisions(bool enable)
{
	SetCollisionGroup(enable ? COLLISION_GROUP_SOLID_BALL : COLLISION_GROUP_NONSOLID_BALL);
}

void CBall::AddToPlayerHands(CSDKPlayer *pPl)
{
	m_pHoldingPlayer = m_pPl;
	m_pPl->m_pHoldingBall = this;
	EnablePlayerCollisions(false);
}

void CBall::RemoveFromPlayerHands(CSDKPlayer *pPl)
{
	if (CSDKPlayer::IsOnField(pPl) && pPl->m_pHoldingBall.Get() == this)
	{
		pPl->m_pHoldingBall = NULL;

		if (pPl->m_Shared.GetCarryAnimation() != PLAYERANIMEVENT_NONE)
			pPl->DoServerAnimationEvent(PLAYERANIMEVENT_CARRY_END);
	}

	if (!IsMarkedForDeletion())
	{
		m_pHoldingPlayer = NULL;
		EnablePlayerCollisions(true);
	}
}

Vector CBall::GetPos()
{
	return m_vPos;
}

QAngle CBall::GetAng()
{
	return m_aAng;
}

Vector CBall::GetVel()
{
	return m_vVel;
}

AngularImpulse CBall::GetRot()
{
	return m_vRot;
}

void CBall::SetSkinName(const char *skinName)
{
	if (m_szSkinName[0] != '\0' && !Q_strcmp(skinName, m_szSkinName))
		return;

	int ballSkinIndex = -1;

	for (int i = 0; i < CBallInfo::m_BallInfo.Count(); i++)
	{
		if (!Q_strcmp(CBallInfo::m_BallInfo[i]->m_szFolderName, skinName))
		{
			ballSkinIndex = i;
			break;
		}
	}

	if (ballSkinIndex == -1)
		ballSkinIndex = g_IOSRand.RandomInt(0, CBallInfo::m_BallInfo.Count() - 1);

	Q_strncpy(m_szSkinName.GetForModify(), CBallInfo::m_BallInfo[ballSkinIndex]->m_szFolderName, sizeof(m_szSkinName));
}

void CBall::CheckPenBoxPosition()
{
	for (int team = TEAM_HOME; team <= TEAM_AWAY; team++)
	{
		Vector min = GetGlobalTeam(team)->m_vPenBoxMin;
		Vector max = GetGlobalTeam(team)->m_vPenBoxMax;

		if (m_vPos.x + BALL_PHYS_RADIUS >= min.x
			&& m_vPos.y + BALL_PHYS_RADIUS >= min.y
			&& m_vPos.x - BALL_PHYS_RADIUS <= max.x
			&& m_vPos.y - BALL_PHYS_RADIUS <= max.y)
		{
			m_nWasInPenBoxOfTeam = m_nInPenBoxOfTeam;
			m_nInPenBoxOfTeam = team;
			return;
		}
	}

	m_nWasInPenBoxOfTeam = m_nInPenBoxOfTeam;
	m_nInPenBoxOfTeam = TEAM_NONE;
}

void CBall::RemoveAllTouches()
{
	if (!m_bHasQueuedState)
		m_Touches.PurgeAndDeleteElements();
}

BallTouchInfo *CBall::LastInfo(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/, CSDKPlayer *pSkipPl2 /*= NULL*/, CSDKPlayer *pSkipPl3 /*= NULL*/)
{
	for (int i = m_Touches.Count() - 1; i >= 0; i--)
	{
		if (pSkipPl && m_Touches[i]->m_pPl == pSkipPl)
			continue;

		if (pSkipPl2 && m_Touches[i]->m_pPl == pSkipPl2)
			continue;

		if (pSkipPl3 && m_Touches[i]->m_pPl == pSkipPl3)
			continue;

		if (!wasShooting || m_Touches[i]->m_bIsShot)
			return m_Touches[i];
	}

	return NULL;
}

CSDKPlayer *CBall::LastPl(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/, CSDKPlayer *pSkipPl2 /*= NULL*/, CSDKPlayer *pSkipPl3 /*= NULL*/)
{
	BallTouchInfo *info = LastInfo(wasShooting, pSkipPl, pSkipPl2, pSkipPl3);
	if (info && CSDKPlayer::IsOnField(info->m_pPl))
		return info->m_pPl;
	
	return NULL;
}

int CBall::LastTeam(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/, CSDKPlayer *pSkipPl2 /*= NULL*/, CSDKPlayer *pSkipPl3 /*= NULL*/)
{
	BallTouchInfo *info = LastInfo(wasShooting, pSkipPl, pSkipPl2, pSkipPl3);
	return info ? info->m_nTeam : TEAM_NONE;
}

int CBall::LastOppTeam(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/, CSDKPlayer *pSkipPl2 /*= NULL*/, CSDKPlayer *pSkipPl3 /*= NULL*/)
{
	BallTouchInfo *info = LastInfo(wasShooting, pSkipPl, pSkipPl2, pSkipPl3);
	return info ? (info->m_nTeam == TEAM_HOME ? TEAM_AWAY : TEAM_HOME) : TEAM_NONE;
}

void CBall::GetPredictedGoalLineCrossPosX(int &xPos, int &team)
{
	Vector vel = GetVel();
	Vector dir = vel;
	dir.z = 0;
	dir.NormalizeInPlace();

	team = TEAM_NONE;

	for (int i = 0; i < 2; i++)
	{
		if (-GetGlobalTeam(i + TEAM_HOME)->m_nForward == ZeroSign(dir.y))
		{
			team = i + TEAM_HOME;
			break;
		}
	}

	if (team == TEAM_NONE)
	{
		xPos = 0;
	}
	else
	{
		Vector pos = GetPos();
		float ang = acos(Vector(0, Sign(vel.y), 0).Dot(dir));
		xPos = pos.x + Sign(vel.x) * tan(ang) * abs(GetGlobalTeam(team)->m_vGoalCenter.GetY() - pos.y);
	}
}