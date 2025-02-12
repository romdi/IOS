//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_sdk_player.h"
#include "c_playerresource.h"
#include "vgui_EntityPanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "sdk_gamerules.h"
#include "c_ios_replaymanager.h"
#include "voice_status.h"
#include "view.h"
#include "c_team.h"
#include "clientmode_shared.h"
#include "c_match_ball.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYER_HINT_DISTANCE	150
#define PLAYER_HINT_DISTANCE_SQ	(PLAYER_HINT_DISTANCE*PLAYER_HINT_DISTANCE)

ConVar hud_names_visible("hud_names_visible", "1", FCVAR_ARCHIVE);

ConVar hud_names_type("hud_names_type", "0", FCVAR_ARCHIVE, "", true, 0, true, 2);

void CC_HudNamesToggle(const CCommand &args)
{
	hud_names_visible.SetValue(!hud_names_visible.GetBool());
}

ConCommand hud_names_toggle("hud_names_toggle", CC_HudNamesToggle);

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSDKNames : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CSDKNames, vgui::Panel );

public:
	CSDKNames( const char *pElementName );
	void Init( void );
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint( void );
	void VidInit( void );
	void FireGameEvent(IGameEvent *event);

private:
	vgui::HFont		m_hFont;
};

DECLARE_HUDELEMENT( CSDKNames );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSDKNames::CSDKNames( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "Names" )
{
	SetHiddenBits(HIDEHUD_PLAYERNAMES);

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_hFont = g_hFontTrebuchet24;
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void CSDKNames::Init( void )
{
}

void CSDKNames::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hFont = scheme->GetFont("IOSPlayerName");

	SetPaintBackgroundEnabled( false );
	SetBounds(0, 0, ScreenWidth(), ScreenHeight());
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CSDKNames::VidInit()
{
	CHudElement::VidInit();
}

void DrawPlayerInfo(HFont font, const Vector &origin, const char *playerText, int teamNumber)
{
	wchar_t wszPlayerText[MAX_PLAYER_NAME_LENGTH];
	g_pVGuiLocalize->ConvertANSIToUnicode(playerText, wszPlayerText, sizeof(wszPlayerText));

	int wide, tall;
	vgui::surface()->GetTextSize(font, wszPlayerText, wide, tall);

	Color c = GetGlobalTeam(teamNumber)->GetHudKitColor();

	c.SetColor(c.r(), c.g(), c.b(), 255 * 0.75f);

	Vector pos = origin;
	pos.z += VEC_HULL_MAX.z + 50;

	int xPos, yPos;
	GetVectorInScreenSpace(pos, xPos, yPos);

	surface()->DrawSetTextFont(font);
	surface()->DrawSetTextColor(c);
	surface()->DrawSetTextPos(xPos - wide / 2, yPos - tall);
	surface()->DrawPrintText(wszPlayerText, wcslen(wszPlayerText));
}

//-----------------------------------------------------------------------------
// Purpose: Draw function for the element
//-----------------------------------------------------------------------------
void CSDKNames::Paint()
{
	C_SDKPlayer *pLocal = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pLocal)
		return;

	if (GetReplayManager() && GetReplayManager()->IsReplaying())
	{		
		for (int i = gpGlobals->maxClients; i <= ClientEntityList().GetHighestEntityIndex(); i++)
		{
			C_ReplayPlayer *pPl = dynamic_cast<C_ReplayPlayer *>(ClientEntityList().GetBaseEntity(i));
			if (!pPl)
				continue;

			if (hud_names_visible.GetBool())
			{
				const char *playerText;

				if (hud_names_type.GetInt() == 0)
					playerText = pPl->m_szPlayerName;
				else if (hud_names_type.GetInt() == 1)
					playerText = g_szPosNames[GetGlobalTeam(pPl->m_nTeamNumber)->GetFormation()->positions[pPl->m_nTeamPosIndex]->type];
				else
					playerText = VarArgs("%d", pPl->m_nShirtNumber);

				DrawPlayerInfo(m_hFont, pPl->GetLocalOrigin(), playerText, pPl->m_nTeamNumber);
			}
		}
	}
	else
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			C_SDKPlayer *pPl = ToSDKPlayer(UTIL_PlayerByIndex(i));
			if (!pPl || pPl->IsDormant())
				continue;

			if (pPl != pLocal && hud_names_visible.GetBool())
			{
				const char *playerText;

				if (hud_names_type.GetInt() == 0)
					playerText = pPl->GetPlayerName();
				else if (hud_names_type.GetInt() == 1)
					playerText = g_szPosNames[pPl->GetTeam()->GetFormation()->positions[GameResources()->GetTeamPosIndex(i)]->type];
				else
					playerText = VarArgs("%d", GameResources()->GetShirtNumber(i));

				DrawPlayerInfo(m_hFont, pPl->GetLocalOrigin(), playerText, pPl->GetTeamNumber());
			}
		}
	}
}

void CSDKNames::FireGameEvent(IGameEvent *event)
{
}