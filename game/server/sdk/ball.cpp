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


ConVar sv_ball_mass( "sv_ball_mass", "0.001", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_damping( "sv_ball_damping", "0.01", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_rotdamping( "sv_ball_rotdamping", "0.75", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_rotinertialimit( "sv_ball_rotinertialimit", "1.5", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_dragcoeff( "sv_ball_dragcoeff", "1", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_inertia( "sv_ball_inertia", "1.5", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_drag_enabled("sv_ball_drag_enabled", "1", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );

ConVar sv_ball_spin( "sv_ball_sidespin", "4000", FCVAR_NOTIFY );
ConVar sv_ball_spin_exponent( "sv_ball_spin_exponent", "0.75", FCVAR_NOTIFY );
ConVar sv_ball_defaultspin( "sv_ball_defaultspin", "150", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_topspin_coeff( "sv_ball_topspin_coeff", "0.1", FCVAR_NOTIFY );
ConVar sv_ball_backspin_coeff( "sv_ball_backspin_coeff", "0.25", FCVAR_NOTIFY );
ConVar sv_ball_curve("sv_ball_curve", "200", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);

ConVar sv_ball_deflectionradius( "sv_ball_deflectionradius", "30", FCVAR_NOTIFY );
ConVar sv_ball_collisionradius( "sv_ball_collisionradius", "15", FCVAR_NOTIFY );

ConVar sv_ball_standing_reach_shortside( "sv_ball_standing_reach_shortside", "35", FCVAR_NOTIFY );
ConVar sv_ball_standing_reach_longside( "sv_ball_standing_reach_longside", "45", FCVAR_NOTIFY );
ConVar sv_ball_standing_reach_shift( "sv_ball_standing_reach_shift", "10", FCVAR_NOTIFY );
ConVar sv_ball_standing_reach_ellipse( "sv_ball_standing_reach_ellipse", "1", FCVAR_NOTIFY );

ConVar sv_ball_slidesidereach_ball( "sv_ball_slidesidereach_ball", "50", FCVAR_NOTIFY );
ConVar sv_ball_slideforwardreach_ball( "sv_ball_slideforwardreach_ball", "60", FCVAR_NOTIFY );
ConVar sv_ball_slidebackwardreach_ball( "sv_ball_slidebackwardreach_ball", "20", FCVAR_NOTIFY );

ConVar sv_ball_slidesidereach_foul( "sv_ball_slidesidereach_foul", "25", FCVAR_NOTIFY );
ConVar sv_ball_slideforwardreach_foul( "sv_ball_slideforwardreach_foul", "60", FCVAR_NOTIFY );
ConVar sv_ball_slidebackwardreach_foul( "sv_ball_slidebackwardreach_foul", "20", FCVAR_NOTIFY );

ConVar sv_ball_slidesidespeedcoeff("sv_ball_slidesidespeedcoeff", "0.66", FCVAR_NOTIFY); 
ConVar sv_ball_slidezstart("sv_ball_slidezstart", "-50", FCVAR_NOTIFY); 
ConVar sv_ball_slidezend("sv_ball_slidezend", "40", FCVAR_NOTIFY); 

ConVar sv_ball_keeper_standing_reach_top( "sv_ball_keeper_standing_reach_top", "65", FCVAR_NOTIFY );
ConVar sv_ball_keeper_standing_reach_bottom( "sv_ball_keeper_standing_reach_bottom", "35", FCVAR_NOTIFY );
ConVar sv_ball_keeper_standing_catchcenteroffset_side( "sv_ball_keeper_standing_catchcenteroffset_side", "0", FCVAR_NOTIFY );
ConVar sv_ball_keeper_standing_catchcenteroffset_z( "sv_ball_keeper_standing_catchcenteroffset_z", "50", FCVAR_NOTIFY );

ConVar sv_ball_keeper_forwarddive_shortsidereach( "sv_ball_keeper_forwarddive_shortsidereach", "50", FCVAR_NOTIFY );
ConVar sv_ball_keeper_forwarddive_longsidereach( "sv_ball_keeper_forwarddive_longsidereach", "100", FCVAR_NOTIFY );
ConVar sv_ball_keeper_forwarddive_longsidereach_opposite( "sv_ball_keeper_forwarddive_longsidereach_opposite", "40", FCVAR_NOTIFY );
ConVar sv_ball_keeper_forwarddive_zstart( "sv_ball_keeper_forwarddive_zstart", "-50", FCVAR_NOTIFY );
ConVar sv_ball_keeper_forwarddive_zend( "sv_ball_keeper_forwarddive_zend", "40", FCVAR_NOTIFY );
ConVar sv_ball_keeper_forwarddive_catchcoeff( "sv_ball_keeper_forwarddive_catchcoeff", "0.75", FCVAR_NOTIFY );

ConVar sv_ball_keeper_sidedive_shortsidereach( "sv_ball_keeper_sidedive_shortsidereach", "50", FCVAR_NOTIFY );
ConVar sv_ball_keeper_sidedive_longsidereach( "sv_ball_keeper_sidedive_longsidereach", "60", FCVAR_NOTIFY );
ConVar sv_ball_keeper_sidedive_longsidereach_opposite( "sv_ball_keeper_sidedive_longsidereach_opposite", "50", FCVAR_NOTIFY );
ConVar sv_ball_keeper_sidedive_zstart( "sv_ball_keeper_sidedive_zstart", "-30", FCVAR_NOTIFY );
ConVar sv_ball_keeper_sidedive_zend( "sv_ball_keeper_sidedive_zend", "70", FCVAR_NOTIFY );
ConVar sv_ball_keeper_sidedive_catchcenteroffset_side( "sv_ball_keeper_sidedive_catchcenteroffset_side", "0", FCVAR_NOTIFY );
ConVar sv_ball_keeper_sidedive_catchcenteroffset_z( "sv_ball_keeper_sidedive_catchcenteroffset_z", "40", FCVAR_NOTIFY );

ConVar sv_ball_keeper_punch_maxyawangle( "sv_ball_keeper_punch_maxyawangle", "130", FCVAR_NOTIFY );
ConVar sv_ball_keeper_punch_maxpitchangle( "sv_ball_keeper_punch_maxpitchangle", "130", FCVAR_NOTIFY );
ConVar sv_ball_keeper_punch_pitchoffset( "sv_ball_keeper_punch_pitchoffset", "0", FCVAR_NOTIFY );
ConVar sv_ball_keeper_punch_shortsidecoeff( "sv_ball_keeper_punch_shortsidecoeff", "0.5", FCVAR_NOTIFY );
ConVar sv_ball_keeper_punch_minstrength( "sv_ball_keeper_punch_minstrength", "0", FCVAR_NOTIFY );
ConVar sv_ball_keeper_punch_minpitchangle( "sv_ball_keeper_punch_minpitchangle", "180", FCVAR_NOTIFY );

ConVar sv_ball_keeperpunchupstrength("sv_ball_keeperpunchupstrength", "500", FCVAR_NOTIFY);
ConVar sv_ball_keeperdeflectioncoeff("sv_ball_keeperdeflectioncoeff", "0.66", FCVAR_NOTIFY);

ConVar sv_ball_shotdelay_setpiece("sv_ball_shotdelay_setpiece", "0.5", FCVAR_NOTIFY);
ConVar sv_ball_shotdelay_global_coeff("sv_ball_shotdelay_global_coeff", "0.5", FCVAR_NOTIFY);
ConVar sv_ball_keepercatchdelay_sidedive_global_coeff("sv_ball_keepercatchdelay_sidedive_global_coeff", "0.75", FCVAR_NOTIFY);
ConVar sv_ball_keepercatchdelay_forwarddive_global_coeff("sv_ball_keepercatchdelay_forwarddive_global_coeff", "0.75", FCVAR_NOTIFY);
ConVar sv_ball_keepercatchdelay_standing_global_coeff("sv_ball_keepercatchdelay_standing_global_coeff", "0.5", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_mindelay("sv_ball_dynamicshotdelay_mindelay", "0.2", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_maxdelay("sv_ball_dynamicshotdelay_maxdelay", "1.0", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_minshotstrength("sv_ball_dynamicshotdelay_minshotstrength", "400", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_maxshotstrength("sv_ball_dynamicshotdelay_maxshotstrength", "1500", FCVAR_NOTIFY);
ConVar sv_ball_dynamicbounce_enabled("sv_ball_dynamicbouncedelay_enabled", "1", FCVAR_NOTIFY);

ConVar sv_ball_bestshotangle("sv_ball_bestshotangle", "-20", FCVAR_NOTIFY);

ConVar sv_ball_pitchdown_exponent_normalshot("sv_ball_pitchdown_exponent_normalshot", "2.5", FCVAR_NOTIFY);
ConVar sv_ball_fixedpitchdowncoeff_normalshot("sv_ball_fixedpitchdowncoeff_normalshot", "0.35", FCVAR_NOTIFY);
ConVar sv_ball_pitchdown_exponent_nonnormalshot("sv_ball_pitchdown_exponent_nonnormalshot", "2", FCVAR_NOTIFY);
ConVar sv_ball_fixedpitchdowncoeff_nonnormalshot("sv_ball_fixedpitchdowncoeff_nonnormalshot", "0.37", FCVAR_NOTIFY);
ConVar sv_ball_pitchup_exponent_normalshot("sv_ball_pitchup_exponent_normalshot", "3", FCVAR_NOTIFY);
ConVar sv_ball_fixedpitchupcoeff_normalshot("sv_ball_fixedpitchupcoeff_normalshot", "0.35", FCVAR_NOTIFY);
ConVar sv_ball_pitchup_exponent_nonnormalshot("sv_ball_pitchup_exponent_nonnormalshot", "3", FCVAR_NOTIFY);
ConVar sv_ball_fixedpitchupcoeff_nonnormalshot("sv_ball_fixedpitchupcoeff_nonnormalshot", "0.35", FCVAR_NOTIFY);

ConVar sv_ball_bestbackspinangle_start("sv_ball_bestbackspinangle_start", "-50", FCVAR_NOTIFY);
ConVar sv_ball_bestbackspinangle_end("sv_ball_bestbackspinangle_end", "-25", FCVAR_NOTIFY);

ConVar sv_ball_besttopspinangle_start("sv_ball_besttopspinangle_start", "-15", FCVAR_NOTIFY);
ConVar sv_ball_besttopspinangle_end("sv_ball_besttopspinangle_end", "0", FCVAR_NOTIFY);

ConVar sv_ball_keepercatchspeed("sv_ball_keepercatchspeed", "1000", FCVAR_NOTIFY);
ConVar sv_ball_keeperpickupangle("sv_ball_keeperpickupangle", "-90", FCVAR_NOTIFY);

ConVar sv_ball_normalshot_strength("sv_ball_normalshot_strength", "900", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_powershot_strength("sv_ball_powershot_strength", "1100", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_chargedshot_minstrength("sv_ball_chargedshot_minstrength", "800", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_chargedshot_maxstrength("sv_ball_chargedshot_maxstrength", "1500", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);

ConVar sv_ball_powerthrow_strength("sv_ball_powerthrow_strength", "800", FCVAR_NOTIFY);
ConVar sv_ball_chargedthrow_minstrength("sv_ball_chargedthrow_minstrength", "500", FCVAR_NOTIFY);
ConVar sv_ball_chargedthrow_maxstrength("sv_ball_chargedthrow_maxstrength", "1100", FCVAR_NOTIFY);

ConVar sv_ball_normalheader_strength("sv_ball_normalheader_strength", "450", FCVAR_NOTIFY); 
ConVar sv_ball_powerheader_strength("sv_ball_powerheader_strength", "700", FCVAR_NOTIFY); 
ConVar sv_ball_chargedheader_minstrength("sv_ball_chargedheader_minstrength", "500", FCVAR_NOTIFY); 
ConVar sv_ball_chargedheader_maxstrength("sv_ball_chargedheader_maxstrength", "1000", FCVAR_NOTIFY); 

ConVar sv_ball_powerdivingheader_strength("sv_ball_powerdivingheader_strength", "1000", FCVAR_NOTIFY); 
ConVar sv_ball_chargeddivingheader_minstrength("sv_ball_chargeddivingheader_minstrength", "600", FCVAR_NOTIFY); 
ConVar sv_ball_chargeddivingheader_maxstrength("sv_ball_chargeddivingheader_maxstrength", "1200", FCVAR_NOTIFY);

ConVar sv_ball_header_mindelay("sv_ball_header_mindelay", "0.75", FCVAR_NOTIFY); 

ConVar sv_ball_slide_strength("sv_ball_slide_strength", "1000", FCVAR_NOTIFY); 

ConVar sv_ball_goalkick_speedcoeff("sv_ball_goalkick_speedcoeff", "1.15", FCVAR_NOTIFY);
ConVar sv_ball_freekick_speedcoeff("sv_ball_freekick_speedcoeff", "1.10", FCVAR_NOTIFY);
ConVar sv_ball_volleyshot_speedcoeff("sv_ball_volleyshot_speedcoeff", "1.125", FCVAR_NOTIFY);

ConVar sv_ball_keepershot_minangle("sv_ball_keepershot_minangle", "-5", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);

ConVar sv_ball_groundshot_minangle("sv_ball_groundshot_minangle", "-7", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_volleyshot_minangle("sv_ball_volleyshot_minangle", "0", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_throwin_minangle("sv_ball_throwin_minangle", "-5", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_throwin_minstrength("sv_ball_throwin_minstrength", "300", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_autopass_minstrength("sv_ball_autopass_minstrength", "500", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_autopass_maxstrength("sv_ball_autopass_maxstrength", "1600", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_autopass_coeff("sv_ball_autopass_coeff", "1", FCVAR_NOTIFY);
ConVar sv_ball_volleyshot_spincoeff("sv_ball_volleyshot_spincoeff", "1.25", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_rainbowflick_spincoeff("sv_ball_rainbowflick_spincoeff", "0.75", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_rainbowflick_angle("sv_ball_rainbowflick_angle", "-30", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_rainbowflick_dist("sv_ball_rainbowflick_dist", "-10", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_header_spincoeff("sv_ball_header_spincoeff", "0.66", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_doubletouchfouls("sv_ball_doubletouchfouls", "1", FCVAR_NOTIFY);

ConVar sv_ball_timelimit_setpiece("sv_ball_timelimit_setpiece", "15", FCVAR_NOTIFY);
ConVar sv_ball_timelimit_remotecontrolled("sv_ball_timelimit_remotecontrolled", "15", FCVAR_NOTIFY);

ConVar sv_ball_setpiece_close_time("sv_ball_setpiece_close_time", "0.75", FCVAR_NOTIFY);
ConVar sv_ball_setpiece_close_dist( "sv_ball_setpiece_close_dist", "75", FCVAR_NOTIFY );

ConVar sv_ball_statetransition_activationdelay_short("sv_ball_statetransition_activationdelay_short", "0.1", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_statetransition_activationdelay_normal("sv_ball_statetransition_activationdelay_normal", "1.25", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_statetransition_activationdelay_long("sv_ball_statetransition_activationdelay_long", "2.0", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_statetransition_messagedelay_normal("sv_ball_statetransition_messagedelay_normal", "0.5", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_statetransition_messagedelay_short("sv_ball_statetransition_messagedelay_short", "0.1", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);

ConVar sv_ball_goalcelebduration("sv_ball_goalcelebduration", "5.0", FCVAR_NOTIFY);
ConVar sv_ball_highlightsdelay_intermissions("sv_ball_highlightsdelay_intermissions", "5.0", FCVAR_NOTIFY);
ConVar sv_ball_highlightsdelay_cooldown("sv_ball_highlightsdelay_cooldown", "30.0", FCVAR_NOTIFY);
ConVar sv_ball_thinkinterval("sv_ball_thinkinterval", "0", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_chestdrop_strength("sv_ball_chestdrop_strength", "100", FCVAR_NOTIFY); 
ConVar sv_ball_chestdrop_angle("sv_ball_chestdrop_angle", "80", FCVAR_NOTIFY); 
ConVar sv_ball_minshotstrength("sv_ball_minshotstrength", "200", FCVAR_NOTIFY);  
ConVar sv_ball_minspeed_passive("sv_ball_minspeed_passive", "1000", FCVAR_NOTIFY); 
ConVar sv_ball_minspeed_bounce("sv_ball_minspeed_bounce", "500", FCVAR_NOTIFY);
ConVar sv_ball_bounce_strength("sv_ball_bounce_strength", "500", FCVAR_NOTIFY);
ConVar sv_ball_player_yellow_red_card_duration("sv_ball_player_yellow_red_card_duration", "7.5", FCVAR_NOTIFY);
ConVar sv_ball_player_red_card_duration("sv_ball_player_red_card_duration", "15", FCVAR_NOTIFY);

ConVar sv_ball_bodypos_feet_start("sv_ball_bodypos_feet_start", "-50", FCVAR_NOTIFY);
ConVar sv_ball_bodypos_hip_start("sv_ball_bodypos_hip_start", "15", FCVAR_NOTIFY);
ConVar sv_ball_bodypos_head_start("sv_ball_bodypos_head_start", "50", FCVAR_NOTIFY);
ConVar sv_ball_bodypos_head_end("sv_ball_bodypos_head_end", "85", FCVAR_NOTIFY);
ConVar sv_ball_bodypos_keeperarms_end("sv_ball_bodypos_keeperarms_end", "105", FCVAR_NOTIFY);

ConVar sv_ball_bodypos_keeperhands("sv_ball_bodypos_keeperhands", "40", FCVAR_NOTIFY);

ConVar sv_ball_bodypos_collision_start("sv_ball_bodypos_collision_start", "15", FCVAR_NOTIFY);
ConVar sv_ball_bodypos_collision_end("sv_ball_bodypos_collision_end", "75", FCVAR_NOTIFY);

ConVar sv_ball_bodypos_deflection_start("sv_ball_bodypos_deflection_start", "0", FCVAR_NOTIFY);
ConVar sv_ball_bodypos_deflection_end("sv_ball_bodypos_deflection_end", "80", FCVAR_NOTIFY);

ConVar sv_ball_yellowcardballdist_forward("sv_ball_yellowcardballdist_forward", "50", FCVAR_NOTIFY);
ConVar sv_ball_yellowcardballdist_backward("sv_ball_yellowcardballdist_backward", "25", FCVAR_NOTIFY);
ConVar sv_ball_goalreplay_count("sv_ball_goalreplay_count", "2", FCVAR_NOTIFY);
ConVar sv_ball_goalreplay_delay("sv_ball_goalreplay_delay", "1", FCVAR_NOTIFY);
ConVar sv_ball_deflectioncoeff("sv_ball_deflectioncoeff", "0.5", FCVAR_NOTIFY);
ConVar sv_ball_collisioncoeff("sv_ball_collisioncoeff", "0.75", FCVAR_NOTIFY);
ConVar sv_ball_update_physics("sv_ball_update_physics", "0", FCVAR_NOTIFY);

ConVar sv_ball_stats_pass_mindist("sv_ball_stats_pass_mindist", "300", FCVAR_NOTIFY);
ConVar sv_ball_stats_clearance_minspeed("sv_ball_stats_clearance_minspeed", "800", FCVAR_NOTIFY);
ConVar sv_ball_stats_shot_mindist("sv_ball_stats_shot_mindist", "300", FCVAR_NOTIFY);
ConVar sv_ball_stats_save_minspeed("sv_ball_stats_save_minspeed", "800", FCVAR_NOTIFY);
ConVar sv_ball_stats_assist_maxtime("sv_ball_stats_assist_maxtime", "8", FCVAR_NOTIFY);

ConVar sv_ball_velocity_coeff("sv_ball_velocity_coeff", "0.9", FCVAR_NOTIFY);

ConVar sv_ball_freekickdist_owngoal("sv_ball_freekickdist_owngoal", "850", FCVAR_NOTIFY);
ConVar sv_ball_freekickdist_opponentgoal("sv_ball_freekickdist_opponentgoal", "1300", FCVAR_NOTIFY);
ConVar sv_ball_freekickangle_opponentgoal("sv_ball_freekickangle_opponentgoal", "60", FCVAR_NOTIFY);
ConVar sv_ball_closetogoaldist("sv_ball_closetogoaldist", "1300", FCVAR_NOTIFY);

ConVar sv_ball_nonnormalshotsblocktime_freekick("sv_ball_nonnormalshotsblocktime_freekick", "4.0", FCVAR_NOTIFY);
ConVar sv_ball_nonnormalshotsblocktime_corner("sv_ball_nonnormalshotsblocktime_corner", "4.0", FCVAR_NOTIFY);
ConVar sv_ball_shotsblocktime_penalty("sv_ball_shotsblocktime_penalty", "4.0", FCVAR_NOTIFY);

ConVar sv_ball_maxcheckdist("sv_ball_maxcheckdist", "200", FCVAR_NOTIFY);

ConVar sv_ball_freecamshot_maxangle("sv_ball_freecamshot_maxangle", "60", FCVAR_NOTIFY);
ConVar sv_ball_heelshot_strength("sv_ball_heelshot_strength", "800", FCVAR_NOTIFY);

ConVar sv_ball_offsidedist("sv_ball_offsidedist", "200", FCVAR_NOTIFY);

ConVar sv_ball_turnovertime("sv_ball_turnovertime", "1.0", FCVAR_NOTIFY);

ConVar sv_ball_keeperautopunch_limit("sv_ball_keeperautopunch_limit", "30", FCVAR_NOTIFY);
ConVar sv_ball_keeperautopunch_pitch("sv_ball_keeperautopunch_pitch", "-45", FCVAR_NOTIFY);
ConVar sv_ball_keeperautopunch_yaw("sv_ball_keeperautopunch_yaw", "45", FCVAR_NOTIFY);

ConVar sv_ball_keepercatchdelay_poscoeffmin("sv_ball_keepercatchdelay_poscoeffmin", "0.5", FCVAR_NOTIFY);

ConVar sv_ball_dribbling_mass("sv_ball_dribbling_mass", "75", FCVAR_NOTIFY);
ConVar sv_ball_dribbling_collisioncoeff("sv_ball_dribbling_collisioncoeff", "1.25", FCVAR_NOTIFY);


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
	SendPropInt(SENDINFO(m_bNonnormalshotsBlocked)),
	SendPropInt(SENDINFO(m_bShotsBlocked)),
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
	m_flStateTimelimit = -1;
	m_pPl = NULL;
	m_pHoldingPlayer = NULL;
	m_ePenaltyState = PENALTY_NONE;
	m_bSetNewPos = false;
	m_bSetNewVel = false;
	m_bSetNewRot = false;
	m_bHasQueuedState = false;
	m_pHoldingPlayer = NULL;
	m_pPossessingPl = NULL;
	m_nPossessingTeam = TEAM_INVALID;
	m_flPossessionStart = -1;
	m_pTurnoverPlayer = NULL;
	m_flLastMatchEventSetTime = -1;
	m_bNonnormalshotsBlocked = false;
	m_bShotsBlocked = false;
	m_bHitThePost = false;
	m_bLastContactWasTouch = false;
	memset(m_szSkinName.GetForModify(), 0, sizeof(m_szSkinName));
}

CBall::~CBall()
{
}

bool CBall::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	return !mp_ball_mass_fix_enabled.GetBool();
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
	SetNextThink(gpGlobals->curtime + sv_ball_thinkinterval.GetFloat());

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

	m_flPhysRadius = BALL_PHYS_RADIUS;
	objectparams_t params =	g_IOSPhysDefaultObjectParams;
	params.pGameData = static_cast<void	*>(this);
	params.damping = sv_ball_damping.GetFloat();
	params.mass = sv_ball_mass.GetFloat();
	params.dragCoefficient = sv_ball_dragcoeff.GetFloat();
	params.inertia = sv_ball_inertia.GetFloat();
	params.rotdamping = sv_ball_rotdamping.GetFloat();
	params.rotInertiaLimit = sv_ball_rotinertialimit.GetFloat();
	int	nMaterialIndex = physprops->GetSurfaceIndex(mp_weather.GetInt() == 0 ? "dryball" : (mp_weather.GetInt() == 1 ? "wetball" : "icyball"));
	m_pPhys = physenv->CreateSphereObject( m_flPhysRadius, nMaterialIndex, GetAbsOrigin(), GetAbsAngles(), &params, false );
	if (!m_pPhys)
		return false;

	VPhysicsSetObject( m_pPhys );
	
	SetSolid( SOLID_VPHYSICS );
	SetSolidFlags( FSOLID_NOT_STANDABLE	);
	UTIL_SetSize(this, -Vector(m_flPhysRadius, m_flPhysRadius, m_flPhysRadius), Vector(m_flPhysRadius, m_flPhysRadius, m_flPhysRadius));

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
	Vector magnusDir = worldAngImp.Cross(vel);

	if (vel.Length() > 0)
		vel += magnusDir * 1e-6 * sv_ball_curve.GetFloat() * gpGlobals->frametime;

	VPhysicsGetObject()->SetVelocity(&vel, &angImp);

	BaseClass::VPhysicsUpdate(pPhysics);

	m_bSetNewPos = false;
	m_bSetNewVel = false;
	m_bSetNewRot = false;
}


void CBall::VPhysicsCollision( int index, gamevcollisionevent_t	*pEvent	)
{
	Vector preVelocity = pEvent->preVelocity[index];
	Vector preDir = preVelocity;
	float flSpeed =	VectorNormalize(preDir);
	int surfaceProps = pEvent->surfaceProps[!index];

	//IOS goal post hacks!!
	if (surfaceProps == 84 && flSpeed > 300.0f)
	{
		CSDKPlayer *pLastPl = LastPl(true);
		if (pLastPl && Sign(m_vPos.y - SDKGameRules()->m_vKickOff.GetY()) == pLastPl->GetTeam()->m_nForward) // Check if it's the opponent's goal
		{
			m_bHitThePost = true;
			//pLastPl->AddShot();
			//pLastPl->AddShotOnGoal();
		}

		EmitSound("Ball.Post");
	}
	else
	{
		//if ball is moving fast when we hit something play a sound
		if (flSpeed > 500.0f)
		{
			EmitSound("Ball.Touch");
		}
	}
	
	//iosgoalnets 85=iosgoalnets, 30=concrete!!! TEMP!!! until pricey changes nets surfaceprop!
	if ((surfaceProps == 85 /*|| surfaceProps == 30*/) && flSpeed > 300.0f)
	{
		EmitSound("Ball.Net");
	}
}

CSDKPlayer *CBall::FindNearestPlayer(int team /*= TEAM_INVALID*/, int posFlags /*= FL_POS_FIELD*/, bool checkIfShooting /*= false*/, int ignoredPlayerBits /*= 0*/, float radius /*= -1*/)
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

			if ((posFlags & FL_POS_FIELD) != 0 && !((1 << posType) & (g_nPosDefense + g_nPosMidfield + g_nPosAttack)))
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

		if (team != TEAM_INVALID && pPlayer->GetTeamNumber() != team)
			continue;

		if (checkIfShooting && (!pPlayer->IsShooting() || !pPlayer->CanShoot()))
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
	//if (!pNearest && team != TEAM_INVALID)
	//	pNearest = FindNearestPlayer();

	return pNearest;
}

void CBall::SetPos(const Vector &pos, bool teleport /*= true*/)
{
	m_vPos = Vector(pos.x, pos.y, pos.z + m_flPhysRadius);
	m_vVel = vec3_origin;
	m_vRot = vec3_origin;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocityInstantaneous(&vec3_origin, &vec3_origin);
	m_pPhys->SetPosition(m_vPos, m_aAng, teleport);
	m_pPhys->EnableMotion(false);
	m_bSetNewPos = true;
}

void CBall::SetAng(const QAngle &ang)
{
	m_aAng = ang;
	m_pPhys->SetPosition(m_vPos, m_aAng, false);
}

void CBall::SetVel(Vector vel, float spinCoeff, int spinFlags, body_part_t bodyPart, bool isDeflection, bool markOffsidePlayers, bool ensureMinShotStrength, float nextShotMinDelay /*= 0*/)
{
	Vector oldVel = m_vVel;

	m_vVel = vel * sv_ball_velocity_coeff.GetFloat();

	if (!isDeflection && mp_player_pressure_enabled.GetBool() && m_pPl->IsPressured())
	{
		float pitch = g_IOSRand.RandomFloat(-mp_player_pressure_effect_pitch.GetFloat(), mp_player_pressure_effect_pitch.GetFloat());
		float yaw = g_IOSRand.RandomFloat(-mp_player_pressure_effect_yaw.GetFloat(), mp_player_pressure_effect_yaw.GetFloat());
		float roll = g_IOSRand.RandomFloat(-mp_player_pressure_effect_roll.GetFloat(), mp_player_pressure_effect_roll.GetFloat());
		VectorRotate(m_vVel, QAngle(pitch, yaw, roll), m_vVel);
	}

	float length = m_vVel.Length();
	m_vVel.NormalizeInPlace();

	if (ensureMinShotStrength)
		length = max(length, sv_ball_minshotstrength.GetInt());

	length = min(length, sv_ball_chargedshot_maxstrength.GetInt());
	m_vVel *= length;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocity(&m_vVel, &m_vRot);
	m_bSetNewVel = true;

	if (spinCoeff != -1)
	{
		SetRot(CalcSpin(spinCoeff, spinFlags));
	}

	float dynamicDelay = RemapValClamped(m_vVel.Length(), sv_ball_dynamicshotdelay_minshotstrength.GetInt(), sv_ball_dynamicshotdelay_maxshotstrength.GetInt(), sv_ball_dynamicshotdelay_mindelay.GetFloat(), sv_ball_dynamicshotdelay_maxdelay.GetFloat());
	
	m_flGlobalLastShot = gpGlobals->curtime;
	m_flGlobalDynamicShotDelay = dynamicDelay;

	if (isDeflection)
	{
		m_pPl->m_flNextShot = m_flGlobalLastShot + m_flGlobalDynamicShotDelay * sv_ball_shotdelay_global_coeff.GetFloat();
	}
	else
	{
		float delay;

		if (State_Get() == BALL_STATE_NORMAL)
			delay = max(dynamicDelay, nextShotMinDelay);
		else
			delay = sv_ball_shotdelay_setpiece.GetFloat();

		m_pPl->m_flNextShot = gpGlobals->curtime + delay;
	}

	Touched(m_pPl, !isDeflection, bodyPart, oldVel);

	
	if (markOffsidePlayers)
		MarkOffsidePlayers();
}

void CBall::SetRot(AngularImpulse rot)
{
	m_vRot = rot;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocity(&m_vVel, &m_vRot);
	m_bSetNewRot = true;
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

void CBall::FindStatePlayer(ball_state_t ballState /*= BALL_STATE_NONE*/)
{
	if (ballState == BALL_STATE_NONE)
		ballState = State_Get();

	switch (ballState)
	{
	case BALL_STATE_THROWIN:
		FindNearestPlayer(LastOppTeam(false));
		break;
	case BALL_STATE_GOALKICK:
		break;
	}
}

void CBall::State_KEEPERHANDS_Enter()
{
	SetPos(m_vPos);
	// Don't ignore triggers when setting the new ball position
	m_bSetNewPos = false;
}

void CBall::State_KEEPERHANDS_Think()
{
	if (m_eNextState == BALL_STATE_GOAL)
		return;

	if (!CSDKPlayer::IsOnField(m_pPl, m_nInPenBoxOfTeam))
	{
		m_pPl = NULL;

		m_pPl = FindNearestPlayer(m_nInPenBoxOfTeam, FL_POS_KEEPER);

		if (!m_pPl)
			return State_Transition(BALL_STATE_NORMAL);

		if (!SDKGameRules()->IsIntermissionState() && !m_bHasQueuedState && SDKGameRules()->State_Get() != MATCH_PERIOD_PENALTIES)
		{
			SDKGameRules()->EnableShield(SHIELD_KEEPERHANDS, m_pPl->GetTeamNumber(), m_vPos);
			m_pPl->m_bIsAtTargetPos = true;
		}

		m_pPl->SetShotButtonsReleased(false);
		m_pHoldingPlayer = m_pPl;
		m_pPl->m_pHoldingBall = this;
		//m_pPl->m_nBody = MODEL_KEEPER_AND_BALL;
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_CARRY);
		//m_pPl->m_nSkin = m_pPl->m_nBaseSkin + m_nSkin;
		//SetEffects(EF_NODRAW);
		EnablePlayerCollisions(false);
		m_flStateTimelimit = -1;
		PlayersAtTargetPos();
	}

	if (!SDKGameRules()->IsIntermissionState() && !m_bHasQueuedState && SDKGameRules()->State_Get() != MATCH_PERIOD_PENALTIES)
	{
		if (m_flStateTimelimit == -1)
		{
			m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit_setpiece.GetFloat();
		}
	}

	UpdateCarrier();

	//SetPos(Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + sv_ball_bodypos_keeperhands.GetFloat()) + m_vPlForward2D * 18);
	Vector handPos;
	QAngle handAng;
	m_pPl->GetAttachment("keeperballrighthand", handPos, handAng);
	handPos.z -= BALL_PHYS_RADIUS;
	SetPos(handPos, false);
	SetAng(handAng);

	// Don't ignore triggers when setting the new ball position
	m_bSetNewPos = false;

	Vector min = GetGlobalTeam(m_pPl->GetTeamNumber())->m_vPenBoxMin + m_flPhysRadius;
	Vector max = GetGlobalTeam(m_pPl->GetTeamNumber())->m_vPenBoxMax - m_flPhysRadius;

	// Ball outside the penalty box
	if (m_vPos.x < min.x || m_vPos.y < min.y || m_vPos.x > max.x || m_vPos.y > max.y)
	{
		Vector dir, pos;
		float vel;

		// Throw the ball towards the kick-off spot instead of where the player is looking if the ball is behind the goal line
		if (m_pPl->GetTeam()->m_nForward == 1 && m_vPos.y < min.y || m_pPl->GetTeam()->m_nForward == -1 && m_vPos.y > max.y)
		{
			QAngle ang = QAngle(g_IOSRand.RandomFloat(-55, -40), m_pPl->GetTeam()->m_nForward * 90 - m_pPl->GetTeam()->m_nForward * Sign(m_vPos.x - SDKGameRules()->m_vKickOff.GetX()) * g_IOSRand.RandomFloat(15, 25), 0);
			AngleVectors(ang, &dir);
			vel = g_IOSRand.RandomFloat(700, 800);
			pos = Vector(m_vPos.x, (m_pPl->GetTeam()->m_nForward == 1 ? min.y : max.y) + m_pPl->GetTeam()->m_nForward * 36, m_vPos.z);
		}
		else
		{
			dir = m_vPlForward2D;
			vel = 300;
			pos = Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + sv_ball_bodypos_keeperhands.GetFloat()) + m_vPlForward2D * 36;
		}

		RemoveAllTouches();
		SetPos(pos);
		m_bSetNewPos = false;
		SetVel(dir * vel, 0, 0, BODY_PART_KEEPERHANDS, false, true, true);

		return State_Transition(BALL_STATE_NORMAL);
	}

	Vector vel;

	if (m_pPl->ShotButtonsReleased() && (m_pPl->IsPowershooting() || m_pPl->IsChargedshooting()) && m_pPl->CanShoot())
	{
		float spin;

		if (m_pPl->IsPowershooting())
		{
			vel = vec3_origin;
			spin = 0;
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KEEPER_HANDS_THROW);
		}
		else
		{
			QAngle ang = m_aPlAng;
			ang[PITCH] = min(sv_ball_keepershot_minangle.GetFloat(), m_aPlAng[PITCH]);
			Vector dir;
			AngleVectors(ang, &dir);
			vel = dir * GetChargedshotStrength(GetPitchCoeff(false), sv_ball_chargedshot_minstrength.GetInt(), sv_ball_chargedshot_maxstrength.GetInt());
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KEEPER_HANDS_KICK);
			spin = sv_ball_volleyshot_spincoeff.GetFloat();

			if (vel.Length() > 1000)
				EmitSound("Ball.Kickhard");
			else
				EmitSound("Ball.Kicknormal");
		}

		RemoveAllTouches();
		SetPos(Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + sv_ball_bodypos_keeperhands.GetFloat()) + m_vPlForward2D * 36);
		m_bSetNewPos = false;
		SetVel(vel, spin, FL_SPIN_PERMIT_ALL, BODY_PART_KEEPERHANDS, false, true, true);

		return State_Transition(BALL_STATE_NORMAL);
	}
}

void CBall::State_KEEPERHANDS_Leave(ball_state_t newState)
{
	RemoveFromPlayerHands(m_pPl);
}

// Make sure that all players are walked to the intended positions when setting shields
bool CBall::PlayersAtTargetPos()
{
	bool playersAtTarget = true;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!CSDKPlayer::IsOnField(pPl))
			continue;

		if (!pPl->m_bIsAtTargetPos)
		{
			if (!(pPl->GetFlags() & FL_REMOTECONTROLLED))
			{
				if (mp_shield_liberal_teammates_positioning.GetBool() && m_pCurStateInfo->m_eBallState != BALL_STATE_KICKOFF && m_pCurStateInfo->m_eBallState != BALL_STATE_PENALTY && pPl->GetTeamNumber() == m_pPl->GetTeamNumber())
					pPl->SetPosOutsideBall(pPl->GetLocalOrigin());
				else
					pPl->SetPosOutsideShield(false);
			}

			if (!pPl->m_bIsAtTargetPos)
			{
				if (pPl->m_flRemoteControlledStartTime == -1)
				{
					pPl->m_flRemoteControlledStartTime = gpGlobals->curtime;
					playersAtTarget = false;
				}
				else if (gpGlobals->curtime >= pPl->m_flRemoteControlledStartTime + sv_ball_timelimit_remotecontrolled.GetFloat()) // Player timed out and blocks progress, so move him to specs
					pPl->SetDesiredTeam(TEAM_SPECTATOR, pPl->GetTeamNumber(), 0, true, false, false);
				else
					playersAtTarget = false;
			}
		}
	}

	return playersAtTarget;
}

bool CBall::CanTouchBallXY()
{
	Vector center = m_vPlPos + m_vPlForward2D * sv_ball_standing_reach_shift.GetFloat();

	if (sv_ball_standing_reach_ellipse.GetBool())
	{
		// http://stackoverflow.com/questions/7946187/point-and-ellipse-rotated-position-test-algorithm

		float ang = DEG2RAD(m_aPlAng[YAW] + 90);
		float cosa = cos(ang);
		float sina = sin(ang);
		float dshortd = Square(sv_ball_standing_reach_shortside.GetFloat());
		float dlongd = Square(sv_ball_standing_reach_longside.GetFloat());

		float a = Square(cosa * (m_vPos.x - center.x) + sina * (m_vPos.y - center.y));
		float b = Square(sina * (m_vPos.x - center.x) - cosa * (m_vPos.y - center.y));

		float ellipse = (a / dshortd) + (b / dlongd);

		if (ellipse <= 1)
			return true;
		else
			return false;
	}
	else
	{
		return abs(m_vPlLocalDirToBall.y) <= sv_ball_standing_reach_shortside.GetFloat()
			&& m_vPlLocalDirToBall.x >= -sv_ball_standing_reach_longside.GetFloat() + sv_ball_standing_reach_shift.GetFloat()
			&& m_vPlLocalDirToBall.x <= sv_ball_standing_reach_longside.GetFloat() + sv_ball_standing_reach_shift.GetFloat();
	}
}

bool CBall::IsPlayerClose()
{
	return (m_vPos - m_vPlPos).Length2DSqr() <= pow(sv_ball_setpiece_close_dist.GetFloat(), 2);
}

bool CBall::CheckFoul(bool canShootBall, const Vector &localDirToBall)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++) 
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!CSDKPlayer::IsOnField(pPl))
			continue;

		if (pPl == m_pPl || pPl->GetTeamNumber() == m_pPl->GetTeamNumber())
			continue;

		Vector plPos = pPl->GetLocalOrigin();

		//if (plPos.x < SDKGameRules()->m_vFieldMin.GetX() || plPos.y < SDKGameRules()->m_vFieldMin.GetY() ||
		//	plPos.x > SDKGameRules()->m_vFieldMax.GetX() || plPos.y > SDKGameRules()->m_vFieldMax.GetY())
		//	continue;

		Vector dirToPl = pPl->GetLocalOrigin() - m_vPlPos;
		float distToPl = dirToPl.Length2D();

		Vector localDirToPl;
		VectorIRotate(dirToPl, m_pPl->EntityToWorldTransform(), localDirToPl);

		dirToPl.z = 0;
		dirToPl.NormalizeInPlace();

		// Can't reach the other player
		if (localDirToPl.x < -sv_ball_slidebackwardreach_foul.GetInt()
			|| localDirToPl.x > sv_ball_slideforwardreach_foul.GetInt()
			|| abs(localDirToPl.y) > sv_ball_slidesidereach_foul.GetInt())		
			continue;

		// Can shoot the ball and ball is closer to the player than the opponent
		if (canShootBall && localDirToBall.x <= localDirToPl.x)
			continue;

		// It's a foul

		PlayerAnimEvent_t anim = RAD2DEG(acos(m_vPlForward2D.Dot(pPl->EyeDirection2D()))) <= 90 ? PLAYERANIMEVENT_TACKLED_BACKWARD : PLAYERANIMEVENT_TACKLED_FORWARD;

		pPl->DoAnimationEvent(anim);

		int teammatesCloserToGoalCount = 0;

		bool isCloseToOwnGoal = ((m_vPos - m_pPl->GetTeam()->m_vGoalCenter).Length2D() <= sv_ball_closetogoaldist.GetInt());

		if (isCloseToOwnGoal)
		{
			for (int j = 1; j <= gpGlobals->maxClients; j++) 
			{
				CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(j));

				if (!CSDKPlayer::IsOnField(pPl) || pPl == m_pPl || pPl->GetTeamNumber() != m_pPl->GetTeamNumber())
					continue;

				if ((m_pPl->GetTeam()->m_vGoalCenter - pPl->GetLocalOrigin()).Length2DSqr() < (m_pPl->GetTeam()->m_vGoalCenter - m_vPlPos).Length2DSqr())
					teammatesCloserToGoalCount += 1;
			}
		}

		bool isPenalty = pPl->m_nInPenBoxOfTeam == m_pPl->GetTeamNumber();

		foul_type_t foulType;

		if (isCloseToOwnGoal && teammatesCloserToGoalCount <= 1)
			foulType = FOUL_NORMAL_RED_CARD;
		else if (anim == PLAYERANIMEVENT_TACKLED_FORWARD && localDirToBall.Length2DSqr() >= Sqr(sv_ball_yellowcardballdist_forward.GetFloat()) ||
				 anim == PLAYERANIMEVENT_TACKLED_BACKWARD && localDirToBall.Length2DSqr() >= Sqr(sv_ball_yellowcardballdist_backward.GetFloat()))
			foulType = FOUL_NORMAL_YELLOW_CARD;
		else
			foulType = FOUL_NORMAL_NO_CARD;

		TriggerFoul(foulType, pPl->GetLocalOrigin(), m_pPl, pPl);
		m_pFoulingPl->AddFoul();
		m_pFouledPl->AddFoulSuffered();

		if (m_eFoulType == FOUL_NORMAL_YELLOW_CARD)
			m_pFoulingPl->AddYellowCard();

		if (m_eFoulType == FOUL_NORMAL_YELLOW_CARD && m_pFoulingPl->GetYellowCards() % 2 == 0 || m_eFoulType == FOUL_NORMAL_RED_CARD)
		{
			m_pFoulingPl->AddRedCard();
			int banDuration = 60 * (m_eFoulType == FOUL_NORMAL_YELLOW_CARD ? sv_ball_player_yellow_red_card_duration.GetFloat() : sv_ball_player_red_card_duration.GetFloat());
			int nextJoin = SDKGameRules()->GetMatchDisplayTimeSeconds(false) + banDuration;
			m_pFoulingPl->SetNextCardJoin(nextJoin);

			if (m_pFoulingPl->GetTeamPosType() != POS_GK)
				m_pFoulingPl->GetTeam()->SetPosNextJoinSeconds(m_pFoulingPl->GetTeamPosIndex(), nextJoin);

			ReplayManager()->AddMatchEvent(m_eFoulType == FOUL_NORMAL_YELLOW_CARD ? MATCH_EVENT_SECONDYELLOWCARD : MATCH_EVENT_REDCARD, m_nFoulingTeam, m_pFoulingPl);
		}
		else if (m_eFoulType == FOUL_NORMAL_YELLOW_CARD)
		{
			ReplayManager()->AddMatchEvent(MATCH_EVENT_YELLOWCARD, m_nFoulingTeam, m_pFoulingPl);
		}

		if (isPenalty)
			State_Transition(BALL_STATE_PENALTY, sv_ball_statetransition_activationdelay_long.GetFloat());
		else
			State_Transition(BALL_STATE_FREEKICK, sv_ball_statetransition_activationdelay_long.GetFloat());

		return true;
	}

	return false;
}

void CBall::TriggerFoul(foul_type_t type, Vector pos, CSDKPlayer *pFoulingPl, CSDKPlayer *pFouledPl /*= NULL*/)
{
	m_eFoulType = type;
	m_pFoulingPl = pFoulingPl;
	m_pFouledPl = pFouledPl;
	m_nFoulingTeam = pFoulingPl->GetTeamNumber();
	m_nFouledTeam = pFoulingPl->GetOppTeamNumber();
	m_vFoulPos.x = clamp(pos.x, SDKGameRules()->m_vFieldMin.GetX() + 2 * m_flPhysRadius, SDKGameRules()->m_vFieldMax.GetX() - 2 * m_flPhysRadius);
	m_vFoulPos.y = clamp(pos.y, SDKGameRules()->m_vFieldMin.GetY() + 2 * m_flPhysRadius, SDKGameRules()->m_vFieldMax.GetY() - 2 * m_flPhysRadius);
	m_vFoulPos.z = SDKGameRules()->m_vKickOff.GetZ();

	// Move the ball to the edge of the penalty box if the foul happened inside. This will probably only be relevant for double touch fouls.

	Vector min = GetGlobalTeam(m_nFoulingTeam)->m_vPenBoxMin - m_flPhysRadius;
	Vector max = GetGlobalTeam(m_nFoulingTeam)->m_vPenBoxMax + m_flPhysRadius;

	// Ball inside the penalty box
	if (m_vFoulPos.x > min.x && m_vFoulPos.x < max.x)
	{
		if (GetGlobalTeam(m_nFoulingTeam)->m_nForward == 1 && m_vFoulPos.y < max.y)
			m_vFoulPos.y = max.y;
		else if (GetGlobalTeam(m_nFoulingTeam)->m_nForward == -1 && m_vFoulPos.y > min.y)
			m_vFoulPos.y = min.y;
	}
}

void CBall::HandleFoul()
{
	if (CSDKPlayer::IsOnField(m_pFoulingPl))
	{
		if (m_eFoulType == FOUL_NORMAL_YELLOW_CARD && m_pFoulingPl->GetYellowCards() % 2 == 0 || m_eFoulType == FOUL_NORMAL_RED_CARD)
		{
			int team = m_pFoulingPl->GetTeamNumber();
			int posIndex = m_pFoulingPl->GetTeamPosIndex();
			int posType = m_pFoulingPl->GetTeamPosType();
			m_pFoulingPl->SetDesiredTeam(TEAM_SPECTATOR, m_pFoulingPl->GetTeamNumber(), 0, true, false, false);

			if (posType == POS_GK)
			{
				for (int i = 1; i <= gpGlobals->maxClients; i++)
				{
					CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));
					if (!CSDKPlayer::IsOnField(pPl) || pPl == m_pFoulingPl || pPl->GetTeamNumber() != team)
						continue;

					pPl->SetDesiredTeam(team, team, posIndex, true, false, false);
					break;
				}
			}
		}
	}
}

bool CBall::IsInCollisionRange(bool isDeflection)
{
	Vector dirToBall = m_vPos - m_vPlPos;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);
	float zDist = dirToBall.z;
	float xyDist = dirToBall.Length2D();

	if (m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_SLIDE || m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_DIVINGHEADER)
	{
		float padding = isDeflection ? 0 : -15;

		return zDist < sv_ball_slidezend.GetFloat() + padding
			&& zDist >= sv_ball_slidezstart.GetFloat() - padding
			&& localDirToBall.x >= -sv_ball_slideforwardreach_ball.GetFloat() - padding
			&& localDirToBall.x <= sv_ball_slideforwardreach_ball.GetFloat() + padding
			&& abs(localDirToBall.y) <= sv_ball_slidesidereach_ball.GetFloat() + padding;
	}
	else
	{
		if (isDeflection)
			return zDist >= sv_ball_bodypos_deflection_start.GetFloat() && zDist < sv_ball_bodypos_deflection_end.GetFloat() && xyDist <= sv_ball_deflectionradius.GetFloat();
		else
			return zDist >= sv_ball_bodypos_collision_start.GetFloat() && zDist < sv_ball_bodypos_collision_end.GetFloat() && xyDist <= sv_ball_collisionradius.GetFloat();
	}

	return false;
}

extern ConVar sv_player_mass;

bool CBall::DoBodyPartAction()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);
	float zDist = dirToBall.z;
	float xyDist = dirToBall.Length2D();
	bool canLegallyCatch = false;

	if (m_pPl->IsNormalshooting()
		&& m_pPl->CanShoot()
		&& m_pPl->GetTeamPosType() == POS_GK
		&& m_nInPenBoxOfTeam == m_pPl->GetTeamNumber()
		&& !m_pPl->m_pHoldingBall
		&& m_pPl->m_nInPenBoxOfTeam == m_pPl->GetTeamNumber())
	{
		if (SDKGameRules()->IsIntermissionState() || SDKGameRules()->State_Get() == MATCH_PERIOD_PENALTIES)
		{
			canLegallyCatch = true;
		}
		else
		{
			// Skip the keeper
			BallTouchInfo *pLastTouch = LastInfo(false, m_pPl);
			BallTouchInfo *pLastShot = LastInfo(true, m_pPl);

			if (pLastTouch && pLastShot)
			{
				// Can catch if opponent has touched or shot the ball
				if (pLastTouch->m_nTeam != m_pPl->GetTeamNumber() || pLastShot->m_nTeam != m_pPl->GetTeamNumber())
				{
					canLegallyCatch = true;
				}
				else
				{
					if (pLastShot->m_eBodyPart == BODY_PART_HEAD || pLastShot->m_eBodyPart == BODY_PART_CHEST)
					{
						// Only allow the keeper to pick up the ball if the shot or touch before the last header or chest action was by an opponent
						for (int i = m_Touches.Count() - 1; i >= 1; i--)
						{
							if (m_Touches[i]->m_bIsShot)
							{
								for (int j = i - 1; j >= 0; j--)
								{
									if (m_Touches[j]->m_bIsShot)
									{
										if (m_Touches[j]->m_nTeam != pLastTouch->m_nTeam)
											canLegallyCatch = true;

										break;
									}
								}

								break;
							}
						}
					}
				}
			}
		}

		// The keeper wants and is allowed to catch the ball, so either he's close enough to do so or we can jump out here
		if (canLegallyCatch)
			return CheckKeeperCatch();
	}

	if (CheckCollision())
		return false;

	if (!m_pPl->IsShooting() || !m_pPl->CanShoot() || gpGlobals->curtime < m_flGlobalLastShot + m_flGlobalDynamicShotDelay * sv_ball_shotdelay_global_coeff.GetFloat())
		return false;

	if (m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_SLIDE)
		return DoSlideAction();

	if (zDist >= sv_ball_bodypos_feet_start.GetFloat()
		&& zDist < sv_ball_bodypos_hip_start.GetFloat()
		&& CanTouchBallXY())
	{
		return DoGroundShot(true);
	}

	if (zDist >= sv_ball_bodypos_hip_start.GetFloat() && zDist < sv_ball_bodypos_head_start.GetFloat() && CanTouchBallXY())
		return DoVolleyShot();

	if (zDist >= sv_ball_bodypos_head_start.GetFloat() && zDist < sv_ball_bodypos_head_end.GetFloat() && CanTouchBallXY())
		return DoHeader();

	return false;
}

bool CBall::CheckCollision()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);
	bool collide = false;
	float collisionCoeff;
	float ballMass;

	if ((!m_pPl->IsShooting() || !m_pPl->CanShoot()) && IsInCollisionRange(false))
	{
		collide = true;

		if (m_bLastContactWasTouch)
		{
			collisionCoeff = sv_ball_dribbling_collisioncoeff.GetFloat();
			ballMass = sv_ball_dribbling_mass.GetFloat();
		}
		else
		{
			collisionCoeff = sv_ball_collisioncoeff.GetFloat();
			ballMass = sv_ball_mass.GetFloat();
		}
	}
	else if (m_pPl->IsShooting() && m_pPl->CanShoot()
		&& gpGlobals->curtime < m_flGlobalLastShot + m_flGlobalDynamicShotDelay * sv_ball_shotdelay_global_coeff.GetFloat()
		&& IsInCollisionRange(true))
	{
		collide = true;
		collisionCoeff = sv_ball_deflectioncoeff.GetFloat();
		ballMass = sv_ball_mass.GetFloat();
	}

	if (!collide)
		return false;

	// http://gamedev.stackexchange.com/a/15936
	Vector normal;

	if (m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_SLIDE || m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_DIVINGHEADER)
	{
		normal = localDirToBall.y > 0 ? -m_vPlRight : m_vPlRight;
		QAngle ang;
		VectorAngles((m_vPos - (m_vPlPos + m_flPhysRadius)), ang);
		VectorRotate(normal, QAngle(ang[PITCH], 0, 0), normal);
	}
	else
	{
		normal = dirToBall;
		normal.z = 0;
	}

	normal.NormalizeInPlace();
	Vector v1 = m_vVel;
	Vector v2 = m_vPlVel;
	float a1 = v1.Dot(normal);
	float a2 = v2.Dot(normal);
	float optimizedP = (2 * (a1 - a2)) / (ballMass + sv_player_mass.GetFloat());
	Vector vel = v1 - optimizedP * sv_player_mass.GetFloat() * normal;
	vel *= collisionCoeff;

	if (optimizedP < 0)
	{
		// TODO: Check if this is still needed
		//if (SDKGameRules()->State_Get() == MATCH_PERIOD_PENALTIES && m_ePenaltyState == PENALTY_KICKED && pPl != m_pPl)
		//{
		//	m_ePenaltyState = PENALTY_SAVED;
		//}

		if (m_vVel.Length() > 900.0f)
			m_pPl->EmitSound ("Player.Oomph");

		Touched(m_pPl, false, BODY_PART_UNKNOWN, m_vVel);

		EmitSound("Ball.Touch");
		m_vVel = vel;
		m_pPhys->SetVelocity(&m_vVel, &m_vRot);
		m_bSetNewVel = true;
	}

	return true;
}

bool CBall::DoSlideAction()
{
	if (!m_pPl->IsShooting() || m_vPlForwardVel2D.Length2DSqr() == 0)
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

	if (!SDKGameRules()->IsIntermissionState() && !m_bHasQueuedState)
	{
		if (CheckFoul(canShootBall, localDirToBall))
			return true;
	}
	
	if (!canShootBall)
		return false;

	Vector forward;
	AngleVectors(QAngle(-15, m_aPlAng[YAW], 0), &forward, NULL, NULL);

	Vector ballVel = forward * GetNormalshotStrength(1.0f, sv_ball_slide_strength.GetInt());

	SetVel(ballVel, 0, FL_SPIN_PERMIT_ALL, BODY_PART_FEET, false, true, true);

	if (!SDKGameRules()->IsIntermissionState() && State_Get() == BALL_STATE_NORMAL && !HasQueuedState())
		m_pPl->AddSlidingTackleCompleted();

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
	float distXZ = sqrt(Sqr(localDirToBall.x) + Sqr(localDirToBall.z));
	float diveTypeCatchCoeff = 1.0f;
	float ballPosCatchCoeff = 1.0f;

	switch (m_pPl->m_Shared.GetAnimEvent())
	{
	case PLAYERANIMEVENT_KEEPER_DIVE_LEFT:
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
	case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT:
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
	case PLAYERANIMEVENT_KEEPER_JUMP:
	default:
		float maxReachXY = (localDirToBall.z < sv_ball_keeper_standing_catchcenteroffset_z.GetFloat() ? sv_ball_keeper_standing_reach_bottom.GetFloat() : sv_ball_keeper_standing_reach_top.GetFloat());

		canReach = (localDirToBall.z < sv_ball_bodypos_keeperarms_end.GetFloat()
			&& localDirToBall.z >= sv_ball_bodypos_feet_start.GetFloat()
			&& distXY <= maxReachXY);

		if (canReach)
		{
			float distY = localDirToBall.y - sv_ball_keeper_standing_catchcenteroffset_side.GetFloat(); 
			float maxYReach = (distY >= 0 ? maxReachXY : -maxReachXY) - sv_ball_keeper_standing_catchcenteroffset_side.GetFloat();

			ballPosCatchCoeff = Square(min(1, abs(distY) / maxYReach));
			diveTypeCatchCoeff = sv_ball_keepercatchdelay_standing_global_coeff.GetFloat();
		}
		break;
	}

	if (!canReach)
		return false;
	
	ballPosCatchCoeff = clamp(ballPosCatchCoeff * (1 - sv_ball_keepercatchdelay_poscoeffmin.GetFloat()) + sv_ball_keepercatchdelay_poscoeffmin.GetFloat(), 0.0f, 1.0f);

	float nextCatch = m_flGlobalLastShot + m_flGlobalDynamicShotDelay * diveTypeCatchCoeff * ballPosCatchCoeff;

	if (m_bHasQueuedState || (gpGlobals->curtime < nextCatch && (!LastInfo(true) || LastInfo(true)->m_eBallState != BALL_STATE_PENALTY))) // Punch ball away
	{
		Vector punchDir;

		QAngle angDiff = m_aPlCamAng - m_aPlAng;

		if (Square(angDiff[PITCH]) + Square(angDiff[YAW]) <= Square(sv_ball_keeperautopunch_limit.GetFloat()))
		{
			int buttonSign;

			if ((m_pPl->m_nButtons & IN_MOVELEFT) && (!(m_pPl->m_nButtons & IN_MOVERIGHT) || m_pPl->m_Shared.m_nLastPressedSingleMoveKey == IN_MOVERIGHT)) 
				buttonSign = 1;
			else if ((m_pPl->m_nButtons & IN_MOVERIGHT) && (!(m_pPl->m_nButtons & IN_MOVELEFT) || m_pPl->m_Shared.m_nLastPressedSingleMoveKey == IN_MOVELEFT)) 
				buttonSign = -1;
			else
				buttonSign = 0;

			QAngle ang = m_aPlAng;

			ang[PITCH] = sv_ball_keeperautopunch_pitch.GetFloat();
			ang[YAW] += sv_ball_keeperautopunch_yaw.GetFloat() * buttonSign;

			AngleVectors(ang, &punchDir);
		}
		else
		{
			AngleVectors(m_aPlCamAng, &punchDir);
		}

		Vector vel = punchDir * max(m_vVel.Length2D(), sv_ball_keeper_punch_minstrength.GetFloat()) * sv_ball_keeperdeflectioncoeff.GetFloat();

		SetVel(vel, 0, 0, BODY_PART_KEEPERPUNCH, false, false, false);
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_BLANK);
	}
	else // Catch ball
	{
		SetVel(vec3_origin, 0, 0, BODY_PART_KEEPERCATCH, false, false, false);
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_BLANK);		
		State_Transition(BALL_STATE_KEEPERHANDS);
	}

	return true;
}

float CBall::GetPitchCoeff(bool isNormalShot, bool useCamViewAngles /*= false*/)
{
	//return pow(cos((m_aPlAng[PITCH] - sv_ball_bestshotangle.GetInt()) / (PITCH_LIMIT - sv_ball_bestshotangle.GetInt()) * M_PI / 2), 2);
	// plot 0.5 + (cos(x/89 * pi/2) * 0.5), x=-89..89

	float bestAng = sv_ball_bestshotangle.GetFloat();
	float pitch = useCamViewAngles ? m_aPlCamAng[PITCH] : m_aPlAng[PITCH];

	float coeff;

	if (pitch <= bestAng)
	{
		if (isNormalShot)
		{
			float upCoeff = sv_ball_fixedpitchupcoeff_normalshot.GetFloat();
			double upExp = sv_ball_pitchup_exponent_normalshot.GetFloat();
			coeff = upCoeff + (1 - upCoeff) * pow(cos((pitch - bestAng) / (mp_pitchup.GetFloat() - bestAng) * M_PI / 2), upExp);		
		}
		else
		{
			float upCoeff = sv_ball_fixedpitchupcoeff_nonnormalshot.GetFloat();
			double upExp = sv_ball_pitchup_exponent_nonnormalshot.GetFloat();
			coeff = upCoeff + (1 - upCoeff) * pow(cos((pitch - bestAng) / (mp_pitchup.GetFloat() - bestAng) * M_PI / 2), upExp);
		}
	}
	else
	{
		if (isNormalShot)
		{
			float downCoeff = sv_ball_fixedpitchdowncoeff_normalshot.GetFloat();
			double downExp = sv_ball_pitchdown_exponent_normalshot.GetFloat();
			coeff = downCoeff + (1 - downCoeff) * pow(cos((pitch - bestAng) / (mp_pitchdown.GetFloat() - bestAng) * M_PI / 2), downExp);		
		}
		else
		{
			float downCoeff = sv_ball_fixedpitchdowncoeff_nonnormalshot.GetFloat();
			double downExp = sv_ball_pitchdown_exponent_nonnormalshot.GetFloat();
			coeff = downCoeff + (1 - downCoeff) * pow(cos((pitch - bestAng) / (mp_pitchdown.GetFloat() - bestAng) * M_PI / 2), downExp);		
		}
	}

	//DevMsg("coeff: %.2f\n", coeff);

	return coeff;
}

float CBall::GetNormalshotStrength(float coeff, int strength)
{
	return coeff * strength;
}

float CBall::GetPowershotStrength(float coeff, int strength)
{
	return coeff * strength;
}

float CBall::GetChargedshotStrength(float coeff, int minStrength, int maxStrength)
{
	float shotStrength = minStrength + (maxStrength - minStrength) * m_pPl->GetChargedShotStrength();

	return coeff * shotStrength;
}

bool CBall::DoGroundShot(bool markOffsidePlayers, float velCoeff /*= 1.0f*/)
{
	if (m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_ROULETTE_CLOCKWISE || m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_ROULETTE_CC)
	{
		float timePassed = gpGlobals->curtime - m_pPl->m_Shared.GetAnimEventStartTime();

		QAngle shotAngle = m_pPl->m_Shared.GetAnimEventStartAngle();
		shotAngle[PITCH] = 0;
		
		shotAngle[YAW] += 45 * (m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_ROULETTE_CLOCKWISE ? -1 : 1);
		
		float shotStrength = mp_runspeed.GetInt() * 2.33f;

		Vector shotDir;
		AngleVectors(shotAngle, &shotDir);

		Vector vel = shotDir * shotStrength;

		SetVel(vel, 0, 0, BODY_PART_FEET, false, markOffsidePlayers, false, 0.75f - timePassed);

		return true;
	}

	if (m_pPl->m_Shared.GetAnimEvent() == PLAYERANIMEVENT_BALL_HOP)
	{
		QAngle shotAngle = m_aPlAng;
		shotAngle[PITCH] = -75;

		Vector shotDir;
		AngleVectors(shotAngle, &shotDir);

		Vector vel = shotDir * 300 + m_vPlVel2D;

		SetVel(vel, 0, 0, BODY_PART_FEET, false, markOffsidePlayers, false, 0.66f);

		return true;
	}

	if (m_pPl->m_nButtons & IN_RELOAD)
	{
		if (m_pPl->m_nButtons & IN_DUCK)
		{
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KICK);
			m_pPl->m_flNextShot = gpGlobals->curtime + 1.0f;

			return true;
		}

		if ((m_pPl->m_nButtons & IN_MOVELEFT) || (m_pPl->m_nButtons & IN_MOVERIGHT))
		{
			if (DotProduct2D(m_vPlForward.AsVector2D(), (m_vPos - m_vPlPos).AsVector2D()) >= 0)
			{
				Vector vel;

				if (m_pPl->m_nButtons & IN_FORWARD)
					vel = m_vPlRight * 300 * ((m_pPl->m_nButtons & IN_MOVELEFT) ? -1 : 1) + m_vPlForward2D * 300;
				else
					vel = m_vPlRight * 350 * ((m_pPl->m_nButtons & IN_MOVELEFT) ? -1 : 1);

				SetVel(vel, 0, 0, BODY_PART_FEET, false, markOffsidePlayers, true);
				return true;
			}
		}

		QAngle shotAngle = m_aPlCamAng;
		shotAngle[PITCH] = sv_ball_groundshot_minangle.GetFloat();
		//shotAngle[PITCH] = min(sv_ball_groundshot_minangle.GetFloat(), shotAngle[PITCH]);
		Vector shotDir;
		AngleVectors(shotAngle, &shotDir);

		float angDiff = AngleDiff(m_aPlAng[YAW], shotAngle[YAW]);

		if (abs(angDiff) > sv_ball_freecamshot_maxangle.GetInt())
		{
			Vector vel = shotDir * max(sv_ball_heelshot_strength.GetInt(), m_vVel.Length2D());

			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEELKICK);
			EmitSound("Ball.Kicknormal");
			SetVel(vel, 0, 0, BODY_PART_FEET, false, markOffsidePlayers, true);

			return true;
		}
	}
	
	bool useCamViewAngles = (m_pPl->m_nButtons & IN_RELOAD);
	int spinFlags = FL_SPIN_PERMIT_ALL;
	float spinCoeff = 1.0f;
	float shotStrength;

	if (m_pPl->IsNormalshooting())
		shotStrength = GetNormalshotStrength(GetPitchCoeff(true, useCamViewAngles), sv_ball_normalshot_strength.GetInt());
	else if (m_pPl->IsPowershooting())
		shotStrength = GetPowershotStrength(GetPitchCoeff(false, useCamViewAngles), sv_ball_powershot_strength.GetInt());
	else
		shotStrength = GetChargedshotStrength(GetPitchCoeff(false, useCamViewAngles), sv_ball_chargedshot_minstrength.GetInt(), sv_ball_chargedshot_maxstrength.GetInt());

	QAngle shotAngle = useCamViewAngles ? m_aPlCamAng : m_aPlAng;
	shotAngle[PITCH] = min(sv_ball_groundshot_minangle.GetFloat(), shotAngle[PITCH]);

	Vector shotDir;
	AngleVectors(shotAngle, &shotDir);

	Vector vel = shotDir * shotStrength * velCoeff;

	if (vel.Length() > 1000)
	{
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KICK);
		EmitSound("Ball.Kickhard");
	}
	else if (vel.Length() > 700)
	{
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_PASS);
		EmitSound("Ball.Kicknormal");
	}
	else
	{
		Vector dirToBall = m_vPos - m_vPlPos;
		Vector localDirToBall;
		VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);

		if (m_pPl->IsNormalshooting() && localDirToBall.x <= sv_ball_rainbowflick_dist.GetFloat() && shotAngle[PITCH] <= sv_ball_rainbowflick_angle.GetFloat())
		{
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEELKICK);
			spinFlags = FL_SPIN_FORCE_TOP;
			spinCoeff = sv_ball_rainbowflick_spincoeff.GetFloat();
		}
		else
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_BLANK);

		EmitSound("Ball.Touch");
	}

	SetVel(vel, spinCoeff, spinFlags, BODY_PART_FEET, false, markOffsidePlayers, true);

	return true;
}

bool CBall::DoVolleyShot()
{
	float shotStrength;

	if (m_pPl->IsNormalshooting())
		shotStrength = GetNormalshotStrength(GetPitchCoeff(true), sv_ball_normalshot_strength.GetInt());
	else if (m_pPl->IsPowershooting())
		shotStrength = GetPowershotStrength(GetPitchCoeff(false), sv_ball_powershot_strength.GetInt());
	else
		shotStrength = GetChargedshotStrength(GetPitchCoeff(false), sv_ball_chargedshot_minstrength.GetInt(), sv_ball_chargedshot_maxstrength.GetInt());

	QAngle shotAngle = m_aPlAng;
	shotAngle[PITCH] = min(sv_ball_volleyshot_minangle.GetFloat(), m_aPlAng[PITCH]);

	Vector shotDir;
	AngleVectors(shotAngle, &shotDir);

	Vector vel = shotDir * shotStrength * sv_ball_volleyshot_speedcoeff.GetFloat();

	if (vel.Length() > 700)
	{
		if (vel.Length() > 1000)
			EmitSound("Ball.Kickhard");
		else
			EmitSound("Ball.Kicknormal");

		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_VOLLEY);
	}
	else
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_BLANK);

	SetVel(vel, sv_ball_volleyshot_spincoeff.GetFloat(), FL_SPIN_PERMIT_ALL, BODY_PART_FEET, false, true, true);

	return true;
}

bool CBall::DoHeader()
{
	Vector vel;

	if (m_pPl->IsNormalshooting())
	{
		vel = m_vPlForward * GetNormalshotStrength(GetPitchCoeff(true), sv_ball_normalheader_strength.GetInt());
		EmitSound("Ball.Kicknormal");
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEADER_STATIONARY);
	}
	else if (m_vPlForwardVel2D.Length2D() >= mp_walkspeed.GetInt()
		&& (m_nInPenBoxOfTeam == m_pPl->GetTeamNumber() || m_nInPenBoxOfTeam == m_pPl->GetOppTeamNumber())
		&& (m_pPl->m_nButtons & IN_SPEED) && m_pPl->GetGroundEntity())
	{
		Vector forward;
		AngleVectors(QAngle(-5, m_aPlAng[YAW], 0), &forward, NULL, NULL);

		if (m_pPl->IsPowershooting())
			vel = forward * GetPowershotStrength(1.0f, sv_ball_powerdivingheader_strength.GetInt());
		else
			vel = forward * GetChargedshotStrength(1.0f, sv_ball_chargeddivingheader_minstrength.GetInt(), sv_ball_chargeddivingheader_maxstrength.GetInt());

		EmitSound("Ball.Kickhard");
		EmitSound("Player.DivingHeader");
		//m_pPl->AddFlag(FL_FREECAM);
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_DIVINGHEADER);
	}
	else
	{
		if (m_pPl->IsPowershooting())
			vel = m_vPlForward * GetPowershotStrength(GetPitchCoeff(false), sv_ball_powerheader_strength.GetInt());
		else
			vel = m_vPlForward * GetChargedshotStrength(GetPitchCoeff(false), sv_ball_chargedheader_minstrength.GetInt(), sv_ball_chargedheader_maxstrength.GetInt());

		if (vel.Length() > 1000)
			EmitSound("Ball.Kickhard");
		else
			EmitSound("Ball.Kicknormal");

		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEADER);
	}

	SetVel(m_vPlForwardVel2D + vel, sv_ball_header_spincoeff.GetFloat(), FL_SPIN_PERMIT_SIDE, BODY_PART_HEAD, false, true, true, sv_ball_header_mindelay.GetFloat());

	return true;
}

AngularImpulse CBall::CalcSpin(float coeff, int spinFlags)
{	
	Vector sideRot = vec3_origin;
	float sideSpin = 0;

	float speedCoeff = pow(sin(RemapValClamped(m_vVel.Length(), sv_ball_dynamicshotdelay_minshotstrength.GetInt(), sv_ball_dynamicshotdelay_maxshotstrength.GetInt(), 0.0f, 1.0f) * M_PI), (double)sv_ball_spin_exponent.GetFloat());

	if (coeff > 0 && (spinFlags & (FL_SPIN_PERMIT_SIDE | FL_SPIN_FORCE_SIDE)))
	{
		sideSpin = speedCoeff * sv_ball_spin.GetInt() * coeff;

		if ((m_pPl->m_nButtons & IN_MOVELEFT) && (!(m_pPl->m_nButtons & IN_MOVERIGHT) || m_pPl->m_Shared.m_nLastPressedSingleMoveKey == IN_MOVERIGHT)) 
		{
			sideRot = Vector(0, 0, m_pPl->IsLegacySideCurl() ? 1 : -1);
		}
		else if ((m_pPl->m_nButtons & IN_MOVERIGHT) && (!(m_pPl->m_nButtons & IN_MOVELEFT) || m_pPl->m_Shared.m_nLastPressedSingleMoveKey == IN_MOVELEFT)) 
		{
			sideRot = Vector(0, 0, m_pPl->IsLegacySideCurl() ? -1 : 1);
		}
	}

	bool useCamViewAngles = (m_pPl->m_nButtons & IN_RELOAD);
	float pitch = useCamViewAngles ? m_aPlCamAng[PITCH] : m_aPlAng[PITCH];

	Vector backRot = m_vPlRight;
	float backSpin = 0;

	Vector topRot = -m_vPlRight;
	float topSpin = 0;

	if (coeff > 0)
	{
		if ((spinFlags & FL_SPIN_PERMIT_BACK)
			&& pitch >= sv_ball_bestbackspinangle_start.GetFloat() && pitch <= sv_ball_bestbackspinangle_end.GetFloat()
			|| (spinFlags & FL_SPIN_FORCE_BACK))
		{
			backSpin = speedCoeff * sv_ball_spin.GetInt() * coeff * sv_ball_backspin_coeff.GetFloat();

			if (!(spinFlags & FL_SPIN_FORCE_BACK))
				backSpin *= pow((double)cos(RemapValClamped(pitch, sv_ball_bestbackspinangle_start.GetFloat(), sv_ball_bestbackspinangle_end.GetFloat(), M_PI * 0.5, M_PI * 1.5)), 2.0);
		}

		if ((!m_pPl->GetGroundEntity() || (m_pPl->m_nButtons & IN_JUMP)) && (spinFlags & FL_SPIN_PERMIT_TOP)
			&& pitch >= sv_ball_besttopspinangle_start.GetFloat() && pitch <= sv_ball_besttopspinangle_end.GetFloat()
			|| (spinFlags & FL_SPIN_FORCE_TOP))
		{
			topSpin = speedCoeff * sv_ball_spin.GetInt() * coeff * sv_ball_topspin_coeff.GetFloat();

			if (!(spinFlags & FL_SPIN_FORCE_TOP))
				topSpin *= pow((double)cos(RemapValClamped(pitch, sv_ball_besttopspinangle_start.GetFloat(), sv_ball_besttopspinangle_end.GetFloat(), M_PI * 0.5, M_PI * 1.5)), 2.0);
		}
	}

	//DevMsg("pitch: %.2f, backtopspin: %.2f\n", pitch, backTopSpin);

	AngularImpulse randRot = vec3_origin;

	//if (sideRot == vec3_origin && backTopRot == vec3_origin)
	{
		for (int i = 0; i < 3; i++)
		{
			// Add some random rotation if there isn't any side, back or top spin, since a non-rotating ball looks unnatural
			randRot[i] = sv_ball_defaultspin.GetInt() * (g_IOSRand.RandomInt(0, 1) == 1 ? 1 : -1);
		}
	}


	// The angular impulse is applied locally in the physics engine, so rotate from the player angles
	return (WorldToLocalRotation(
		SetupMatrixAngles(m_aAng), sideRot, sideSpin)
		+ WorldToLocalRotation(SetupMatrixAngles(m_aAng), backRot, backSpin)
		+ WorldToLocalRotation(SetupMatrixAngles(m_aAng), topRot, topSpin)
		+ randRot);
}

void CBall::Think( void	)
{
	SetNextThink(gpGlobals->curtime + sv_ball_thinkinterval.GetFloat());

	State_Think();
}

void CBall::TriggerPenaltyBox(int team)
{
	m_nInPenBoxOfTeam = team;
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
		m_vPlForwardVel2D = m_vPlForward2D * max(0, (m_vPlVel2D.x * m_vPlForward2D.x + m_vPlVel2D.y * m_vPlForward2D.y));
		
		m_vPlDirToBall = m_vPos - m_vPlPos;
		VectorIRotate(m_vPlDirToBall, m_pPl->EntityToWorldTransform(), m_vPlLocalDirToBall);
	}
}

extern ConVar sv_singlekeeper;

void CBall::MarkOffsidePlayers()
{
	if (SDKGameRules()->IsIntermissionState() || m_bHasQueuedState || SDKGameRules()->State_Get() == MATCH_PERIOD_PENALTIES)
		return;

	m_vOffsidePos = m_vPos;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (pPl)
			pPl->SetOffside(false);

		if (!CSDKPlayer::IsOnField(pPl) || pPl == m_pPl || pPl->GetTeamNumber() != m_pPl->GetTeamNumber())
			continue;

		Vector pos = pPl->GetLocalOrigin();
		int forward = pPl->GetTeam()->m_nForward;

		// In opponent half?
		if (Sign((pos - SDKGameRules()->m_vKickOff).y) != forward)
			continue;

		// Player closer to goal than the ball?
		if (Sign(pos.y - m_vPos.y) != forward)
			continue;

		int oppPlayerCount = 0;
		int nearerPlayerCount = 0;
		CSDKPlayer *pLastPl = NULL;
		float shortestDist = FLT_MAX;

		// Count opponent players who are nearer to the goal
		for (int j = 1; j <= gpGlobals->maxClients; j++)
		{
			CSDKPlayer *pOpp = ToSDKPlayer(UTIL_PlayerByIndex(j));
			if (!CSDKPlayer::IsOnField(pOpp) || pOpp->GetTeamNumber() == pPl->GetTeamNumber())
				continue;

			oppPlayerCount += 1;

			if (Sign(pOpp->GetLocalOrigin().y - pos.y) == forward)
			{
				nearerPlayerCount += 1;
			}
			else
			{
				float dist = abs(pos.y - pOpp->GetLocalOrigin().y);
				if (dist < shortestDist)
				{
					shortestDist = dist;
					pLastPl = pOpp;
				}
			}
		}

		// If the defending team doesn't have a keeper, just assume there is one at his goal line and increase the counter by one.
		// This is mainly relevant for matches with sv_singlekeeper enabled when the keeper spot of the defending team is empty when the attackers play a pass.
		// An attacker can be caught offside then, even if a defender is closer to the goal than he is.
		// It also helps preventing abuse by keepers who go to spec during an attack to force false offsides.
		if (!m_pPl->GetOppTeam()->GetPlayerByPosType(POS_GK))
		{
			nearerPlayerCount += 1;
		}

		// Require at least two opponent players on the field to consider an offside. Useful in public matches with few players.
		if (oppPlayerCount >= 2 && nearerPlayerCount <= 1)
		{
			pPl->SetOffside(true);
			pPl->SetOffsidePos(pPl->GetLocalOrigin());
			pPl->SetOffsideBallPos(m_vPos);

			Vector lastSafePos;

			if (abs(pLastPl->GetLocalOrigin().y - pPl->GetLocalOrigin().y) < abs(SDKGameRules()->m_vKickOff.GetY() - pPl->GetLocalOrigin().y))
				lastSafePos = pLastPl->GetLocalOrigin();
			else
				lastSafePos = SDKGameRules()->m_vKickOff;

			pPl->SetOffsideLastOppPlayerPos(lastSafePos);
		}
	}
}

void CBall::UnmarkOffsidePlayers()
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (CSDKPlayer::IsOnField(pPl))
			pPl->SetOffside(false);
	}
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
	return info ? info->m_nTeam : TEAM_INVALID;
}

int CBall::LastOppTeam(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/, CSDKPlayer *pSkipPl2 /*= NULL*/, CSDKPlayer *pSkipPl3 /*= NULL*/)
{
	BallTouchInfo *info = LastInfo(wasShooting, pSkipPl, pSkipPl2, pSkipPl3);
	return info ? (info->m_nTeam == TEAM_A ? TEAM_B : TEAM_A) : TEAM_INVALID;
}

void CBall::UpdatePossession(CSDKPlayer *pNewPossessor)
{
	if (m_pPossessingPl == pNewPossessor)
		return;

	if (m_flPossessionStart != -1)
	{
		float duration = gpGlobals->curtime - m_flPossessionStart;

		if (m_pPossessingPl)
			m_pPossessingPl->AddPossessionTime(duration);

		float total = GetGlobalTeam(TEAM_A)->m_flPossessionTime + GetGlobalTeam(TEAM_B)->m_flPossessionTime;

		GetGlobalTeam(TEAM_A)->m_Possession = floor(GetGlobalTeam(TEAM_A)->m_flPossessionTime * 100 / max(1, total) + 0.5f);
		GetGlobalTeam(TEAM_B)->m_Possession = 100 - GetGlobalTeam(TEAM_A)->m_Possession;

		int possSum = 0;

		struct remainder_t
		{
			int dataIndex;
			float remainder;
			remainder_t() : dataIndex(0), remainder(0) {}
		};

		remainder_t *sortedRemainders = new remainder_t[CPlayerPersistentData::m_PlayerPersistentData.Count()];

		for (int i = 0; i < CPlayerPersistentData::m_PlayerPersistentData.Count(); i++)
		{
			CPlayerMatchData *pData = CPlayerPersistentData::m_PlayerPersistentData[i]->m_pMatchData;
			
			float exactPossession = pData->m_flPossessionTime * 100 / max(1, total);
			pData->m_nPossession = (int)exactPossession;
			float remainder = exactPossession - pData->m_nPossession;

			possSum += pData->m_nPossession;

			for (int j = 0; j <= i; j++)
			{
				if (sortedRemainders[j].remainder <= remainder)
				{
					for (int k = i; k > j; k--)
					{
						sortedRemainders[k] = sortedRemainders[k - 1];
					}

					sortedRemainders[j].dataIndex = i;
					sortedRemainders[j].remainder = remainder;

					break;
				}
			}
		}

		for (int i = 0; i < CPlayerPersistentData::m_PlayerPersistentData.Count(); i++)
		{
			if (possSum == 100)
				break;

			CPlayerMatchData *pData = CPlayerPersistentData::m_PlayerPersistentData[sortedRemainders[i].dataIndex]->m_pMatchData;
			pData->m_nPossession += 1;
			possSum += 1;
		}

		delete[] sortedRemainders;
	}

	if (CSDKPlayer::IsOnField(pNewPossessor))
	{
		m_pPossessingPl = pNewPossessor;
		m_nPossessingTeam = pNewPossessor->GetTeamNumber();
		m_flPossessionStart = gpGlobals->curtime;
	}
	else
	{
		m_pPossessingPl = NULL;
		m_nPossessingTeam = TEAM_INVALID;
		m_flPossessionStart = -1;
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
	RemoveAllTouches();
	m_ePenaltyState = PENALTY_NONE;
	UnmarkOffsidePlayers();
	m_bSetNewPos = false;
	m_bSetNewVel = false;
	m_bSetNewRot = false;
	m_bHasQueuedState = false;
	RemoveEffects(EF_NODRAW);
	EnablePlayerCollisions(true);
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pHoldingPlayer = NULL;
	m_pPossessingPl = NULL;
	m_nPossessingTeam = TEAM_INVALID;
	m_flPossessionStart = -1;
	m_pTurnoverPlayer = NULL;
	m_flLastMatchEventSetTime = -1;
	m_bNonnormalshotsBlocked = false;
	m_bShotsBlocked = false;
	m_bHitThePost = false;
	m_bLastContactWasTouch = false;
}

void CBall::ReloadSettings()
{
	CreateVPhysics();
}

void CBall::SetPenaltyTaker(CSDKPlayer *pPl)
{
	m_pFouledPl = pPl;
	m_nFouledTeam = pPl->GetTeamNumber();
	m_nFoulingTeam = pPl->GetOppTeamNumber();
}

void CBall::EnablePlayerCollisions(bool enable)
{
	SetCollisionGroup(enable ? COLLISION_GROUP_SOLID_BALL : COLLISION_GROUP_NONSOLID_BALL);
}

void CBall::RemoveFromPlayerHands(CSDKPlayer *pPl)
{
	if (CSDKPlayer::IsOnField(pPl) && pPl->GetTeamPosType() == POS_GK && pPl->m_pHoldingBall.Get() == this)
	{
		pPl->m_pHoldingBall = NULL;
		pPl->DoServerAnimationEvent(PLAYERANIMEVENT_CARRY_END);
	}

	m_pHoldingPlayer = NULL;
	RemoveEffects(EF_NODRAW);
	EnablePlayerCollisions(true);
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
}

Vector CBall::GetPos()
{
	if (m_bSetNewPos)
		return m_vPos;
	else
	{
		Vector pos;
		m_pPhys->GetPosition(&pos, NULL);
		return pos;
	}
}

QAngle CBall::GetAng()
{
	if (m_bSetNewPos)
		return m_aAng;
	else
	{
		QAngle ang;
		m_pPhys->GetPosition(NULL, &ang);
		return ang;
	}
}

Vector CBall::GetVel()
{
	if (m_bSetNewVel)
		return m_vVel;
	else
	{
		Vector vel;
		m_pPhys->GetVelocity(&vel, NULL);
		return vel;
	}
}

AngularImpulse CBall::GetRot()
{
	if (m_bSetNewRot)
		return m_vRot;
	else
	{
		AngularImpulse rot;
		m_pPhys->GetVelocity(NULL, &rot);
		return rot;
	}
}

// -100 = ball at the goal line of the home team
// 0 = ball at the the half-way line
// 100 = ball at the goal line of the away team
float CBall::CalcFieldZone()
{
	Vector pos = GetPos();
	float fieldLength = SDKGameRules()->m_vFieldMax.GetY() - SDKGameRules()->m_vFieldMin.GetY();
	float dist = GetGlobalTeam(TEAM_A)->m_nForward * (pos.y - SDKGameRules()->m_vKickOff.GetY());
	return clamp(dist * 100 / (fieldLength / 2), -100, 100);
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