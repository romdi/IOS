#include "cbase.h"
#include "iosoptions.h"
#include "ienginevgui.h"
#include "c_sdk_player.h"

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

#define LABEL_WIDTH 150
#define INPUT_WIDTH 300
#define TEXT_HEIGHT 30

#define SHIRT_NUMBER_COUNT 11

enum { PADDING = 15, TOP_PADDING = 15 };
enum { BUTTON_WIDTH = 110, BUTTON_HEIGHT = 30, BUTTON_MARGIN = 5 };

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
	m_pContent = new Panel(this, "");
	m_pPlayerNameLabel = new Label(m_pContent, "", "Player Name:");
	m_pPlayerNameText = new TextEntry(m_pContent, "");
	m_pPlayerNameText->SetMaximumCharCount(MAX_PLAYER_NAME_LENGTH - 1);
	m_pClubNameLabel = new Label(m_pContent, "", "IOS Club Initials:");
	m_pClubNameText = new TextEntry(m_pContent, "");
	m_pClubNameText->SetMaximumCharCount(MAX_CLUBNAME_LENGTH - 1);
	m_pCountryNameLabel = new Label(m_pContent, "", "Country Fallback Name:");
	m_pCountryNameList = new ComboBox(m_pContent, "", COUNTRY_NAMES_COUNT, false);

	m_pOKButton = new Button(m_pContent, "", "OK");
	m_pCancelButton = new Button(m_pContent, "", "Cancel");
	m_pSaveButton = new Button(m_pContent, "", "Apply");

	m_pCountryNameList->RemoveAll();

	for (int i = 0; i < COUNTRY_NAMES_COUNT; i++)
	{
		KeyValues *kv = new KeyValues("UserData", "index", i);
		m_pCountryNameList->AddItem(g_szCountryNames[i], kv);
		kv->deleteThis();
	}

	m_pPreferredShirtNumberLabel = new Label(m_pContent, "", "Preferred Shirt Number:");
	m_pPreferredShirtNumberList = new ComboBox(m_pContent, "", SHIRT_NUMBER_COUNT, false);
	m_pPreferredShirtNumberList->RemoveAll();

	m_pInterpDurationLabel = new Label(m_pContent, "", "Interpolation Duration:");
	m_pInterpDurationList = new ComboBox(m_pContent, "", 0, false);

	m_pSmoothDurationLabel = new Label(m_pContent, "", "Smoothing Duration:");
	m_pSmoothDurationList = new ComboBox(m_pContent, "", 0, false);

	KeyValues *kv;

	kv = new KeyValues("UserData", "index", 0);
	m_pPreferredShirtNumberList->AddItem("<None>", kv);
	kv->deleteThis();

	for (int i = 1; i < SHIRT_NUMBER_COUNT; i++)
	{
		kv = new KeyValues("UserData", "index", i);
		m_pPreferredShirtNumberList->AddItem(VarArgs("%d", i + 1), kv);
		kv->deleteThis();
	}

	for (int i = 0; i < INTERP_VALUES; i++)
	{
		kv = new KeyValues("UserData", "value", interpValues[i]);
		m_pInterpDurationList->AddItem(interpTexts[i], kv);
		kv->deleteThis();
	}

	for (int i = 0; i < SMOOTH_VALUES; i++)
	{
		kv = new KeyValues("UserData", "value", smoothValues[i]);
		m_pSmoothDurationList->AddItem(smoothTexts[i], kv);
		kv->deleteThis();
	}
}

CIOSOptionsPanel::~CIOSOptionsPanel()
{
}

void CIOSOptionsPanel::ApplySchemeSettings( IScheme *pScheme )
{
	m_pScheme = pScheme;
	BaseClass::ApplySchemeSettings( pScheme );

	SetTitle("PLAYER SETTINGS", false);
	SetProportional(false);
	SetSizeable(false);
	SetBounds(0, 0, 480, 300);
	SetBgColor(Color(0, 0, 0, 255));
	SetPaintBackgroundEnabled(true);
	MoveToCenterOfScreen();

	m_pContent->SetBounds(PADDING, PADDING + TOP_PADDING, GetWide() - 2 * PADDING, GetTall() - 2 * PADDING - TOP_PADDING);

	m_pPlayerNameLabel->SetBounds(0, 0, LABEL_WIDTH, TEXT_HEIGHT);
	m_pPlayerNameText->SetBounds(LABEL_WIDTH, 0, INPUT_WIDTH, TEXT_HEIGHT);
	//m_pNameText->SetEditable(true);
	//m_pNameText->SetEnabled(true);
	m_pPlayerNameText->AddActionSignalTarget( this );
	m_pPlayerNameText->SendNewLine(true); // with the txtEntry Type you need to set it to pass the return key as a message
	//m_pPlayerNameText->SetFgColor(Color(0, 0, 0, 255));

	m_pClubNameLabel->SetBounds(0, TEXT_HEIGHT, LABEL_WIDTH, TEXT_HEIGHT);
	m_pClubNameText->SetBounds(LABEL_WIDTH, TEXT_HEIGHT, INPUT_WIDTH, TEXT_HEIGHT);
	//m_pClubNameText->SetFgColor(Color(0, 0, 0, 255));

	m_pCountryNameLabel->SetBounds(0, 2 * TEXT_HEIGHT, LABEL_WIDTH, TEXT_HEIGHT);
	//m_pCountryNameLabel->SetVisible(false);
	m_pCountryNameList->SetBounds(LABEL_WIDTH, 2 * TEXT_HEIGHT, INPUT_WIDTH, TEXT_HEIGHT);
	//m_pCountryNameList->GetMenu()->AddActionSignalTarget(this);
	m_pCountryNameList->GetMenu()->MakeReadyForUse();
	//m_pCountryNameList->SetVisible(false);
	//m_pCountryNameList->GetMenu()->SetFgColor(Color(0, 0, 0, 255));
	//m_pCountryNameList->GetMenu()->SetBgColor(Color(255, 255, 255, 255));
	//m_pCountryNameList->SetSelectionUnfocusedBgColor(Color(255, 0, 0, 255));
	//m_pCountryNameList->SetSelectionBgColor(Color(255, 255, 0, 255));
	//m_pCountryNameList->SetFgColor(Color(0, 0, 0, 255));
	//m_pCountryNameList->SetBgColor(Color(255, 255, 255, 255));
	//m_pCountryNameList->SetDisabledBgColor(Color(255, 255, 255, 255));
	//m_pCountryNameList->SetSelectionTextColor(Color(0, 0, 0, 255));

	m_pPreferredShirtNumberLabel->SetBounds(0, 3 * TEXT_HEIGHT, LABEL_WIDTH, TEXT_HEIGHT);
	m_pPreferredShirtNumberList->SetBounds(LABEL_WIDTH, 3 * TEXT_HEIGHT, INPUT_WIDTH, TEXT_HEIGHT);
	m_pPreferredShirtNumberList->GetMenu()->MakeReadyForUse();

	m_pInterpDurationLabel->SetBounds(0, 4 * TEXT_HEIGHT, LABEL_WIDTH, TEXT_HEIGHT);
	m_pInterpDurationList->SetBounds(LABEL_WIDTH, 4 * TEXT_HEIGHT, INPUT_WIDTH, TEXT_HEIGHT);

	m_pSmoothDurationLabel->SetBounds(0, 5 * TEXT_HEIGHT, LABEL_WIDTH, TEXT_HEIGHT);
	m_pSmoothDurationList->SetBounds(LABEL_WIDTH, 5 * TEXT_HEIGHT, INPUT_WIDTH, TEXT_HEIGHT);

	m_pOKButton->SetBounds(m_pContent->GetWide() - 3 * BUTTON_WIDTH - 2 * BUTTON_MARGIN, m_pContent->GetTall() - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT);
	m_pOKButton->SetCommand("save_and_close");
	m_pOKButton->AddActionSignalTarget(this);

	m_pCancelButton->SetBounds(m_pContent->GetWide() - 2 * BUTTON_WIDTH - BUTTON_MARGIN, m_pContent->GetTall() - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT);
	m_pCancelButton->SetCommand("close");
	m_pCancelButton->AddActionSignalTarget(this);

	m_pSaveButton->SetBounds(m_pContent->GetWide() - BUTTON_WIDTH, m_pContent->GetTall() - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT);
	m_pSaveButton->SetCommand("save_settings");
	m_pSaveButton->AddActionSignalTarget(this);
	//m_pSaveButton->SetDefaultColor(Color(0, 0, 0, 255), Color(200, 200, 200, 255));
	//m_pSaveButton->SetArmedColor(Color(50, 50, 50, 255), Color(150, 150, 150, 255));
	//m_pSaveButton->SetDepressedColor(Color(100, 100, 100, 255), Color(200, 200, 200, 255));
	//m_pSaveButton->SetCursor(dc_hand);
	//m_pSaveButton->SetFont(m_pScheme->GetFont("IOSTeamMenuNormal"));
	//m_pSaveButton->SetContentAlignment(Label::a_center);
}

void CIOSOptionsPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CIOSOptionsPanel::OnThink()
{
	BaseClass::OnThink();

	//SetTall((int)(gpGlobals->curtime * 100) % 100);
	//m_pSettingsPanel->Update();

	C_SDKPlayer *pLocal = C_SDKPlayer::GetLocalSDKPlayer();

	if (pLocal)
	{
		if (gpGlobals->curtime < pLocal->m_flNextClientSettingsChangeTime)
		{
			char *text = VarArgs("Wait %d seconds", (int)(pLocal->m_flNextClientSettingsChangeTime - gpGlobals->curtime));
			m_pOKButton->SetText(text);
			m_pOKButton->SetEnabled(false);
			m_pSaveButton->SetText(text);
			m_pSaveButton->SetEnabled(false);
			return;
		}
	}

	m_pOKButton->SetText("OK");
	m_pOKButton->SetEnabled(true);
	m_pSaveButton->SetText("Apply");
	m_pSaveButton->SetEnabled(true);
}

void CIOSOptionsPanel::OnCommand(const char *cmd)
{
	if (!stricmp(cmd, "save_settings") || !stricmp(cmd, "save_and_close"))
	{
		char text[64];
		m_pPlayerNameText->GetText(text, sizeof(text));
		g_pCVar->FindVar("playername")->SetValue(text);
		m_pClubNameText->GetText(text, sizeof(text));
		g_pCVar->FindVar("clubname")->SetValue(text);
		g_pCVar->FindVar("fallbackcountryindex")->SetValue(m_pCountryNameList->GetActiveItemUserData()->GetInt("index"));
		m_pPreferredShirtNumberList->GetText(text, sizeof(text));
		g_pCVar->FindVar("preferredshirtnumber")->SetValue(atoi(text));

		g_pCVar->FindVar("cl_interp_ratio")->SetValue(atoi(m_pInterpDurationList->GetActiveItemUserData()->GetString("value")));
		g_pCVar->FindVar("cl_smoothtime")->SetValue(atoi(m_pSmoothDurationList->GetActiveItemUserData()->GetString("value")) / 100.0f);

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

	if (Q_strlen(g_pCVar->FindVar("playername")->GetString()) == 0)
		g_pCVar->FindVar("playername")->SetValue(g_pCVar->FindVar("name")->GetString());

	m_pPlayerNameText->SetText(g_pCVar->FindVar("playername")->GetString());
	m_pCountryNameList->SetText(g_szCountryNames[clamp(g_pCVar->FindVar("fallbackcountryindex")->GetInt(), 0, COUNTRY_NAMES_COUNT)]);
	m_pClubNameText->SetText(g_pCVar->FindVar("clubname")->GetString());
	int shirtNum = g_pCVar->FindVar("preferredshirtnumber")->GetInt();
	m_pPreferredShirtNumberList->SetText(shirtNum == 0 ? "<None>" : VarArgs("%d", clamp(shirtNum, 2, 11)));

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
}