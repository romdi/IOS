//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Normal HUD mode
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "clientmode_shared.h"
#include "iinput.h"
#include "view_shared.h"
#include "iviewrender.h"
#include "hud_basechat.h"
#include <vgui/IVGUI.h>
#include <vgui/Cursor.h>
#include <vgui/IPanel.h>
#include "engine/ienginesound.h"
#include <keyvalues.h>
#include <vgui_controls/AnimationController.h>
#include "vgui_int.h"
#include "hud_macros.h"
#include "particlemgr.h"
#include "c_vguiscreen.h"
#include "c_team.h"
#include "c_rumble.h"
#include "fmtstr.h"
#include "achievementmgr.h"
#include "c_playerresource.h"
#include <vgui/ILocalize.h>
#include "sdk_gamerules.h"
#include "c_ios_replaymanager.h"
#include "c_match_ball.h"
#include "in_buttons.h"
#include "c_ios_mapentities.h"
#include "ios_camera.h"
#include "spectatorgui.h"
#if defined( _X360 )
#include "xbox/xbox_console.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CHudWeaponSelection;
class CHudChat;

static vgui::HContext s_hVGuiContext = DEFAULT_VGUI_CONTEXT;

ConVar cl_drawhud( "cl_drawhud", "1", FCVAR_CHEAT, "Enable the rendering of the hud" );
ConVar hud_takesshots( "hud_takesshots", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Auto-save a scoreboard screenshot at the end of a map." );

extern ConVar v_viewmodel_fov;

extern bool IsInCommentaryMode( void );

CON_COMMAND( hud_reloadscheme, "Reloads hud layout and animation scripts." )
{
	ClientModeShared *mode = ( ClientModeShared * )GetClientModeNormal();
	if ( !mode )
		return;

	mode->ReloadScheme();
}

#ifdef _DEBUG
CON_COMMAND_F( crash, "Crash the client. Optional parameter -- type of crash:\n 0: read from NULL\n 1: write to NULL\n 2: DmCrashDump() (xbox360 only)", FCVAR_CHEAT )
{
	int crashtype = 0;
	int dummy;
	if ( args.ArgC() > 1 )
	{
		 crashtype = Q_atoi( args[1] );
	}
	switch (crashtype)
	{
		case 0:
			dummy = *((int *) NULL);
			Msg("Crashed! %d\n", dummy); // keeps dummy from optimizing out
			break;
		case 1:
			*((int *)NULL) = 42;
			break;
#if defined( _X360 )
		case 2:
			XBX_CrashDump(false);
			break;
#endif
		default:
			Msg("Unknown variety of crash. You have now failed to crash. I hope you're happy.\n");
			break;
	}
}
#endif // _DEBUG

static void __MsgFunc_Rumble( bf_read &msg )
{
	unsigned char waveformIndex;
	unsigned char rumbleData;
	unsigned char rumbleFlags;

	waveformIndex = msg.ReadByte();
	rumbleData = msg.ReadByte();
	rumbleFlags = msg.ReadByte();

	RumbleEffect( waveformIndex, rumbleData, rumbleFlags );
}

static void __MsgFunc_VGUIMenu( bf_read &msg )
{
	char panelname[2048]; 
	
	msg.ReadString( panelname, sizeof(panelname) );

	bool  bShow = msg.ReadByte()!=0;
	
	IViewPortPanel *viewport = gViewPortInterface->FindPanelByName( panelname );

	if ( !viewport )
	{
		// DevMsg("VGUIMenu: couldn't find panel '%s'.\n", panelname );
		return;
	}

	int count = msg.ReadByte();

	if ( count > 0 )
	{
		KeyValues *keys = new KeyValues("data");

		for ( int i=0; i<count; i++)
		{
			char name[255];
			char data[255];

			msg.ReadString( name, sizeof(name) );
			msg.ReadString( data, sizeof(data) );

			keys->SetString( name, data );
		}

		viewport->SetData( keys );

		keys->deleteThis();
	}

	// is the server telling us to show the scoreboard (at the end of a map)?
	if ( Q_stricmp( panelname, "scores" ) == 0 )
	{
		if ( hud_takesshots.GetBool() == true )
		{
			gHUD.SetScreenShotTime( gpGlobals->curtime + 1.0 ); // take a screenshot in 1 second
		}
	}

	gViewPortInterface->ShowPanel( viewport, bShow );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeShared::ClientModeShared()
{
	m_pViewport = NULL;
	m_pChatElement = NULL;
	m_nRootSize[ 0 ] = m_nRootSize[ 1 ] = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeShared::~ClientModeShared()
{
	delete m_pViewport; 
}

void ClientModeShared::ReloadScheme( void )
{
	m_pViewport->ReloadScheme( "resource/ClientScheme.res" );
	ClearKeyValuesCache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Init()
{
	m_pChatElement = ( CBaseHudChat * )GET_HUDELEMENT( CHudChat );
	Assert( m_pChatElement );

	// Derived ClientMode class must make sure m_Viewport is instantiated
	Assert( m_pViewport );
	m_pViewport->LoadControlSettings( "scripts/HudLayout.res" );

	ListenForGameEvent( "player_connect" );
	ListenForGameEvent( "player_disconnect" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "player_specteam" );
	ListenForGameEvent( "server_cvar" );
	ListenForGameEvent( "player_changename" );
	ListenForGameEvent( "player_changeclub" );
	ListenForGameEvent( "player_changenationalteam" );
	ListenForGameEvent( "teamplay_broadcast_audio" );
	ListenForGameEvent( "achievement_earned" );
	ListenForGameEvent( "team_formation" );

	Camera()->Init();

	m_CursorNone = vgui::dc_none;

	HOOK_MESSAGE( VGUIMenu );
	HOOK_MESSAGE( Rumble );
}


void ClientModeShared::InitViewport()
{
}


void ClientModeShared::VGui_Shutdown()
{
	delete m_pViewport;
	m_pViewport = NULL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Shutdown()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : frametime - 
//			*cmd - 
//-----------------------------------------------------------------------------
bool ClientModeShared::CreateMove( float flInputSampleTime, CUserCmd *cmd )
{
	// Let the player override the view.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return true;

	// Let the player at it
	return pPlayer->CreateMove( flInputSampleTime, cmd );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSetup - 
//-----------------------------------------------------------------------------
void ClientModeShared::OverrideView( CViewSetup *pSetup )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawEntity(C_BaseEntity *pEnt)
{
	return true;
}

bool ClientModeShared::ShouldDrawParticles( )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Allow weapons to override mouse input (for binoculars)
//-----------------------------------------------------------------------------
void ClientModeShared::OverrideMouseInput( float *x, float *y )
{
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->OverrideMouseInput( x, y );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawViewModel()
{
	return true;
}

bool ClientModeShared::ShouldDrawDetailObjects( )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawCrosshair( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Don't draw the current view entity if we are not in 3rd person
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawLocalPlayer( C_BasePlayer *pPlayer )
{
	if ( ( pPlayer->index == render->GetViewEntity() ) && !C_BasePlayer::ShouldDrawLocalPlayer() )
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: The mode can choose to not draw fog
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawFog( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::AdjustEngineViewport( int& x, int& y, int& width, int& height )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::PreRender( CViewSetup *pSetup )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::PostRender()
{
	// Let the particle manager simulate things that haven't been simulated.
	ParticleMgr()->PostRender();
}

void ClientModeShared::PostRenderVGui()
{
}

void ClientModeShared::PostRenderVGuiOnTop()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Update()
{
	if ( m_pViewport->IsVisible() != cl_drawhud.GetBool() )
	{
		m_pViewport->SetVisible( cl_drawhud.GetBool() );
	}

	UpdateRumbleEffects();
}

//-----------------------------------------------------------------------------
// This processes all input before SV Move messages are sent
//-----------------------------------------------------------------------------

void ClientModeShared::ProcessInput(bool bActive)
{
	gHUD.ProcessInput( bActive );
}

//-----------------------------------------------------------------------------
// Purpose: We've received a keypress from the engine. Return 1 if the engine is allowed to handle it.
//-----------------------------------------------------------------------------
int	ClientModeShared::KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if ( engine->Con_IsVisible() )
		return 1;
	
	// Should we start typing a message?
	if ( pszCurrentBinding &&
		( Q_strcmp( pszCurrentBinding, "messagemode" ) == 0 ||
		  Q_strcmp( pszCurrentBinding, "say" ) == 0 ) )
	{
		if ( down )
		{
			StartMessageMode( MM_SAY );
		}
		return 0;
	}
	else if ( pszCurrentBinding &&
				( Q_strcmp( pszCurrentBinding, "messagemode2" ) == 0 ||
				  Q_strcmp( pszCurrentBinding, "say_team" ) == 0 ) )
	{
		if ( down )
		{
			StartMessageMode( MM_SAY_TEAM );
		}
		return 0;
	}
	else if ( pszCurrentBinding &&
				( Q_strcmp( pszCurrentBinding, "messagemode3" ) == 0 ||
				  Q_strcmp( pszCurrentBinding, "say_spec" ) == 0 ) )
	{
		if ( down )
		{
			StartMessageMode( MM_SAY_SPEC );
		}
		return 0;
	}
	
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	// if ingame spectator mode, let spectator input intercept key event here
	if( pPlayer
		&& pPlayer->IsObserver()
		&& !HandleSpectatorKeyInput( down, keynum, pszCurrentBinding ) )
	{
		return 0;
	}

	// Let game-specific hud elements get a crack at the key input
	if ( !HudElementKeyInput( down, keynum, pszCurrentBinding ) )
	{
		return 0;
	}

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		return pWeapon->KeyInput( down, keynum, pszCurrentBinding );
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: See if spectator input occurred. Return 0 if the key is swallowed.
//-----------------------------------------------------------------------------
int ClientModeShared::HandleSpectatorKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	// we are in spectator mode, open spectator menu
	if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+duck" ) == 0 )
	{
		//m_pViewport->ShowPanel( PANEL_SPECMENU, true );
		CSpectatorMenu *pSpecMenu = (CSpectatorMenu *)m_pViewport->FindPanelByName(PANEL_SPECMENU);
		if (pSpecMenu->IsMouseInputEnabled())
		{
			pSpecMenu->SetMouseInputEnabled(false);
			pSpecMenu->SetVisible(false);
		}
		else
		{
			pSpecMenu->SetMouseInputEnabled(true);
			pSpecMenu->SetVisible(true);
		}

		return 0; // we handled it, don't handle twice or send to server
	}
	else if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+moveright" ) == 0 && Camera()->GetCamMode() != CAM_MODE_ROAMING)
	{
		engine->ClientCmd( "spec_next" );
		return 0;
	}
	else if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+moveleft" ) == 0 && Camera()->GetCamMode() != CAM_MODE_ROAMING)
	{
		engine->ClientCmd( "spec_prev" );
		return 0;
	}
	else if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+jump" ) == 0 )
	{
		engine->ClientCmd( "cam_mode" );
		return 0;
	}
	else if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+strafe" ) == 0 )
	{
		return 0;
	}
	else if ( down && pszCurrentBinding && Q_strncmp( pszCurrentBinding, "slot", 4 ) == 0 )
	{
		engine->ClientCmd(VarArgs("tvcam_mode %d", (pszCurrentBinding[4] - '0') - 1));
		return 0;
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: See if hud elements want key input. Return 0 if the key is swallowed
//-----------------------------------------------------------------------------
int ClientModeShared::HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::Panel *ClientModeShared::GetMessagePanel()
{
	if ( m_pChatElement && m_pChatElement->GetInputPanel() && m_pChatElement->GetInputPanel()->IsVisible() )
		return m_pChatElement->GetInputPanel();

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: The player has started to type a message
//-----------------------------------------------------------------------------
void ClientModeShared::StartMessageMode( int iMessageModeType )
{
	// Can only show chat UI in multiplayer!!!
	if ( gpGlobals->maxClients == 1 )
	{
		return;
	}
	if ( m_pChatElement )
	{
		m_pChatElement->StartMessageMode( iMessageModeType );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *newmap - 
//-----------------------------------------------------------------------------
void ClientModeShared::LevelInit( const char *newmap )
{
	m_pViewport->GetAnimationController()->StartAnimationSequence("LevelInit");

	// Tell the Chat Interface
	if ( m_pChatElement )
	{
		m_pChatElement->LevelInit( newmap );
	}

	// we have to fake this event clientside, because clients connect after that
	IGameEvent *event = gameeventmanager->CreateEvent( "game_newmap" );
	if ( event )
	{
		event->SetString("mapname", newmap );
		gameeventmanager->FireEventClientSide( event );
	}

	// Create a vgui context for all of the in-game vgui panels...
	if ( s_hVGuiContext == DEFAULT_VGUI_CONTEXT )
	{
		s_hVGuiContext = vgui::ivgui()->CreateContext();
	}

	// Reset any player explosion/shock effects
	CLocalPlayerFilter filter;
	enginesound->SetPlayerDSP( filter, 0, true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::LevelShutdown( void )
{
	if ( m_pChatElement )
	{
		m_pChatElement->LevelShutdown();
	}
	if ( s_hVGuiContext != DEFAULT_VGUI_CONTEXT )
	{
		vgui::ivgui()->DestroyContext( s_hVGuiContext );
 		s_hVGuiContext = DEFAULT_VGUI_CONTEXT;
	}

	// Reset any player explosion/shock effects
	CLocalPlayerFilter filter;
	enginesound->SetPlayerDSP( filter, 0, true );
}


void ClientModeShared::Enable()
{
	vgui::VPANEL pRoot;

	// Add our viewport to the root panel.
	if( (pRoot = VGui_GetClientDLLRootPanel() ) != NULL )
	{
		m_pViewport->SetParent( pRoot );
	}

	// All hud elements should be proportional
	// This sets that flag on the viewport and all child panels
	m_pViewport->SetProportional( true );

	m_pViewport->SetCursor( m_CursorNone );
	vgui::surface()->SetCursor( m_CursorNone );

	m_pViewport->SetVisible( true );
	if ( m_pViewport->IsKeyBoardInputEnabled() )
	{
		m_pViewport->RequestFocus();
	}

	Layout();
}


void ClientModeShared::Disable()
{
	vgui::VPANEL pRoot;

	// Remove our viewport from the root panel.
	if( ( pRoot = VGui_GetClientDLLRootPanel() ) != NULL )
	{
		m_pViewport->SetParent( (vgui::VPANEL)NULL );
	}

	m_pViewport->SetVisible( false );
}


void ClientModeShared::Layout()
{
	vgui::VPANEL pRoot;
	int wide, tall;

	// Make the viewport fill the root panel.
	if( ( pRoot = VGui_GetClientDLLRootPanel() ) != NULL )
	{
		vgui::ipanel()->GetSize(pRoot, wide, tall);

		bool changed = wide != m_nRootSize[ 0 ] || tall != m_nRootSize[ 1 ];
		m_nRootSize[ 0 ] = wide;
		m_nRootSize[ 1 ] = tall;

		m_pViewport->SetBounds(0, 0, wide, tall);
		if ( changed )
		{
			ReloadScheme();
		}
	}
}

float ClientModeShared::GetViewModelFOV( void )
{
	return v_viewmodel_fov.GetFloat();
}

class CHudChat;

bool PlayerNameNotSetYet( const char *pszName )
{
	if ( pszName && pszName[0] )
	{
		// Don't show "unconnected" if we haven't got the players name yet
		if ( Q_strnicmp(pszName,"unconnected",11) == 0 )
			return true;
		if ( Q_strnicmp(pszName,"NULLNAME",11) == 0 )
			return true;
	}

	return false;
}

void ClientModeShared::FireGameEvent( IGameEvent *event )
{
	CBaseHudChat *hudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );

	const char *eventname = event->GetName();

	if ( Q_strcmp( "player_connect", eventname ) == 0 )
	{
		if ( !hudChat )
			return;
		if ( PlayerNameNotSetYet(event->GetString("name")) )
			return;

		if ( !IsInCommentaryMode() )
		{
			wchar_t wszLocalized[100];
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("name"), wszPlayerName, sizeof(wszPlayerName) );
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_joined_game" ), 1, wszPlayerName );

			char szLocalized[100];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_JOINLEAVE, "%s", szLocalized );
		}
	}
	else if ( Q_strcmp( "player_disconnect", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );

		if ( !hudChat || !pPlayer )
			return;
		if ( PlayerNameNotSetYet(event->GetString("name")) )
			return;

		if ( !IsInCommentaryMode() )
		{
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );

			wchar_t wszReason[64];
			g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("reason"), wszReason, sizeof(wszReason) );

			wchar_t wszLocalized[100];
			if (IsPC())
			{
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_left_game" ), 2, wszPlayerName, wszReason );
			}
			else
			{
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_left_game" ), 1, wszPlayerName );
			}

			char szLocalized[100];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_JOINLEAVE, "%s", szLocalized );
		}
	}
	else if ( Q_strcmp( "player_team", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );
		if ( !hudChat )
			return;

		bool bDisconnected = event->GetBool("disconnect");

		if ( bDisconnected )
			return;

		int newTeam = event->GetInt("newteam");
		int oldTeam = event->GetInt("oldteam");

		int newTeamPos = event->GetInt("newteampos");
		int oldTeamPos = event->GetInt("oldteampos");

		int newSpecTeam = event->GetInt("newspecteam");
		int oldSpecTeam = event->GetInt("oldspecteam");

		int maxplayers = event->GetInt("maxplayers");

		if (newTeam == TEAM_NONE)
			return;

		const char *pszName = event->GetString("name");
		if ( PlayerNameNotSetYet( pszName ) )
			return;

		if (!event->GetBool("silent"))
		{
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pszName, wszPlayerName, sizeof(wszPlayerName) );

			wchar_t wszNewTeam[64];
			C_Team *pNewTeam = GetGlobalTeam( newTeam );
			if ( pNewTeam )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pNewTeam->GetCode(), wszNewTeam, sizeof(wszNewTeam) );
			}
			else
			{
				_snwprintf ( wszNewTeam, sizeof( wszNewTeam ) / sizeof( wchar_t ), L"%d", newTeam );
			}

			wchar_t wszOldTeam[64];
			C_Team *pOldTeam = GetGlobalTeam( oldTeam );
			if ( pOldTeam )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pOldTeam->GetCode(), wszOldTeam, sizeof(wszOldTeam) );
			}
			else
			{
				_snwprintf ( wszOldTeam, sizeof( wszOldTeam ) / sizeof( wchar_t ), L"%d", oldTeam );
			}

			wchar_t wszLocalized[100];

			wchar_t wszNewTeamPos[4];
			if (newTeam == TEAM_HOME || newTeam == TEAM_AWAY)
				g_pVGuiLocalize->ConvertANSIToUnicode(g_szPosNames[(int)GetGlobalTeam(newTeam)->GetFormation()->positions[newTeamPos]->type], wszNewTeamPos, sizeof(wszNewTeamPos));

			wchar_t wszOldTeamPos[4];
			if (oldTeam == TEAM_HOME || oldTeam == TEAM_AWAY)
				g_pVGuiLocalize->ConvertANSIToUnicode(g_szPosNames[(int)GetGlobalTeam(oldTeam)->GetFormation()->positions[oldTeamPos]->type], wszOldTeamPos, sizeof(wszOldTeamPos));

			if (newTeam == TEAM_HOME || newTeam == TEAM_AWAY)
			{
				if ((oldTeam == TEAM_HOME || oldTeam == TEAM_AWAY) && newTeam != oldTeam)
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_switched_team" ), 3, wszPlayerName, wszNewTeam, wszNewTeamPos );
				else if ((oldTeam == TEAM_HOME || oldTeam == TEAM_AWAY) && newTeamPos != oldTeamPos)
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_switched_pos" ), 2, wszPlayerName, wszNewTeamPos );
				else
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_joined_team" ), 3, wszPlayerName, wszNewTeam, wszNewTeamPos );
			}
			else
			{
				wchar_t wszNewSpecTeam[64] = {};
				if (newSpecTeam == TEAM_HOME || newSpecTeam == TEAM_AWAY)
				{
					C_Team *pNewSpecTeam = GetGlobalTeam( newSpecTeam );
					g_pVGuiLocalize->ConvertANSIToUnicode( pNewSpecTeam->GetCode(), wszNewSpecTeam, sizeof(wszNewSpecTeam) );
				}

				if (oldTeam == TEAM_HOME || oldTeam == TEAM_AWAY)
				{
					if (newSpecTeam == TEAM_SPECTATOR)
						g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_left_field_spectating" ), 3, wszPlayerName, wszOldTeam, wszOldTeamPos );
					else
						g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_left_field_bench" ), 4, wszPlayerName, wszOldTeam, wszOldTeamPos, wszNewSpecTeam );
				}
				else
				{
					if (newSpecTeam == TEAM_SPECTATOR)
						g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_joined_spectator" ), 1, wszPlayerName );
					else
					{

						g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_joined_bench" ), 2, wszPlayerName, wszNewSpecTeam );
					}
				}
			}

			char szLocalized[100];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_TEAMCHANGE, "%s", szLocalized );
		}

		if ( pPlayer && pPlayer->IsLocalPlayer() )
		{
			// that's me
			pPlayer->TeamChange( newTeam );
		}
	}
	else if ( Q_strcmp( "player_changename", eventname ) == 0 )
	{
		if ( !hudChat )
			return;

		const char *pszOldName = event->GetString("oldname");
		if ( PlayerNameNotSetYet(pszOldName) )
			return;

		wchar_t wszOldName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszOldName, wszOldName, sizeof(wszOldName) );

		wchar_t wszNewName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString( "newname" ), wszNewName, sizeof(wszNewName) );

		wchar_t wszLocalized[100];
		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_changed_name" ), 2, wszOldName, wszNewName );

		char szLocalized[100];
		g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

		hudChat->Printf( CHAT_FILTER_NAMECHANGE, "%s", szLocalized );
	}
	else if ( Q_strcmp( "player_changeclub", eventname ) == 0 )
	{
		if ( !hudChat )
			return;

		C_SDKPlayer *pPl = ToSDKPlayer(USERID2PLAYER(event->GetInt("userid")));
		if (!pPl)
			return;

		const char *pszOldClub = event->GetString("oldclub")[0] == 0 ? "''" : event->GetString("oldclub");

		wchar_t wszOldClub[MAX_CLUBNAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszOldClub, wszOldClub, sizeof(wszOldClub) );

		wchar_t wszNewClub[MAX_CLUBNAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("newclub")[0] == 0 ? "''" : event->GetString("newclub"), wszNewClub, sizeof(wszNewClub) );

		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pPl->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );

		wchar_t wszLocalized[100];
		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_changed_club" ), 3, wszPlayerName, wszOldClub, wszNewClub );

		char szLocalized[100];
		g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

		hudChat->Printf( CHAT_FILTER_NAMECHANGE, "%s", szLocalized );
	}
	else if ( Q_strcmp( "player_changenationalteam", eventname ) == 0 )
	{
		if ( !hudChat )
			return;

		C_SDKPlayer *pPl = ToSDKPlayer(USERID2PLAYER(event->GetInt("userid")));
		if (!pPl)
			return;

		const char *pszOldNationalTeam = event->GetString("oldnationalteam")[0] == 0 ? "''" : event->GetString("oldnationalteam");

		wchar_t wszOldNationalTeam[MAX_CLUBNAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszOldNationalTeam, wszOldNationalTeam, sizeof(wszOldNationalTeam) );

		wchar_t wszNewNationalTeam[MAX_CLUBNAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("newnationalteam")[0] == 0 ? "''" : event->GetString("newnationalteam"), wszNewNationalTeam, sizeof(wszNewNationalTeam) );

		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pPl->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );

		wchar_t wszLocalized[100];
		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_changed_nationalteam" ), 3, wszPlayerName, wszOldNationalTeam, wszNewNationalTeam );

		char szLocalized[100];
		g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

		hudChat->Printf( CHAT_FILTER_NAMECHANGE, "%s", szLocalized );
	}
	else if ( Q_strcmp( "teamplay_broadcast_audio", eventname ) == 0 )
	{
		int team = event->GetInt( "team" );

		bool bValidTeam = false;

		if ( (GetLocalTeam() && GetLocalTeam()->GetTeamNumber() == team) )
		{
			bValidTeam = true;
		}

		if ( team == 0 && GetLocalTeam() > 0 )
		{
			bValidTeam = false;
		}

		if ( bValidTeam == true )
		{
			CLocalPlayerFilter filter;
			const char *pszSoundName = event->GetString("sound");
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, pszSoundName );
		}
	}
	else if ( Q_strcmp( "teamplay_broadcast_audio", eventname ) == 0 )
	{
		int team = event->GetInt( "team" );
		if ( !team || (GetLocalTeam() && GetLocalTeam()->GetTeamNumber() == team) )
		{
			CLocalPlayerFilter filter;
			const char *pszSoundName = event->GetString("sound");
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, pszSoundName );
		}
	}
	else if ( Q_strcmp( "server_cvar", eventname ) == 0 )
	{
		if ( !IsInCommentaryMode() )
		{
			wchar_t wszCvarName[64];
			g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("cvarname"), wszCvarName, sizeof(wszCvarName) );

			wchar_t wszCvarValue[16];
			g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("cvarvalue"), wszCvarValue, sizeof(wszCvarValue) );

			wchar_t wszLocalized[100];
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_server_cvar_changed" ), 2, wszCvarName, wszCvarValue );

			char szLocalized[100];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_SERVERMSG, "%s", szLocalized );
		}
	}
	else if ( Q_strcmp( "achievement_earned", eventname ) == 0 )
	{
		int iPlayerIndex = event->GetInt( "player" );
		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( iPlayerIndex );
		int iAchievement = event->GetInt( "achievement" );

		if ( !hudChat || !pPlayer )
			return;

		if ( !IsInCommentaryMode() )
		{
			CAchievementMgr *pAchievementMgr = dynamic_cast<CAchievementMgr *>( engine->GetAchievementMgr() );
			if ( !pAchievementMgr )
				return;

			IAchievement *pAchievement = pAchievementMgr->GetAchievementByID( iAchievement );
			if ( pAchievement )
			{
				if ( !pPlayer->IsDormant() )
				{
					// no particle effect if the local player is the one with the achievement or the player is dead
					if ( !pPlayer->IsLocalPlayer() && pPlayer->IsAlive() ) 
					{
						//tagES using the "head" attachment won't work for CS and DoD
						pPlayer->ParticleProp()->Create( "achieved", PATTACH_POINT_FOLLOW, "head" );
					}

					pPlayer->OnAchievementAchieved( iAchievement );
				}

				if ( g_PR )
				{
					wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
					g_pVGuiLocalize->ConvertANSIToUnicode( g_PR->GetPlayerName( iPlayerIndex ), wszPlayerName, sizeof( wszPlayerName ) );

					const wchar_t *pchLocalizedAchievement = ACHIEVEMENT_LOCALIZED_NAME_FROM_STR( pAchievement->GetName() );
					if ( pchLocalizedAchievement )
					{
						wchar_t wszLocalizedString[128];
						g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( "#Achievement_Earned" ), 2, wszPlayerName, pchLocalizedAchievement );

						char szLocalized[128];
						g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalizedString, szLocalized, sizeof( szLocalized ) );

						hudChat->ChatPrintf( iPlayerIndex, CHAT_FILTER_SERVERMSG, "%s", szLocalized );
					}
				}
			}
		}
	}
	else if ( Q_strcmp( "team_formation", eventname ) == 0 )
	{
		wchar_t wszTeamName[MAX_SHORTTEAMNAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( GetGlobalTeam(event->GetInt("team"))->GetCode(), wszTeamName, sizeof(wszTeamName) );

		wchar_t wszOldFormation[16];
		g_pVGuiLocalize->ConvertANSIToUnicode( SDKGameRules()->GetFormations()[event->GetInt("old_formation")]->name, wszOldFormation, sizeof(wszOldFormation) );

		wchar_t wszNewFormation[16];
		g_pVGuiLocalize->ConvertANSIToUnicode( SDKGameRules()->GetFormations()[event->GetInt("new_formation")]->name, wszNewFormation, sizeof(wszNewFormation) );

		wchar_t wszLocalized[128];
		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#team_formation_changed" ), 3, wszTeamName, wszOldFormation, wszNewFormation );

		char szLocalized[128];
		g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

		hudChat->Printf( CHAT_FILTER_SERVERMSG, "%s", szLocalized );
	}
	else
	{
		DevMsg( 2, "Unhandled GameEvent in ClientModeShared::FireGameEvent - %s\n", event->GetName()  );
	}
}


	


//-----------------------------------------------------------------------------
// In-game VGUI context 
//-----------------------------------------------------------------------------
void ClientModeShared::ActivateInGameVGuiContext( vgui::Panel *pPanel )
{
	vgui::ivgui()->AssociatePanelWithContext( s_hVGuiContext, pPanel->GetVPanel() );
	vgui::ivgui()->ActivateContext( s_hVGuiContext );
}

void ClientModeShared::DeactivateInGameVGuiContext()
{
	vgui::ivgui()->ActivateContext( DEFAULT_VGUI_CONTEXT );
}

