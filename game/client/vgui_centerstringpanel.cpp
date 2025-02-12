//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <stdarg.h>
#include "vguicenterprint.h"
#include "ivrenderview.h"
#include <vgui/IVgui.h>
#include "VguiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar		cl_centermessagetime( "cl_centermessagetime", "5" );

//-----------------------------------------------------------------------------
// Purpose: Implements Center String printing
//-----------------------------------------------------------------------------
class CCenterStringLabel : public vgui::Label
{
	DECLARE_CLASS_SIMPLE( CCenterStringLabel, vgui::Label );

public:
						CCenterStringLabel( vgui::VPANEL parent );
	virtual				~CCenterStringLabel( void );

	// vgui::Panel
	virtual void		ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void		OnTick( void );
	virtual bool		ShouldDraw( void );

	// CVGuiCenterPrint
	virtual void		SetTextColor( int r, int g, int b, int a );
	virtual void		Print( char *text );
	virtual void		Print( wchar_t *text );
	virtual void		ColorPrint( int r, int g, int b, int a, char *text );
	virtual void		ColorPrint( int r, int g, int b, int a, wchar_t *text );
	virtual void		Clear( void );

protected:
	MESSAGE_FUNC_INT_INT( OnScreenSizeChanged, "OnScreenSizeChanged", oldwide, oldtall );

private:
	void ComputeSize( void );

	vgui::HFont			m_hFont;

	float				m_flCentertimeStart;
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *parent - 
//-----------------------------------------------------------------------------
CCenterStringLabel::CCenterStringLabel( vgui::VPANEL parent ) : 
	BaseClass( NULL, "CCenterStringLabel", " " )
{
	SetScheme("ClientScheme");

	SetParent( parent );
	ComputeSize();
	SetVisible( false );
	SetCursor( null );
	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );
	SetContentAlignment( vgui::Label::a_center );

	m_hFont = 0;
	SetFgColor( Color( 255, 255, 255, 255 ) );

	SetPaintBackgroundEnabled( false );

	m_flCentertimeStart = -1;

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CCenterStringLabel::~CCenterStringLabel( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Updates panel to handle the new screen size
//-----------------------------------------------------------------------------
void CCenterStringLabel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);
	ComputeSize();
}

//-----------------------------------------------------------------------------
// Purpose: Computes panel's desired size and position
//-----------------------------------------------------------------------------
void CCenterStringLabel::ComputeSize( void )
{
	int w, h;
	w = ScreenWidth();
	h = ScreenHeight();

	int iHeight = (int)(h * 0.3);

	SetSize( w, iHeight );
	SetPos( 0, ( h * 0.1f ) - ( iHeight / 2 ) );
}

void CCenterStringLabel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// Use a large font
	m_hFont = pScheme->GetFont( "IOSCenterMessage" );
	assert( m_hFont );
	SetFont( m_hFont );

	int w, h;
	w = ScreenWidth();
	h = ScreenHeight();
	int iHeight = (int)(h * 0.3);
	SetSize( w, iHeight );
	SetPos( 0, ( h * 0.1f ) - ( iHeight / 2 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : r - 
//			g - 
//			b - 
//			a - 
//-----------------------------------------------------------------------------
void CCenterStringLabel::SetTextColor( int r, int g, int b, int a )
{
	SetFgColor( Color( r, g, b, a ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCenterStringLabel::Print( char *text )
{
	SetText( text );
	
	m_flCentertimeStart = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCenterStringLabel::Print( wchar_t *text )
{
	SetText( text );
	
	m_flCentertimeStart = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCenterStringLabel::ColorPrint( int r, int g, int b, int a, char *text )
{
	SetTextColor( r, g, b, a );
	Print( text );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCenterStringLabel::ColorPrint( int r, int g, int b, int a, wchar_t *text )
{
	SetTextColor( r, g, b, a );
	Print( text );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCenterStringLabel::Clear( void )
{
	m_flCentertimeStart = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCenterStringLabel::OnTick( void )
{
	SetVisible( ShouldDraw() );

	if (m_flCentertimeStart != -1)
	{
		if (gpGlobals->curtime < m_flCentertimeStart + 0.5f)
		{
			SetAlpha((gpGlobals->curtime - m_flCentertimeStart) / 0.5f * 255);
		}
		else if (gpGlobals->curtime < m_flCentertimeStart + cl_centermessagetime.GetFloat() - 0.5f)
		{
			SetAlpha(255);
		}
		else
		{
			SetAlpha(1 - ((gpGlobals->curtime - (m_flCentertimeStart + cl_centermessagetime.GetFloat())) / 0.5f) * 255);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
// FIXME, this has dependencies on the engine that should go away
//-----------------------------------------------------------------------------
bool CCenterStringLabel::ShouldDraw( void )
{
	if ( engine->IsDrawingLoadingImage() )
	{
		return false;
	}

	if ( m_flCentertimeStart == -1 || gpGlobals->curtime >= m_flCentertimeStart + cl_centermessagetime.GetFloat() )
	{
		m_flCentertimeStart = -1;

		// not time to turn off the message yet
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
CCenterPrint::CCenterPrint( void )
{
	vguiCenterString = NULL;
}

void CCenterPrint::SetTextColor( int r, int g, int b, int a )
{
	if ( vguiCenterString )
	{
		vguiCenterString->SetTextColor( r, g, b, a );
	}
}

void CCenterPrint::Print( char *text )
{
	if ( vguiCenterString )
	{
		vguiCenterString->ColorPrint( 255, 255, 255, 255, text );
	}
}

void CCenterPrint::Print( wchar_t *text )
{
	if ( vguiCenterString )
	{
		vguiCenterString->ColorPrint( 255, 255, 255, 255, text );
	}
}

void CCenterPrint::ColorPrint( int r, int g, int b, int a, char *text )
{
	if ( vguiCenterString )
	{
		vguiCenterString->ColorPrint( r, g, b, a, text );
	}
}

void CCenterPrint::ColorPrint( int r, int g, int b, int a, wchar_t *text )
{
	if ( vguiCenterString )
	{
		vguiCenterString->ColorPrint( r, g, b, a, text );
	}
}

void CCenterPrint::Clear( void )
{
	if ( vguiCenterString )
	{
		vguiCenterString->Clear();
	}
}

void CCenterPrint::Create( vgui::VPANEL parent )
{
	if ( vguiCenterString )
	{
		Destroy();
	}

	vguiCenterString = new CCenterStringLabel( parent );
}

void CCenterPrint::Destroy( void )
{
	if ( vguiCenterString )
	{
		vguiCenterString->SetParent( (vgui::Panel *)NULL );
		delete vguiCenterString;
		vguiCenterString = NULL;
	}
}

static CCenterPrint g_CenterString;
CCenterPrint *internalCenterPrint = &g_CenterString;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CCenterPrint, ICenterPrint, VCENTERPRINT_INTERFACE_VERSION, g_CenterString );