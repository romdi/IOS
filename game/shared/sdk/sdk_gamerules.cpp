//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "multiplay_gamerules.h"
#include "sdk_gamerules.h"
#include "ammodef.h"
#include "KeyValues.h"
#include "weapon_sdkbase.h"

extern void Bot_RunAll( void );

#ifdef CLIENT_DLL

	#include "precache_register.h"
	#include "c_sdk_player.h"
	#include "c_sdk_team.h"

#else
	
	#include "voice_gamemgr.h"
	#include "sdk_player.h"
	#include "sdk_team.h"
	#include "sdk_playerclass_info_parse.h"
	#include "player_resource.h"
	#include "mapentities.h"
	#include "sdk_basegrenade_projectile.h"
	#include "sdk_player.h"		//ios
	#include "game.h"			//ios
	#include "ios_mapentities.h"

	#include "movehelper_server.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CUniformRandomStream g_IOSRand;

static const s_KitData gKitDesc[] =
{
	{	"Brazil", "Brazil",		YELLOW },
	{	"England", "England",		WHITE },
	{	"Germany", "Germany",		WHITE },
	{	"Italy",	"Italy",	BLUE },
	{	"Scotland", "Scotland",		BLUE },
	{	"Barcelona", "FC Barcelona",	BLUE },
	{	"Bayern",	"FC Bayern Munich",	RED },
	{	"Liverpool", "Liverpool FC",	RED },
	{	"Milan",	"AC Milan", 	RED },
	{	"Palmeiras", "Sociedade Esportiva Palmeiras",	GREEN },
	{	"END", "END", END },
};

#ifndef CLIENT_DLL

class CSpawnPoint : public CPointEntity
{
public:
	bool IsDisabled() { return m_bDisabled; }
	void InputEnable( inputdata_t &inputdata ) { m_bDisabled = false; }
	void InputDisable( inputdata_t &inputdata ) { m_bDisabled = true; }

private:
	bool m_bDisabled;
	DECLARE_DATADESC();
};

BEGIN_DATADESC(CSpawnPoint)

	// Keyfields
	DEFINE_KEYFIELD( m_bDisabled,	FIELD_BOOLEAN,	"StartDisabled" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),

END_DATADESC();

	LINK_ENTITY_TO_CLASS( info_player_deathmatch, CSpawnPoint );
#if defined( SDK_USE_TEAMS )
	LINK_ENTITY_TO_CLASS( info_player_blue, CSpawnPoint );
	LINK_ENTITY_TO_CLASS( info_player_red, CSpawnPoint );
#endif

#endif


REGISTER_GAMERULES_CLASS( CSDKGameRules );

//#ifdef CLIENT_DLL
//void RecvProxy_MatchState( const CRecvProxyData *pData, void *pStruct, void *pOut )
//{
//	CSDKGameRules *pGamerules = ( CSDKGameRules *)pStruct;
//	match_state_t eMatchState = (match_state_t)pData->m_Value.m_Int;
//	pGamerules->SetMatchState( eMatchState );
//}
//#endif 

BEGIN_NETWORK_TABLE_NOBASE( CSDKGameRules, DT_SDKGameRules )
#if defined ( CLIENT_DLL )
	RecvPropFloat( RECVINFO( m_flStateEnterTime ) ),
	//RecvPropFloat( RECVINFO( m_fStart) ),
	//RecvPropInt( RECVINFO( m_iDuration) ),
	RecvPropInt( RECVINFO( m_eMatchState) ),// 0, RecvProxy_MatchState ),
	RecvPropInt( RECVINFO( m_nAnnouncedInjuryTime) ),// 0, RecvProxy_MatchState ),

	RecvPropInt(RECVINFO(m_nShieldFlags)),
	RecvPropInt(RECVINFO(m_nCircShieldRadius)),
	RecvPropVector(RECVINFO(m_vCircShieldPos)),
	RecvPropVector(RECVINFO(m_vRectShieldMin)),
	RecvPropVector(RECVINFO(m_vRectShieldMax)),
#else
	SendPropFloat( SENDINFO( m_flStateEnterTime ), 32, SPROP_NOSCALE ),
	//SendPropFloat( SENDINFO( m_fStart) ),
	//SendPropInt( SENDINFO( m_iDuration) ),
	SendPropInt( SENDINFO( m_eMatchState )),
	SendPropInt( SENDINFO( m_nAnnouncedInjuryTime )),

	SendPropInt(SENDINFO(m_nShieldFlags)),
	SendPropInt(SENDINFO(m_nCircShieldRadius)),
	SendPropVector(SENDINFO(m_vCircShieldPos), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vRectShieldMin), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vRectShieldMax), -1, SPROP_COORD),
#endif
END_NETWORK_TABLE()

#if defined ( SDK_USE_PLAYERCLASSES )
	ConVar mp_allowrandomclass( "mp_allowrandomclass", "1", FCVAR_REPLICATED, "Allow players to select random class" );
#endif


LINK_ENTITY_TO_CLASS( sdk_gamerules, CSDKGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( SDKGameRulesProxy, DT_SDKGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_SDKGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CSDKGameRules *pRules = SDKGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CSDKGameRulesProxy, DT_SDKGameRulesProxy )
		RecvPropDataTable( "sdk_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_SDKGameRules ), RecvProxy_SDKGameRules )
	END_RECV_TABLE()
#else
	void *SendProxy_SDKGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CSDKGameRules *pRules = SDKGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CSDKGameRulesProxy, DT_SDKGameRulesProxy )
		SendPropDataTable( "sdk_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_SDKGameRules ), SendProxy_SDKGameRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL
	ConVar sk_plr_dmg_grenade( "sk_plr_dmg_grenade","0");		
#endif

ConVar mp_limitteams( 
	"mp_limitteams", 
	"2", 
	FCVAR_REPLICATED | FCVAR_NOTIFY,
	"Max # of players 1 team can have over another (0 disables check)",
	true, 0,	// min value
	true, 30	// max value
);

static CSDKViewVectors g_SDKViewVectors(

	Vector( 0, 0, 58 ),			//VEC_VIEW
	//Vector( 0, 0, 100 ),			//VEC_VIEW
								
	//Vector(-16, -16, 0 ),		//VEC_HULL_MIN
	//Vector( 16,  16,  72 ),		//VEC_HULL_MAX
	
	Vector(-10, -10, 0 ),		//VEC_HULL_MIN
	Vector( 10,  10,  72 ),		//VEC_HULL_MAX
													
	Vector(-16, -16, 0 ),		//VEC_DUCK_HULL_MIN
	Vector( 16,  16, 45 ),		//VEC_DUCK_HULL_MAX
	Vector( 0, 0, 34 ),			//VEC_DUCK_VIEW
													
	Vector(-10, -10, -10 ),		//VEC_OBS_HULL_MIN
	Vector( 10,  10,  10 ),		//VEC_OBS_HULL_MAX
													
	Vector( 0, 0, 14 )			//VEC_DEAD_VIEWHEIGHT
#if defined ( SDK_USE_PRONE )			
	,Vector(-16, -16, 0 ),		//VEC_PRONE_HULL_MIN
	Vector( 16,  16, 24 ),		//VEC_PRONE_HULL_MAX
	Vector( 0,	0, 10 )			//VEC_PRONE_VIEW
#endif
);

const CViewVectors* CSDKGameRules::GetViewVectors() const
{
	return (CViewVectors*)GetSDKViewVectors();
}

const CSDKViewVectors *CSDKGameRules::GetSDKViewVectors() const
{
	return &g_SDKViewVectors;
}

#ifdef CLIENT_DLL


#else

// --------------------------------------------------------------------------------------------------- //
// Voice helper
// --------------------------------------------------------------------------------------------------- //

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
	{
		// Dead players can only be heard by other dead team mates
		if ( pTalker->IsAlive() == false )
		{
			if ( pListener->IsAlive() == false )
				return ( pListener->InSameTeam( pTalker ) );

			return false;
		}

		return ( pListener->InSameTeam( pTalker ) );
	}
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;



// --------------------------------------------------------------------------------------------------- //
// Globals.
// --------------------------------------------------------------------------------------------------- //
static const char *s_PreserveEnts[] =
{
	"player",
	"viewmodel",
	"worldspawn",
	"soundent",
	"ai_network",
	"ai_hint",
	"sdk_gamerules",
	"sdk_team_manager",
	"sdk_team_unassigned",
	"sdk_team_blue",
	"sdk_team_red",
	"sdk_player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sprite",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_illusionary",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_red",
	"info_player_blue",
	"point_viewcontrol",
	"shadow_control",
	"sky_camera",
	"scene_manager",
	"trigger_soundscape",
	"point_commentary_node",
	"func_precipitation",
	"func_team_wall",
	"", // END Marker
};

// --------------------------------------------------------------------------------------------------- //
// Global helper functions.
// --------------------------------------------------------------------------------------------------- //

// World.cpp calls this but we don't use it in SDK.
void InitBodyQue()
{
}


// --------------------------------------------------------------------------------------------------- //
// CSDKGameRules implementation.
// --------------------------------------------------------------------------------------------------- //

CSDKGameRules::CSDKGameRules()
{
	g_IOSRand.SetSeed(gpGlobals->curtime);

	m_pCurStateInfo = NULL;

	m_nShieldFlags = 0;
	m_nCircShieldRadius = 0;
	m_vCircShieldPos = vec3_origin;
	m_vRectShieldMin = vec3_origin;
	m_vRectShieldMax = vec3_origin;
	
	//ios m_bLevelInitialized = false;

	//m_flMatchStartTime = 0;

}
void CSDKGameRules::ServerActivate()
{
	//Tony; initialize the level
	//ios CheckLevelInitialized();

	//Tony; do any post stuff
	//m_flMatchStartTime = gpGlobals->curtime;
	/*if ( !IsFinite( m_flMatchStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flMatchStartTime.GetForModify() = 0.0f;
	}*/

	InitMapSpots();

	State_Transition(MATCH_INIT);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSDKGameRules::~CSDKGameRules()
{
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: TF2 Specific Client Commands
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CSDKGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
	CSDKPlayer *pPlayer = ToSDKPlayer( pEdict );
#if 0
	const char *pcmd = args[0];
	if ( FStrEq( pcmd, "somecommand" ) )
	{
		if ( args.ArgC() < 2 )
			return true;

		// Do something here!

		return true;
	}
	else 
#endif
	// Handle some player commands here as they relate more directly to gamerules state
	if ( pPlayer->ClientCommand( args ) )
	{
		return true;
	}
	else if ( BaseClass::ClientCommand( pEdict, args ) )
	{
		return true;
	}
	return false;
}

void CSDKGameRules::Think()
{
	State_Think();
	//BaseClass::Think();		//this breaks stuff, like voice comms!

	GetVoiceGameMgr()->Update( gpGlobals->frametime );

	Bot_RunAll();	//ios

	//IOS hold on intermission
	if (m_flIntermissionEndTime > gpGlobals->curtime)
		return;

	if ( g_fGameOver )   // someone else quit the game already
	{
		ChangeLevel(); // intermission is over
		return;
	}

	//if (GetMapRemainingTime() < 0)
	//	GoToIntermission();
}

Vector DropToGround( 
					CBaseEntity *pMainEnt, 
					const Vector &vPos, 
					const Vector &vMins, 
					const Vector &vMaxs )
{
	trace_t trace;
	UTIL_TraceHull( vPos, vPos + Vector( 0, 0, -500 ), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace );
	return trace.endpos;
}

//the 'recountteams' in teamplay_gamerules looks like the hl1 version so usin CTeam stuff here
void CSDKGameRules::CountTeams(void)
{
	//clr current
	for (int j = 0; j < TEAMS_COUNT; j++)
		m_PlayersOnTeam[j] = 0;

	//count players on team
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
	{
		CSDKPlayer *plr = (CSDKPlayer*)UTIL_PlayerByIndex( i );

		if (!plr)
			continue;

		if (!plr->IsPlayer())
			continue;

		if ( !plr->IsConnected() )
			continue;

		//check for null,disconnected player
		if (strlen(plr->GetPlayerName()) == 0)
			continue;

		m_PlayersOnTeam[plr->GetTeamNumber()]++;
	}
}

extern ConVar mp_chattime;
extern ConVar tv_delaymapchange;
#include "hltvdirector.h"
#include "viewport_panel_names.h"

void CSDKGameRules::GoToIntermission( void )
{
	if ( g_fGameOver )
		return;

	g_fGameOver = true;

	float flWaitTime = mp_chattime.GetInt();

	if ( tv_delaymapchange.GetBool() && HLTVDirector()->IsActive() )	
	{
		flWaitTime = max ( flWaitTime, HLTVDirector()->GetDelay() );
	}
			
	m_flIntermissionEndTime = gpGlobals->curtime + flWaitTime;
}


///////////////////////////////////////////////////
// auto balance teams if mismatched
//
void CSDKGameRules::AutobalanceTeams(void)
{
	CSDKPlayer *pSwapPlayer=NULL;
	int teamNow = 0, teamToBe;
	float mostRecent = 0.0f;

	bool bDoAutobalance = autobalance.GetBool();

	if (!bDoAutobalance)
		return;

	//check if teams need autobalancing
	CountTeams();
	if (m_PlayersOnTeam[TEAM_A] > m_PlayersOnTeam[TEAM_B]+1) 
	{
		teamNow  = TEAM_A;
		teamToBe = TEAM_B;
	} 
	else if (m_PlayersOnTeam[TEAM_B] > m_PlayersOnTeam[TEAM_A]+1) 
	{
		teamNow  = TEAM_B;
		teamToBe = TEAM_A;
	}
	else 
	{
		//no balancing required
		return;
	}

   //find 1 player to change per round?
   bool bFoundValidPlayer = false;
	//clear all the players data
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
	{
		CSDKPlayer *plr = (CSDKPlayer*)UTIL_PlayerByIndex( i );

		if (!plr)
			continue;

		if (!plr->IsPlayer())
			continue;

		if ( !plr->IsConnected() )
			continue;

		//check for null,disconnected player
		if (strlen(plr->GetPlayerName()) == 0)
			continue;

		//ignore bots
		if (plr->GetFlags() & FL_FAKECLIENT)
			continue;

		if (plr->GetTeamNumber() < TEAM_A)
			continue;

		//dont switch keepers
		if (plr->m_TeamPos == 1)
			continue;

		//found one
		if (plr->GetTeamNumber() == teamNow) 
		{
			//see if player is new to the game
			if (plr->m_JoinTime > mostRecent) 
			{
				pSwapPlayer = plr;
				mostRecent = plr->m_JoinTime;
				bFoundValidPlayer = true;
			}
		}
	}  

	if (!bFoundValidPlayer)
		return;

	//mimic vgui team selection
	//SetPlayerTeam(swapPlayer, teamToBe+1);
	pSwapPlayer->ChangeTeam(teamToBe);

	int trypos = 11;
	while (!pSwapPlayer->TeamPosFree(teamToBe, trypos) && trypos > 1) 
	{
		trypos--;
	}

	pSwapPlayer->m_TeamPos = trypos;
	//pSwapPlayer->ChooseModel();

	pSwapPlayer->Spawn();
	g_pGameRules->GetPlayerSpawnSpot( pSwapPlayer );
	pSwapPlayer->RemoveEffects( EF_NODRAW );
	pSwapPlayer->SetSolid( SOLID_BBOX );
	pSwapPlayer->RemoveFlag(FL_FROZEN);


	//print autobalance msg - markg
	//char text[256];
	//sprintf( text, "%s was team autobalanced\n",  STRING( swapPlayer->pev->netname ));
	//MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
	//	WRITE_BYTE( ENTINDEX(swapPlayer->edict()) );
	//	WRITE_STRING( text );
	//MESSAGE_END();
}

void CSDKGameRules::PlayerSpawn( CBasePlayer *p )
{	
	CSDKPlayer *pPlayer = ToSDKPlayer( p );

	int team = pPlayer->GetTeamNumber();

	if( team != TEAM_SPECTATOR )
	{
		//pPlayer->PrecacheModel( "models/player/barcelona/barcelona.mdl" );
		//pPlayer->SetModel( "models/player/barcelona/barcelona.mdl" );
		//pPlayer->SetHitboxSet( 0 );
	}
}

void CSDKGameRules::InitTeams( void )
{
	Assert( g_Teams.Count() == 0 );

	g_Teams.Purge();	// just in case

	ChooseTeamNames();

	// Create the team managers
	for ( int i = 0; i < ARRAYSIZE( pszTeamNames ); i++ )
	{
		CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName( "sdk_team_manager" ));

		pTeam->Init( pszTeamNames[i], i );

		g_Teams.AddToTail( pTeam );
	}

	CreateEntityByName( "sdk_gamerules" );
}

//ios
void CSDKGameRules::ChooseTeamNames()
{
	int numKits = 0;

	//count the available kits
	while (strcmp(gKitDesc[numKits].m_KitName, "END"))
		numKits++;

	numKits--;		//adjust the final values

	//now look for two random that dont have the same colour
	int teamtype1 = 0;
	int teamtype2 = 0;

	while (gKitDesc[teamtype1].m_KitColour == gKitDesc[teamtype2].m_KitColour)
	{
		teamtype1 = g_IOSRand.RandomInt(0,numKits);
		teamtype2 = g_IOSRand.RandomInt(0,numKits);
	}

	SetTeams(gKitDesc[teamtype1].m_KitName, gKitDesc[teamtype2].m_KitName, false);
}

/* create some proxy entities that we use for transmitting data */
void CSDKGameRules::CreateStandardEntities()
{
	// Create the player resource
	g_pPlayerResource = (CPlayerResource*)CBaseEntity::Create( "sdk_player_manager", vec3_origin, vec3_angle );

	// Create the entity that will send our data to the client.
#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
		CBaseEntity::Create( "sdk_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
}

#endif

bool CSDKGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap(collisionGroup0,collisionGroup1);
	}

	//Don't stand on COLLISION_GROUP_WEAPON
	if( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

//Tony; keep this in shared space.
#if defined ( SDK_USE_PLAYERCLASSES )
const char *CSDKGameRules::GetPlayerClassName( int cls, int team )
{
	CSDKTeam *pTeam = GetGlobalSDKTeam( team );

	if( cls == PLAYERCLASS_RANDOM )
	{
		return "#class_random";
	}

	if( cls < 0 || cls >= pTeam->GetNumPlayerClasses() )
	{
		Assert( false );
		return NULL;
	}

	const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( cls );

	return pClassInfo.m_szPrintName;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Init CS ammo definitions
//-----------------------------------------------------------------------------

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			1	

// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef* GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;

		for (int i=WEAPON_NONE+1;i<WEAPON_MAX;i++)
		{
			//Tony; ignore grenades, shotgun and the crowbar, grenades and shotgun are handled seperately because of their damage type not being DMG_BULLET.
			if (i == SDK_WEAPON_GRENADE || i == SDK_WEAPON_CROWBAR || i == SDK_WEAPON_SHOTGUN)
				continue;

			def.AddAmmoType( WeaponIDToAlias(i), DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 200/*max carry*/, 1, 0 );
		}

		// def.AddAmmoType( BULLET_PLAYER_50AE,		DMG_BULLET, TRACER_LINE, 0, 0, "ammo_50AE_max",		2400, 0, 10, 14 );
		def.AddAmmoType( "shotgun", DMG_BUCKSHOT, TRACER_NONE, 0, 0,	200/*max carry*/, 1, 0 );
		def.AddAmmoType( "grenades", DMG_BLAST, TRACER_NONE, 0, 0,	4/*max carry*/, 1, 0 );

		//Tony; added for the sdk_jeep
		def.AddAmmoType( "JeepAmmo",	DMG_SHOCK,					TRACER_NONE,			"sdk_jeep_weapon_damage",		"sdk_jeep_weapon_damage", "sdk_jeep_max_rounds", BULLET_IMPULSE(650, 8000), 0 );
	}

	return &def;
}


#ifndef CLIENT_DLL

const char *CSDKGameRules::GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer )
{
	//Tony; no prefix for now, it isn't needed.
	//ios return "";

	if (bTeamOnly)
		return "(TEAM)";
	else
		return "";
}

const char *CSDKGameRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
		return NULL;

	const char *pszFormat = NULL;

	if ( bTeamOnly )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			pszFormat = "SDK_Chat_Spec";
		else
		{
			if (pPlayer->m_lifeState != LIFE_ALIVE )
				pszFormat = "SDK_Chat_Team_Dead";
			else
				pszFormat = "SDK_Chat_Team";
		}
	}
	else
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
			pszFormat = "SDK_Chat_AllSpec";
		else
		{
			if (pPlayer->m_lifeState != LIFE_ALIVE )
				pszFormat = "SDK_Chat_All_Dead";
			else
				pszFormat = "SDK_Chat_All";
		}
	}

	return pszFormat;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between players (teamplay vs. deathmatch)
//-----------------------------------------------------------------------------
int CSDKGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
		return GR_TEAMMATE;

#endif

	return GR_NOTTEAMMATE;
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Player has just left the game
//-----------------------------------------------------------------------------
void CSDKGameRules::ClientDisconnected( edict_t *pClient )
{
	CSDKPlayer *pPlayer = (CSDKPlayer *)CBaseEntity::Instance( pClient );
	if ( pPlayer )
	{
		pPlayer->m_TeamPos = -1;

		//check all balls for interaction with this player
		CBall	*pBall = pPlayer->GetBall(NULL);
		while (pBall)
		{
			if (pBall->m_BallShieldPlayer == pPlayer)		//remove ball shield
			{
				pBall->ballStatusTime = 0;
				pBall->ShieldOff();
			}

			if (pBall->m_Foulee == pPlayer)
				pBall->m_Foulee = NULL;

			if (pBall->m_KeeperCarrying == pPlayer)
				pBall->DropBall();

			pBall = pPlayer->GetBall(pBall);
		}
	}
	BaseClass::ClientDisconnected( pClient );
}
#endif



#ifndef CLIENT_DLL

void CSDKGameRules::RestartMatch()
{
	State_Transition(MATCH_WARMUP);
}

void CC_SV_Restart(const CCommand &args)
{
	//ffs!
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
        return;

	if (args.ArgC() > 1)
		mp_timelimit_warmup.SetValue((float)atof(args[1]));

	SDKGameRules()->RestartMatch();
}


ConCommand sv_restart( "sv_restart", CC_SV_Restart, "Restart game", 0 );

#endif

ConVar mp_showstatetransitions( "mp_showstatetransitions", "1", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Show game state transitions." );

ConVar mp_timelimit_match( "mp_timelimit_match", "10", FCVAR_NOTIFY|FCVAR_REPLICATED, "match duration in minutes without breaks (90 is real time)" );
ConVar mp_timelimit_warmup( "mp_timelimit_warmup", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "time before match start" );
ConVar mp_timelimit_cooldown( "mp_timelimit_cooldown", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "time after match end" );
ConVar mp_timelimit_halftime( "mp_timelimit_halftime", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "half time duration" );
ConVar mp_timelimit_extratime_halftime( "mp_timelimit_extratime_halftime", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "extra time halftime duration" );
ConVar mp_timelimit_extratime_intermission( "mp_timelimit_extratime_intermission", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "time before extra time start" );
ConVar mp_timelimit_penalties( "mp_timelimit_penalties", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "limit for penalties duration" );
ConVar mp_timelimit_penalties_intermission( "mp_timelimit_penalties_intermission", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "time before penalties start" );
ConVar mp_extratime( "mp_extratime", "1", FCVAR_NOTIFY|FCVAR_REPLICATED );
ConVar mp_penalties( "mp_penalties", "1", FCVAR_NOTIFY|FCVAR_REPLICATED );

#ifdef GAME_DLL

void CSDKGameRules::State_Transition( match_state_t newState )
{
	State_Leave();
	State_Enter( newState );
}

void CSDKGameRules::State_Enter( match_state_t newState )
{
	m_eMatchState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );
	m_flStateEnterTime = gpGlobals->curtime;
	m_flStateInjuryTime = 0.0f;
	m_nAnnouncedInjuryTime = 0;

	if ( mp_showstatetransitions.GetInt() > 0 )
	{
		if ( m_pCurStateInfo )
			Msg( "Gamerules: entering state '%s'\n", m_pCurStateInfo->m_pStateName );
		else
			Msg( "Gamerules: entering state #%d\n", newState );
	}

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
	{
		(this->*m_pCurStateInfo->pfnEnterState)();
	}
}

void CSDKGameRules::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}

void CSDKGameRules::State_Think()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnThink )
	{
		if (m_pCurStateInfo->m_MinDurationConVar == NULL)
			m_flStateTimeLeft = 0.0f;
		else
			m_flStateTimeLeft = (m_flStateEnterTime + m_pCurStateInfo->m_MinDurationConVar->GetFloat() * 60 / m_pCurStateInfo->m_flMinDurationDivisor) - gpGlobals->curtime;
		
		(this->*m_pCurStateInfo->pfnThink)();
	}
}

CSDKGameRulesStateInfo* CSDKGameRules::State_LookupInfo( match_state_t state )
{
	static CSDKGameRulesStateInfo playerStateInfos[] =
	{
		{ MATCH_INIT,						"MATCH_INIT",						&CSDKGameRules::State_Enter_INIT,					NULL, &CSDKGameRules::State_Think_INIT,						NULL, 1	},
		{ MATCH_WARMUP,						"MATCH_WARMUP",						&CSDKGameRules::State_Enter_WARMUP,					NULL, &CSDKGameRules::State_Think_WARMUP,					&mp_timelimit_warmup, 1	},
		{ MATCH_FIRST_HALF,					"MATCH_FIRST_HALF",					&CSDKGameRules::State_Enter_FIRST_HALF,				NULL, &CSDKGameRules::State_Think_FIRST_HALF,				&mp_timelimit_match, 2 },
		{ MATCH_HALFTIME,					"MATCH_HALFTIME",					&CSDKGameRules::State_Enter_HALFTIME,				NULL, &CSDKGameRules::State_Think_HALFTIME,					&mp_timelimit_halftime, 1 },
		{ MATCH_SECOND_HALF,				"MATCH_SECOND_HALF",				&CSDKGameRules::State_Enter_SECOND_HALF,			NULL, &CSDKGameRules::State_Think_SECOND_HALF,				&mp_timelimit_match, 2 },
		{ MATCH_EXTRATIME_INTERMISSION,		"MATCH_EXTRATIME_INTERMISSION",		&CSDKGameRules::State_Enter_EXTRATIME_INTERMISSION, NULL, &CSDKGameRules::State_Think_EXTRATIME_INTERMISSION,	&mp_timelimit_extratime_intermission, 1	},
		{ MATCH_EXTRATIME_FIRST_HALF,		"MATCH_EXTRATIME_FIRST_HALF",		&CSDKGameRules::State_Enter_EXTRATIME_FIRST_HALF,	NULL, &CSDKGameRules::State_Think_EXTRATIME_FIRST_HALF,		&mp_timelimit_match, 6 },
		{ MATCH_EXTRATIME_HALFTIME,			"MATCH_EXTRATIME_HALFTIME",			&CSDKGameRules::State_Enter_EXTRATIME_HALFTIME,		NULL, &CSDKGameRules::State_Think_EXTRATIME_HALFTIME,		&mp_timelimit_extratime_halftime, 1 },
		{ MATCH_EXTRATIME_SECOND_HALF,		"MATCH_EXTRATIME_SECOND_HALF",		&CSDKGameRules::State_Enter_EXTRATIME_SECOND_HALF,	NULL, &CSDKGameRules::State_Think_EXTRATIME_SECOND_HALF,	&mp_timelimit_match, 6 },
		{ MATCH_PENALTIES_INTERMISSION,		"MATCH_PENALTIES_INTERMISSION",		&CSDKGameRules::State_Enter_PENALTIES_INTERMISSION, NULL, &CSDKGameRules::State_Think_PENALTIES_INTERMISSION,	&mp_timelimit_penalties_intermission, 1 },
		{ MATCH_PENALTIES,					"MATCH_PENALTIES",					&CSDKGameRules::State_Enter_PENALTIES,				NULL, &CSDKGameRules::State_Think_PENALTIES,				&mp_timelimit_penalties, 1 },
		{ MATCH_COOLDOWN,					"MATCH_COOLDOWN",					&CSDKGameRules::State_Enter_COOLDOWN,				NULL, &CSDKGameRules::State_Think_COOLDOWN,					&mp_timelimit_cooldown, 1 },
		{ MATCH_END,						"MATCH_END",						&CSDKGameRules::State_Enter_END,					NULL, &CSDKGameRules::State_Think_END,						NULL, 1},
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_eMatchState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

void CSDKGameRules::State_Enter_INIT()
{
	InitTeams();
}

void CSDKGameRules::State_Think_INIT()
{
	State_Transition(MATCH_WARMUP);
}

void CSDKGameRules::State_Enter_WARMUP()
{
}

void CSDKGameRules::State_Think_WARMUP()
{
	if (m_flStateTimeLeft <= 0.0f)
		State_Transition(MATCH_FIRST_HALF);
}

void CSDKGameRules::State_Enter_FIRST_HALF()
{
	//clear all the players data
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
	{
		CSDKPlayer *plr = (CSDKPlayer*)UTIL_PlayerByIndex( i );

		if (!plr)
			continue;

		plr->ResetMatchStats();

		if (plr->m_PlayerAnim==PLAYER_THROWIN)
			plr->DoAnimationEvent( PLAYERANIMEVENT_KICK );		//weird throwin bug

		//get rid of any stationary anim states
		plr->RemoveFlag(FL_ATCONTROLS);
		plr->m_HoldAnimTime = 0.0f;
		plr->SetAnimation( PLAYER_IDLE );
	}

	//reset (kick off) the first ball we find
	CBall *pBall = dynamic_cast<CBall*>(gEntList.FindEntityByClassname( NULL, "football" ));
	if (pBall)
	{
		pBall->DropBall();
		pBall->ballStatusTime = 0;
		pBall->ShieldOff();
		pBall->CreateVPhysics();
		//pBall->HandleKickOff();
		pBall->State_Transition(BALL_KICKOFF);
	}
}

void CSDKGameRules::State_Think_FIRST_HALF()
{
	if (m_flStateTimeLeft <= 10 && m_nAnnouncedInjuryTime == 0)
	{
		m_nAnnouncedInjuryTime = max(1, (int)m_flStateInjuryTime);
	}
	else if (m_flStateTimeLeft + m_flStateInjuryTime <= 0)
	{	
		//if (m_pBall->IsNearGoal())
		//	m_flStateInjuryTime += 5; // let players finish their attack
		//else
		State_Transition(MATCH_HALFTIME);
	}
}

void CSDKGameRules::State_Enter_HALFTIME()
{
}

void CSDKGameRules::State_Think_HALFTIME()
{
	if (m_flStateTimeLeft <= 0)
		State_Transition(MATCH_SECOND_HALF);
}

void CSDKGameRules::State_Enter_SECOND_HALF()
{
	SwapTeams();
}

void CSDKGameRules::State_Think_SECOND_HALF()
{
	if (m_flStateTimeLeft <= 10 && m_nAnnouncedInjuryTime == 0)
	{
		m_nAnnouncedInjuryTime = max(1, (int)m_flStateInjuryTime);
	}
	else if (m_flStateTimeLeft + m_flStateInjuryTime <= 0)
	{
		if (mp_extratime.GetBool())
			State_Transition(MATCH_EXTRATIME_INTERMISSION);
		else if (mp_penalties.GetBool())
			State_Transition(MATCH_PENALTIES_INTERMISSION);
		else
			State_Transition(MATCH_COOLDOWN);
	}
}

void CSDKGameRules::State_Enter_EXTRATIME_INTERMISSION()
{

}

void CSDKGameRules::State_Think_EXTRATIME_INTERMISSION()
{
	if (m_flStateTimeLeft <= 0)
	{
		State_Transition(MATCH_EXTRATIME_FIRST_HALF);
	}
}

void CSDKGameRules::State_Enter_EXTRATIME_FIRST_HALF()
{
	SwapTeams();
}

void CSDKGameRules::State_Think_EXTRATIME_FIRST_HALF()
{
	if (m_flStateTimeLeft <= 10 && m_nAnnouncedInjuryTime == 0)
	{
		m_nAnnouncedInjuryTime = max(1, (int)m_flStateInjuryTime);
	}
	else if (m_flStateTimeLeft + m_flStateInjuryTime <= 0)
	{
		State_Transition(MATCH_EXTRATIME_HALFTIME);
	}
}

void CSDKGameRules::State_Enter_EXTRATIME_HALFTIME()
{
}

void CSDKGameRules::State_Think_EXTRATIME_HALFTIME()
{
	if (m_flStateTimeLeft <= 0)
	{
		State_Transition(MATCH_EXTRATIME_SECOND_HALF);
	}
}

void CSDKGameRules::State_Enter_EXTRATIME_SECOND_HALF()
{
	SwapTeams();
}

void CSDKGameRules::State_Think_EXTRATIME_SECOND_HALF()
{
	if (m_flStateTimeLeft <= 10 && m_nAnnouncedInjuryTime == 0)
	{
		m_nAnnouncedInjuryTime = max(1, (int)m_flStateInjuryTime);
	}
	else if (m_flStateTimeLeft + m_flStateInjuryTime <= 0)
	{
		if (mp_penalties.GetBool())
			State_Transition(MATCH_PENALTIES_INTERMISSION);
		else
			State_Transition(MATCH_COOLDOWN);
	}
}

void CSDKGameRules::State_Enter_PENALTIES_INTERMISSION()
{
}

void CSDKGameRules::State_Think_PENALTIES_INTERMISSION()
{
	if (m_flStateTimeLeft <= 0)
	{
		State_Transition(MATCH_PENALTIES);
	}
}

void CSDKGameRules::State_Enter_PENALTIES()
{
}

void CSDKGameRules::State_Think_PENALTIES()
{
	if (m_flStateTimeLeft <= 0)
	{
		State_Transition(MATCH_COOLDOWN);
	}
}

void CSDKGameRules::State_Enter_COOLDOWN()
{
	//who won?
	int winners = 0;
	int scoreA = GetGlobalTeam( TEAM_A )->GetScore();
	int scoreB = GetGlobalTeam( TEAM_B )->GetScore();
	if (scoreA > scoreB)
		winners = TEAM_A;
	if (scoreB > scoreA)
		winners = TEAM_B;

	//for ( int i = 0; i < MAX_PLAYERS; i++ )		//maxclients?
	for ( int i = 0; i < gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = (CSDKPlayer*)UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		//pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD );

		//is this player on the winning team
		if (pPlayer->GetTeamNumber() == winners)
		{
			pPlayer->AddFlag (FL_CELEB);
		}
		//else
		//	pPlayer->AddFlag (FL_ATCONTROLS);

		//freezes the players
		//pPlayer->AddFlag (FL_ATCONTROLS);
	}


	//find a ball
	CBall *pBall = dynamic_cast<CBall*>(gEntList.FindEntityByClassname( NULL, "football" ));
	if (pBall)
	{
		//this test doesnt show because the scoreboard is on front
		pBall->SendMatchEvent(MATCH_EVENT_FINAL_WHISTLE);
		pBall->EmitAmbientSound(pBall->entindex(), pBall->GetAbsOrigin(), "Ball.whistle");
		//cheer
		pBall->EmitAmbientSound(pBall->entindex(), pBall->GetAbsOrigin(), "Ball.cheer");
	}
}

void CSDKGameRules::State_Think_COOLDOWN()
{
	if (m_flStateTimeLeft <= 0)
	{
		State_Transition(MATCH_END);
	}
}

void CSDKGameRules::State_Enter_END()
{
	GoToIntermission();
}

void CSDKGameRules::State_Think_END()
{
}

void CSDKGameRules::SwapTeams()
{
	//reset (kick off) the first ball we find
	//CBall *pBall = dynamic_cast<CBall*>(gEntList.FindEntityByClassname( NULL, "football" ));
	//if (pBall)
	//{
	//	pBall->DropBall();
	//	pBall->ballStatusTime = 0;
	//	pBall->ShieldOff();
	//	pBall->HandleKickOff();
	//	pBall->CreateVPhysics();
	//}

	// swap players
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPlayer = (CSDKPlayer *)UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;
		int team = pPlayer->GetTeamNumber();
		if (team < TEAM_A)
			continue;

		pPlayer->ChangeTeam((team == TEAM_A ? TEAM_B : TEAM_A));
		GetPlayerSpawnSpot(pPlayer);
		//pPlayer->ChooseModel();
	}

	// swap teams
	SetTeams(pszTeamNames[TEAM_B], pszTeamNames[TEAM_A]);
}

#endif

//#ifdef CLIENT_DLL
//
//void CSDKGameRules::SetMatchState(match_state_t eMatchState)
//{
//	m_eMatchState = eMatchState;
//}
//
//#endif

#ifdef GAME_DLL

void SetTeams(const char *teamHome, const char *teamAway, bool bInitialize)
{
	// copy strings to avoid problems (e.g. when swapping teams)
	char teamHomeCpy[32], teamAwayCpy[32];
	Q_strncpy(teamHomeCpy, teamHome, sizeof(teamHomeCpy));
	Q_strncpy(teamAwayCpy, teamAway, sizeof(teamAwayCpy));

	Q_strncpy(pszTeamNames[TEAM_A], teamHomeCpy, sizeof(pszTeamNames[TEAM_A]));
	Q_strncpy(pszTeamNames[TEAM_B], teamAwayCpy, sizeof(pszTeamNames[TEAM_B]));

	if (bInitialize)
	{
		//update the team names
		for ( int i = 0; i < ARRAYSIZE( pszTeamNames ); i++ )
		{
			CTeam *pTeam = g_Teams[i];
			pTeam->Init( pszTeamNames[i], i );
		}
	}
}

//void cc_Teams( const CCommand& args )
//{
//	if ( !UTIL_IsCommandIssuedByServerAdmin() )
//		return;
//
//	if ( args.ArgC() < 3 )
//	{
//		Msg( "Format: mp_teams <home team> <away team>\n" );
//		return;
//	}
//
//	SetTeams(args[1], args[2]);
//}
//
//static ConCommand mp_teams( "mp_teams", cc_Teams, "Set teams" );

void OnTeamlistChange(IConVar *var, const char *pOldValue, float flOldValue)
{
	//if (gpGlobals->curtime > 10)
	if (SDKGameRules() != NULL)
	{
		char teamlist[256];
		Q_strncpy(teamlist, ((ConVar*)var)->GetString(), sizeof(teamlist));
		//CUtlVector<char*, CUtlMemory<char*> > teams;
		//Q_SplitString(teamlist, ";", teams);
		//teams
		char *home = strtok(teamlist, ";");
		char *away = strtok(NULL, ";");
		if (home == NULL || away == NULL)
			Msg( "Format: mp_teamlist \"<home team>;<away team>\"\n" );
		else
		{
			//ReadTeamInfo(home);
			//ReadTeamInfo(away);
			SetTeams(home, away);
		}
	}
}
static ConVar mp_teamlist("mp_teamlist", "ENGLAND;BRAZIL", FCVAR_REPLICATED|FCVAR_NOTIFY, "Set team names", &OnTeamlistChange);
static ConVar sv_teamrotation("mp_teamrotation", "brazil;germany;italy;scotland;barcelona;bayern;liverpool;milan;palmeiras", 0, "Set available teams");


void CSDKGameRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
	/* TODO: handle skin, model & team changes 

  	char text[1024];

	// skin/color/model changes
	int iTeam = Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "cl_team" ) );
	int iClass = Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "cl_class" ) );

	if ( defaultteam.GetBool() )
	{
		// int clientIndex = pPlayer->entindex();

		// engine->SetClientKeyValue( clientIndex, "model", pPlayer->TeamName() );
		// engine->SetClientKeyValue( clientIndex, "team", pPlayer->TeamName() );
		UTIL_SayText( "Not allowed to change teams in this game!\n", pPlayer );
		return;
	}

	if ( defaultteam.GetFloat() || !IsValidTeam( mdls ) )
	{
		// int clientIndex = pPlayer->entindex();

		// engine->SetClientKeyValue( clientIndex, "model", pPlayer->TeamName() );
		Q_snprintf( text,sizeof(text), "Can't change team to \'%s\'\n", mdls );
		UTIL_SayText( text, pPlayer );
		Q_snprintf( text,sizeof(text), "Server limits teams to \'%s\'\n", m_szTeamList );
		UTIL_SayText( text, pPlayer );
		return;
	}

	ChangePlayerTeam( pPlayer, mdls, true, true );
	// recound stuff
	RecountTeams(); */

	const char *pszClubName = engine->GetClientConVarValue( pPlayer->entindex(), "clubname" );

	((CSDKPlayer *)pPlayer)->SetClubName(pszClubName);

	const char *pszName = engine->GetClientConVarValue( pPlayer->entindex(), "name" );

	const char *pszOldName = pPlayer->GetPlayerName();

	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	// Note, not using FStrEq so that this is case sensitive
	if ( pszOldName[0] != 0 && Q_strcmp( pszOldName, pszName ) )
	{
		IGameEvent * event = gameeventmanager->CreateEvent( "player_changename" );
		if ( event )
		{
			event->SetInt( "userid", pPlayer->GetUserID() );
			event->SetString( "oldname", pszOldName );
			event->SetString( "newname", pszName );
			gameeventmanager->FireEvent( event );
		}
		
		pPlayer->SetPlayerName( pszName );
	}
}

void CSDKGameRules::EnableCircShield(int nTeamFlag, int nRadius, Vector vPos)
{
	m_nShieldFlags |= FL_SHIELD_CIRC | nTeamFlag;
	m_nCircShieldRadius = nRadius;
	m_vCircShieldPos = vPos;
}

void CSDKGameRules::EnableRectShield(int nTeamFlag, Vector vMin, Vector vMax)
{
	m_nShieldFlags |= FL_SHIELD_RECT | nTeamFlag;
	m_vRectShieldMin = vMin;
	m_vRectShieldMax = vMax;
}

void CSDKGameRules::DisableShields()
{
	m_nShieldFlags = 0;
}

#endif

#ifdef CLIENT_DLL

#include "Filesystem.h"
#include "utlbuffer.h"

struct kit
{
	char type[16];
	char firstColor[16];
	char secondColor[16];
};

struct teamInfo
{
	char teamCode[8];
	char shortName[16];
	char fullName[32];
	kit kits[8];
};

void ReadTeamInfo(const char *teamname)
{
	//char filename[64];
	//Q_snprintf(filename, sizeof(filename), "materials/models/player_new/%s/teaminfo", teamname);
	//V_SetExtension(filename, ".txt", sizeof(filename));
	//V_FixSlashes(filename);

	//CUtlBuffer buf;
	//if (filesystem->ReadFile(filename, "GAME", buf))
	//{
	//	char* gameInfo = new char[buf.Size() + 1];
	//	buf.GetString(gameInfo);
	//	gameInfo[buf.Size()] = 0; // null terminator

	//	DevMsg("Team info: %s\n", gameInfo);

	//	delete[] gameInfo;
	//}

	char path[64], filename[64];
	Q_snprintf(path, sizeof(path), "materials/models/player_new/%s", teamname);

	int length;
	CUtlVector<char*, CUtlMemory<char*> > lines, values;
	Q_snprintf(filename, sizeof(filename), "%s/teaminfo.txt", path);
	char *teaminfostr = (char *)UTIL_LoadFileForMe(filename, &length);
	if (teaminfostr && length > 0)
	{
		const char *separators[2] = { "\n", ";" };
		Q_SplitString2(teaminfostr, separators, 2, lines);
		//teamInfo t = { teaminfo[0], teaminfo[1], teaminfo[2], teaminfo[3], teaminfo[4], teaminfo[5] };
		teamInfo ti;
		Q_strncpy(ti.teamCode, lines[0], sizeof(ti.teamCode));
		Q_strncpy(ti.shortName, lines[1], sizeof(ti.shortName));
		Q_strncpy(ti.fullName, lines[2], sizeof(ti.fullName));

		for (int i = 3; i < lines.Count(); i += 3)
		{
			kit k;
			Q_strncpy(k.type, lines[i], sizeof(k.type));
			Q_strncpy(k.firstColor, lines[i + 1], sizeof(k.firstColor));
			Q_strncpy(k.secondColor, lines[i + 2], sizeof(k.secondColor));
			ti.kits[i/3-1] = k; //todo: neue variable vom stack?
		}
	}
}

#include "curl/curl.h"
#include "Filesystem.h"
#include "utlbuffer.h"
  
struct curl_t
{
	char filename[32];
	CUtlBuffer buf;
	FileHandle_t fh;
};

// Called when curl receives data from the server
static size_t rcvData(void *ptr, size_t size, size_t nmemb, curl_t* vars)
{
	//Msg((char*)ptr); // up to 989 characters each time
	//CUtlBuffer buf(0, 0, CUtlBuffer::TEXT_BUFFER);
	//vars->buf.Put(ptr, nmemb);
	filesystem->Write(ptr, nmemb, vars->fh);
	//filesystem->WriteFile(VarArgs("materials/models/player_new/foobar/%s", vars->filename), "MOD", buf);
	return size * nmemb;
}

 
unsigned DoCurl( void *params )
{
	curl_t* vars = (curl_t*) params; // always use a struct!
 
	// do some stuff
 
	/*vars->buf = CUtlBuffer(0, 0, CUtlBuffer::TEXT_BUFFER);
	vars->buf.SetBufferType(false, false);*/
	//vars->buf = CUtlBuffer();

	//filesystem->UnzipFile("test.zip", "MOD", "unziptest");

	vars->fh = filesystem->Open(VarArgs("materials/models/player_new/foobar/%s", vars->filename), "a+b", "MOD");

	if (vars->fh)
	{
		CURL *curl;
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, VarArgs("http://127.0.0.1:8000/%s", vars->filename));
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, rcvData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, vars);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		filesystem->Close(vars->fh);

		//filesystem->WriteFile(VarArgs("materials/models/player_new/foobar/%s", vars->filename), "MOD", vars->buf);

		//Msg("Cannot print to console from this threaded function\n");
	}
	// clean up the memory
	delete vars;

	return 0;
}

void Curl(const CCommand &args)
{
	if (args.ArgC() < 2)
	{
		Msg("Which file?\n");
		return;
	}

	curl_t* vars = new curl_t;
	Q_strncpy(vars->filename, args[1], sizeof(vars->filename));
	CreateSimpleThread( DoCurl, vars );
}

static ConCommand cl_curl("cl_curl", Curl);

#endif
