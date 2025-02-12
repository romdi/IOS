//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "team.h"
#include "player.h"
#include "team_spawnpoint.h"
#include "sdk_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CUtlVector< CTeam * > g_Teams;

//-----------------------------------------------------------------------------
// Purpose: SendProxy that converts the Team's player UtlVector to entindexes
//-----------------------------------------------------------------------------
void SendProxy_PlayerList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CTeam *pTeam = (CTeam*)pData;

	// If this assertion fails, then SendProxyArrayLength_PlayerArray must have failed.
	Assert( iElement < pTeam->m_aPlayers.Size() );

	CBasePlayer *pPlayer = pTeam->m_aPlayers[iElement];
	pOut->m_Int = pPlayer->entindex();
}


int SendProxyArrayLength_PlayerArray( const void *pStruct, int objectID )
{
	CTeam *pTeam = (CTeam*)pStruct;
	return pTeam->m_aPlayers.Count();
}

void SendProxy_String_tToStringT( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	string_t *pString = (string_t*)pData;
	pOut->m_pString = (char*)STRING( *pString );
}

// Datatable
IMPLEMENT_SERVERCLASS_ST_NOBASE(CTeam, DT_Team)
	SendPropInt( SENDINFO(m_iTeamNum), 3 ),
	SendPropInt( SENDINFO(m_nPenaltyGoals), 5, SPROP_UNSIGNED),
	SendPropInt( SENDINFO(m_nPenaltyGoalBits), 32, SPROP_UNSIGNED),
	SendPropInt( SENDINFO(m_nPenaltyRound), 5, SPROP_UNSIGNED),
	SendPropInt( SENDINFO(m_nTimeoutsLeft), 4, SPROP_UNSIGNED),
	SendPropInt( SENDINFO(m_nTimeoutTimeLeft), 10, SPROP_UNSIGNED),
	SendPropString( SENDINFO( m_szServerKitName ) ),
	SendPropString( SENDINFO( m_szServerCode ) ),
	SendPropString( SENDINFO( m_szServerShortName ) ),

	SendPropVector(SENDINFO(m_vCornerLeft), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vCornerRight), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vGoalkickLeft), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vGoalkickRight), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vPenalty), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vGoalCenter), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vPenBoxMin), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vPenBoxMax), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vSixYardBoxMin), -1, SPROP_COORD),
	SendPropVector(SENDINFO(m_vSixYardBoxMax), -1, SPROP_COORD),
	SendPropInt(SENDINFO(m_nForward), 2),
	SendPropInt(SENDINFO(m_nRight), 2),
	SendPropIntWithMinusOneFlag(SENDINFO(m_nCaptainPosIndex), 4),

	SendPropArray2( 
		SendProxyArrayLength_PlayerArray,
		SendPropInt("player_array_element", 0, 4, 10, SPROP_UNSIGNED, SendProxy_PlayerList), 
		MAX_PLAYERS, 
		0, 
		"player_array"
		),

	SendPropArray3( SENDINFO_ARRAY3(m_szMatchEventPlayers), SendPropString( SENDINFO_ARRAY(m_szMatchEventPlayers), 0, SendProxy_String_tToStringT ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_eMatchEventTypes), SendPropInt( SENDINFO_ARRAY(m_eMatchEventTypes), 5, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_eMatchEventMatchPeriods), SendPropInt( SENDINFO_ARRAY(m_eMatchEventMatchPeriods), 4, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_nMatchEventSeconds), SendPropInt( SENDINFO_ARRAY(m_nMatchEventSeconds), 13, SPROP_UNSIGNED ) ),

	SendPropArray3( SENDINFO_ARRAY3(m_PosNextJoinSeconds), SendPropInt( SENDINFO_ARRAY(m_PosNextJoinSeconds), 13, SPROP_UNSIGNED ) ),

	SendPropInt(SENDINFO(m_RedCards), 4, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_YellowCards), 4, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Fouls), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_FoulsSuffered), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_SlidingTackles), 8, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_SlidingTacklesCompleted), 8, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_GoalsConceded), 5, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Shots), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_ShotsOnGoal), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_PassesCompleted), 9, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Interceptions), 8, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Offsides), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Goals), 5, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_OwnGoals), 5, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Assists), 5, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Possession), 7, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_DistanceCovered), 11, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Passes), 9, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_FreeKicks), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Penalties), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Corners), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_ThrowIns), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_KeeperSaves), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_KeeperSavesCaught), 6, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_GoalKicks), 5, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Ping), 10, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_Rating), 7, SPROP_UNSIGNED),

	SendPropInt(SENDINFO(m_nFormationIndex), 4, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_nOffensiveLevel), 3, 0),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( team_manager, CTeam );

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified team manager
//-----------------------------------------------------------------------------
CTeam *GetGlobalTeam( int iIndex )
{
	if ( iIndex < 0 || iIndex >= GetNumberOfTeams() )
		return NULL;

	return g_Teams[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: Get the number of team managers
//-----------------------------------------------------------------------------
int GetNumberOfTeams( void )
{
	return g_Teams.Size();
}


//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
CTeam::CTeam( void )
{
	memset( m_szServerKitName.GetForModify(), 0, sizeof(m_szServerKitName) );
	memset( m_szServerCode.GetForModify(), 0, sizeof(m_szServerCode) );
	memset( m_szServerShortName.GetForModify(), 0, sizeof(m_szServerShortName) );
	ResetStats();
	UpdatePosIndices(true);
	m_nTimeoutsLeft = mp_timeout_count.GetInt();
	m_nTimeoutTimeLeft = mp_timeout_duration.GetInt() * 60;
	m_nFormationIndex = 0;
	m_nOffensiveLevel = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTeam::~CTeam( void )
{
	m_aPlayers.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame
//-----------------------------------------------------------------------------
void CTeam::Think( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Teams are always transmitted to clients
//-----------------------------------------------------------------------------
int CTeam::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Visibility/scanners
//-----------------------------------------------------------------------------
bool CTeam::ShouldTransmitToPlayer( CBasePlayer* pRecipient, CBaseEntity* pEntity )
{
	return true;
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
void CTeam::Init( const char *pName, int iNumber )
{
	SetKitName(pName);
	m_iTeamNum = iNumber;
}

int CTeam::GetTeamNumber( void ) const
{
	return m_iTeamNum;
}

int CTeam::GetOppTeamNumber( void ) const
{
	if (m_iTeamNum != TEAM_HOME && m_iTeamNum != TEAM_AWAY)
		return m_iTeamNum;

	return m_iTeamNum == TEAM_HOME ? TEAM_AWAY : TEAM_HOME;
}

CTeam *CTeam::GetOppTeam( void ) const
{
	if (m_iTeamNum != TEAM_HOME && m_iTeamNum != TEAM_AWAY)
		return GetGlobalTeam(m_iTeamNum);

	return m_iTeamNum == TEAM_HOME ? GetGlobalTeam(TEAM_AWAY) : GetGlobalTeam(TEAM_HOME);
}

const char *CTeam::GetTeamCode( void )
{
	return m_szServerCode;
}

const char *CTeam::GetShortTeamName( void )
{
	return m_szServerShortName;
}

void CTeam::SetTeamNumber(int teamNum)
{
	m_iTeamNum = teamNum;
}

void CTeam::SetKitName(const char *pName)
{
	Q_strncpy( m_szServerKitName.GetForModify(), pName, MAX_KITNAME_LENGTH );

	for (int i = 0; i < m_aPlayers.Count(); i++)
		ToSDKPlayer(m_aPlayers[i])->UpdateBodyGroups();
}

void CTeam::SetTeamCode(const char *pCode)
{
	Q_strncpy( m_szServerCode.GetForModify(), pCode, MAX_TEAMCODE_LENGTH );
}

void CTeam::SetShortTeamName(const char *pName)
{
	Q_strncpy( m_szServerShortName.GetForModify(), pName, MAX_SHORTTEAMNAME_LENGTH );
}


//-----------------------------------------------------------------------------
// Purpose: Get the team's name
//-----------------------------------------------------------------------------
const char *CTeam::GetKitName( void )
{
	return m_szServerKitName;
}

//-----------------------------------------------------------------------------
// Purpose: Update the player's client data
//-----------------------------------------------------------------------------
void CTeam::UpdateClientData( CBasePlayer *pPlayer )
{
}

//-----------------------------------------------------------------------------
// Purpose: Add the specified player to this team. Remove them from their current team, if any.
//-----------------------------------------------------------------------------
void CTeam::AddPlayer( CBasePlayer *pPlayer, int posIndex )
{
	m_aPlayers.AddToTail( pPlayer );
	m_PosIndexPlayerIndices[posIndex] = m_aPlayers.Count() - 1;
	NetworkStateChanged();
}

//-----------------------------------------------------------------------------
// Purpose: Remove this player from the team
//-----------------------------------------------------------------------------
void CTeam::RemovePlayer( CBasePlayer *pPlayer )
{
	m_LastPlayerCoordsByPosIndex[ToSDKPlayer(pPlayer)->GetTeamPosIndex()].coords = pPlayer->GetLocalOrigin();
	m_LastPlayerCoordsByPosIndex[ToSDKPlayer(pPlayer)->GetTeamPosIndex()].leaveTime = gpGlobals->curtime;

	if ((GetTeamNumber() == TEAM_HOME || GetTeamNumber() == TEAM_AWAY) && pPlayer == GetCaptain())
		SetCaptainPosIndex(-1);

	m_aPlayers.FindAndRemove( pPlayer );

	UpdatePosIndices(false);

	NetworkStateChanged();
}

void CTeam::UpdatePosIndices(bool reset)
{
	for (int i = 0; i < 11; i++)
		m_PosIndexPlayerIndices[i] = -1;

	for (int i = 0; i < m_aPlayers.Count(); i++)
	{
		m_PosIndexPlayerIndices[ToSDKPlayer(m_aPlayers[i])->GetTeamPosIndex()] = i;
	}

	if (reset)
	{
		SetCaptainPosIndex(-1);
		UnblockAllPos();
	}
}

Vector CTeam::GetLastPlayerCoordsByPosIndex(int posIndex)
{
	return gpGlobals->curtime > m_LastPlayerCoordsByPosIndex[posIndex].leaveTime + mp_joincoordduration.GetFloat() ? vec3_invalid : m_LastPlayerCoordsByPosIndex[posIndex].coords;
}

extern ConVar sv_singlekeeper;

bool CTeam::IsPosBlocked(int posIndex)
{
	return SDKGameRules()->GetMatchDisplayTimeSeconds(true, false) < GetPosNextJoinSeconds(posIndex)
		|| sv_singlekeeper.GetBool() && posIndex == GetPosIndexByPosType(POS_GK) && GetOppTeam()->GetPlayerByPosType(POS_GK);
}

//-----------------------------------------------------------------------------
// Purpose: Return the number of players in this team.
//-----------------------------------------------------------------------------
int CTeam::GetNumPlayers( void )
{
	return m_aPlayers.Size();
}

//-----------------------------------------------------------------------------
// Purpose: Get a specific player
//-----------------------------------------------------------------------------
CBasePlayer *CTeam::GetPlayer( int iIndex )
{
	Assert( iIndex >= 0 && iIndex < m_aPlayers.Size() );
	return m_aPlayers[ iIndex ];
}

CSDKPlayer *CTeam::GetPlayerByPosIndex(int posIndex)
{
	if (posIndex < 0 || posIndex > 10 || m_PosIndexPlayerIndices[posIndex] == -1)
		return NULL;
	else
		return ToSDKPlayer(m_aPlayers[m_PosIndexPlayerIndices[posIndex]]);
}

int CTeam::GetPosIndexByPosType(PosTypes_t posType)
{
	for (int i = 0; i < GetFormation()->positions.Count(); i++)
	{
		if (GetFormation()->positions[i]->type == posType)
			return i;
	}

	return -1;
}

CSDKPlayer *CTeam::GetPlayerByPosType(PosTypes_t posType)
{
	return GetPlayerByPosIndex(GetPosIndexByPosType(posType));
}

int CTeam::GetPosNextJoinSeconds(int posIndex)
{
	return m_PosNextJoinSeconds[posIndex];
}

void CTeam::SetPosNextJoinSeconds(int posIndex, int seconds)
{
	m_PosNextJoinSeconds.Set(posIndex, seconds);
}

void CTeam::UnblockAllPos()
{
	for (int i = 0; i < 11; i++)
		m_PosNextJoinSeconds.Set(i, 0);
}

//------------------------------------------------------------------------------------------------------------------
// SCORING
//-----------------------------------------------------------------------------
// Purpose: Add / Remove score for this team
//-----------------------------------------------------------------------------
//void CTeam::AddGoal()
//{
//	m_Goals += 1;
//}

void CTeam::SetGoals( int goals )
{
	m_Goals = goals;
}

//-----------------------------------------------------------------------------
// Purpose: Get this team's score
//-----------------------------------------------------------------------------
int CTeam::GetGoals( void )
{
	return m_Goals;
}

void CTeam::SetPossession( int possession )
{
	m_Possession = possession;
}

int CTeam::GetPossession( void )
{
	return m_Possession;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeam::AwardAchievement( int iAchievement )
{
	Assert( iAchievement >= 0 && iAchievement < 255 );	// must fit in short 

	CRecipientFilter filter;

	int iNumPlayers = GetNumPlayers();

	for ( int i=0;i<iNumPlayers;i++ )
	{
		if ( GetPlayer(i) )
		{
			filter.AddRecipient( GetPlayer(i) );
		}
	}

	UserMessageBegin( filter, "AchievementEvent" );
		WRITE_SHORT( iAchievement );
	MessageEnd();
}

Vector CTeam::GetSpotPos(const char *name)
{
	CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, name);
	if (pEnt)
		return Vector(pEnt->GetLocalOrigin().x, pEnt->GetLocalOrigin().y, SDKGameRules()->m_vKickOff.GetZ());
	else
	{
		Error(UTIL_VarArgs("'%s' missing", name));
		return vec3_invalid;
	}
}

void CTeam::InitFieldSpots(bool isBottomTeam)
{
	float zPos = SDKGameRules()->m_vKickOff.GetZ();
	const int setPieceOffset = 15;

	Vector penboxMin, penboxMax;
	CBaseEntity *pPenbox = gEntList.FindEntityByClassname(NULL, "trigger_penaltybox");
	if (!pPenbox)
		Error("'trigger_penaltybox' entity is missing from map");
	pPenbox->CollisionProp()->WorldSpaceAABB(&penboxMin, &penboxMax);
	penboxMin += Vector(1, 1, 1);
	penboxMax -= Vector(1, 1, 1);

	Vector sixyardMin, sixyardMax;
	CBaseEntity *pSixyard = gEntList.FindEntityByClassname(NULL, "trigger_sixyardbox");
	if (!pSixyard)
		Error("'trigger_sixyardbox' entity is missing from map");
	pSixyard->CollisionProp()->WorldSpaceAABB(&sixyardMin, &sixyardMax);
	sixyardMin += Vector(1, 1, 1);
	sixyardMax -= Vector(1, 1, 1);

	if (isBottomTeam)
	{
		m_vGoalCenter = Vector(SDKGameRules()->m_vKickOff.GetX(), SDKGameRules()->m_vFieldMin.GetY(), zPos);

		m_vCornerLeft = SDKGameRules()->m_vFieldMin + Vector(setPieceOffset, setPieceOffset, 0);
		m_vCornerRight = Vector(SDKGameRules()->m_vFieldMax.GetX(), SDKGameRules()->m_vFieldMin.GetY(), zPos) + Vector(-setPieceOffset, setPieceOffset, 0);

		m_vPenBoxMin = Vector(SDKGameRules()->m_vKickOff.GetX() - (penboxMax.x - penboxMin.x), SDKGameRules()->m_vFieldMin.GetY(), zPos);
		m_vPenBoxMax = Vector(SDKGameRules()->m_vKickOff.GetX() + (penboxMax.x - penboxMin.x), SDKGameRules()->m_vFieldMin.GetY() + (penboxMax.y - penboxMin.y), zPos);

		m_vPenalty = Vector(m_vGoalCenter.GetX(), m_vGoalCenter.GetY() + (penboxMax.y - penboxMin.y) / 3 * 2, zPos);

		m_vSixYardBoxMin = Vector(SDKGameRules()->m_vKickOff.GetX() - (sixyardMax.x - sixyardMin.x), SDKGameRules()->m_vFieldMin.GetY(), zPos);
		m_vSixYardBoxMax = Vector(SDKGameRules()->m_vKickOff.GetX() + (sixyardMax.x - sixyardMin.x), SDKGameRules()->m_vFieldMin.GetY() + (sixyardMax.y - sixyardMin.y), zPos);

		m_vGoalkickLeft = Vector(m_vSixYardBoxMin.GetX(), m_vSixYardBoxMax.GetY(), zPos) + Vector(setPieceOffset, 0, 0);
		m_vGoalkickRight = m_vSixYardBoxMax + Vector(-setPieceOffset, 0, 0);
	}
	else
	{
		m_vGoalCenter = Vector(SDKGameRules()->m_vKickOff.GetX(), SDKGameRules()->m_vFieldMax.GetY(), zPos);

		m_vCornerLeft = SDKGameRules()->m_vFieldMax + Vector(-setPieceOffset, -setPieceOffset, 0);
		m_vCornerRight = Vector(SDKGameRules()->m_vFieldMin.GetX(), SDKGameRules()->m_vFieldMax.GetY(), zPos) + Vector(setPieceOffset, -setPieceOffset, 0);

		m_vPenBoxMin = Vector(SDKGameRules()->m_vKickOff.GetX() - (penboxMax.x - penboxMin.x), SDKGameRules()->m_vFieldMax.GetY() - (penboxMax.y - penboxMin.y), zPos);
		m_vPenBoxMax = Vector(SDKGameRules()->m_vKickOff.GetX() + (penboxMax.x - penboxMin.x), SDKGameRules()->m_vFieldMax.GetY(), zPos);

		m_vPenalty = Vector(m_vGoalCenter.GetX(), m_vGoalCenter.GetY() - (penboxMax.y - penboxMin.y) / 3 * 2, zPos);

		m_vSixYardBoxMin = Vector(SDKGameRules()->m_vKickOff.GetX() - (sixyardMax.x - sixyardMin.x), SDKGameRules()->m_vFieldMax.GetY() - (sixyardMax.y - sixyardMin.y), zPos);
		m_vSixYardBoxMax = Vector(SDKGameRules()->m_vKickOff.GetX() + (sixyardMax.x - sixyardMin.x), SDKGameRules()->m_vFieldMax.GetY(), zPos);

		m_vGoalkickLeft = Vector(m_vSixYardBoxMax.GetX(), m_vSixYardBoxMin.GetY(), zPos) + Vector(-setPieceOffset, 0, 0);
		m_vGoalkickRight = m_vSixYardBoxMin + Vector(setPieceOffset, 0, 0);
	}

	m_nForward = Sign(SDKGameRules()->m_vKickOff.GetY() - m_vGoalCenter.GetY());
	m_nRight = Sign(m_vCornerRight.GetX() - m_vGoalCenter.GetX());
}

void CTeam::ResetStats()
{
	m_RedCards = 0;
	m_YellowCards = 0;
	m_Fouls = 0;
	m_FoulsSuffered = 0;
	m_SlidingTackles = 0;
	m_SlidingTacklesCompleted = 0;
	m_GoalsConceded = 0;
	m_Shots = 0;
	m_ShotsOnGoal = 0;
	m_PassesCompleted = 0;
	m_Interceptions = 0;
	m_Offsides = 0;
	m_Goals = 0;
	m_OwnGoals = 0;
	m_Assists = 0;
	m_Possession = 0;
	m_DistanceCovered = 0;
	m_Passes = 0;
	m_FreeKicks = 0;
	m_Penalties = 0;
	m_Corners = 0;
	m_ThrowIns = 0;
	m_KeeperSaves = 0;
	m_KeeperSavesCaught = 0;
	m_GoalKicks = 0;
	m_flPossessionTime = 0;
	m_flExactDistanceCovered = 0;
	m_nMatchEventIndex = 0;
	m_Rating = 0;
	m_Ping = 0;
	m_nTimeoutsLeft = mp_timeout_count.GetInt();
	m_nTimeoutTimeLeft = mp_timeout_duration.GetInt() * 60;
	m_nOffensiveLevel = 0;

	for (int i = 0; i < MAX_MATCH_EVENTS; i++)
	{
		memset(m_szMatchEventPlayersMemory, 0, sizeof(m_szMatchEventPlayersMemory));
		m_szMatchEventPlayers.Set(i, MAKE_STRING(""));
		m_eMatchEventTypes.Set(i, 0);
		m_eMatchEventMatchPeriods.Set(i, 0);
		m_nMatchEventSeconds.Set(i, 0);
	}

	UnblockAllPos();

	for (int i = 0; i < 11; i++)
	{
		m_LastPlayerCoordsByPosIndex[i].coords = vec3_invalid;
		m_LastPlayerCoordsByPosIndex[i].leaveTime = -1;
	}

	m_PeriodData.PurgeAndDeleteElements();
}

void CTeam::AddMatchEvent(match_period_t matchPeriod, int seconds, match_event_t event, const char *text)
{
	if (m_nMatchEventIndex == MAX_MATCH_EVENTS)
		return;

	m_nMatchEventSeconds.Set(m_nMatchEventIndex, seconds);
	m_eMatchEventTypes.Set(m_nMatchEventIndex, event);
	m_eMatchEventMatchPeriods.Set(m_nMatchEventIndex, matchPeriod);
	Q_strncpy(m_szMatchEventPlayersMemory[m_nMatchEventIndex], text, MAX_MATCH_EVENT_PLAYER_NAME_LENGTH);
	m_szMatchEventPlayers.Set(m_nMatchEventIndex, MAKE_STRING(m_szMatchEventPlayersMemory[m_nMatchEventIndex]));
	m_nMatchEventIndex += 1;
}

Formation *CTeam::GetFormation()
{
	return SDKGameRules()->GetFormations()[m_nFormationIndex];
}

int CTeam::GetOffensiveLevel()
{
	return m_nOffensiveLevel;
}

void CTeam::IncreaseOffensiveLevel()
{
	m_nOffensiveLevel = min(2, m_nOffensiveLevel + 1);
}

void CTeam::DecreaseOffensiveLevel()
{
	m_nOffensiveLevel = max(-2, m_nOffensiveLevel - 1);
}

void CTeam::SetFormationIndex(int index, bool silent)
{
	if (index == m_nFormationIndex || index < 0 || index > SDKGameRules()->GetFormations().Count() - 1)
		return;

	if (!silent)
	{
		IGameEvent *pEvent = gameeventmanager->CreateEvent("team_formation");
		if (pEvent)
		{
			pEvent->SetInt("team", GetTeamNumber());
			pEvent->SetInt("old_formation", m_nFormationIndex);
			pEvent->SetInt("new_formation", index);
			gameeventmanager->FireEvent(pEvent);
		}
	}

	m_nFormationIndex = index;
}

void CTeamPeriodData::ResetData()
{
	m_nRedCards = 0;
	m_nYellowCards = 0;
	m_nFouls = 0;
	m_nFoulsSuffered = 0;
	m_nSlidingTackles = 0;
	m_nSlidingTacklesCompleted = 0;
	m_nGoalsConceded = 0;
	m_nShots = 0;
	m_nShotsOnGoal = 0;
	m_nPassesCompleted = 0;
	m_nInterceptions = 0;
	m_nOffsides = 0;
	m_nGoals = 0;
	m_nOwnGoals = 0;
	m_nAssists = 0;
	m_nPasses = 0;
	m_nFreeKicks = 0;
	m_nPenalties = 0;
	m_nCorners = 0;
	m_nThrowIns = 0;
	m_nKeeperSaves = 0;
	m_nKeeperSavesCaught = 0;
	m_nGoalKicks = 0;
	m_nRating = 0;
	m_nPossession = 0;
	m_flPossessionTime = 0.0f;
	m_nDistanceCovered = 0;
	m_flExactDistanceCovered = 0.0f;

	m_nAnnouncedInjuryTimeSeconds = 0;
	m_nActualInjuryTimeSeconds = 0;
}