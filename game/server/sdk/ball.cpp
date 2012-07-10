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


ConVar sv_ball_mass( "sv_ball_mass", "1", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_damping( "sv_ball_damping", "0.4", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_rotdamping( "sv_ball_rotdamping", "0.3", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_spin( "sv_ball_spin", "300", FCVAR_NOTIFY );
ConVar sv_ball_defaultspin( "sv_ball_defaultspin", "10000", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY );
ConVar sv_ball_topspin_multiplier( "sv_ball_topspin_multiplier", "0.75", FCVAR_NOTIFY );
ConVar sv_ball_backspin_multiplier( "sv_ball_backspin_multiplier", "0.75", FCVAR_NOTIFY );
ConVar sv_ball_curve("sv_ball_curve", "150", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_touchradius( "sv_ball_touchradius", "60", FCVAR_NOTIFY );
ConVar sv_ball_slidesidereach( "sv_ball_slidesidereach", "60", FCVAR_NOTIFY );
ConVar sv_ball_slideforwardreach( "sv_ball_slideforwardreach", "100", FCVAR_NOTIFY );
ConVar sv_ball_slidemultiplier("sv_ball_slidemultiplier", "3.0", FCVAR_NOTIFY); 
ConVar sv_ball_slidesidemultiplier("sv_ball_slidesidemultiplier", "0.66", FCVAR_NOTIFY); 
ConVar sv_ball_keepertouchradius( "sv_ball_keepertouchradius", "60", FCVAR_NOTIFY );
ConVar sv_ball_keepershortsidereach( "sv_ball_keepershortsidereach", "60", FCVAR_NOTIFY );
ConVar sv_ball_keeperlongsidereach( "sv_ball_keeperlongsidereach", "120", FCVAR_NOTIFY );
ConVar sv_ball_keeperpunchupstrength("sv_ball_keeperpunchupstrength", "500", FCVAR_NOTIFY);
ConVar sv_ball_shotdelay_normal("sv_ball_shotdelay_normal", "0.25", FCVAR_NOTIFY);
ConVar sv_ball_shotdelay_setpiece("sv_ball_shotdelay_setpiece", "0.5", FCVAR_NOTIFY);
ConVar sv_ball_shotdelay_global("sv_ball_shotdelay_global", "0.25", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_enabled("sv_ball_dynamicshotdelay_enabled", "1", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_mindelay("sv_ball_dynamicshotdelay_mindelay", "0.05", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_maxdelay("sv_ball_dynamicshotdelay_maxdelay", "1.0", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_minshotstrength("sv_ball_dynamicshotdelay_minshotstrength", "100", FCVAR_NOTIFY);
ConVar sv_ball_dynamicshotdelay_maxshotstrength("sv_ball_dynamicshotdelay_maxshotstrength", "1200", FCVAR_NOTIFY);
ConVar sv_ball_bestshotangle("sv_ball_bestshotangle", "-30", FCVAR_NOTIFY);
ConVar sv_ball_keepercatchspeed("sv_ball_keepercatchspeed", "500", FCVAR_NOTIFY);
ConVar sv_ball_keeperpickupangle("sv_ball_keeperpickupangle", "45", FCVAR_NOTIFY);
ConVar sv_ball_normalshot_strength("sv_ball_normalshot_strength", "800", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_powershot_minstrength("sv_ball_powershot_minstrength", "600", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_powershot_maxstrength("sv_ball_powershot_maxstrength", "1600", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_autopass_minstrength("sv_ball_autopass_minstrength", "300", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_autopass_maxstrength("sv_ball_autopass_maxstrength", "1600", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_autopass_multiplier("sv_ball_autopass_multiplier", "1", FCVAR_NOTIFY);
ConVar sv_ball_volleyshot_spinmultiplier("sv_ball_volleyshot_spinmultiplier", "1.25", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_doubletouchfouls("sv_ball_doubletouchfouls", "1", FCVAR_NOTIFY);
ConVar sv_ball_timelimit("sv_ball_timelimit", "10", FCVAR_NOTIFY);
ConVar sv_ball_statetransitiondelay("sv_ball_statetransitiondelay", "1.0", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_goalcelebduration("sv_ball_goalcelebduration", "5.0", FCVAR_NOTIFY);
ConVar sv_ball_thinkinterval("sv_ball_thinkinterval", "0", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY);
ConVar sv_ball_throwin_minstrength("sv_ball_throwin_minstrength", "250", FCVAR_NOTIFY);
ConVar sv_ball_throwin_maxstrength("sv_ball_throwin_maxstrength", "750", FCVAR_NOTIFY); 
ConVar sv_ball_chestdrop_strength("sv_ball_chestdrop_strength", "250", FCVAR_NOTIFY); 
ConVar sv_ball_powerdivingheader_minstrength("sv_ball_powerdivingheader_minstrength", "350", FCVAR_NOTIFY); 
ConVar sv_ball_powerdivingheader_maxstrength("sv_ball_powerdivingheader_maxstrength", "700", FCVAR_NOTIFY); 
ConVar sv_ball_normalheader_strength("sv_ball_normalheader_strength", "250", FCVAR_NOTIFY); 
ConVar sv_ball_powerheader_minstrength("sv_ball_powerheader_minstrength", "250", FCVAR_NOTIFY); 
ConVar sv_ball_powerheader_maxstrength("sv_ball_powerheader_maxstrength", "500", FCVAR_NOTIFY); 
ConVar sv_ball_minshotstrength("sv_ball_minshotstrength", "100", FCVAR_NOTIFY);  
ConVar sv_ball_minspeed_passive("sv_ball_minspeed_passive", "1000", FCVAR_NOTIFY); 
ConVar sv_ball_minspeed_deflect("sv_ball_minspeed_deflect", "500", FCVAR_NOTIFY); 
ConVar sv_ball_player_yellow_red_card_duration("sv_ball_player_yellow_red_card_duration", "6", FCVAR_NOTIFY);
ConVar sv_ball_player_red_card_duration("sv_ball_player_red_card_duration", "9", FCVAR_NOTIFY);
ConVar sv_ball_reset_stamina_on_freekicks("sv_ball_reset_stamina_on_freekicks", "1", FCVAR_NOTIFY);


CBall *CreateBall(const Vector &pos, CSDKPlayer *pCreator)
{
	CBall *pBall = static_cast<CBall*>(CreateEntityByName("football"));
	pBall->SetAbsOrigin(pos);
	pBall->SetCreator(pCreator);
	pBall->Spawn();
	pBall->SetPos(pos);
	return pBall;
}

void CC_CreatePlayerBall(const CCommand &args)
{
	if (!SDKGameRules()->IsIntermissionState())
		return;

	CSDKPlayer *pPl = ToSDKPlayer(UTIL_GetCommandClient());
	if (!CSDKPlayer::IsOnField(pPl))
		return;

	Vector pos = pPl->GetLocalOrigin() + VEC_VIEW + pPl->EyeDirection3D() * 150;
	pos.z = max(pos.z, SDKGameRules()->m_vKickOff.GetZ());

	if (pPl->GetPlayerBall())
	{
		if (pPl->GetPlayerBall()->GetHoldingPlayer())
		{
			pPl->GetPlayerBall()->RemoveFromPlayerHands(pPl->GetPlayerBall()->GetHoldingPlayer());
			pPl->GetPlayerBall()->State_Transition(BALL_NORMAL);
		}

		pPl->GetPlayerBall()->SetPos(pos);
	}
	else
		pPl->SetPlayerBall(CreateBall(pos, pPl));
}

static ConCommand createplayerball("createplayerball", CC_CreatePlayerBall);

CBall *g_pBall = NULL;

CBall *GetBall()
{
	return g_pBall;
}

CBall *GetNearestBall(const Vector &pos)
{
	CBall *pNearestBall = GetBall();
	Vector ballPos;
	pNearestBall->VPhysicsGetObject()->GetPosition(&ballPos, NULL);
	float shortestDist = (ballPos - pos).Length2DSqr();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPl || !CSDKPlayer::IsOnField(pPl))
			continue;

		CBall *pBall = pPl->GetPlayerBall();
		if (!pBall)
			continue;

		Vector ballPos;
		pBall->VPhysicsGetObject()->GetPosition(&ballPos, NULL);

		float dist = (ballPos - pos).Length2DSqr();
		if (dist < shortestDist)
		{
			shortestDist = dist;
			pNearestBall = pBall;
		}
	}

	return pNearestBall;
}

LINK_ENTITY_TO_CLASS( football,	CBall );

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
	SendPropFloat(SENDINFO(m_flOffsideLineBallPosY), 0, SPROP_COORD),
	SendPropFloat(SENDINFO(m_flOffsideLineOffsidePlayerPosY), 0, SPROP_COORD),
	SendPropFloat(SENDINFO(m_flOffsideLineLastOppPlayerPosY), 0, SPROP_COORD),
	SendPropBool(SENDINFO(m_bOffsideLinesEnabled))
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
	m_eBallState = BALL_NOSTATE;
	m_eNextState = BALL_NOSTATE;
	m_flStateLeaveTime = gpGlobals->curtime;
	m_flStateTimelimit = -1;
	m_eFoulType = FOUL_NONE;
	m_pPossessingPl = NULL;
	m_nPossessingTeam = TEAM_INVALID;
	m_flPossessionStart = -1;
	m_pPl = NULL;
	m_nPlTeam = TEAM_INVALID;
	m_bSetNewPos = false;
	m_bHasQueuedState = false;
	m_ePenaltyState = PENALTY_NONE;
	m_pCreator = NULL;
	m_pHoldingPlayer = NULL;
	m_flNextShot = gpGlobals->curtime;
	m_nInPenBoxOfTeam = TEAM_INVALID;
}

CBall::~CBall()
{
}

void CBall::RemovePlayerBalls()
{
	CBall *pBall = NULL;

	while (true)
	{
		pBall = static_cast<CBall *>(gEntList.FindEntityByClassname(pBall, "football"));
		if (!pBall)
			break;

		if (pBall != this)
		{
			if (pBall->GetHoldingPlayer())
				pBall->GetHoldingPlayer()->m_pHoldingBall = NULL;

			UTIL_Remove(pBall);
		}
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPl)
			continue;

		pPl->SetPlayerBall(NULL);
	}
}

//==========================================================
//	
//	
//==========================================================
void CBall::Spawn (void)
{
	if (!m_pCreator)
		g_pBall = this;

	//RomD: Don't fade the ball
	SetFadeDistance(-1, 0);
	DisableAutoFade();

	PrecacheModel(BALL_MODEL);
	SetModel(BALL_MODEL);

	CreateVPhysics();

	SetThink(&CBall::Think);
	SetNextThink(gpGlobals->curtime + sv_ball_thinkinterval.GetFloat());

	m_nBody = 0; 
	m_nSkin = g_IOSRand.RandomInt(0,5);
	m_pPhys->SetPosition(GetLocalOrigin(), GetLocalAngles(), true);
	m_pPhys->SetVelocityInstantaneous(&vec3_origin, &vec3_origin);

	PrecacheScriptSound( "Ball.kicknormal" );
	PrecacheScriptSound( "Ball.kickhard" );
	PrecacheScriptSound( "Ball.touch" );
	PrecacheScriptSound( "Ball.post" );
	PrecacheScriptSound( "Ball.net" );
	PrecacheScriptSound( "Ball.whistle" );
	PrecacheScriptSound( "Ball.cheer" );

	State_Transition(BALL_NORMAL);
}

bool CBall::CreateVPhysics()
{	
	if (m_pPhys)
	{
		VPhysicsDestroyObject();
		m_pPhys = NULL;
	}

	m_flPhysRadius = 5.0f;
	objectparams_t params =	g_IOSPhysDefaultObjectParams;
	params.pGameData = static_cast<void	*>(this);
	int	nMaterialIndex = physprops->GetSurfaceIndex("ios");
	m_pPhys = physenv->CreateSphereObject( m_flPhysRadius, nMaterialIndex, GetAbsOrigin(), GetAbsAngles(), &params, false );
	if (!m_pPhys)
		return false;

	VPhysicsSetObject( m_pPhys );
	
	SetSolid( SOLID_VPHYSICS );
	SetSolidFlags( FSOLID_NOT_STANDABLE	);
	//UTIL_SetSize( this,	-Vector(5.0f,5.0f,5.0f), Vector(5.0f,5.0f,5.0f)	);
	UTIL_SetSize( this,	-Vector(3.0f,3.0f,3.0f), Vector(3.0f,3.0f,3.0f)	);

	SetMoveType( MOVETYPE_VPHYSICS );

	PhysSetGameFlags(m_pPhys, FVPHYSICS_NO_PLAYER_PICKUP);

	m_pPhys->SetMass(sv_ball_mass.GetFloat());//0.05f	);
	m_fMass	= m_pPhys->GetMass();
	//m_pPhys->EnableGravity(	sv_ball_enable_gravity.GetFloat() );
	m_pPhys->EnableGravity(true);
	//m_pPhys->EnableDrag( sv_ball_enable_drag.GetFloat() );
	m_pPhys->EnableDrag(false);
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
	m_pPhys->Wake();

	return true;
}

void CBall::VPhysicsUpdate(IPhysicsObject *pPhysics)
{
	//if (!SDKGameRules()->IsIntermissionState())
	//	CReplayManager::GetInstance()->CheckReplay();

	if (m_eBallState == BALL_KEEPERHANDS && CSDKPlayer::IsOnField(m_pPl))
	{
		SetPos(Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + BODY_CHEST_START) + m_vPlForward2D * 2 * VEC_HULL_MAX.x);
		m_bSetNewPos = false;
	}

	if (!m_bSetNewPos)
	{
		Vector vel, worldAngImp;
		AngularImpulse angImp;
		m_pPhys->GetVelocity(&vel, &angImp);
		VectorRotate(angImp, EntityToWorldTransform(), worldAngImp);
		Vector magnusDir = worldAngImp.Cross(vel);

		if (vel.Length() > 0)
			vel += magnusDir * 1e-6 * sv_ball_curve.GetFloat() * gpGlobals->frametime;

		VPhysicsGetObject()->SetVelocity(&vel, &angImp);
	}

	BaseClass::VPhysicsUpdate(pPhysics);

	if (m_bSetNewPos)
		m_bSetNewPos = false;
}


void CBall::VPhysicsCollision( int index, gamevcollisionevent_t	*pEvent	)
{
	Vector preVelocity = pEvent->preVelocity[index];
	float flSpeed =	VectorNormalize( preVelocity );
	int surfaceProps = pEvent->surfaceProps[!index];

	//IOS goal post hacks!!
	if (surfaceProps == 81 && flSpeed > 300.0f)
	{
		EmitSound("Ball.post");
	}
	else
	{
		//if ball is moving fast when we hit something play a sound
		if (flSpeed > 500.0f)
		{
			EmitSound("Ball.touch");
		}
	}
	
	//iosgoalnets 82=iosgoalnets, 30=concrete!!! TEMP!!! until pricey changes nets surfaceprop!
	if ((surfaceProps == 82 /*|| surfaceProps == 30*/) && flSpeed > 300.0f)
	{
		EmitSound("Ball.net");
	}

	///////////////////////////////////////////////////////////////////////
	// player
	//this doesnt seem to get called often enough to be any use!!
	IPhysicsObject *pPhysObj = pEvent->pObjects[!index];
	CBaseEntity *pOther = static_cast<CBaseEntity *>(pPhysObj->GetGameData());
	if (pOther && pOther->IsPlayer())
	{
		CSDKPlayer *pPl = ToSDKPlayer(pOther);
		if (flSpeed > 900.0f)
			pPl->EmitSound ("Player.Oomph");

		if (m_pCurStateInfo->m_eBallState == BALL_NORMAL)
		{
			Vector pos;
			m_pPhys->GetPosition(&pos, NULL);
			Touched(pPl, false, BODY_BOUNCE);
		}

		EmitSound("Ball.touch");
	}

	//Warning ("surfaceprops index %d\n", surfaceProps);

	BaseClass::VPhysicsCollision( index, pEvent );
}

void CBall::SendMatchEvent(match_event_t matchEvent, CSDKPlayer *pPlayer)
{
	if (!pPlayer)
		pPlayer = m_pPl;

	Assert(pPlayer);

	SendMatchEvent(matchEvent, pPlayer->GetPlayerName(), pPlayer->GetTeamNumber(), pPlayer->GetUserID(), pPlayer->GetNetworkIDString());
}

void CBall::SendMatchEvent(match_event_t matchEvent, MatchEventPlayerInfo *pMatchEventPlayerInfo)
{
	SendMatchEvent(matchEvent, pMatchEventPlayerInfo->szPlayerName, pMatchEventPlayerInfo->team, pMatchEventPlayerInfo->userId, pMatchEventPlayerInfo->szNetworkIDString);
}

void CBall::SendMatchEvent(match_event_t matchEvent, const char *szPlayerName, int playerTeam, int playerUserID, const char *szPlayerNetworkIDString)
{
	IOS_LogPrintf( "\"%s<%d><%s><%s>\" triggered \"%s\"\n", szPlayerName, playerUserID, szPlayerNetworkIDString, GetGlobalTeam(playerTeam)->GetKitName(), g_szMatchEventNames[matchEvent]);

	CReliableBroadcastRecipientFilter filter;
	UserMessageBegin(filter, "MatchEvent");
		WRITE_BYTE(matchEvent);
		WRITE_STRING(szPlayerName);
		WRITE_BYTE(playerTeam);
	MessageEnd();
}

CSDKPlayer *CBall::FindNearestPlayer(int team /*= TEAM_INVALID*/, int posFlags /*= FL_POS_FIELD*/, bool checkIfShooting /*= false*/, int ignoredPlayerBits /*= 0*/)
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

		if (!(posFlags & FL_POS_ANY) && ((posFlags & FL_POS_KEEPER) && pPlayer->GetTeamPosition() != 1 || (posFlags & FL_POS_FIELD) && pPlayer->GetTeamPosition() == 1))
			continue;

		if (team != TEAM_INVALID && pPlayer->GetTeamNumber() != team)
			continue;

		if (checkIfShooting && (!pPlayer->IsShooting() || pPlayer->m_flNextShot > gpGlobals->curtime))
			continue;

		Vector dir = m_vPos - pPlayer->GetLocalOrigin();
		float xyDist = dir.Length2D();
		float zDist = m_vPos.z - (pPlayer->GetLocalOrigin().z + SDKGameRules()->GetViewVectors()->m_vHullMax.z); //pPlayer->GetPlayerMaxs().z);// 
		float dist = max(xyDist, zDist);

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

void CBall::SetPos(const Vector &pos)
{
	m_vPos = Vector(pos.x, pos.y, pos.z + m_flPhysRadius);
	m_vVel = vec3_origin;
	m_vRot = vec3_origin;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocityInstantaneous(&vec3_origin, &vec3_origin);
	m_pPhys->SetPosition(m_vPos, m_aAng, true);
	m_pPhys->EnableMotion(false);
	m_bSetNewPos = true;
}

void CBall::SetVel(const Vector &vel)
{
	m_vVel = vel;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocity(&m_vVel, &m_vRot);
}

void CBall::SetRot(const AngularImpulse &rot)
{
	m_vRot = rot;
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pPhys->SetVelocity(&m_vVel, &m_vRot);
}

ConVar mp_showballstatetransitions( "mp_showballstatetransitions", "1", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Show ball state transitions." );

void CBall::State_Transition(ball_state_t newState, float delay /*= 0.0f*/, bool cancelQueuedState /*= false*/)
{
	if (delay == 0)
	{
		State_Enter(newState, cancelQueuedState);
	}
	else
	{
		m_eNextState = newState;
		m_flStateLeaveTime = gpGlobals->curtime + delay;
		m_bHasQueuedState = true;
	}
}

void CBall::State_Enter(ball_state_t newState, bool cancelQueuedState)
{
	if (cancelQueuedState)
	{
		m_eNextState = BALL_NOSTATE;
		m_bHasQueuedState = false;
	}

	m_eBallState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );
	m_flStateEnterTime = gpGlobals->curtime;
	m_flStateTimelimit = -1;

	m_pPl = NULL;
	m_pOtherPl = NULL;
	//m_bIgnoreTriggers = false;
	SDKGameRules()->DisableShield();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!CSDKPlayer::IsOnField(pPl))
			continue;

		if (newState != BALL_KEEPERHANDS && pPl->GetTeamPosition() == 1 && pPl->m_pHoldingBall == this)
		{	
			RemoveFromPlayerHands(pPl);
			break;
		}
	}

	if (newState != BALL_NORMAL)
	{
		SDKGameRules()->StartInjuryTime();

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));
			if (!CSDKPlayer::IsOnField(pPl))
				continue;

			pPl->m_bIsAtTargetPos = false;
		}
	}

	if ( mp_showballstatetransitions.GetInt() > 0 )
	{
		if ( m_pCurStateInfo )
			IOS_LogPrintf( "Ball: entering state '%s'\n", m_pCurStateInfo->m_pStateName );
		else
			IOS_LogPrintf( "Ball: entering state #%d\n", newState );
	}

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
	{
		(this->*m_pCurStateInfo->pfnEnterState)();
	}
}

void CBall::State_Think()
{
	m_pPhys->GetPosition(&m_vPos, &m_aAng);
	m_pPhys->GetVelocity(&m_vVel, &m_vRot);

	if (m_eNextState != BALL_NOSTATE && m_flStateLeaveTime <= gpGlobals->curtime)
	{
		State_Enter(m_eNextState, true);
	}

	if (m_pCurStateInfo && m_eBallState != BALL_NORMAL && m_eNextState == BALL_NOSTATE && m_flStateTimelimit != -1 && gpGlobals->curtime >= m_flStateTimelimit)
	{
		if (CSDKPlayer::IsOnField(m_pPl))
		{
			m_pPl->ChangeTeam(TEAM_SPECTATOR);
		}
	}

	if (m_pCurStateInfo && m_pCurStateInfo->pfnThink)
	{	
		(this->*m_pCurStateInfo->pfnThink)();
	}
}

CBallStateInfo* CBall::State_LookupInfo( ball_state_t state )
{
	static CBallStateInfo ballStateInfos[] =
	{
		{ BALL_NORMAL,		"BALL_NORMAL",		&CBall::State_NORMAL_Enter,			&CBall::State_NORMAL_Think },
		{ BALL_KICKOFF,		"BALL_KICKOFF",		&CBall::State_KICKOFF_Enter,		&CBall::State_KICKOFF_Think },
		{ BALL_THROWIN,		"BALL_THROWIN",		&CBall::State_THROWIN_Enter,		&CBall::State_THROWIN_Think },
		{ BALL_GOALKICK,	"BALL_GOALKICK",	&CBall::State_GOALKICK_Enter,		&CBall::State_GOALKICK_Think },
		{ BALL_CORNER,		"BALL_CORNER",		&CBall::State_CORNER_Enter,			&CBall::State_CORNER_Think },
		{ BALL_GOAL,		"BALL_GOAL",		&CBall::State_GOAL_Enter,			&CBall::State_GOAL_Think },
		{ BALL_FREEKICK,	"BALL_FREEKICK",	&CBall::State_FREEKICK_Enter,		&CBall::State_FREEKICK_Think },
		{ BALL_PENALTY,		"BALL_PENALTY",		&CBall::State_PENALTY_Enter,		&CBall::State_PENALTY_Think },
		{ BALL_KEEPERHANDS,	"BALL_KEEPERHANDS",	&CBall::State_KEEPERHANDS_Enter,	&CBall::State_KEEPERHANDS_Think },
	};

	for ( int i=0; i < ARRAYSIZE( ballStateInfos ); i++ )
	{
		if ( ballStateInfos[i].m_eBallState == state )
			return &ballStateInfos[i];
	}

	return NULL;
}

void CBall::State_NORMAL_Enter()
{
	m_pPhys->EnableMotion(true);
	EnablePlayerCollisions(true);
	RemoveEffects(EF_NODRAW);
	m_pPhys->Wake();
	SDKGameRules()->EndInjuryTime();
}

void CBall::State_NORMAL_Think()
{
	for (int ignoredPlayerBits = 0;;)
	{
		if (SDKGameRules()->State_Get() == MATCH_PENALTIES && m_ePenaltyState == PENALTY_KICKED)
			m_pPl = FindNearestPlayer(m_nFoulingTeam, FL_POS_KEEPER, true);
		else
			m_pPl = FindNearestPlayer(TEAM_INVALID, FL_POS_ANY, true, ignoredPlayerBits);

		if (!m_pPl)
			return;

		UpdateCarrier();

		if (DoBodyPartAction())
			break;

		if (SDKGameRules()->State_Get() == MATCH_PENALTIES && m_ePenaltyState == PENALTY_KICKED)
			break;

		ignoredPlayerBits |= (1 << (m_pPl->entindex() - 1));
	}

	MarkOffsidePlayers();
}

void CBall::State_KICKOFF_Enter()
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!CSDKPlayer::IsOnField(pPl))
			continue;

		pPl->RemoveFlag(FL_CELEB);
		pPl->DoServerAnimationEvent(PLAYERANIMEVENT_CANCEL);
		//pPl->m_nInPenBoxOfTeam = TEAM_INVALID;
	}

	SetPos(SDKGameRules()->m_vKickOff);
}

void CBall::State_KICKOFF_Think()
{
	if (!CSDKPlayer::IsOnField(m_pPl))
	{
		m_pPl = FindNearestPlayer(SDKGameRules()->GetKickOffTeam());
		if (!m_pPl)
			m_pPl = FindNearestPlayer(GetGlobalTeam(SDKGameRules()->GetKickOffTeam())->GetOppTeamNumber());
		if (!m_pPl)
		{
			SDKGameRules()->EnableShield(SHIELD_KICKOFF, GetGlobalTeam(TEAM_A)->GetTeamNumber(), SDKGameRules()->m_vKickOff);
			if (!PlayersAtTargetPos(false))
				return;

			return State_Transition(BALL_NORMAL);
		}

		SDKGameRules()->EnableShield(SHIELD_KICKOFF, m_pPl->GetTeamNumber(), SDKGameRules()->m_vKickOff);
		UpdatePossession(m_pPl);
		m_pPl->SetPosInsideShield(Vector(m_vPos.x - m_pPl->GetTeam()->m_nRight * 30, m_vPos.y, SDKGameRules()->m_vKickOff.GetZ()), true);
		m_flStateTimelimit = -1;
		EmitSound("Ball.whistle");
		SendMatchEvent(MATCH_EVENT_KICKOFF);
		//PlayersAtTargetPos(false);
	}

	if (!CSDKPlayer::IsOnField(m_pOtherPl) || m_pOtherPl == m_pPl)
	{
		m_pOtherPl = FindNearestPlayer(m_pPl->GetTeamNumber(), FL_POS_FIELD, false, (1 << (m_pPl->entindex() - 1)));
		if (m_pOtherPl)
			m_pOtherPl->SetPosInsideShield(Vector(m_vPos.x + m_pPl->GetTeam()->m_nRight * 100, m_vPos.y, SDKGameRules()->m_vKickOff.GetZ()), true);
	}

	if (!PlayersAtTargetPos(false))
		return;

	for (int i = 1; i <= gpGlobals->maxClients; i++) 
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!CSDKPlayer::IsOnField(pPl))
			continue;

		if (pPl->GetFlags() & FL_ATCONTROLS)
		{
			if (pPl != m_pPl && pPl != m_pOtherPl)
				pPl->RemoveFlag(FL_ATCONTROLS);
		}
	}

	if (m_flStateTimelimit == -1)
		m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit.GetFloat();

	UpdateCarrier();

	if (m_pPl->m_bShotButtonsReleased && m_pPl->IsShooting())
	{
		RemoveAllTouches();
		SetVel(m_vPlForward2D * 200);
		Kicked(BODY_FEET, 0);
		m_pPl->RemoveFlag(FL_ATCONTROLS);
		if (m_pOtherPl)
			m_pOtherPl->RemoveFlag(FL_ATCONTROLS);
		State_Transition(BALL_NORMAL);
	}
}

void CBall::State_THROWIN_Enter()
{
	UnmarkOffsidePlayers();
	EnablePlayerCollisions(false);
	SetPos(Vector(m_vTriggerTouchPos.x + 0 * Sign(SDKGameRules()->m_vKickOff.GetX() - m_vTriggerTouchPos.x), m_vTriggerTouchPos.y, SDKGameRules()->m_vKickOff.GetZ()));
}

void CBall::State_THROWIN_Think()
{
	if (!CSDKPlayer::IsOnField(m_pPl))
	{
		m_pPl = FindNearestPlayer(LastOppTeam(false));
		if (!m_pPl)
			return State_Transition(BALL_NORMAL);

		SDKGameRules()->EnableShield(SHIELD_THROWIN, m_pPl->GetOppTeamNumber(), m_vPos);
		UpdatePossession(m_pPl);
		m_pPl->SetPosInsideShield(Vector(m_vTriggerTouchPos.x, m_vTriggerTouchPos.y, SDKGameRules()->m_vKickOff.GetZ()), true);
		m_flStateTimelimit = -1;
		SendMatchEvent(MATCH_EVENT_THROWIN);
		EmitSound("Ball.whistle");
		PlayersAtTargetPos(false);
	}

	//if (!PlayersAtTargetPos(false))
	//	return;

	if (!m_pPl->m_bIsAtTargetPos)
		return;

	if (m_pPl->GetFlags() & FL_ATCONTROLS)
	{
		SetPos(Vector(m_vTriggerTouchPos.x, m_vTriggerTouchPos.y, SDKGameRules()->m_vKickOff.GetZ() + VEC_HULL_MAX.z + 2));
		m_pPl->RemoveFlag(FL_ATCONTROLS);
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_THROWIN);
	}

	if (m_flStateTimelimit == -1)
		m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit.GetFloat();

	UpdateCarrier();

	if (m_pPl->m_bShotButtonsReleased && m_pPl->IsShooting())
	{
		QAngle ang = m_pPl->EyeAngles();

		if (ang[PITCH] > -5) //20
			ang[PITCH] = -5;

		Vector dir;
		AngleVectors(ang, &dir);

		if (m_pPl->IsNormalshooting())
		{
			SetVel(dir * sv_ball_throwin_minstrength.GetInt());
		}
		else
		{
			SetVel(dir * GetPowershotStrength(GetPitchMultiplier(), sv_ball_throwin_minstrength.GetInt(), sv_ball_throwin_maxstrength.GetInt()));
		}

		RemoveAllTouches();
		Kicked(BODY_HANDS, 0);
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_THROW);
		EnablePlayerCollisions(true);
		State_Transition(BALL_NORMAL);
	}
}

void CBall::State_GOALKICK_Enter()
{
	Vector ballPos;
	if ((m_vTriggerTouchPos - SDKGameRules()->m_vKickOff).x > 0)
		ballPos = GetGlobalTeam(LastOppTeam(false))->m_vGoalkickLeft;
	else
		ballPos = GetGlobalTeam(LastOppTeam(false))->m_vGoalkickRight;

	SetPos(ballPos);
}

void CBall::State_GOALKICK_Think()
{
	if (!CSDKPlayer::IsOnField(m_pPl))
	{
		m_pPl = FindNearestPlayer(LastOppTeam(false), FL_POS_KEEPER);
		if (!m_pPl)
			m_pPl = FindNearestPlayer(LastOppTeam(false));
		if (!m_pPl)
			return State_Transition(BALL_NORMAL);

		SDKGameRules()->EnableShield(SHIELD_GOALKICK, m_pPl->GetTeamNumber());
		UpdatePossession(m_pPl);
		m_pPl->SetPosInsideShield(Vector(m_vPos.x, m_vPos.y - 100 * m_pPl->GetTeam()->m_nForward, SDKGameRules()->m_vKickOff.GetZ()), false);
		m_flStateTimelimit = -1;
		SendMatchEvent(MATCH_EVENT_GOALKICK);
		EmitSound("Ball.whistle");
		PlayersAtTargetPos(false);
	}

	if (!m_pPl->m_bIsAtTargetPos)
		return;

	if (m_flStateTimelimit == -1)
		m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit.GetFloat();

	UpdateCarrier();

	if (m_pPl->m_bShotButtonsReleased && m_pPl->IsShooting() && (m_vPos - m_vPlPos).Length2D() <= sv_ball_touchradius.GetFloat())
	{
		RemoveAllTouches();
		DoGroundShot();
		MarkOffsidePlayers();
		State_Transition(BALL_NORMAL);
	}
}

void CBall::State_CORNER_Enter()
{
	UnmarkOffsidePlayers();

	Vector ballPos;
	CTeam *pTeam = GetGlobalTeam(LastTeam(false));

	if (Sign((m_vTriggerTouchPos - SDKGameRules()->m_vKickOff).x) == -pTeam->m_nRight)
		ballPos = pTeam->m_vCornerLeft;
	else
		ballPos = pTeam->m_vCornerRight;
	
	SetPos(ballPos);
}

void CBall::State_CORNER_Think()
{
	if (!CSDKPlayer::IsOnField(m_pPl))
	{
		m_pPl = FindNearestPlayer(LastOppTeam(false));
		if (!m_pPl)
			return State_Transition(BALL_NORMAL);

		SDKGameRules()->EnableShield(SHIELD_CORNER, m_pPl->GetOppTeamNumber(), m_vPos);
		m_pPl->SetPosInsideShield(Vector(m_vPos.x - 50 * Sign((SDKGameRules()->m_vKickOff - m_vPos).x), m_vPos.y - 50 * Sign((SDKGameRules()->m_vKickOff - m_vPos).y), SDKGameRules()->m_vKickOff[2]), false);
		UpdatePossession(m_pPl);
		m_flStateTimelimit = -1;
		SendMatchEvent(MATCH_EVENT_CORNER);
		EmitSound("Ball.whistle");
		PlayersAtTargetPos(false);
	}

	//if (!PlayersAtTargetPos(false))
	//	return;

	if (!m_pPl->m_bIsAtTargetPos)
		return;

	if (m_flStateTimelimit == -1)
		m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit.GetFloat();

	UpdateCarrier();

	if (m_pPl->m_bShotButtonsReleased && m_pPl->IsShooting() && (m_vPos - m_vPlPos).Length2D() <= sv_ball_touchradius.GetFloat())
	{
		RemoveAllTouches();
		DoGroundShot();
		State_Transition(BALL_NORMAL);
	}
}

void CBall::State_GOAL_Enter()
{
	UnmarkOffsidePlayers();
	UpdatePossession(NULL);
	//m_bIgnoreTriggers = true;
	SDKGameRules()->SetKickOffTeam(m_nTeam);
	int scoringTeam;

	if (m_nTeam == LastTeam(true))
	{
		scoringTeam = LastOppTeam(true);
		SendMatchEvent(MATCH_EVENT_OWNGOAL, LastPl(true));	
	}
	else
	{
		scoringTeam = LastTeam(true);

		CSDKPlayer *pScorer = LastPl(true);
		if (pScorer)
		{
			pScorer->m_Goals += 1;
			SendMatchEvent(MATCH_EVENT_GOAL, LastPl(true));

			CSDKPlayer *pAssister = LastPl(true, pScorer);

			if (pAssister && pAssister->GetTeam() == pScorer->GetTeam())
			{
				pAssister->m_Assists += 1;
				SendMatchEvent(MATCH_EVENT_ASSIST, pAssister);
			}
		}
	}

	GetGlobalTeam(scoringTeam)->AddGoal();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!CSDKPlayer::IsOnField(pPl))
			continue;

		if (pPl->GetTeamNumber() == scoringTeam)
			pPl->AddFlag(FL_CELEB);
	}

	EmitSound("Ball.whistle");
	State_Transition(BALL_KICKOFF, sv_ball_goalcelebduration.GetFloat());
}

void CBall::State_GOAL_Think()
{
}

void CBall::State_FREEKICK_Enter()
{
	SetPos(m_vFoulPos);

	match_event_t matchEvent;

	switch (m_eFoulType)
	{
	case FOUL_NORMAL_NO_CARD:
	case FOUL_NORMAL_YELLOW_CARD:
	case FOUL_NORMAL_RED_CARD:
		matchEvent = MATCH_EVENT_FOUL;
		break;
	case FOUL_DOUBLETOUCH:
		matchEvent = MATCH_EVENT_DOUBLETOUCH;
		break;
	case FOUL_OFFSIDE:
		matchEvent = MATCH_EVENT_OFFSIDE;
		SetOffsideLinesEnabled(true);
		break;
	default:
		matchEvent = MATCH_EVENT_NONE;
	}

	if (CSDKPlayer::IsOnField(m_pFoulingPl))
	{
		m_pFoulingPl->m_Fouls += 1;
		SendMatchEvent(matchEvent, m_pFoulingPl);

		if (m_eFoulType == FOUL_NORMAL_YELLOW_CARD)
		{
			m_pFoulingPl->m_YellowCards += 1;
			SendMatchEvent(MATCH_EVENT_YELLOWCARD, m_pFoulingPl);
		}

		if (m_eFoulType == FOUL_NORMAL_YELLOW_CARD && m_pFoulingPl->m_YellowCards % 2 == 0 || m_eFoulType == FOUL_NORMAL_RED_CARD)
		{
			m_pFoulingPl->m_RedCards += 1;
			SendMatchEvent(MATCH_EVENT_REDCARD, m_pFoulingPl);

			float banDuration = (m_eFoulType == FOUL_NORMAL_YELLOW_CARD ? sv_ball_player_yellow_red_card_duration.GetFloat() : sv_ball_player_red_card_duration.GetFloat()) * 60 / (90.0f / mp_timelimit_match.GetFloat());

			m_pFoulingPl->m_flNextJoin = gpGlobals->curtime + banDuration;
			m_pFoulingPl->ChangeTeam(TEAM_SPECTATOR);
		}
	}
}

void CBall::State_FREEKICK_Think()
{
	if (!CSDKPlayer::IsOnField(m_pPl))
	{
		if (abs(m_vPos.y - GetGlobalTeam(m_nFoulingTeam)->GetOppTeam()->m_vPlayerSpawns[0].y) <= 1000)
			m_pPl = FindNearestPlayer(GetGlobalTeam(m_nFoulingTeam)->GetOppTeamNumber(), FL_POS_KEEPER);
		else
			m_pPl = m_pFouledPl;

		if (!CSDKPlayer::IsOnField(m_pPl) || m_pPl->GetTeamPosition() == 1 && m_pPl->IsBot())
			m_pPl = FindNearestPlayer(GetGlobalTeam(m_nFoulingTeam)->GetOppTeamNumber(), FL_POS_FIELD);

		if (!m_pPl)
			return State_Transition(BALL_NORMAL);

		SDKGameRules()->EnableShield(SHIELD_FREEKICK, m_pPl->GetOppTeamNumber(), m_vPos);
		m_pPl->SetPosInsideShield(Vector(m_vPos.x, m_vPos.y - 100 * m_pPl->GetTeam()->m_nForward, SDKGameRules()->m_vKickOff.GetZ()), false);
		UpdatePossession(m_pPl);
		m_flStateTimelimit = -1;
		SendMatchEvent(MATCH_EVENT_FREEKICK);
		EmitSound("Ball.whistle");
		PlayersAtTargetPos(false);
	}

	if (!m_pPl->m_bIsAtTargetPos)
		return;

	if (m_flStateTimelimit == -1)
	{
		m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit.GetFloat();

		if (sv_ball_reset_stamina_on_freekicks.GetBool())
			m_pPl->m_Shared.SetStamina(0);
	}

	UpdateCarrier();

	if (m_pPl->m_bShotButtonsReleased && m_pPl->IsShooting() && (m_vPos - m_vPlPos).Length2D() <= sv_ball_touchradius.GetFloat())
	{
		SetOffsideLinesEnabled(false);
		RemoveAllTouches();
		DoGroundShot();
		MarkOffsidePlayers();
		State_Transition(BALL_NORMAL);
	}
}

void CBall::State_PENALTY_Enter()
{
	UnmarkOffsidePlayers();

	if (SDKGameRules()->State_Get() == MATCH_PENALTIES)
	{
		SetPos(GetGlobalTeam(m_nFoulingTeam)->m_vPenalty);
	}
	else
	{
		SetPos(GetGlobalTeam(m_nFoulingTeam)->m_vPenalty);

		if (m_pFoulingPl)
			SendMatchEvent(MATCH_EVENT_FOUL, m_pFoulingPl);
	}
}

void CBall::State_PENALTY_Think()
{
	if (!CSDKPlayer::IsOnField(m_pPl))
	{
		m_pPl = m_pFouledPl;

		if (SDKGameRules()->State_Get() == MATCH_PENALTIES)
		{
			if (!CSDKPlayer::IsOnField(m_pPl))
			{
				m_ePenaltyState = PENALTY_ABORTED_NO_TAKER;
				return State_Transition(BALL_NORMAL);
			}

			SDKGameRules()->EnableShield(SHIELD_PENALTY, m_nFoulingTeam, GetGlobalTeam(m_nFoulingTeam)->m_vPenalty);
			m_pPl->SetPosInsideShield(Vector(m_vPos.x, m_vPos.y - 150 * m_pPl->GetTeam()->m_nForward, SDKGameRules()->m_vKickOff.GetZ()), true);
		}
		else
		{
			if (!CSDKPlayer::IsOnField(m_pPl))
			{
				m_pPl = FindNearestPlayer(GetGlobalTeam(m_nFoulingTeam)->GetOppTeamNumber());
				if (!m_pPl)
					return State_Transition(BALL_NORMAL);
			}

			SDKGameRules()->EnableShield(SHIELD_PENALTY, m_nFoulingTeam, GetGlobalTeam(m_nFoulingTeam)->m_vPenalty);
			m_pPl->SetPosInsideShield(Vector(m_vPos.x, m_vPos.y - 150 * m_pPl->GetTeam()->m_nForward, SDKGameRules()->m_vKickOff.GetZ()), true);
		}

		UpdatePossession(m_pPl);
		m_flStateTimelimit = -1;
		SendMatchEvent(MATCH_EVENT_PENALTY);
		EmitSound("Ball.whistle");
	}

	if (!CSDKPlayer::IsOnField(m_pOtherPl))
	{
		if (SDKGameRules()->State_Get() == MATCH_PENALTIES)
		{
			m_pOtherPl = FindNearestPlayer(m_pPl->GetOppTeamNumber(), FL_POS_KEEPER);
			if (!m_pOtherPl)
			{
				m_ePenaltyState = PENALTY_ABORTED_NO_KEEPER;
				return State_Transition(BALL_NORMAL);
			}
		}
		else
		{
			m_pOtherPl = FindNearestPlayer(m_nInPenBoxOfTeam, FL_POS_KEEPER);
			if (!m_pOtherPl)
				return State_Transition(BALL_NORMAL);

		}

		m_pOtherPl->SetPosInsideShield(m_pOtherPl->GetTeam()->m_vPlayerSpawns[0], true);
	}

	if (!PlayersAtTargetPos(false))
		return;

	if (m_pPl->GetFlags() & FL_ATCONTROLS)
		m_pPl->RemoveFlag(FL_ATCONTROLS);

	if (m_pOtherPl->GetFlags() & FL_ATCONTROLS)
	{
		m_pOtherPl->RemoveFlag(FL_ATCONTROLS);
		m_pOtherPl->AddFlag(FL_NO_Y_MOVEMENT);
	}

	if (m_flStateTimelimit == -1)
		m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit.GetFloat();

	UpdateCarrier();

	if (m_pPl->m_bShotButtonsReleased && m_pPl->IsShooting() && (m_vPos - m_vPlPos).Length2D() <= sv_ball_touchradius.GetFloat())
	{
		RemoveAllTouches();
		m_ePenaltyState = PENALTY_KICKED;
		m_pPl->m_ePenaltyState = PENALTY_KICKED;
		m_pOtherPl->RemoveFlag(FL_NO_Y_MOVEMENT);
		DoGroundShot();
		State_Transition(BALL_NORMAL);
	}
}

void CBall::State_KEEPERHANDS_Enter()
{
}

void CBall::State_KEEPERHANDS_Think()
{
	if (!CSDKPlayer::IsOnField(m_pPl))
	{
		m_pPl = FindNearestPlayer(m_nInPenBoxOfTeam, FL_POS_KEEPER);
		if (!m_pPl)
		{
			return State_Transition(BALL_NORMAL);
		}

		if (!SDKGameRules()->IsIntermissionState() && !m_bHasQueuedState)
		{
			SDKGameRules()->EnableShield(SHIELD_KEEPERHANDS, m_pPl->GetTeamNumber());
			UpdatePossession(m_pPl);
			m_pPl->m_bIsAtTargetPos = true;
			PlayersAtTargetPos(false);
		}

		if (m_pPl->m_nButtons & (IN_ATTACK | (IN_ATTACK2 | IN_ALT1 | IN_ALT2)))
				m_pPl->m_bShotButtonsReleased = false;

		m_pHoldingPlayer = m_pPl;
		m_pPl->m_pHoldingBall = this;
		m_pPl->m_nBody = MODEL_KEEPER_AND_BALL;
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_CARRY);
		m_pPl->m_nSkin = m_pPl->m_nBaseSkin + m_nSkin;
		SetEffects(EF_NODRAW);
		EnablePlayerCollisions(false);
		m_flStateTimelimit = -1;
		Touched(m_pPl, true, BODY_HANDS);
	}

	if (!SDKGameRules()->IsIntermissionState() && !m_bHasQueuedState && SDKGameRules()->State_Get() != MATCH_PENALTIES)
	{
		if (m_flStateTimelimit == -1)
			m_flStateTimelimit = gpGlobals->curtime + sv_ball_timelimit.GetFloat();
	}

	UpdateCarrier();

	//SetPos(Vector(m_vPlPos.x, m_vPlPos.y, m_vPlPos.z + BODY_CHEST_START) + m_vPlForward2D * 2 * VEC_HULL_MAX.x);

	if (m_nInPenBoxOfTeam != m_pPl->GetTeamNumber())
	{
		RemoveAllTouches();
		Kicked(BODY_HANDS, 0);
		MarkOffsidePlayers();
		RemoveFromPlayerHands(m_pPl);
		return State_Transition(BALL_NORMAL);
	}

	if (m_pPl->m_bShotButtonsReleased && m_pPl->IsShooting() && m_pPl->m_flNextShot <= gpGlobals->curtime)
	{
		float spin;

		if (m_pPl->IsPowershooting())
		{
			SetVel(m_vPlForward * GetPowershotStrength(GetPitchMultiplier(), sv_ball_powershot_minstrength.GetInt(), sv_ball_powershot_maxstrength.GetInt()));
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KEEPER_HANDS_KICK);
			spin = 1;
		}
		else
		{
			SetVel(m_vPlForward * sv_ball_normalshot_strength.GetInt() * GetPitchMultiplier());
			m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_KEEPER_HANDS_THROW);
			spin = 0;
		}

		RemoveAllTouches();
		Kicked(BODY_HANDS, spin);
		MarkOffsidePlayers();
		RemoveFromPlayerHands(m_pPl);

		return State_Transition(BALL_NORMAL);
	}
}

bool CBall::PlayersAtTargetPos(bool holdAtTargetPos)
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
				if (mp_shield_liberal_teammates_positioning.GetBool() && m_eBallState != BALL_KICKOFF && m_eBallState != BALL_PENALTY && pPl->GetTeamNumber() == m_pPl->GetTeamNumber())
					pPl->SetPosOutsideBall();
				else
					pPl->SetPosOutsideShield(holdAtTargetPos);
			}

			if (!pPl->m_bIsAtTargetPos)
				playersAtTarget = false;
		}
	}

	return playersAtTarget;
}

bool CBall::CheckFoul(bool canShootBall)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++) 
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!CSDKPlayer::IsOnField(pPl))
			continue;

		if (pPl == m_pPl || pPl->GetTeamNumber() == m_nPlTeam)
			continue;

		Vector plPos = pPl->GetLocalOrigin();

		if (plPos.x < SDKGameRules()->m_vFieldMin.GetX() || plPos.y < SDKGameRules()->m_vFieldMin.GetY() ||
			plPos.x > SDKGameRules()->m_vFieldMax.GetX() || plPos.y > SDKGameRules()->m_vFieldMax.GetY())
			continue;

		Vector dirToPl = pPl->GetLocalOrigin() - m_vPlPos;
		float distToPl = dirToPl.Length2D();

		Vector localDirToPl;
		VectorIRotate(dirToPl, m_pPl->EntityToWorldTransform(), localDirToPl);

		dirToPl.z = 0;
		dirToPl.NormalizeInPlace();
		//if (RAD2DEG(acos(m_vPlForward2D.Dot(dirToPl))) > sv_ball_slideangle.GetFloat())
		if (localDirToPl.x < 0 || localDirToPl.x > sv_ball_slideforwardreach.GetInt() || abs(localDirToPl.y) > sv_ball_slidesidereach.GetInt())		
			continue;

		if (/*canShootBall && */distToPl >= (m_vPos - m_vPlPos).Length2D())
			continue;

		PlayerAnimEvent_t anim = RAD2DEG(acos(m_vPlForward2D.Dot(pPl->EyeDirection2D()))) <= 90 ? PLAYERANIMEVENT_TACKLED_BACKWARD : PLAYERANIMEVENT_TACKLED_FORWARD;

		pPl->DoAnimationEvent(anim);

		int teammatesCloserToGoalCount = 0;

		for (int j = 1; j <= gpGlobals->maxClients; j++) 
		{
			CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(j));

			if (!CSDKPlayer::IsOnField(pPl) || pPl == m_pPl || pPl->GetTeamNumber() != m_nPlTeam || pPl->GetTeamPosition() == 1)
				continue;

			if (Sign(m_vPlPos.y - pPl->GetLocalOrigin().y) == m_pPl->GetTeam()->m_nForward)
				teammatesCloserToGoalCount += 1;
		}

		foul_type_t foulType;

		if (teammatesCloserToGoalCount == 0)
			foulType = FOUL_NORMAL_RED_CARD;
		else if (anim == PLAYERANIMEVENT_TACKLED_BACKWARD)
			foulType = FOUL_NORMAL_YELLOW_CARD;
		else
			foulType = FOUL_NORMAL_NO_CARD;

		TriggerFoul(foulType, pPl->GetLocalOrigin(), m_pPl, pPl);

		if (pPl->m_nInPenBoxOfTeam == m_nPlTeam)
			State_Transition(BALL_PENALTY, sv_ball_statetransitiondelay.GetFloat());
		else
			State_Transition(BALL_FREEKICK, sv_ball_statetransitiondelay.GetFloat());

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
	m_vFoulPos.x = clamp(pos.x, SDKGameRules()->m_vFieldMin.GetX() + 2 * m_flPhysRadius, SDKGameRules()->m_vFieldMax.GetX() - 2 * m_flPhysRadius);
	m_vFoulPos.y = clamp(pos.y, SDKGameRules()->m_vFieldMin.GetY() + 2 * m_flPhysRadius, SDKGameRules()->m_vFieldMax.GetY() - 2 * m_flPhysRadius);
	m_vFoulPos.z = SDKGameRules()->m_vKickOff.GetZ();
}

bool CBall::DoBodyPartAction()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	float zDist = dirToBall.z;
	float xyDist = dirToBall.Length2D();

	if (m_pPl->m_ePlayerAnimEvent == PLAYERANIMEVENT_SLIDE)
		return DoSlideAction();

	if (m_pPl->GetTeamPosition() == 1 && m_nInPenBoxOfTeam == m_pPl->GetTeamNumber() && !m_pPl->m_pHoldingBall)
	{
		if (CheckKeeperCatch())
			return true;
	}

	if (m_vVel.Length2D() >= sv_ball_minspeed_passive.GetInt() && gpGlobals->curtime < m_flNextShot)
		return false;

	if (m_vVel.Length2D() >= sv_ball_minspeed_deflect.GetInt() && gpGlobals->curtime < m_flNextShot)
		return false;

	if (/*zDist >= BODY_FEET_START && */zDist < BODY_FEET_END && xyDist <= sv_ball_touchradius.GetInt())
		return DoGroundShot();

	if (zDist >= BODY_FEET_END && zDist < BODY_HIP_END && xyDist <= sv_ball_touchradius.GetInt())
	{
		if (DoVolleyShot())
			return true;
		else
			return DoChestDrop();
	}

	if (zDist >= BODY_HIP_END && zDist < BODY_CHEST_END && xyDist <= sv_ball_touchradius.GetInt())
		return DoChestDrop();

	if (zDist >= BODY_CHEST_END && zDist < BODY_HEAD_END && xyDist <= sv_ball_touchradius.GetInt())
		return DoHeader();

	return false;
}

bool CBall::DoSlideAction()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	float zDist = dirToBall.z;
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);

	//bool canShootBall = RAD2DEG(acos(m_vPlForward2D.Dot(dirToBall))) <= sv_ball_slideangle.GetFloat() && zDist <= BODY_FEET_END;
	bool canShootBall = zDist < BODY_HIP_END && localDirToBall.x >= 0 && localDirToBall.x <= sv_ball_slideforwardreach.GetFloat() && abs(localDirToBall.y) <= sv_ball_slidesidereach.GetFloat();

	if (!canShootBall)
		return false;

	if (!SDKGameRules()->IsIntermissionState() && !m_bHasQueuedState)
	{
		if (CheckFoul(canShootBall))
			return true;
	}

	Vector ballVel = m_vPlVel2D + m_vPlForward2D * sv_ball_slidemultiplier.GetFloat();

	if (m_pPl->m_nButtons & IN_MOVELEFT)
	{
		VectorYawRotate(ballVel, 45, ballVel);
		ballVel *= sv_ball_slidesidemultiplier.GetFloat();
	}
	else if (m_pPl->m_nButtons & IN_MOVERIGHT)
	{
		VectorYawRotate(ballVel, -45, ballVel);
		ballVel *= sv_ball_slidesidemultiplier.GetFloat();
	}

	SetVel(ballVel);
	Kicked(BODY_FEET, 0);

	return true;
}

bool CBall::CheckKeeperCatch()
{
	Vector dirToBall = m_vPos - m_vPlPos;
	float zDist = dirToBall.z;
	float xyDist = dirToBall.Length2D();
	Vector localDirToBall;
	VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);

	bool canCatch;

	switch (m_pPl->m_ePlayerAnimEvent)
	{
	case PLAYERANIMEVENT_KEEPER_DIVE_LEFT:
		canCatch = zDist < BODY_HEAD_END && abs(localDirToBall.x) <= sv_ball_keepershortsidereach.GetInt() && localDirToBall.y >= 0 && localDirToBall.y <= sv_ball_keeperlongsidereach.GetInt();
		break;
	case PLAYERANIMEVENT_KEEPER_DIVE_RIGHT:
		canCatch = zDist < BODY_HEAD_END && abs(localDirToBall.x) <= sv_ball_keepershortsidereach.GetInt() && localDirToBall.y <= 0 && localDirToBall.y >= -sv_ball_keeperlongsidereach.GetInt();
		break;
	case PLAYERANIMEVENT_KEEPER_DIVE_FORWARD:
		canCatch = zDist < BODY_CHEST_END && localDirToBall.x >= 0 && localDirToBall.x <= sv_ball_keeperlongsidereach.GetInt() && abs(localDirToBall.y) <= sv_ball_keepershortsidereach.GetInt();
		break;
	case PLAYERANIMEVENT_KEEPER_DIVE_BACKWARD:
		canCatch = zDist < BODY_HEAD_END + 20 && localDirToBall.x <= 0 && localDirToBall.x >= -sv_ball_keeperlongsidereach.GetInt() && abs(localDirToBall.y) <= sv_ball_keepershortsidereach.GetInt();
		break;
	case PLAYERANIMEVENT_KEEPER_JUMP:
		canCatch = xyDist <= sv_ball_keepertouchradius.GetFloat() && zDist >= BODY_CHEST_START && zDist < BODY_HEAD_END + 20;
		break;
	default:
		canCatch = xyDist <= sv_ball_keepertouchradius.GetFloat() && (zDist >= BODY_CHEST_START && zDist < BODY_HEAD_END || zDist < BODY_FEET_END && m_aPlAng[PITCH] >= sv_ball_keeperpickupangle.GetInt());
		break;
	}

	if (!canCatch)
		return false;

	if (!SDKGameRules()->IsIntermissionState() && SDKGameRules()->State_Get() != MATCH_PENALTIES)
	{
		if (LastPl(true) == m_pPl && !LastPl(true, m_pPl))
			return false;

		BallTouchInfo *pInfo = LastInfo(true, m_pPl);
		if (!pInfo)
			return false;

		if (pInfo->m_nTeam == m_pPl->GetTeamNumber() && pInfo->m_eBodyPart != BODY_HEAD && pInfo->m_eBodyPart != BODY_CHEST)
			return false;
	}

	if (m_vVel.Length2D() > sv_ball_keepercatchspeed.GetInt()/* || SDKGameRules()->IsIntermissionState() || IgnoreTriggers()*/)
	{
		Vector ballVel = m_vVel;

		//if (SDKGameRules()->IsIntermissionState() || IgnoreTriggers())
		//{
		//	if (m_vVel.Length2D() <= sv_ball_keepercatchspeed.GetInt())
		//		ballVel = -m_vPlForward2D * sv_ball_keepercatchspeed.GetInt();
		//}

		if (m_pPl->m_ePlayerAnimEvent == PLAYERANIMEVENT_KEEPER_DIVE_BACKWARD)
		{
			SetVel(Vector(ballVel.x, ballVel.y, max(ballVel.z, sv_ball_keeperpunchupstrength.GetInt())));
		}
		else
			SetVel(m_vPlForward * ballVel.Length2D() * 0.75f);

		Kicked(BODY_HANDS, 0);
	}
	else
		State_Transition(BALL_KEEPERHANDS);

	return true;
}

float CBall::GetPitchMultiplier()
{
	return pow(cos((m_aPlAng[PITCH] - sv_ball_bestshotangle.GetInt()) / (PITCH_LIMIT - sv_ball_bestshotangle.GetInt()) * M_PI / 2), 2);
}

float CBall::GetPowershotStrength(float multiplier, int minStrength, int maxStrength)
{
	int powershotStrength;
	
	if (m_pPl->m_nButtons & IN_ALT1)
		powershotStrength = m_pPl->m_nPowershotStrength;
	else if (m_pPl->m_nButtons & IN_ALT2)
		powershotStrength = 100;
	else
		powershotStrength = mp_powershot_fixed_strength.GetInt();

	if (State_Get() == BALL_PENALTY)
		powershotStrength = min(powershotStrength, mp_powershot_fixed_strength.GetInt());

	powershotStrength = min(powershotStrength, m_pPl->m_Shared.GetStamina());

	m_pPl->m_Shared.SetStamina(m_pPl->m_Shared.GetStamina() - multiplier * powershotStrength);

	return max(sv_ball_minshotstrength.GetInt(), multiplier * (minStrength + (maxStrength - minStrength) * (powershotStrength / 100.0f)));
}

bool CBall::DoGroundShot()
{
	float spin;

	if (m_pPl->IsAutoPassing())
	{
		CSDKPlayer *pPl = m_pPl->FindClosestPlayerToSelf(true, true);
		if (!pPl)
			pPl = m_pPl->FindClosestPlayerToSelf(true, false);
		if (!pPl)
			return false;

		Vector dir = pPl->GetLocalOrigin() - m_vPlPos;
		float length = dir.Length2D();
		QAngle ang;
		VectorAngles(dir, ang);
		ang[PITCH] = -30;
		AngleVectors(ang, &dir);
		float shotStrength = length * sv_ball_autopass_multiplier.GetFloat();
		float multiplier = (shotStrength - sv_ball_autopass_minstrength.GetInt()) / (sv_ball_autopass_maxstrength.GetInt() - sv_ball_autopass_minstrength.GetInt());
		SetVel(dir * GetPowershotStrength(clamp(multiplier, 0.0f, 1.0f), sv_ball_autopass_minstrength.GetInt(), sv_ball_autopass_maxstrength.GetInt()));
		spin = 0;
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_PASS);
		EmitSound("Ball.kicknormal");
	}
	else
	{
		Vector dirToBall = m_vPos - m_vPlPos;
		Vector localDirToBall;
		VectorIRotate(dirToBall, m_pPl->EntityToWorldTransform(), localDirToBall);

		QAngle shotAngle = m_aPlAng;
		shotAngle[PITCH] = min(-5, m_aPlAng[PITCH]);

		Vector shotDir;
		AngleVectors(shotAngle, &shotDir);

		float shotStrength;

		if (m_pPl->IsPowershooting())
		{
			shotStrength = GetPowershotStrength(GetPitchMultiplier(), sv_ball_powershot_minstrength.GetInt(), sv_ball_powershot_maxstrength.GetInt());
		}
		else
		{
			shotStrength = sv_ball_normalshot_strength.GetFloat() * GetPitchMultiplier();		//do normal kick instead
		}

		SetVel(shotDir * max(shotStrength, sv_ball_minshotstrength.GetInt()));

		if (m_vVel.Length() > 600)
		{
			PlayerAnimEvent_t anim = PLAYERANIMEVENT_NONE;
			EmitSound("Ball.kickhard");

			//if (m_vVel.Length() > 800)
			anim = PLAYERANIMEVENT_KICK;
			//else
			//	anim = PLAYERANIMEVENT_PASS;

			m_pPl->DoServerAnimationEvent(anim);
		}
		else
		{
			if (localDirToBall.x < 0 && m_aPlAng[PITCH] <= -45)
				m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEELKICK);

			EmitSound("Ball.touch");
		}

		spin = 1;
	}

	Kicked(BODY_FEET, spin);

	return true;
}

bool CBall::DoVolleyShot()
{
	if (!m_pPl->IsPowershooting() || m_pPl->GetGroundEntity())
		return false;

	SetVel(m_vPlForward * GetPowershotStrength(GetPitchMultiplier(), sv_ball_powershot_minstrength.GetInt(), sv_ball_powershot_maxstrength.GetInt()));
	EmitSound("Ball.kickhard");
	m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_VOLLEY);
	Kicked(BODY_FEET, sv_ball_volleyshot_spinmultiplier.GetFloat());

	return true;
}

bool CBall::DoChestDrop()
{
	SetVel(m_vPlVel2D + m_vPlForward2D * sv_ball_chestdrop_strength.GetInt());
	EmitSound("Ball.kicknormal");
	Kicked(BODY_CHEST, 0);

	return true;
}

bool CBall::DoHeader()
{
	if (m_pPl->IsPowershooting() && m_vPlVel.Length2D() >= mp_walkspeed.GetInt() && m_nInPenBoxOfTeam == m_pPl->GetOppTeamNumber() && (m_pPl->m_nButtons & IN_SPEED))
	{
		SetVel(m_vPlVel2D + m_vPlForward2D * GetPowershotStrength(1.0f, sv_ball_powerdivingheader_minstrength.GetInt(), sv_ball_powerdivingheader_maxstrength.GetInt()));
		EmitSound("Ball.kickhard");
		m_pPl->AddFlag(FL_FREECAM);
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_DIVINGHEADER);
	}
	else if (m_pPl->IsPowershooting())
	{
		SetVel(m_vPlVel + m_vPlForward * GetPowershotStrength(1.0f, sv_ball_powerheader_minstrength.GetInt(), sv_ball_powerheader_maxstrength.GetInt()));
		EmitSound("Ball.kickhard");
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEADER);
	}
	else
	{
		SetVel(m_vPlVel + m_vPlForward * sv_ball_normalheader_strength.GetInt());
		EmitSound("Ball.kicknormal");
		m_pPl->DoServerAnimationEvent(PLAYERANIMEVENT_HEADER_STATIONARY);
	}

	Kicked(BODY_HEAD, 0);

	return true;
}

void CBall::SetSpin(float multiplier)
{
	Vector rot(0, 0, 0);

	if (m_pPl->m_nButtons & IN_MOVELEFT) 
		rot += Vector(0, 0, 1);
	else if (m_pPl->m_nButtons & IN_MOVERIGHT) 
		rot += Vector(0, 0, -1);

	if (m_pPl->m_nButtons & IN_TOPSPIN)
		rot += -m_vPlRight * sv_ball_topspin_multiplier.GetFloat();
	else if (m_pPl->m_nButtons & IN_BACKSPIN)
		rot += m_vPlRight * sv_ball_backspin_multiplier.GetFloat();

	if (rot.z != 0)
		rot.NormalizeInPlace();
	//float spin = min(1, m_vVel.Length() / sv_ball_maxspin.GetInt()) * sv_ball_spin.GetFloat();
	float spin = m_vVel.Length() * sv_ball_spin.GetInt() * multiplier / 100.0f;

	AngularImpulse randRot = AngularImpulse(0, 0, 0);
	//if (rot.Length() == 0)
	{
		for (int i = 0; i < 3; i++)
		{
			randRot[i] = sv_ball_defaultspin.GetInt() / 100.0f * (g_IOSRand.RandomInt(0, 1) == 1 ? 1 : -1);
			//randRot[i] = m_vVel.Length() * g_IOSRand.RandomFloat(-sv_ball_defaultspin.GetInt(), sv_ball_defaultspin.GetInt()) / 100.0f;
		}
	}

	SetRot(WorldToLocalRotation(SetupMatrixAngles(m_aAng), rot, spin) + randRot);
}

void CBall::Think( void	)
{
	SetNextThink(gpGlobals->curtime + sv_ball_thinkinterval.GetFloat());

	State_Think();
}

void CBall::TriggerGoal(int team)
{
	if (SDKGameRules()->State_Get() == MATCH_PENALTIES)
	{
		if (team == m_nFoulingTeam)
		{
			GetGlobalTeam(m_nFoulingTeam)->GetOppTeam()->AddGoal();
			SendMatchEvent(MATCH_EVENT_GOAL, m_pFouledPl);
			m_bHasQueuedState = true;
		}

		return;
	}

	m_nTeam = team;
	State_Transition(BALL_GOAL, sv_ball_statetransitiondelay.GetFloat());
}

void CBall::TriggerGoalLine(int team)
{
	//DevMsg("Trigger goal line\n");
	if (SDKGameRules()->State_Get() == MATCH_PENALTIES)
		return;

	m_pPhys->GetPosition(&m_vTriggerTouchPos, NULL);

	if (LastTeam(false) == team)
		State_Transition(BALL_CORNER, sv_ball_statetransitiondelay.GetFloat());
	else
		State_Transition(BALL_GOALKICK, sv_ball_statetransitiondelay.GetFloat());
}

void CBall::TriggerSideline()
{
	if (SDKGameRules()->State_Get() == MATCH_PENALTIES)
		return;

	Vector ballPos;
	m_pPhys->GetPosition(&ballPos, NULL);
	CBaseEntity *pThrowIn = gEntList.FindEntityByClassnameNearest("info_throw_in", ballPos, 1000);
	if (!pThrowIn)
		return;

	m_vTriggerTouchPos = pThrowIn->GetLocalOrigin();
	State_Transition(BALL_THROWIN, sv_ball_statetransitiondelay.GetFloat());
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
		AngleVectors(m_aPlAng, &m_vPlForward, &m_vPlRight, &m_vPlUp);
		m_vPlForward2D = m_vPlForward;
		m_vPlForward2D.z = 0;
		m_vPlForward2D.NormalizeInPlace();
		m_nPlTeam = m_pPl->GetTeamNumber();
		m_nPlPos = m_pPl->GetTeamPosition();
	}
	else
	{
		m_nPlTeam = TEAM_INVALID;
		m_nPlPos = 0;
	}
}

void CBall::MarkOffsidePlayers()
{
	if (SDKGameRules()->IsIntermissionState() || m_bHasQueuedState || SDKGameRules()->State_Get() == MATCH_PENALTIES)
		return;

	m_vOffsidePos = m_vPos;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (CSDKPlayer::IsOnField(pPl))
			pPl->SetOffside(false);

		if (!(CSDKPlayer::IsOnField(pPl) && pPl != m_pPl && pPl->GetTeamNumber() == LastTeam(true)))
			continue;

		Vector pos = pPl->GetLocalOrigin();
		int forward = pPl->GetTeam()->m_nForward;

		// In opponent half?
		if (Sign((pos - SDKGameRules()->m_vKickOff).y) != forward)
			continue;

		// Closer to goal than the ball?
		if (Sign((pos - m_vPos).y) != forward)
			continue;

		int oppPlayerCount = 0;
		int nearerPlayerCount = 0;
		CSDKPlayer *pLastPl = NULL;
		float shortestDist = FLT_MAX;
		// Count players who are nearer to goal
		for (int j = 1; j <= gpGlobals->maxClients; j++)
		{
			CSDKPlayer *pOpp = ToSDKPlayer(UTIL_PlayerByIndex(j));
			if (!(pOpp && pOpp->GetTeamNumber() == pPl->GetOppTeamNumber()))
				continue;

			oppPlayerCount += 1;

			if (Sign(pOpp->GetLocalOrigin().y - pos.y) == forward)
				nearerPlayerCount += 1;
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

void CBall::Kicked(body_part_t bodyPart, float spin)
{
	float delay;

	if (State_Get() == BALL_NORMAL)
	{
		if (sv_ball_dynamicshotdelay_enabled.GetBool())
			delay = RemapValClamped(m_vVel.Length(), sv_ball_dynamicshotdelay_minshotstrength.GetInt(), sv_ball_dynamicshotdelay_maxshotstrength.GetInt(), sv_ball_dynamicshotdelay_mindelay.GetFloat(), sv_ball_dynamicshotdelay_maxdelay.GetFloat());
		else
			delay = sv_ball_shotdelay_normal.GetFloat();
	}
	else
	{
		delay = sv_ball_shotdelay_setpiece.GetFloat();
	}

	SetSpin(spin);
	m_pPl->m_flNextShot = gpGlobals->curtime + delay;
	m_flNextShot = gpGlobals->curtime + sv_ball_shotdelay_global.GetFloat();
	Touched(m_pPl, true, bodyPart);
}

void CBall::Touched(CSDKPlayer *pPl, bool isShot, body_part_t bodyPart)
{
	if (SDKGameRules()->IsIntermissionState() || m_bHasQueuedState || SDKGameRules()->State_Get() == MATCH_PENALTIES)
		return;

	//DevMsg("Touched %0.2f\n", gpGlobals->curtime);
	if (m_Touches.Count() > 0 && m_Touches.Tail().m_pPl == pPl && m_Touches.Tail().m_nTeam == pPl->GetTeamNumber())
	{
		if (sv_ball_doubletouchfouls.GetBool() && m_Touches.Tail().m_eBallState != BALL_NORMAL && m_Touches.Tail().m_eBallState != BALL_KEEPERHANDS && pPl->GetTeam()->GetNumPlayers() >= 3)
		{
			//pPl->m_Fouls += 1;
			TriggerFoul(FOUL_DOUBLETOUCH, pPl->GetLocalOrigin(), pPl);
			State_Transition(BALL_FREEKICK, sv_ball_statetransitiondelay.GetFloat());
			return;
		}
		else
		{
			m_Touches.Tail().m_bIsShot = isShot;
			m_Touches.Tail().m_eBodyPart = bodyPart;
			m_Touches.Tail().m_eBallState = m_pCurStateInfo->m_eBallState;
		}
	}
	else
	{
		UpdatePossession(pPl);
		BallTouchInfo info = { pPl, pPl->GetTeamNumber(), isShot, bodyPart, m_pCurStateInfo->m_eBallState };
		m_Touches.AddToTail(info);
	}
	
	if (pPl->IsOffside())
	{
		pPl->m_Offsides += 1;
		TriggerFoul(FOUL_OFFSIDE, pPl->GetOffsidePos(), pPl);
		SetOffsideLinePositions(pPl->GetOffsideBallPos().y, pPl->GetOffsidePos().y, pPl->GetOffsideLastOppPlayerPos().y);
		State_Transition(BALL_FREEKICK, sv_ball_statetransitiondelay.GetFloat());
	}
}

void CBall::RemoveAllTouches()
{
	if (!m_bHasQueuedState)
		m_Touches.RemoveAll();
}

BallTouchInfo *CBall::LastInfo(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/)
{
	for (int i = m_Touches.Count() - 1; i >= 0; i--)
	{
		if (pSkipPl && m_Touches[i].m_pPl == pSkipPl)
			continue;

		if (!wasShooting || m_Touches[i].m_bIsShot)
			return &m_Touches[i];
	}

	return NULL;
}

CSDKPlayer *CBall::LastPl(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/)
{
	BallTouchInfo *info = LastInfo(wasShooting, pSkipPl);
	if (info && CSDKPlayer::IsOnField(info->m_pPl))
		return info->m_pPl;
	
	return NULL;
}

int CBall::LastTeam(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/)
{
	BallTouchInfo *info = LastInfo(wasShooting, pSkipPl);
	return info ? info->m_nTeam : TEAM_INVALID;
}

int CBall::LastOppTeam(bool wasShooting, CSDKPlayer *pSkipPl /*= NULL*/)
{
	BallTouchInfo *info = LastInfo(wasShooting, pSkipPl);
	return info ? (info->m_nTeam == TEAM_A ? TEAM_B : TEAM_A) : TEAM_INVALID;
}

void CBall::UpdatePossession(CSDKPlayer *pNewPossessor)
{
	if (m_pPossessingPl == pNewPossessor)
		return;

	if (m_flPossessionStart != -1)
	{
		float duration = gpGlobals->curtime - m_flPossessionStart;
		CTeam *pPossessingTeam = GetGlobalTeam(m_nPossessingTeam);
		CTeam *pOtherTeam = GetGlobalTeam(m_nPossessingTeam == TEAM_A ? TEAM_B : TEAM_A);
		pPossessingTeam->m_flPossessionTime += duration;
		float total = max(1, pPossessingTeam->m_flPossessionTime + pOtherTeam->m_flPossessionTime);
		pPossessingTeam->m_nPossession = 100 * pPossessingTeam->m_flPossessionTime / total;		
		pOtherTeam->m_nPossession = 100 - pPossessingTeam->m_nPossession;

		if (CSDKPlayer::IsOnField(m_pPossessingPl))
		{
			m_pPossessingPl->m_flPossessionTime += duration;
			m_pPossessingPl->m_Possession = 100 * m_pPossessingPl->m_flPossessionTime / total;
		}
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

void CBall::SetOffsideLinePositions(float ballPosY, float offsidePlayerPosY, float lastOppPlayerPosY)
{
	m_flOffsideLineBallPosY = ballPosY;
	m_flOffsideLineOffsidePlayerPosY = offsidePlayerPosY;
	m_flOffsideLineLastOppPlayerPosY = lastOppPlayerPosY;
	m_bOffsideLinesEnabled = true;
}

void CBall::SetOffsideLinesEnabled(bool enable)
{
	m_bOffsideLinesEnabled = enable;
}

int CBall::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CBall::ResetMatch()
{
	CreateVPhysics();
	m_pPl = NULL;
	m_pOtherPl = NULL;
	RemoveAllTouches();
	m_ePenaltyState = PENALTY_NONE;
	SetOffsideLinesEnabled(false);
	SDKGameRules()->DisableShield();
	UnmarkOffsidePlayers();
	m_bSetNewPos = false;
	m_bHasQueuedState = false;
	RemoveEffects(EF_NODRAW);
	EnablePlayerCollisions(true);
	m_pPhys->EnableMotion(true);
	m_pPhys->Wake();
	m_pHoldingPlayer = NULL;

	GetGlobalTeam(TEAM_A)->ResetStats();
	GetGlobalTeam(TEAM_B)->ResetStats();

	CPlayerPersistentData::RemoveAllPlayerData();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPl)
			continue;

		pPl->ResetFlags();
		pPl->ResetStats();
	}
}

void CBall::SetPenaltyTaker(CSDKPlayer *pPl)
{
	m_pFouledPl = pPl;
	m_nFoulingTeam = pPl->GetOppTeamNumber();
}

void CBall::SetCreator(CSDKPlayer *pCreator)
{
	m_pCreator = pCreator;
}

void CBall::EnablePlayerCollisions(bool enable)
{
	SetCollisionGroup(enable ? COLLISION_GROUP_SOLID_BALL : COLLISION_GROUP_NONSOLID_BALL);
}

void CBall::RemoveFromPlayerHands(CSDKPlayer *pPl)
{
	m_pHoldingPlayer = NULL;
	pPl->m_pHoldingBall = NULL;
	pPl->m_nBody = MODEL_KEEPER;
	pPl->DoServerAnimationEvent(PLAYERANIMEVENT_CARRY_END);
	RemoveEffects(EF_NODRAW);
	EnablePlayerCollisions(true);
	m_pPhys->Wake();
}