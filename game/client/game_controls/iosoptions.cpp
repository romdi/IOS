#include "cbase.h"
#include "iosoptions.h"
#include "ienginevgui.h"
#include "c_sdk_player.h"
#include "c_playerresource.h"
#include "sdk_gamerules.h"
#include "ios_teamkit_parse.h"

class CIOSOptionsMenu : public IIOSOptionsMenu
{
private:
	CIOSOptionsPanel *m_pIOSOptionsPanel;
	vgui::VPANEL m_hParent;
 
public:
	CIOSOptionsMenu( void )
	{
		m_pIOSOptionsPanel = NULL;
	}
 
	void Create( vgui::VPANEL parent )
	{
		// Create immediately
		m_pIOSOptionsPanel = new CIOSOptionsPanel(parent);
	}
 
	void Destroy( void )
	{
		if ( m_pIOSOptionsPanel )
		{
			m_pIOSOptionsPanel->SetParent( (vgui::Panel *)NULL );
			delete m_pIOSOptionsPanel;
		}
	}
	CIOSOptionsPanel *GetPanel()
	{
		return m_pIOSOptionsPanel;
	}
};

static CIOSOptionsMenu g_IOSOptionsMenu;
IIOSOptionsMenu *iosOptionsMenu = (IIOSOptionsMenu *)&g_IOSOptionsMenu;

void CC_IOSOptionsMenu(const CCommand &args)
{
	if (!iosOptionsMenu->GetPanel()->IsVisible())
		iosOptionsMenu->GetPanel()->Activate();
	else
		iosOptionsMenu->GetPanel()->Close();
}

ConCommand iosoptionsmenu("iosoptionsmenu", CC_IOSOptionsMenu);

#define SHIRT_NUMBER_COUNT 11

enum { LABEL_WIDTH = 260, INPUT_WIDTH = 260, SHORTINPUT_WIDTH = 200, TEXT_HEIGHT = 26, TEXT_MARGIN = 5 };
enum { PANEL_TOPMARGIN = 70, PANEL_MARGIN = 5, PANEL_WIDTH = (1024 - 2 * PANEL_MARGIN), PANEL_HEIGHT = (720 - 2 * PANEL_MARGIN) };
enum { PADDING = 10, TOP_PADDING = 30 };
enum { BUTTON_WIDTH = 80, BUTTON_HEIGHT = 26, BUTTON_MARGIN = 5 };
enum { SUGGESTED_VALUE_WIDTH = 100, SUGGESTED_VALUE_MARGIN = 5 };
enum { INFOBUTTON_WIDTH = 30, INFOBUTTON_MARGIN = 5 };
enum { APPEARANCE_HOFFSET = 270, APPEARANCE_RADIOBUTTONWIDTH = 70 };

#define INTERP_VALUES 5
const int interpValues[INTERP_VALUES] = { 1, 2, 3, 4, 5 };
const char *interpTexts[INTERP_VALUES] = { "Very Short (cl_interp_ratio 1)", "Short (cl_interp_ratio 2)", "Medium (cl_interp_ratio 3)", "Long (cl_interp_ratio 4)", "Very Long (cl_interp_ratio 5)" };
#define SMOOTH_VALUES 5
const int smoothValues[SMOOTH_VALUES] = { 1, 5, 10, 25, 50 };
const char *smoothTexts[SMOOTH_VALUES] = { "Very Short (cl_smoothtime 0.01)", "Short (cl_smoothtime 0.05)", "Medium (cl_smoothtime 0.1)", "Long (cl_smoothtime 0.25)", "Very Long (cl_smoothtime 0.5)" };

CIOSOptionsPanel::CIOSOptionsPanel(VPANEL parent) : BaseClass(NULL, "IOSOptionsPanel")
{
	SetScheme("SourceScheme");
	SetParent(parent);
	m_pContent = new PropertySheet(this, "");
	m_pSettingPanels[SETTING_PANEL_NETWORK] = new CNetworkSettingPanel(m_pContent, "");
	m_pSettingPanels[SETTING_PANEL_APPEARANCE] = new CAppearanceSettingPanel(m_pContent, "");
	m_pSettingPanels[SETTING_PANEL_GAMEPLAY] = new CGameplaySettingPanel(m_pContent, "");
	m_pSettingPanels[SETTING_PANEL_VISUAL] = new CVisualSettingPanel(m_pContent, "");
	m_pSettingPanels[SETTING_PANEL_SOUND] = new CSoundSettingPanel(m_pContent, "");
	m_pContent->AddPage(dynamic_cast<CNetworkSettingPanel *>(m_pSettingPanels[SETTING_PANEL_NETWORK]), "Network");
	m_pContent->AddPage(dynamic_cast<CAppearanceSettingPanel *>(m_pSettingPanels[SETTING_PANEL_APPEARANCE]), "Appearance");
	m_pContent->AddPage(dynamic_cast<CGameplaySettingPanel *>(m_pSettingPanels[SETTING_PANEL_GAMEPLAY]), "Gameplay");
	m_pContent->AddPage(dynamic_cast<CVisualSettingPanel *>(m_pSettingPanels[SETTING_PANEL_VISUAL]), "Visuals");
	m_pContent->AddPage(dynamic_cast<CSoundSettingPanel *>(m_pSettingPanels[SETTING_PANEL_SOUND]), "Sound");
	m_pOKButton = new Button(this, "", "OK", this, "save_and_close");
	m_pCancelButton = new Button(this, "", "Cancel", this, "close");
	m_pSaveButton = new Button(this, "", "Apply", this, "save_settings");
	m_pChangeInfoText = new Label(this, "", "Go spectator mode to change");
}

CIOSOptionsPanel::~CIOSOptionsPanel()
{
}

void CIOSOptionsPanel::ApplySchemeSettings( IScheme *pScheme )
{
	m_pScheme = pScheme;
	BaseClass::ApplySchemeSettings( pScheme );

	SetTitle("Player Settings", false);
	SetProportional(false);
	SetSizeable(false);
	SetBounds(0, 0, 600, PANEL_HEIGHT);
	//SetBgColor(Color(0, 0, 0, 255));
	SetPaintBackgroundEnabled(true);
	MoveToCenterOfScreen();

	m_pContent->SetBounds(PADDING, PADDING + TOP_PADDING, GetWide() - 2 * PADDING, GetTall() - 2 * PADDING - TOP_PADDING - BUTTON_HEIGHT - PADDING);
	m_pOKButton->SetBounds(this->GetWide() - 3 * BUTTON_WIDTH - 2 * BUTTON_MARGIN - PADDING, this->GetTall() - BUTTON_HEIGHT - PADDING, BUTTON_WIDTH, BUTTON_HEIGHT);
	m_pCancelButton->SetBounds(this->GetWide() - 2 * BUTTON_WIDTH - BUTTON_MARGIN - PADDING, this->GetTall() - BUTTON_HEIGHT - PADDING, BUTTON_WIDTH, BUTTON_HEIGHT);
	m_pSaveButton->SetBounds(this->GetWide() - BUTTON_WIDTH - PADDING, this->GetTall() - BUTTON_HEIGHT - PADDING, BUTTON_WIDTH, BUTTON_HEIGHT);
	m_pChangeInfoText->SetBounds(PADDING, this->GetTall() - BUTTON_HEIGHT - PADDING, this->GetWide() - 3 * BUTTON_WIDTH, BUTTON_HEIGHT); 
	m_pChangeInfoText->SetFgColor(Color(255, 153, 153, 255));
}

void CIOSOptionsPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CIOSOptionsPanel::OnThink()
{
	BaseClass::OnThink();

	//SetTall((int)(gpGlobals->curtime * 100) % 100);
	//m_pSettingPanel->Update();
	for (int i = 0; i < SETTING_PANEL_COUNT; i++)
		m_pSettingPanels[i]->Update();

	C_SDKPlayer *pLocal = C_SDKPlayer::GetLocalSDKPlayer();

	if (pLocal)
	{
		if (gpGlobals->curtime < pLocal->m_flNextClientSettingsChangeTime)
		{
			//char *text = VarArgs("Wait %d seconds", (int)(pLocal->m_flNextClientSettingsChangeTime - gpGlobals->curtime));
			//m_pOKButton->SetText(text);
			m_pOKButton->SetEnabled(false);
			//m_pSaveButton->SetText(text);
			m_pSaveButton->SetEnabled(false);
			m_pChangeInfoText->SetVisible(true);
			m_pChangeInfoText->SetText(VarArgs("Wait %d seconds to change", (int)(pLocal->m_flNextClientSettingsChangeTime - gpGlobals->curtime)));
			return;
		}
		else if (!SDKGameRules()->IsIntermissionState() && (GetLocalPlayerTeam() == TEAM_A || GetLocalPlayerTeam() == TEAM_B))
		{
			//char *text = VarArgs("Go spec first", (int)(pLocal->m_flNextClientSettingsChangeTime - gpGlobals->curtime));
			//m_pOKButton->SetText(text);
			m_pOKButton->SetEnabled(false);
			//m_pSaveButton->SetText(text);
			m_pSaveButton->SetEnabled(false);
			m_pChangeInfoText->SetVisible(true);
			m_pChangeInfoText->SetText("Go spectator mode to change");
			return;
		}
	}

	//m_pOKButton->SetText("OK");
	m_pOKButton->SetEnabled(true);
	//m_pSaveButton->SetText("Apply");
	m_pSaveButton->SetEnabled(true);
	m_pChangeInfoText->SetVisible(false);
}

void CIOSOptionsPanel::OnCommand(const char *cmd)
{
	if (!stricmp(cmd, "save_settings") || !stricmp(cmd, "save_and_close"))
	{
		for (int i = 0; i < SETTING_PANEL_COUNT; i++)
			m_pSettingPanels[i]->Save();
		
		engine->ClientCmd("host_writeconfig\n");

		if (!stricmp(cmd, "save_and_close"))
			Close();
	}
	else if (!stricmp(cmd, "close"))
	{
		Close();
	}
	else
		BaseClass::OnCommand(cmd);
}

void CIOSOptionsPanel::Activate()
{
	BaseClass::Activate();

	for (int i = 0; i < SETTING_PANEL_COUNT; i++)
		m_pSettingPanels[i]->Load();
}

CNetworkSettingPanel::CNetworkSettingPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_pContent = new Panel(this, "");

	m_pPlayerNameLabel = new Label(m_pContent, "", "Player Name:");
	m_pPlayerNameText = new TextEntry(m_pContent, "");
	m_pPlayerNameText->SetMaximumCharCount(MAX_PLAYER_NAME_LENGTH - 1);
	m_pClubNameLabel = new Label(m_pContent, "", "IOS Club Initials:");
	m_pClubNameText = new TextEntry(m_pContent, "");
	m_pClubNameText->SetMaximumCharCount(MAX_CLUBNAME_LENGTH - 1);
	m_pCountryNameLabel = new Label(m_pContent, "", "Country Fallback Name:");
	m_pCountryNameList = new ComboBox(m_pContent, "", COUNTRY_NAMES_COUNT, false);

	m_pCountryNameList->RemoveAll();

	KeyValues *kv = NULL;

	kv = new KeyValues("UserData", "index", 0);
	m_pCountryNameList->AddItem("<None>", kv);
	kv->deleteThis();

	for (int i = 1; i < COUNTRY_NAMES_COUNT; i++)
	{
		kv = new KeyValues("UserData", "index", i);
		m_pCountryNameList->AddItem(g_szCountryNames[i], kv);
		kv->deleteThis();
	}

	const char *suggestedValueText = "Set Best Value";

	m_pInterpDurationLabel = new Label(m_pContent, "", "Interpolation Duration:");
	m_pInterpDurationList = new ComboBox(m_pContent, "", 0, false);
	m_pInterpDurationSuggestedValueButton = new Button(m_pContent, "", suggestedValueText, this, "suggested_interpduration");
	m_pInterpDurationInfoButton = new Button(m_pContent, "", "?");
	m_pInterpDurationInfoButton->GetTooltip()->SetText("The shorter the interpolation duration, the quicker your client will display updated player and ball positions received from the server.\nIf you notice that other players and the ball don't move smoothly, it could mean that too many packets are lost on the way between you and the server.\nTry increasing the interpolation duration until the game is smooth again.");
	m_pInterpDurationInfoButton->GetTooltip()->SetTooltipDelay(0);

	for (int i = 0; i < INTERP_VALUES; i++)
	{
		kv = new KeyValues("UserData", "value", interpValues[i]);
		m_pInterpDurationList->AddItem(interpTexts[i], kv);
		kv->deleteThis();
	}

	m_pSmoothDurationLabel = new Label(m_pContent, "", "Smoothing Duration:");
	m_pSmoothDurationList = new ComboBox(m_pContent, "", 0, false);
	m_pSmoothDurationInfoButton = new Button(m_pContent, "", "?");
	m_pSmoothDurationSuggestedValueButton = new Button(m_pContent, "", suggestedValueText, this, "suggested_smoothduration");
	m_pSmoothDurationInfoButton->GetTooltip()->SetText("The shorter the smoothing duration, the quicker your client will set your local player to the correct position, should your client have incorrectly predicted your own position.\nTo make the game feel more reponsive, your client immediately performs certain actions like moving around and jumping, instead of waiting for the server to give confirmation for them.\nSometimes, when other players or the ball is close to you, the predictions of the client will be wrong and your local player can't actually move to the position he just went to during the prediction.\nThe smoothing duration is the time your client spends moving your own player to the correct position as received from the server.");
	m_pSmoothDurationInfoButton->GetTooltip()->SetTooltipDelay(0);

	for (int i = 0; i < SMOOTH_VALUES; i++)
	{
		kv = new KeyValues("UserData", "value", smoothValues[i]);
		m_pSmoothDurationList->AddItem(smoothTexts[i], kv);
		kv->deleteThis();
	}

	m_pRateLabel = new Label(m_pContent, "", "Max bandwidth used (rate) / 1000:");
	m_pRateList = new ComboBox(m_pContent, "", 0, false);
	m_pRateSuggestedValueButton = new Button(m_pContent, "", suggestedValueText, this, "suggested_rate");
	m_pRateInfoButton = new Button(m_pContent, "", "?");
	m_pRateInfoButton->GetTooltip()->SetText("'rate' sets the maximum bandwidth available for receiving packets from the server.\nIf 'net_graph 3' shows choke, increase the rate until the choke value shows 0.\nIf you can't increase 'Rate' any further due to a slow connection, consider lowering 'Update Rate' and 'Command Rate'.");
	m_pRateInfoButton->GetTooltip()->SetTooltipDelay(0);

	for (int i = 1; i <= 20; i++)
	{
		int value = i * 5;
		KeyValues *kv = new KeyValues("UserData", "value", VarArgs("%d", value));
		m_pRateList->AddItem(VarArgs("%d KB/s", value), kv);
		kv->deleteThis();
	}

	m_pUpdaterateLabel = new Label(m_pContent, "", "Max packets received per sec (cl_updaterate):");
	m_pUpdaterateList = new ComboBox(m_pContent, "", 0, false);
	m_pUpdaterateSuggestedValueButton = new Button(m_pContent, "", suggestedValueText, this, "suggested_updaterate");
	m_pUpdaterateInfoButton = new Button(m_pContent, "", "?");
	m_pUpdaterateInfoButton->GetTooltip()->SetText("'cl_updaterate' sets the number of updates per second you want to receive from the server.\nThe maximum value is the current server tickrate, which is usually 66 or 100.\nThe higher 'Update Rate' the more download bandwidth will be used.");
	m_pUpdaterateInfoButton->GetTooltip()->SetTooltipDelay(0);

	for (int i = 1; i <= 10; i++)
	{
		int value = i * 10;
		KeyValues *kv = new KeyValues("UserData", "value", VarArgs("%d", value));
		m_pUpdaterateList->AddItem(VarArgs("%d/s", value), kv);
		kv->deleteThis();
	}

	m_pCommandrateLabel = new Label(m_pContent, "", "Max packets sent per sec (cl_cmdrate):");
	m_pCommandrateList = new ComboBox(m_pContent, "", 0, false);
	m_pCommandrateSuggestedValueButton = new Button(m_pContent, "", suggestedValueText, this, "suggested_commandrate");
	m_pCommandrateInfoButton = new Button(m_pContent, "", "?");
	m_pCommandrateInfoButton->GetTooltip()->SetText("'cl_cmdrate' sets the number of input updates per second you want to send to the server.\nThe maximum value is the current server tickrate, which is usually 66 or 100.\nThe higher 'Command Rate' the more upload bandwidth will be used.");
	m_pCommandrateInfoButton->GetTooltip()->SetTooltipDelay(0);

	for (int i = 1; i <= 10; i++)
	{
		int value = i * 10;
		KeyValues *kv = new KeyValues("UserData", "value", VarArgs("%d", value));
		m_pCommandrateList->AddItem(VarArgs("%d/s", value), kv);
		kv->deleteThis();
	}
}

void CNetworkSettingPanel::ApplySchemeSettings( IScheme *pScheme )
{
	//BaseClass::ApplySchemeSettings( pScheme );

	//SetSize(400, 500);

	m_pContent->SetBounds(PADDING, PADDING, GetWide() - 2 * PADDING, GetTall() - 2 * PADDING);

	m_pPlayerNameLabel->SetBounds(0, 0, LABEL_WIDTH, TEXT_HEIGHT);
	m_pPlayerNameText->SetBounds(0, TEXT_HEIGHT, INPUT_WIDTH, TEXT_HEIGHT);
	m_pPlayerNameText->AddActionSignalTarget( this );
	m_pPlayerNameText->SendNewLine(true); // with the txtEntry Type you need to set it to pass the return key as a message

	m_pClubNameLabel->SetBounds(0, 2 * TEXT_HEIGHT + TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pClubNameText->SetBounds(0, 3 * TEXT_HEIGHT + TEXT_MARGIN, INPUT_WIDTH, TEXT_HEIGHT);

	m_pCountryNameLabel->SetBounds(0, 4 * TEXT_HEIGHT + 2 * TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pCountryNameList->SetBounds(0, 5 * TEXT_HEIGHT + 2 * TEXT_MARGIN, INPUT_WIDTH, TEXT_HEIGHT);

	m_pInterpDurationLabel->SetBounds(0, 6 * TEXT_HEIGHT + 3 * TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pInterpDurationList->SetBounds(0, 7 * TEXT_HEIGHT + 3 * TEXT_MARGIN, INPUT_WIDTH, TEXT_HEIGHT);
	m_pInterpDurationSuggestedValueButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN, 7 * TEXT_HEIGHT + 3 * TEXT_MARGIN, SUGGESTED_VALUE_WIDTH, TEXT_HEIGHT);
	m_pInterpDurationSuggestedValueButton->SetContentAlignment(Label::a_center);
	m_pInterpDurationInfoButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN + SUGGESTED_VALUE_WIDTH + INFOBUTTON_MARGIN, 7 * TEXT_HEIGHT + 3 * TEXT_MARGIN, INFOBUTTON_WIDTH, TEXT_HEIGHT);
	m_pInterpDurationInfoButton->SetContentAlignment(Label::a_center);

	m_pSmoothDurationLabel->SetBounds(0, 8 * TEXT_HEIGHT + 4 * TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pSmoothDurationList->SetBounds(0, 9 * TEXT_HEIGHT + 4 * TEXT_MARGIN, INPUT_WIDTH, TEXT_HEIGHT);
	m_pSmoothDurationSuggestedValueButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN, 9 * TEXT_HEIGHT + 4 * TEXT_MARGIN, SUGGESTED_VALUE_WIDTH, TEXT_HEIGHT);
	m_pSmoothDurationSuggestedValueButton->SetContentAlignment(Label::a_center);
	m_pSmoothDurationInfoButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN + SUGGESTED_VALUE_WIDTH + INFOBUTTON_MARGIN, 9 * TEXT_HEIGHT + 4 * TEXT_MARGIN, INFOBUTTON_WIDTH, TEXT_HEIGHT);
	m_pSmoothDurationInfoButton->SetContentAlignment(Label::a_center);

	m_pRateLabel->SetBounds(0, 10 * TEXT_HEIGHT + 5 * TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pRateList->SetBounds(0, 11 * TEXT_HEIGHT + 5 * TEXT_MARGIN, INPUT_WIDTH, TEXT_HEIGHT);
	m_pRateSuggestedValueButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN, 11 * TEXT_HEIGHT + 5 * TEXT_MARGIN, SUGGESTED_VALUE_WIDTH, TEXT_HEIGHT);
	m_pRateSuggestedValueButton->SetContentAlignment(Label::a_center);
	m_pRateInfoButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN + SUGGESTED_VALUE_WIDTH + INFOBUTTON_MARGIN, 11 * TEXT_HEIGHT + 5 * TEXT_MARGIN, INFOBUTTON_WIDTH, TEXT_HEIGHT);
	m_pRateInfoButton->SetContentAlignment(Label::a_center);

	m_pUpdaterateLabel->SetBounds(0, 12 * TEXT_HEIGHT + 6 * TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pUpdaterateList->SetBounds(0, 13 * TEXT_HEIGHT + 6 * TEXT_MARGIN, INPUT_WIDTH, TEXT_HEIGHT);
	m_pUpdaterateSuggestedValueButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN, 13 * TEXT_HEIGHT + 6 * TEXT_MARGIN, SUGGESTED_VALUE_WIDTH, TEXT_HEIGHT);
	m_pUpdaterateSuggestedValueButton->SetContentAlignment(Label::a_center);
	m_pUpdaterateInfoButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN + SUGGESTED_VALUE_WIDTH + INFOBUTTON_MARGIN, 13 * TEXT_HEIGHT + 6 * TEXT_MARGIN, INFOBUTTON_WIDTH, TEXT_HEIGHT);
	m_pUpdaterateInfoButton->SetContentAlignment(Label::a_center);

	m_pCommandrateLabel->SetBounds(0, 14 * TEXT_HEIGHT + 7 * TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pCommandrateList->SetBounds(0, 15 * TEXT_HEIGHT + 7 * TEXT_MARGIN, INPUT_WIDTH, TEXT_HEIGHT);
	m_pCommandrateSuggestedValueButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN, 15 * TEXT_HEIGHT + 7 * TEXT_MARGIN, SUGGESTED_VALUE_WIDTH, TEXT_HEIGHT);
	m_pCommandrateSuggestedValueButton->SetContentAlignment(Label::a_center);
	m_pCommandrateInfoButton->SetBounds(INPUT_WIDTH + SUGGESTED_VALUE_MARGIN + SUGGESTED_VALUE_WIDTH + INFOBUTTON_MARGIN, 15 * TEXT_HEIGHT + 7 * TEXT_MARGIN, INFOBUTTON_WIDTH, TEXT_HEIGHT);
	m_pCommandrateInfoButton->SetContentAlignment(Label::a_center);
}

void CNetworkSettingPanel::OnCommand(const char *cmd)
{
	if (!Q_strcmp(cmd, "suggested_interpduration"))
		m_pInterpDurationList->ActivateItemByRow(0);
	if (!Q_strcmp(cmd, "suggested_smoothduration"))
		m_pSmoothDurationList->ActivateItemByRow(0);
	else if (!Q_strcmp(cmd, "suggested_rate"))
		m_pRateList->ActivateItemByRow(m_pRateList->GetItemCount() - 1);
	else if (!Q_strcmp(cmd, "suggested_updaterate"))
		m_pUpdaterateList->ActivateItemByRow(m_pUpdaterateList->GetItemCount() - 1);
	else if (!Q_strcmp(cmd, "suggested_commandrate"))
		m_pCommandrateList->ActivateItemByRow(m_pCommandrateList->GetItemCount() - 1);
	else
		BaseClass::OnCommand(cmd);
}

void CNetworkSettingPanel::Save()
{
	char text[64];
	m_pPlayerNameText->GetText(text, sizeof(text));
	g_pCVar->FindVar("playername")->SetValue(text);
	m_pClubNameText->GetText(text, sizeof(text));
	g_pCVar->FindVar("clubname")->SetValue(text);
	g_pCVar->FindVar("fallbackcountryindex")->SetValue(m_pCountryNameList->GetActiveItemUserData()->GetInt("index"));

	g_pCVar->FindVar("cl_interp_ratio")->SetValue(atoi(m_pInterpDurationList->GetActiveItemUserData()->GetString("value")));
	g_pCVar->FindVar("cl_smoothtime")->SetValue(atoi(m_pSmoothDurationList->GetActiveItemUserData()->GetString("value")) / 100.0f);

	g_pCVar->FindVar("rate")->SetValue(m_pRateList->GetActiveItemUserData()->GetInt("value") * 1000);
	g_pCVar->FindVar("cl_updaterate")->SetValue(m_pUpdaterateList->GetActiveItemUserData()->GetInt("value"));
	g_pCVar->FindVar("cl_cmdrate")->SetValue(m_pCommandrateList->GetActiveItemUserData()->GetInt("value"));
}

void CNetworkSettingPanel::Load()
{
	if (Q_strlen(g_pCVar->FindVar("playername")->GetString()) == 0)
		g_pCVar->FindVar("playername")->SetValue(g_pCVar->FindVar("name")->GetString());

	m_pPlayerNameText->SetText(g_pCVar->FindVar("playername")->GetString());
	m_pCountryNameList->ActivateItemByRow(g_pCVar->FindVar("fallbackcountryindex")->GetInt());
	m_pClubNameText->SetText(g_pCVar->FindVar("clubname")->GetString());

	m_pInterpDurationList->ActivateItemByRow(0);

	for (int i = 0; i < INTERP_VALUES; i++)
	{
		if (interpValues[i] == (int)g_pCVar->FindVar("cl_interp_ratio")->GetFloat())
		{
			m_pInterpDurationList->ActivateItemByRow(i);
			break;
		}
	}

	m_pSmoothDurationList->ActivateItemByRow(0);

	for (int i = 0; i < SMOOTH_VALUES; i++)
	{
		if (smoothValues[i] == (int)(g_pCVar->FindVar("cl_smoothtime")->GetFloat() * 100))
		{
			m_pSmoothDurationList->ActivateItemByRow(i);
			break;
		}
	}

	m_pRateList->ActivateItemByRow(g_pCVar->FindVar("rate")->GetInt() / 1000 / 5 - 1);
	m_pUpdaterateList->ActivateItemByRow(g_pCVar->FindVar("cl_updaterate")->GetInt() / 10 - 1);
	m_pCommandrateList->ActivateItemByRow(g_pCVar->FindVar("cl_cmdrate")->GetInt() / 10 - 1);
}

void CNetworkSettingPanel::Update()
{
}

CAppearanceSettingPanel::CAppearanceSettingPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_pContent = new Panel(this, "");

	m_pPlayerPreviewPanel = new ImagePanel(m_pContent, "");
	m_pSkinIndexLabel = new Label(m_pContent, "", "Player Skin:");
	m_pSkinIndexList = new ComboBox(m_pContent, "", 0, false);

	KeyValues *kv = NULL;

	kv = new KeyValues("UserData", "value", -1);
	m_pSkinIndexList->AddItem("<Random>", kv);
	kv->deleteThis();

	kv = new KeyValues("UserData", "value", 0);
	m_pSkinIndexList->AddItem("Dark skin", kv);
	kv->deleteThis();

	kv = new KeyValues("UserData", "value", 1);
	m_pSkinIndexList->AddItem("Blond hair", kv);
	kv->deleteThis();

	kv = new KeyValues("UserData", "value", 2);
	m_pSkinIndexList->AddItem("Brown hair", kv);
	kv->deleteThis();

	kv = new KeyValues("UserData", "value", 3);
	m_pSkinIndexList->AddItem("Black hair", kv);
	kv->deleteThis();

	kv = new KeyValues("UserData", "value", 4);
	m_pSkinIndexList->AddItem("Black hair with beard", kv);
	kv->deleteThis();

	kv = new KeyValues("UserData", "value", 5);
	m_pSkinIndexList->AddItem("Darkish skin", kv);
	kv->deleteThis();

	m_pPreferredShirtNumberLabel = new Label(m_pContent, "", "Preferred Shirt Number:");
	m_pPreferredShirtNumberList = new ComboBox(m_pContent, "", SHIRT_NUMBER_COUNT, false);
	m_pPreferredShirtNumberList->RemoveAll();

	kv = new KeyValues("UserData", "index", 0);
	m_pPreferredShirtNumberList->AddItem("<None>", kv);
	kv->deleteThis();

	for (int i = 1; i < SHIRT_NUMBER_COUNT; i++)
	{
		kv = new KeyValues("UserData", "index", i);
		m_pPreferredShirtNumberList->AddItem(VarArgs("%d", i + 1), kv);
		kv->deleteThis();
	}

	m_pPlayerAngleLabel = new Label(m_pContent, "", "Rotation Angle:");
	m_pPlayerAngleSlider = new Slider(m_pContent, "");
	m_pPlayerAngleSlider->SetRange(-180, 180);
	m_pPlayerAngleSlider->SetValue(0);

	m_pPreviewTeamLabel = new Label(m_pContent, "", "Preview Team Kit:");
	m_pPreviewTeamList = new ComboBox(m_pContent, "", m_TeamKitInfoDatabase.Count(), false);
	m_pPreviewTeamList->RemoveAll();

	for (unsigned int i = 0; i < m_TeamKitInfoDatabase.Count(); i++)
	{
		kv = new KeyValues("UserData", "kitname", m_TeamKitInfoDatabase[i]->m_szKitName);
		m_pPreviewTeamList->AddItem(m_TeamKitInfoDatabase[i]->m_szKitName, kv);
		kv->deleteThis();
	}

	m_pPreviewTeamList->ActivateItemByRow(0);

	m_pBodypartPanel = new Panel(m_pContent, "");
	m_pBodypartRadioButtons[0] = new RadioButton(m_pBodypartPanel, "", "Head");
	m_pBodypartRadioButtons[1] = new RadioButton(m_pBodypartPanel, "", "Body");
	m_pBodypartRadioButtons[2] = new RadioButton(m_pBodypartPanel, "", "Shoes");
	m_pBodypartRadioButtons[1]->SetSelected(true);

	m_pConnectionInfoLabel = new Label(m_pContent, "", "Join or create a server to activate the preview");
}

void CAppearanceSettingPanel::ApplySchemeSettings(IScheme *pScheme)
{
	//BaseClass::ApplySchemeSettings( pScheme );

	m_pContent->SetBounds(PADDING, PADDING, GetWide() - 2 * PADDING, GetTall() - 2 * PADDING);

	m_pPlayerPreviewPanel->SetBounds(APPEARANCE_RADIOBUTTONWIDTH, 0, GetParent()->GetWide(), GetParent()->GetTall() - 2 * TEXT_HEIGHT);
	m_pPlayerPreviewPanel->SetImage("../_rt_playermodel");

	m_pSkinIndexLabel->SetBounds(APPEARANCE_HOFFSET + APPEARANCE_RADIOBUTTONWIDTH, 0, LABEL_WIDTH, TEXT_HEIGHT);
	m_pSkinIndexList->SetBounds(APPEARANCE_HOFFSET + APPEARANCE_RADIOBUTTONWIDTH, TEXT_HEIGHT, SHORTINPUT_WIDTH, TEXT_HEIGHT);

	m_pPreferredShirtNumberLabel->SetBounds(APPEARANCE_HOFFSET + APPEARANCE_RADIOBUTTONWIDTH, 2 * TEXT_HEIGHT + TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pPreferredShirtNumberList->SetBounds(APPEARANCE_HOFFSET + APPEARANCE_RADIOBUTTONWIDTH, 3 * TEXT_HEIGHT + TEXT_MARGIN, SHORTINPUT_WIDTH, TEXT_HEIGHT);

	m_pPlayerAngleLabel->SetBounds(APPEARANCE_RADIOBUTTONWIDTH, 2 * TEXT_HEIGHT, LABEL_WIDTH, TEXT_HEIGHT);
	m_pPlayerAngleLabel->SetVisible(false);
	m_pPlayerAngleSlider->SetBounds(APPEARANCE_RADIOBUTTONWIDTH, 512, 264, TEXT_HEIGHT);

	m_pPreviewTeamLabel->SetBounds(APPEARANCE_HOFFSET + APPEARANCE_RADIOBUTTONWIDTH, 4 * TEXT_HEIGHT + 2 * TEXT_MARGIN, LABEL_WIDTH, TEXT_HEIGHT);
	m_pPreviewTeamList->SetBounds(APPEARANCE_HOFFSET + APPEARANCE_RADIOBUTTONWIDTH, 5 * TEXT_HEIGHT + 2 * TEXT_MARGIN, SHORTINPUT_WIDTH, TEXT_HEIGHT);

	m_pBodypartPanel->SetBounds(0, 0, APPEARANCE_RADIOBUTTONWIDTH, m_pPlayerPreviewPanel->GetTall());
	m_pBodypartRadioButtons[0]->SetBounds(0, 0, APPEARANCE_RADIOBUTTONWIDTH, TEXT_HEIGHT);
	m_pBodypartRadioButtons[1]->SetBounds(0, 512 / 2 - TEXT_HEIGHT / 2, APPEARANCE_RADIOBUTTONWIDTH, TEXT_HEIGHT);
	m_pBodypartRadioButtons[2]->SetBounds(0, 512 - TEXT_HEIGHT, APPEARANCE_RADIOBUTTONWIDTH, TEXT_HEIGHT);

	m_pConnectionInfoLabel->SetBounds(APPEARANCE_RADIOBUTTONWIDTH, 512 + TEXT_HEIGHT, GetParent()->GetWide() - APPEARANCE_RADIOBUTTONWIDTH, TEXT_HEIGHT);
	m_pConnectionInfoLabel->SetFgColor(Color(255, 153, 153, 255));
}

void CAppearanceSettingPanel::Save()
{
	g_pCVar->FindVar("modelskinindex")->SetValue(m_pSkinIndexList->GetActiveItemUserData()->GetInt("value"));
	char text[64];
	m_pPreferredShirtNumberList->GetText(text, sizeof(text));
	g_pCVar->FindVar("preferredshirtnumber")->SetValue(atoi(text));
}

void CAppearanceSettingPanel::Load()
{
	m_pSkinIndexList->ActivateItemByRow(clamp(g_pCVar->FindVar("modelskinindex")->GetInt(), -1, 5) + 1);
	int shirtNum = g_pCVar->FindVar("preferredshirtnumber")->GetInt();
	m_pPreferredShirtNumberList->SetText(shirtNum == 0 ? "<None>" : VarArgs("%d", clamp(shirtNum, 2, 11)));
}

void CAppearanceSettingPanel::Update()
{
	bool isConnected = CSDKPlayer::GetLocalSDKPlayer();

	m_pConnectionInfoLabel->SetVisible(!isConnected);
	m_pSkinIndexList->SetEnabled(isConnected);
	m_pPreferredShirtNumberList->SetEnabled(isConnected);
	m_pPlayerAngleSlider->SetEnabled(isConnected);
	m_pPreviewTeamList->SetEnabled(isConnected);

	for (int i = 0; i < 3; i++)
		m_pBodypartRadioButtons[i]->SetEnabled(isConnected);
}

int CAppearanceSettingPanel::GetPlayerSkin()
{
	char text[64];
	m_pPreferredShirtNumberList->GetText(text, sizeof(text));
	int number = atoi(text);
	int skin = m_pSkinIndexList->GetActiveItemUserData()->GetInt("value");

	return clamp(number, 2, 11) - 2 + (skin * 10);
}

int CAppearanceSettingPanel::GetPlayerNumber()
{
	char text[64];
	m_pPreferredShirtNumberList->GetText(text, sizeof(text));
	int number = atoi(text);

	return clamp(number, 2, 11);
}

int CAppearanceSettingPanel::GetPlayerBodypart()
{
	for (int i = 0; i < 3; i++)
	{
		if (m_pBodypartRadioButtons[i]->IsSelected())
			return i;
	}

	return 0;
}

const char *CAppearanceSettingPanel::GetPlayerTeam()
{
	return m_pPreviewTeamList->GetActiveItemUserData()->GetString("kitname");
}

CGameplaySettingPanel::CGameplaySettingPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_pContent = new Panel(this, "");
	m_pLegacySideCurl = new CheckButton(m_pContent, "", "Legacy Side Curl");
	m_pLegacyVerticalLook = new CheckButton(m_pContent, "", "Legacy Vertical Look");
	m_pInvertKeeperSprint = new CheckButton(m_pContent, "", "Invert Keeper Sprint");
}

void CGameplaySettingPanel::ApplySchemeSettings(IScheme *pScheme)
{
	m_pContent->SetBounds(PADDING, PADDING, GetWide() - 2 * PADDING, GetTall() - 2 * PADDING);
	m_pLegacySideCurl->SetBounds(0, 0, LABEL_WIDTH + INPUT_WIDTH, TEXT_HEIGHT);
	m_pLegacyVerticalLook->SetBounds(0, TEXT_HEIGHT + TEXT_MARGIN, LABEL_WIDTH + INPUT_WIDTH, TEXT_HEIGHT);
	m_pInvertKeeperSprint->SetBounds(0, 2 * (TEXT_HEIGHT + TEXT_MARGIN), LABEL_WIDTH + INPUT_WIDTH, TEXT_HEIGHT);
}

void CGameplaySettingPanel::Save()
{
	g_pCVar->FindVar("legacysidecurl")->SetValue(m_pLegacySideCurl->IsSelected() ? 1 : 0);
	g_pCVar->FindVar("legacyverticallook")->SetValue(m_pLegacyVerticalLook->IsSelected() ? 1 : 0);
	g_pCVar->FindVar("invertkeepersprint")->SetValue(m_pInvertKeeperSprint->IsSelected() ? 1 : 0);
}

void CGameplaySettingPanel::Load()
{
	m_pLegacySideCurl->SetSelected(g_pCVar->FindVar("legacysidecurl")->GetBool());
	m_pLegacyVerticalLook->SetSelected(g_pCVar->FindVar("legacyverticallook")->GetBool());
	m_pInvertKeeperSprint->SetSelected(g_pCVar->FindVar("invertkeepersprint")->GetBool());
}

void CGameplaySettingPanel::Update()
{
}

CVisualSettingPanel::CVisualSettingPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_pContent = new Panel(this, "");
}

void CVisualSettingPanel::ApplySchemeSettings(IScheme *pScheme)
{
	m_pContent->SetBounds(PADDING, PADDING, GetWide() - 2 * PADDING, GetTall() - 2 * PADDING);
}

void CVisualSettingPanel::Save()
{
}

void CVisualSettingPanel::Load()
{
}

void CVisualSettingPanel::Update()
{
}

CSoundSettingPanel::CSoundSettingPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_pContent = new Panel(this, "");
}

void CSoundSettingPanel::ApplySchemeSettings(IScheme *pScheme)
{
	m_pContent->SetBounds(PADDING, PADDING, GetWide() - 2 * PADDING, GetTall() - 2 * PADDING);
}

void CSoundSettingPanel::Save()
{
}

void CSoundSettingPanel::Load()
{
}

void CSoundSettingPanel::Update()
{
}