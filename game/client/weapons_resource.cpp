//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapons Resource implementation
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "c_baseplayer.h"
#include "hud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

WeaponsResource gWR;

void FreeHudTextureList( CUtlDict< CHudTexture *, int >& list );

static CHudTexture *FindHudTextureInDict( CUtlDict< CHudTexture *, int >& list, const char *psz )
{
	int idx = list.Find( psz );
	if ( idx == list.InvalidIndex() )
		return NULL;

	return list[ idx ];
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
WeaponsResource::WeaponsResource( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
WeaponsResource::~WeaponsResource( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void WeaponsResource::Init( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WeaponsResource::Reset( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Load all the sprites needed for all registered weapons
//-----------------------------------------------------------------------------
void WeaponsResource::LoadAllWeaponSprites( void )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if ( player->GetWeapon(i) )
		{
			LoadWeaponSprites( player->GetWeapon(i)->GetWeaponFileInfoHandle() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WeaponsResource::LoadWeaponSprites( WEAPON_FILE_INFO_HANDLE hWeaponFileInfo )
{
	// WeaponsResource is a friend of C_BaseCombatWeapon
	FileWeaponInfo_t *pWeaponInfo = GetFileWeaponInfoFromHandle( hWeaponFileInfo );

	if ( !pWeaponInfo )
		return;

	// Already parsed the hud elements?
	if ( pWeaponInfo->bLoadedHudElements )
		return;

	pWeaponInfo->bLoadedHudElements = true;

	pWeaponInfo->iconActive = NULL;
	pWeaponInfo->iconInactive = NULL;
	pWeaponInfo->iconAmmo = NULL;
	pWeaponInfo->iconAmmo2 = NULL;
	pWeaponInfo->iconCrosshair = NULL;
	pWeaponInfo->iconAutoaim = NULL;
	pWeaponInfo->iconSmall = NULL;

	char sz[128];
	Q_snprintf(sz, sizeof( sz ), "scripts/%s", pWeaponInfo->szClassName);

	CUtlDict< CHudTexture *, int > tempList;

	LoadHudTextures( tempList, sz, g_pGameRules->GetEncryptionKey() );

	if ( !tempList.Count() )
	{
		// no sprite description file for weapon, use default small blocks
		pWeaponInfo->iconActive = gHUD.GetIcon( "selection" );
		pWeaponInfo->iconInactive = gHUD.GetIcon( "selection" );
		pWeaponInfo->iconAmmo = gHUD.GetIcon( "bucket1" );
		return;
	}

	CHudTexture *p;
	p = FindHudTextureInDict( tempList, "crosshair" );
	if ( p )
	{
		pWeaponInfo->iconCrosshair = gHUD.AddUnsearchableHudIconToList( *p );
	}

	p = FindHudTextureInDict( tempList, "autoaim" );
	if ( p )
	{
		pWeaponInfo->iconAutoaim = gHUD.AddUnsearchableHudIconToList( *p );
	}

	p = FindHudTextureInDict( tempList, "zoom" );
	if ( p )
	{
		pWeaponInfo->iconZoomedCrosshair = gHUD.AddUnsearchableHudIconToList( *p );
	}
	else
	{
		pWeaponInfo->iconZoomedCrosshair = pWeaponInfo->iconCrosshair; //default to non-zoomed crosshair
	}

	p = FindHudTextureInDict( tempList, "zoom_autoaim" );
	if ( p )
	{
		pWeaponInfo->iconZoomedAutoaim = gHUD.AddUnsearchableHudIconToList( *p );
	}
	else
	{
		pWeaponInfo->iconZoomedAutoaim = pWeaponInfo->iconZoomedCrosshair;  //default to zoomed crosshair
	}

	FreeHudTextureList( tempList );
}

//-----------------------------------------------------------------------------
// Purpose: Helper function to return a Ammo pointer from id
//-----------------------------------------------------------------------------
CHudTexture *WeaponsResource::GetAmmoIconFromWeapon( int iAmmoId )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return NULL;

	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		C_BaseCombatWeapon *weapon = player->GetWeapon( i );
		if ( !weapon )
			continue;

		if ( weapon->GetPrimaryAmmoType() == iAmmoId )
		{
			return weapon->GetWpnData().iconAmmo;
		}
		else if ( weapon->GetSecondaryAmmoType() == iAmmoId )
		{
			return weapon->GetWpnData().iconAmmo2;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to a weapon using this ammo
//-----------------------------------------------------------------------------
const FileWeaponInfo_t *WeaponsResource::GetWeaponFromAmmo( int iAmmoId )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return NULL;

	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		C_BaseCombatWeapon *weapon = player->GetWeapon( i );
		if ( !weapon )
			continue;

		if ( weapon->GetPrimaryAmmoType() == iAmmoId )
		{
			return &weapon->GetWpnData();
		}
		else if ( weapon->GetSecondaryAmmoType() == iAmmoId )
		{
			return &weapon->GetWpnData();
		}
	}

	return NULL;
}

