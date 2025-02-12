//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CLIENTSCOREBOARDDIALOG_H
#define CLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <game/client/iviewport.h>
#include "gameeventlistener.h"
#include "vgui_bitmapbutton.h"
#include "statsmenu.h"
#include "formationmenu.h"
#include "matcheventmenu.h"

using namespace vgui;

enum stats_t
{
	CORNERS,
	DISTANCECOVERED,
	FOULS,
	FOULSSUFFERED,
	FREEKICKS,
	INTERCEPTIONS,
	OFFSIDES,
	OWNGOALS,
	PASSES,
	PASSESCOMPLETED,
	POSSESSION,
	REDCARDS,
	KEEPERSAVES,
	KEEPERSAVESCAUGHT,
	SHOTS,
	SHOTSONGOAL,
	YELLOWCARDS,
	STAT_COUNT,
};

static const char g_szStatIdentifiers[STAT_COUNT][32] =
{
	"corners",
	"distancecovered",
	"fouls",
	"foulssuffered",
	"freekicks",
	"interceptions",
	"offsides",
	"owngoals",
	"passes",
	"passescompleted",
	"possession",
	"redcards",
	"keepersaves",
	"keepersavescaught",
	"shots",
	"shotsongoal",
	"yellowcards",
};

static const char g_szStatNames[STAT_COUNT][32] =
{
	"Corners",
	"Distance",
	"Fouls",
	"Fouls suffered",
	"Free kicks",
	"Interceptions",
	"Offsides",
	"Own goals",
	"Passes",
	"Passes completed",
	"Possession",
	"Reds",
	"Saves",
	"Saves caught",
	"Shots",
	"Shots on goal",
	"Yellows",
};

enum stat_categories_t
{
	DEFAULT_STATS = -1,
	GENERAL = 0,
	TACKLES,
	//SET_PIECES,
	ATTACK,
	KEEPER,
	STAT_CATEGORY_COUNT,
};

static const char g_szStatCategoryNames[STAT_CATEGORY_COUNT][32] =
{
	"General",
	"Tackles",
	//"Set Pieces",
	"Attack",
	"Keeper",
};

struct SpecInfo
{
	int playerIndex;
	char playerName[MAX_PLAYER_NAME_LENGTH];
};

enum panel_types_t { STATS_MENU, FORMATION_MENU_NORMAL, FORMATION_MENU_HIGHLIGHT, MATCHEVENT_MENU };

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CClientScoreBoardDialog : public vgui::EditablePanel, public IViewPortPanel, public CGameEventListener
{
private:
	DECLARE_CLASS_SIMPLE( CClientScoreBoardDialog, vgui::EditablePanel );

protected:
// column widths at 640
	//ios enum { NAME_WIDTH = 160, SCORE_WIDTH = 60, DEATH_WIDTH = 60, PING_WIDTH = 80, VOICE_WIDTH = 0, FRIENDS_WIDTH = 0 };
	enum { NAME_WIDTH = 100, SCORE_WIDTH = 60, DEATH_WIDTH = 60, PING_WIDTH = 80, VOICE_WIDTH = 0, FRIENDS_WIDTH = 0, SMALL_WIDTH = 35, VSMALL_WIDTH = 20};
	// total = 340
	enum { SPEC_FONT_COUNT = 4 };

public:
	CClientScoreBoardDialog( IViewPort *pViewPort );
	~CClientScoreBoardDialog();

	virtual const char *GetName( void ) { return PANEL_SCOREBOARD; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	void ToggleMenu();

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
 	
	// IGameEventListener interface:
	virtual void FireGameEvent( IGameEvent *event);

	virtual void UpdatePlayerAvatar( int playerIndex, KeyValues *kv );
	virtual int GetCountryFlagImageIndex(int countryIndex);
	virtual int GetCardImageIndex(int cardIndex);

	void SetSelectedItem(int side, int itemId);
			
protected:
	MESSAGE_FUNC_INT( OnPollHideCode, "PollHideCode", code );
	MESSAGE_FUNC_PTR( OnCursorEntered, "OnCursorEntered", panel );
	MESSAGE_FUNC_PTR( OnCursorExited, "OnCursorExited", panel );

	// functions to override
	virtual bool GetPlayerInfo(int playerIndex, KeyValues *kv);
	virtual bool GetTeamInfo(int team, KeyValues *kv);
	virtual void UpdateTeamInfo();
	virtual void UpdatePlayerInfo();
	virtual void OnThink();
	virtual void AddHeader(); // add the start header of the scoreboard
	//virtual void AddSection(int teamType, int teamNumber); // add a new section header for a team
	virtual int GetAdditionalHeight() { return 0; } 

	void Paint();

	// sorts players within a section
	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// finds the player in the scoreboard
	int FindItemIDForPlayerIndex(int playerIndex, int &side);

	MESSAGE_FUNC_PARAMS(OnItemSelected, "ItemSelected", data);

	MESSAGE_FUNC_PARAMS(OnItemLeftClick, "ItemLeftClick", data);

	int m_iNumTeams;

	vgui::SectionedListPanel *m_pPlayerList[2];
	int				m_iSectionId; // the current section we are entering into

	int s_VoiceImage[5];
	int TrackerImage;
	int	m_HLTVSpectators;
	float m_fNextUpdateTime;

	void MoveLabelToFront(const char *textEntryName);
	void MoveToCenterOfScreen();

	void OnCommand(char const *cmd);
	
	vgui::ImagePanel *m_pImagePanel; //ios

	vgui::ImageList				*m_pImageList;
	int							m_iImageAvatars[MAX_PLAYERS+1];
	CUtlMap<int,int>			m_mapAvatarsToImageList;

	int							m_iCountryFlags[MAX_PLAYERS+1];
	CUtlMap<int,int>			m_mapCountryFlagsToImageList;

	int							m_iCards[MAX_PLAYERS+1];
	CUtlMap<int,int>			m_mapCardsToImageList;

	//CPanelAnimationVar( int, m_iAvatarWidth, "avatar_width", "34" );		// Avatar width doesn't scale with resolution
	//CPanelAnimationVarAliasType( int, m_iAvatarWidth, "avatar_width", "34", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iNameWidth, "name_width", "136", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iClassWidth, "class_width", "35", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_nGoalsWidth, "score_width", "35", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iDeathWidth, "death_width", "35", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iPingWidth, "ping_width", "23", "proportional_int" );

	int m_iAvatarWidth;

private:
	int			m_iPlayerIndexSymbol;
	int			m_iDesiredHeight;
	IViewPort	*m_pViewPort;
	ButtonCode_t m_nCloseKey;
	Panel		*m_pMainPanel;
	Panel		*m_pExtraInfoPanel;
	Label		*m_pSpectatorText;
	Panel		*m_pSpectatorNames;
	Button		*m_pSpectateButtons[3];
	Panel		*m_pSpectatorContainer;
	Label		*m_pPlayerCount[2];
	Label		*m_pPossession[2];
	Panel		*m_pStatButtonContainer;
	Label		*m_pStatText;
	Button		*m_pStatButtons[STAT_COUNT];
	Panel		*m_pPlayerListDivider;
	Button		*m_pMatchEvents;
	Button		*m_pToggleCaptainMenu;
	Label		*m_pTimeoutHeader;
	Label		*m_pTimeoutInfo;
	Button		*m_pRequestTimeout;
	Button		*m_pToggleCaptaincy;

	CStatsMenu	*m_pStatsMenu;
	CFormationMenu	*m_pFormationMenu;
	CMatchEventMenu	*m_pMatchEventMenu;

	Label		*m_pSpecInfo;
	
	Panel		*m_pTopBar;
	Label		*m_pServerInfo;
	Panel		*m_pServerInfoLine;
	Label		*m_pMatchInfo;
	Label		*m_pMatchPeriod;

	CUtlVector<SpecInfo> m_SpecList;

	int			m_nCurStat;
	int			m_nCurSpecIndex;
	Button		*m_pCurSpecButton;

	int			m_nSelectedPlayerIndex;

	panel_types_t m_eActivePanelType;

	bool		m_bShowCaptainMenu;

	int			m_nCursorPosX;
	int			m_nCursorPosY;

	bool		m_bCanSetSetpieceTaker;

	int			m_nLocalPlayerHighlight;

	IScheme *m_pScheme;
};


#endif // CLIENTSCOREBOARDDIALOG_H
