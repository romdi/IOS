//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements shared baseplayer class functionality
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "movevars_shared.h"
#include "util_shared.h"
#include "datacache/imdlcache.h"

#if defined( CLIENT_DLL )

	#include "iclientvehicle.h"
	#include "prediction.h"
	#include "c_basedoor.h"
	#include "c_world.h"
	#include "view.h"
	#include "ios_camera.h"

	#define CRecipientFilter C_RecipientFilter

#else

	#include "iservervehicle.h"
	#include "trains.h"
	#include "world.h"
	#include "doors.h"
	#include "ai_basenpc.h"
	#include "env_zoom.h"

	extern int TrainSpeed(int iSpeed, int iMax);
	
#endif

#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "tier0/vprof.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "decals.h"
#include "obstacle_pushaway.h"
#include "sdk_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined(GAME_DLL) && !defined(_XBOX)
	extern ConVar sv_pushaway_max_force;
	extern ConVar sv_pushaway_force;
	extern ConVar sv_turbophysics;

	class CUsePushFilter : public CTraceFilterEntitiesOnly
	{
	public:
		bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
		{
			CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

			// Static prop case...
			if ( !pEntity )
				return false;

			// Only impact on physics objects
			if ( !pEntity->VPhysicsGetObject() )
				return false;

			return g_pGameRules->CanEntityBeUsePushed( pEntity );
		}
	};
#endif

#ifdef CLIENT_DLL
ConVar mp_usehwmmodels( "mp_usehwmmodels", "0", NULL, "Enable the use of the hw morph models. (-1 = never, 1 = always, 0 = based upon GPU)" ); // -1 = never, 0 = if hasfastvertextextures, 1 = always
#endif

bool UseHWMorphModels()
{
#ifdef CLIENT_DLL 
	if ( mp_usehwmmodels.GetInt() == 0 )
		return g_pMaterialSystemHardwareConfig->HasFastVertexTextures();

	return mp_usehwmmodels.GetInt() > 0;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBasePlayer::GetTimeBase( void ) const
{
	return m_nTickBase * TICK_INTERVAL;
}

//-----------------------------------------------------------------------------
// Eye angles
//-----------------------------------------------------------------------------
const QAngle &CBasePlayer::EyeAngles( )
{
	// NOTE: Viewangles are measured *relative* to the parent's coordinate system
	CBaseEntity *pMoveParent = const_cast<CBasePlayer*>(this)->GetMoveParent();

	if ( !pMoveParent )
	{
		return pl.v_angle;
	}

	// FIXME: Cache off the angles?
	matrix3x4_t eyesToParent, eyesToWorld;
	AngleMatrix( pl.v_angle, eyesToParent );
	ConcatTransforms( pMoveParent->EntityToWorldTransform(), eyesToParent, eyesToWorld );

	static QAngle angEyeWorld;
	MatrixAngles( eyesToWorld, angEyeWorld );
	return angEyeWorld;
}


const QAngle &CBasePlayer::LocalEyeAngles()
{
	return pl.v_angle;
}

//-----------------------------------------------------------------------------
// Actual Eye position + angles
//-----------------------------------------------------------------------------
Vector CBasePlayer::EyePosition( )
{
	return BaseClass::EyePosition();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CBasePlayer::GetPlayerMins( void ) const
{
	return VEC_HULL_MIN;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CBasePlayer::GetPlayerMaxs( void ) const
{	
	return VEC_HULL_MAX;
}

//-----------------------------------------------------------------------------
// Returns eye vectors
//-----------------------------------------------------------------------------
void CBasePlayer::EyeVectors( Vector *pForward, Vector *pRight, Vector *pUp )
{
	AngleVectors( EyeAngles(), pForward, pRight, pUp );
}

//-----------------------------------------------------------------------------
// Purpose: Returns the eye position and angle vectors.
//-----------------------------------------------------------------------------
void CBasePlayer::EyePositionAndVectors( Vector *pPosition, Vector *pForward,
										 Vector *pRight, Vector *pUp )
{
	VectorCopy( BaseClass::EyePosition(), *pPosition );
	AngleVectors( EyeAngles(), pForward, pRight, pUp );
}

#ifdef CLIENT_DLL
surfacedata_t * CBasePlayer::GetFootstepSurface( const Vector &origin, const char *surfaceName )
{
	return physprops->GetSurfaceData( physprops->GetSurfaceIndex( surfaceName ) );
}
#endif

void CBasePlayer::UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity )
{
	bool bWalking;
	float fvol = 1.0f;
	Vector knee;
	Vector feet;
	float height;
	float speed;
	float velrun;
	float velwalk;
	int	fLadder;

	if ( m_flStepSoundTime > 0 )
	{
		m_flStepSoundTime -= 1000.0f * gpGlobals->frametime;
		if ( m_flStepSoundTime < 0 )
		{
			m_flStepSoundTime = 0;
		}
	}

	if ( m_flStepSoundTime > 0 )
		return;

	if ( GetFlags() & (FL_FROZEN|FL_ATCONTROLS))
		return;

	if ( GetMoveType() == MOVETYPE_NOCLIP )
		return;

	if ( !sv_footsteps.GetFloat() )
		return;

	speed = VectorLength( vecVelocity );
	float groundspeed = Vector2DLength( vecVelocity.AsVector2D() );

	// determine if we are on a ladder
	fLadder = false;

	GetStepSoundVelocities( &velwalk, &velrun );

	bool onground = ( GetFlags() & FL_ONGROUND );
	bool movingalongground = ( groundspeed > 0.0001f );
	bool moving_fast_enough =  ( speed >= velwalk );

	// To hear step sounds you must be either on a ladder or moving along the ground AND
	// You must be moving fast enough

	if ( !moving_fast_enough || !( onground && movingalongground ) )
			return;

//	MoveHelper()->PlayerSetAnimation( PLAYER_WALK );

	bWalking = speed < velrun;		

	VectorCopy( vecOrigin, knee );
	VectorCopy( vecOrigin, feet );

	height = GetPlayerMaxs()[ 2 ] - GetPlayerMins()[ 2 ];

	knee[2] = vecOrigin[2] + 0.2 * height;

	if ( !psurface )
		return;

	SetStepSoundTime( STEPSOUNDTIME_NORMAL, bWalking );

#ifdef CLIENT_DLL
	if (this == GetLocalPlayer())
	{
		fvol = 0.1f;
	}
#endif

	PlayStepSound( feet, psurface, fvol, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : step - 
//			fvol - 
//			force - force sound to play
//-----------------------------------------------------------------------------
void CBasePlayer::PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force )
{
	if ( gpGlobals->maxClients > 1 && !sv_footsteps.GetFloat() )
		return;

#if defined( CLIENT_DLL )
	// during prediction play footstep sounds only once
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
#endif

	if ( !psurface )
		return;

	int nSide = m_Local.m_nStepside;
	unsigned short stepSoundName = nSide ? psurface->sounds.stepleft : psurface->sounds.stepright;
	if ( !stepSoundName )
		return;

	m_Local.m_nStepside = !nSide;

	CSoundParameters params;

	Assert( nSide == 0 || nSide == 1 );

	if ( m_StepSoundCache[ nSide ].m_usSoundNameIndex == stepSoundName )
	{
		params = m_StepSoundCache[ nSide ].m_SoundParameters;
	}
	else
	{
		IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
		const char *pSoundName = physprops->GetString( stepSoundName );
		if ( !CBaseEntity::GetParametersForSound( pSoundName, params, NULL ) )
			return;

		// Only cache if there's one option.  Otherwise we'd never here any other sounds
		if ( params.count == 1 )
		{
			m_StepSoundCache[ nSide ].m_usSoundNameIndex = stepSoundName;
			m_StepSoundCache[ nSide ].m_SoundParameters = params;
		}
	}

	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

#ifndef CLIENT_DLL
	// in MP, server removes all players in the vecOrigin's PVS, these players generate the footsteps client side
	if ( gpGlobals->maxClients > 1 )
	{
		filter.RemoveRecipientsByPVS( vecOrigin );
	}
#endif

	EmitSound_t ep;
	ep.m_nChannel = CHAN_BODY;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

void CBasePlayer::UpdateButtonState( int nUserCmdButtonMask )
{
	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = m_nButtons;

	// Get button states
	m_nButtons = nUserCmdButtonMask;
 	int buttonsChanged = m_afButtonLast ^ m_nButtons;
	
	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed =  buttonsChanged & m_nButtons;		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & (~m_nButtons);	// The ones not down are "released"
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::GetStepSoundVelocities( float *velwalk, float *velrun )
{
	*velwalk = 90;
	*velrun = 220;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking )
{
	switch ( iStepSoundTime )
	{
	case STEPSOUNDTIME_NORMAL:
	case STEPSOUNDTIME_WATER_FOOT:
		m_flStepSoundTime = bWalking ? 400 : 300;
		break;

	case STEPSOUNDTIME_ON_LADDER:
		m_flStepSoundTime = 350;
		break;

	case STEPSOUNDTIME_WATER_KNEE:
		m_flStepSoundTime = 600;
		break;

	default:
		Assert(0);
		break;
	}
}

void CBasePlayer::SetAnimationExtension( const char *pExtension )
{
	Q_strncpy( m_szAnimExtension, pExtension, sizeof(m_szAnimExtension) );
}

#if !defined( NO_ENTITY_PREDICTION )
void CBasePlayer::AddToPlayerSimulationList( CBaseEntity *other )
{
	CHandle< CBaseEntity > h;
	h = other;
	// Already in list
	if ( m_SimulatedByThisPlayer.Find( h ) != m_SimulatedByThisPlayer.InvalidIndex() )
		return;

	Assert( other->IsPlayerSimulated() );

	m_SimulatedByThisPlayer.AddToTail( h );
}

//-----------------------------------------------------------------------------
// Purpose: Fixme, this should occur if the player fails to drive simulation
//  often enough!!!
// Input  : *other - 
//-----------------------------------------------------------------------------
void CBasePlayer::RemoveFromPlayerSimulationList( CBaseEntity *other )
{
	if ( !other )
		return;

	Assert( other->IsPlayerSimulated() );
	Assert( other->GetSimulatingPlayer() == this );


	CHandle< CBaseEntity > h;
	h = other;

	m_SimulatedByThisPlayer.FindAndRemove( h );
}

void CBasePlayer::SimulatePlayerSimulatedEntities( void )
{
	int c = m_SimulatedByThisPlayer.Count();
	int i;

	for ( i = c - 1; i >= 0; i-- )
	{
		CHandle< CBaseEntity > h;
		
		h = m_SimulatedByThisPlayer[ i ];
		CBaseEntity *e = h;

		if ( !e || !e->IsPlayerSimulated() )
		{
			m_SimulatedByThisPlayer.Remove( i );
			continue;
		}

#if defined( CLIENT_DLL )
		if ( e->IsClientCreated() && prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		{
			continue;
		}
#endif
		Assert( e->IsPlayerSimulated() );
		Assert( e->GetSimulatingPlayer() == this );

		e->PhysicsSimulate();
	}

	// Loop through all entities again, checking their untouch if flagged to do so
	c = m_SimulatedByThisPlayer.Count();

	for ( i = c - 1; i >= 0; i-- )
	{
		CHandle< CBaseEntity > h;
		
		h = m_SimulatedByThisPlayer[ i ];

		CBaseEntity *e = h;
		if ( !e || !e->IsPlayerSimulated() )
		{
			m_SimulatedByThisPlayer.Remove( i );
			continue;
		}

#if defined( CLIENT_DLL )
		if ( e->IsClientCreated() && prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		{
			continue;
		}
#endif

		Assert( e->IsPlayerSimulated() );
		Assert( e->GetSimulatingPlayer() == this );

		if ( !e->GetCheckUntouch() )
			continue;

		e->PhysicsCheckForEntityUntouch();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ClearPlayerSimulationList( void )
{
	int c = m_SimulatedByThisPlayer.Size();
	int i;

	for ( i = c - 1; i >= 0; i-- )
	{
		CHandle< CBaseEntity > h;
		
		h = m_SimulatedByThisPlayer[ i ];
		CBaseEntity *e = h;
		if ( e )
		{
			e->UnsetPlayerSimulated();
		}
	}

	m_SimulatedByThisPlayer.RemoveAll();
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConVar sv_debug_player_use( "sv_debug_player_use", "0", FCVAR_REPLICATED, "Visualizes +use logic. Green cross=trace success, Red cross=trace too far, Green box=radius success" );
float IntervalDistance( float x, float x0, float x1 )
{
	// swap so x0 < x1
	if ( x0 > x1 )
	{
		float tmp = x0;
		x0 = x1;
		x1 = tmp;
	}

	if ( x < x0 )
		return x0-x;
	else if ( x > x1 )
		return x - x1;
	return 0;
}

CBaseEntity *CBasePlayer::FindUseEntity()
{
	Vector forward, up;
	EyeVectors( &forward, NULL, &up );

	trace_t tr;
	// Search for objects in a sphere (tests for entities that are not solid, yet still useable)
	Vector searchCenter = EyePosition();

	// NOTE: Some debris objects are useable too, so hit those as well
	// A button, etc. can be made out of clip brushes, make sure it's +useable via a traceline, too.
	int useableContents = MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_PLAYERCLIP;

#ifdef CSTRIKE_DLL
	useableContents = MASK_NPCSOLID_BRUSHONLY | MASK_OPAQUE_AND_NPCS;
#endif

#ifdef HL1_DLL
	useableContents = MASK_SOLID;
#endif
#ifndef CLIENT_DLL
	CBaseEntity *pFoundByTrace = NULL;
#endif

	// UNDONE: Might be faster to just fold this range into the sphere query
	CBaseEntity *pObject = NULL;

	float nearestDist = FLT_MAX;
	// try the hit entity if there is one, or the ground entity if there isn't.
	CBaseEntity *pNearest = NULL;

	const int NUM_TANGENTS = 8;
	// trace a box at successive angles down
	//							forward, 45 deg, 30 deg, 20 deg, 15 deg, 10 deg, -10, -15
	const float tangents[NUM_TANGENTS] = { 0, 1, 0.57735026919f, 0.3639702342f, 0.267949192431f, 0.1763269807f, -0.1763269807f, -0.267949192431f };
	for ( int i = 0; i < NUM_TANGENTS; i++ )
	{
		if ( i == 0 )
		{
			UTIL_TraceLine( searchCenter, searchCenter + forward * 1024, useableContents, this, COLLISION_GROUP_NONE, &tr );
		}
		else
		{
			Vector down = forward - tangents[i]*up;
			VectorNormalize(down);
			UTIL_TraceHull( searchCenter, searchCenter + down * 72, -Vector(16,16,16), Vector(16,16,16), useableContents, this, COLLISION_GROUP_NONE, &tr );
		}
		pObject = tr.m_pEnt;

#ifndef CLIENT_DLL
		pFoundByTrace = pObject;
#endif
		bool bUsable = IsUseableEntity(pObject, 0);
		while ( pObject && !bUsable && pObject->GetMoveParent() )
		{
			pObject = pObject->GetMoveParent();
			bUsable = IsUseableEntity(pObject, 0);
		}

		if ( bUsable )
		{
			Vector delta = tr.endpos - tr.startpos;
			float centerZ = CollisionProp()->WorldSpaceCenter().z;
			delta.z = IntervalDistance( tr.endpos.z, centerZ + CollisionProp()->OBBMins().z, centerZ + CollisionProp()->OBBMaxs().z );
			float dist = delta.Length();
			if ( dist < PLAYER_USE_RADIUS )
			{
#ifndef CLIENT_DLL

				if ( sv_debug_player_use.GetBool() )
				{
					NDebugOverlay::Line( searchCenter, tr.endpos, 0, 255, 0, true, 30 );
					NDebugOverlay::Cross3D( tr.endpos, 16, 0, 255, 0, true, 30 );
				}

				if ( pObject->MyNPCPointer() && pObject->MyNPCPointer()->IsPlayerAlly( this ) )
				{
					// If about to select an NPC, do a more thorough check to ensure
					// that we're selecting the right one from a group.
					pObject = DoubleCheckUseNPC( pObject, searchCenter, forward );
				}
#endif
				if ( sv_debug_player_use.GetBool() )
				{
					Msg( "Trace using: %s\n", pObject ? pObject->GetDebugName() : "no usable entity found" );
				}

				pNearest = pObject;
				
				// if this is directly under the cursor just return it now
				if ( i == 0 )
					return pObject;
			}
		}
	}

	// check ground entity first
	// if you've got a useable ground entity, then shrink the cone of this search to 45 degrees
	// otherwise, search out in a 90 degree cone (hemisphere)
	if ( GetGroundEntity() && IsUseableEntity(GetGroundEntity(), FCAP_USE_ONGROUND) )
	{
		pNearest = GetGroundEntity();
	}
	if ( pNearest )
	{
		// estimate nearest object by distance from the view vector
		Vector point;
		pNearest->CollisionProp()->CalcNearestPoint( searchCenter, &point );
		nearestDist = CalcDistanceToLine( point, searchCenter, forward );
		if ( sv_debug_player_use.GetBool() )
		{
			Msg("Trace found %s, dist %.2f\n", pNearest->GetClassname(), nearestDist );
		}
	}

	for ( CEntitySphereQuery sphere( searchCenter, PLAYER_USE_RADIUS ); ( pObject = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pObject )
			continue;

		if ( !IsUseableEntity( pObject, FCAP_USE_IN_RADIUS ) )
			continue;

		// see if it's more roughly in front of the player than previous guess
		Vector point;
		pObject->CollisionProp()->CalcNearestPoint( searchCenter, &point );

		Vector dir = point - searchCenter;
		VectorNormalize(dir);
		float dot = DotProduct( dir, forward );

		// Need to be looking at the object more or less
		if ( dot < 0.8 )
			continue;

		float dist = CalcDistanceToLine( point, searchCenter, forward );

		if ( sv_debug_player_use.GetBool() )
		{
			Msg("Radius found %s, dist %.2f\n", pObject->GetClassname(), dist );
		}

		if ( dist < nearestDist )
		{
			// Since this has purely been a radius search to this point, we now
			// make sure the object isn't behind glass or a grate.
			trace_t trCheckOccluded;
			UTIL_TraceLine( searchCenter, point, useableContents, this, COLLISION_GROUP_NONE, &trCheckOccluded );

			if ( trCheckOccluded.fraction == 1.0 || trCheckOccluded.m_pEnt == pObject )
			{
				pNearest = pObject;
				nearestDist = dist;
			}
		}
	}

#ifndef CLIENT_DLL
	if ( !pNearest )
	{
		// Haven't found anything near the player to use, nor any NPC's at distance.
		// Check to see if the player is trying to select an NPC through a rail, fence, or other 'see-though' volume.
		trace_t trAllies;
		UTIL_TraceLine( searchCenter, searchCenter + forward * PLAYER_USE_RADIUS, MASK_OPAQUE_AND_NPCS, this, COLLISION_GROUP_NONE, &trAllies );

		if ( trAllies.m_pEnt && IsUseableEntity( trAllies.m_pEnt, 0 ) && trAllies.m_pEnt->MyNPCPointer() && trAllies.m_pEnt->MyNPCPointer()->IsPlayerAlly( this ) )
		{
			// This is an NPC, take it!
			pNearest = trAllies.m_pEnt;
		}
	}

	if ( pNearest && pNearest->MyNPCPointer() && pNearest->MyNPCPointer()->IsPlayerAlly( this ) )
	{
		pNearest = DoubleCheckUseNPC( pNearest, searchCenter, forward );
	}

	if ( sv_debug_player_use.GetBool() )
	{
		if ( !pNearest )
		{
			NDebugOverlay::Line( searchCenter, tr.endpos, 255, 0, 0, true, 30 );
			NDebugOverlay::Cross3D( tr.endpos, 16, 255, 0, 0, true, 30 );
		}
		else if ( pNearest == pFoundByTrace )
		{
			NDebugOverlay::Line( searchCenter, tr.endpos, 0, 255, 0, true, 30 );
			NDebugOverlay::Cross3D( tr.endpos, 16, 0, 255, 0, true, 30 );
		}
		else
		{
			NDebugOverlay::Box( pNearest->WorldSpaceCenter(), Vector(-8, -8, -8), Vector(8, 8, 8), 0, 255, 0, true, 30 );
		}
	}
#endif

	if ( sv_debug_player_use.GetBool() )
	{
		Msg( "Radial using: %s\n", pNearest ? pNearest->GetDebugName() : "no usable entity found" );
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
// Purpose: Handles USE keypress
//-----------------------------------------------------------------------------
void CBasePlayer::PlayerUse ( void )
{
#ifdef GAME_DLL
	// Was use pressed or released?
	if ( ! ((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	if ( IsObserver() )
	{
		// do special use operation in oberserver mode
		if ( m_afButtonPressed & IN_USE )
			ObserverUse( true );
		else if ( m_afButtonReleased & IN_USE )
			ObserverUse( false );
		
		return;
	}
#endif
}

ConVar	sv_suppress_viewpunch( "sv_suppress_viewpunch", "0", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ViewPunch( const QAngle &angleOffset )
{
	//See if we're suppressing the view punching
	if ( sv_suppress_viewpunch.GetBool() )
		return;

	// We don't allow view kicks in the vehicle
	if ( IsInAVehicle() )
		return;

	m_Local.m_vecPunchAngleVel += angleOffset * 20;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ViewPunchReset( float tolerance )
{
	if ( tolerance != 0 )
	{
		tolerance *= tolerance;	// square
		float check = m_Local.m_vecPunchAngleVel->LengthSqr() + m_Local.m_vecPunchAngle->LengthSqr();
		if ( check > tolerance )
			return;
	}
	m_Local.m_vecPunchAngle = vec3_angle;
	m_Local.m_vecPunchAngleVel = vec3_angle;
}

#if defined( CLIENT_DLL )

#include "iviewrender.h"
#include "ivieweffects.h"

#endif

static ConVar smoothstairs( "smoothstairs", "1", FCVAR_REPLICATED, "Smooth player eye z coordinate when traversing stairs." );

//-----------------------------------------------------------------------------
// Handle view smoothing when going up or down stairs
//-----------------------------------------------------------------------------
void CBasePlayer::SmoothViewOnStairs( Vector& eyeOrigin )
{
	CBaseEntity *pGroundEntity = GetGroundEntity();
	float flCurrentPlayerZ = GetLocalOrigin().z;
	float flCurrentPlayerViewOffsetZ = GetViewOffset().z;

	// Smooth out stair step ups
	// NOTE: Don't want to do this when the ground entity is moving the player
	if ( ( pGroundEntity != NULL && pGroundEntity->GetMoveType() == MOVETYPE_NONE ) && ( flCurrentPlayerZ != m_flOldPlayerZ ) && smoothstairs.GetBool() &&
		 m_flOldPlayerViewOffsetZ == flCurrentPlayerViewOffsetZ )
	{
		int dir = ( flCurrentPlayerZ > m_flOldPlayerZ ) ? 1 : -1;

		float steptime = gpGlobals->frametime;
		if (steptime < 0)
		{
			steptime = 0;
		}

		m_flOldPlayerZ += steptime * 150 * dir;

		const float stepSize = 18.0f;

		if ( dir > 0 )
		{
			if (m_flOldPlayerZ > flCurrentPlayerZ)
			{
				m_flOldPlayerZ = flCurrentPlayerZ;
			}
			if (flCurrentPlayerZ - m_flOldPlayerZ > stepSize)
			{
				m_flOldPlayerZ = flCurrentPlayerZ - stepSize;
			}
		}
		else
		{
			if (m_flOldPlayerZ < flCurrentPlayerZ)
			{
				m_flOldPlayerZ = flCurrentPlayerZ;
			}
			if (flCurrentPlayerZ - m_flOldPlayerZ < -stepSize)
			{
				m_flOldPlayerZ = flCurrentPlayerZ + stepSize;
			}
		}

		eyeOrigin[2] += m_flOldPlayerZ - flCurrentPlayerZ;
	}
	else
	{
		m_flOldPlayerZ = flCurrentPlayerZ;
		m_flOldPlayerViewOffsetZ = flCurrentPlayerViewOffsetZ;
	}
}

static bool IsWaterContents( int contents )
{
	if ( contents & MASK_WATER )
		return true;

//	if ( contents & CONTENTS_TESTFOGVOLUME )
//		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : eyeOrigin - 
//			eyeAngles - 
//			zNear - 
//			zFar - 
//			fov - 
//-----------------------------------------------------------------------------
void CBasePlayer::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	eyeOrigin = EyePosition();
	eyeAngles = EyeAngles();

#if defined( CLIENT_DLL )
	Camera()->CalcView(eyeOrigin, eyeAngles, fov);
#endif
}


void CBasePlayer::CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles)
{
	for ( int i = 0; i < MAX_VIEWMODELS; i++ )
	{
		CBaseViewModel *vm = GetViewModel( i );
		if ( !vm )
			continue;
	
		vm->CalcViewModelView( this, eyeOrigin, eyeAngles );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Compute roll angle for a particular lateral velocity
// Input  : angles - 
//			velocity - 
//			rollangle - 
//			rollspeed - 
// Output : float CViewRender::CalcRoll
//-----------------------------------------------------------------------------
float CBasePlayer::CalcRoll (const QAngle& angles, const Vector& velocity, float rollangle, float rollspeed)
{
    float   sign;
    float   side;
    float   value;
	
	Vector  forward, right, up;
	
    AngleVectors (angles, &forward, &right, &up);
	
	// Get amount of lateral movement
    side = DotProduct( velocity, right );
	// Right or left side?
    sign = side < 0 ? -1 : 1;
    side = fabs(side);
    
	value = rollangle;
	// Hit 100% of rollangle at rollspeed.  Below that get linear approx.
    if ( side < rollspeed )
	{
		side = side * value / rollspeed;
	}
    else
	{
		side = value;
	}

	// Scale by right/left sign
    return side*sign;
}

//-----------------------------------------------------------------------------
// Purpose: Determine view roll, including data kick
//-----------------------------------------------------------------------------
void CBasePlayer::CalcViewRoll( QAngle& eyeAngles )
{
	if ( GetMoveType() == MOVETYPE_NOCLIP )
		return;

	float side = CalcRoll( GetAbsAngles(), GetAbsVelocity(), sv_rollangle.GetFloat(), sv_rollspeed.GetFloat() );
	eyeAngles[ROLL] += side;
}


void CBasePlayer::DoMuzzleFlash()
{
	for ( int i = 0; i < MAX_VIEWMODELS; i++ )
	{
		CBaseViewModel *vm = GetViewModel( i );
		if ( !vm )
			continue;

		vm->DoMuzzleFlash();
	}

	BaseClass::DoMuzzleFlash();
}


float CBasePlayer::GetFOVDistanceAdjustFactor()
{
	float defaultFOV	= (float)GetDefaultFOV();
	float localFOV		= (float)GetFOV();

	if ( localFOV == defaultFOV || defaultFOV < 0.001f )
	{
		return 1.0f;
	}

	// If FOV is lower, then we're "zoomed" in and this will give a factor < 1 so apparent LOD distances can be
	//  shorted accordingly
	return localFOV / defaultFOV;

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecTracerSrc - 
//			&tr - 
//			iTracerType - 
//-----------------------------------------------------------------------------
void CBasePlayer::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->MakeTracer( vecTracerSrc, tr, iTracerType );
		return;
	}

	BaseClass::MakeTracer( vecTracerSrc, tr, iTracerType );
}


void CBasePlayer::SharedSpawn()
{
	SetMoveType( MOVETYPE_WALK );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetFriction( 1.0f );

	pl.deadflag	= false;
	m_lifeState	= LIFE_ALIVE;
	m_iHealth = 100;
	m_takedamage		= DAMAGE_NO;

	m_Local.m_bDrawViewmodel = true;
	m_Local.m_flStepSize = sv_stepsize.GetFloat();
	m_Local.m_bAllowAutoMovement = true;

	m_nRenderFX = kRenderFxNone;
	m_flNextAttack	= gpGlobals->curtime;
	m_flMaxspeed		= 0.0f;

	MDLCACHE_CRITICAL_SECTION();
	SetSequence( SelectWeightedSequence( ACT_IDLE ) );

	//if ( GetFlags() & FL_DUCKING ) 
	//	SetCollisionBounds( VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX );
	//else
		SetCollisionBounds( VEC_HULL_MIN, VEC_HULL_MAX );

	// dont let uninitialized value here hurt the player
	m_Local.m_flFallVelocity = 0;

	SetBloodColor( BLOOD_COLOR_RED );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CBasePlayer::GetDefaultFOV( void ) const
{
	int iFOV = ( m_iDefaultFOV == 0 ) ? g_pGameRules->DefaultFOV() : m_iDefaultFOV;

	return iFOV;
}

void CBasePlayer::AvoidPhysicsProps( CUserCmd *pCmd )
{
#ifndef _XBOX
	// Don't avoid if noclipping or in movetype none
	switch ( GetMoveType() )
	{
	case MOVETYPE_NOCLIP:
	case MOVETYPE_NONE:
		return;
	default:
		break;
	}

	if ( !IsObserver() )
		return;

	AvoidPushawayProps( this, pCmd );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const char
//-----------------------------------------------------------------------------
const char *CBasePlayer::GetTracerType( void )
{
	if ( GetActiveWeapon() )
	{
		return GetActiveWeapon()->GetTracerType();
	}

	return BaseClass::GetTracerType();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ClearZoomOwner( void )
{
	m_hZoomOwner = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the FOV of the client, doing interpolation between old and new if requested
// Input  : FOV - New FOV
//			zoomRate - Amount of time (in seconds) to move between old and new FOV
//-----------------------------------------------------------------------------
bool CBasePlayer::SetFOV( CBaseEntity *pRequester, int FOV, float zoomRate, int iZoomStart /* = 0 */ )
{
	//NOTENOTE: You MUST specify who is requesting the zoom change
	assert( pRequester != NULL );
	if ( pRequester == NULL )
		return false;

	// If we already have an owner, we only allow requests from that owner
	if ( ( m_hZoomOwner.Get() != NULL ) && ( m_hZoomOwner.Get() != pRequester ) )
	{
#ifdef GAME_DLL
		if ( CanOverrideEnvZoomOwner( m_hZoomOwner.Get() ) == false )
#endif
			return false;
	}
	else
	{
		//FIXME: Maybe do this is as an accessor instead
		if ( FOV == 0 )
		{
			m_hZoomOwner = NULL;
		}
		else
		{
			m_hZoomOwner = pRequester;
		}
	}

	// Setup our FOV and our scaling time

	if ( iZoomStart > 0 )
	{
		m_iFOVStart = iZoomStart;
	}
	else
	{
		m_iFOVStart = GetFOV();
	}

	m_flFOVTime = gpGlobals->curtime;
	m_iFOV = FOV;

	m_Local.m_flFOVRate	= zoomRate;

	return true;
}

void CBasePlayer::SetPreviouslyPredictedOrigin( const Vector &vecAbsOrigin )
{
	m_vecPreviouslyPredictedOrigin = vecAbsOrigin;
}

const Vector &CBasePlayer::GetPreviouslyPredictedOrigin() const
{
	return m_vecPreviouslyPredictedOrigin;
}

bool fogparams_t::operator !=( const fogparams_t& other ) const
{
	if ( this->enable != other.enable ||
		this->blend != other.blend ||
		!VectorsAreEqual(this->dirPrimary, other.dirPrimary, 0.01f ) || 
		this->colorPrimary.Get() != other.colorPrimary.Get() ||
		this->colorSecondary.Get() != other.colorSecondary.Get() ||
		this->start != other.start ||
		this->end != other.end ||
		this->farz != other.farz ||
		this->maxdensity != other.maxdensity ||
		this->colorPrimaryLerpTo.Get() != other.colorPrimaryLerpTo.Get() ||
		this->colorSecondaryLerpTo.Get() != other.colorSecondaryLerpTo.Get() ||
		this->startLerpTo != other.startLerpTo ||
		this->endLerpTo != other.endLerpTo ||
		this->lerptime != other.lerptime ||
		this->duration != other.duration )
		return true;

	return false;
}

void CBasePlayer::IncrementEFNoInterpParity()
{
	// Only matters in multiplayer
	if ( gpGlobals->maxClients == 1 )
		return;
	m_ubEFNoInterpParity = (m_ubEFNoInterpParity + 1) % NOINTERP_PARITY_MAX;
}

int CBasePlayer::GetEFNoInterpParity() const
{
	return (int)m_ubEFNoInterpParity;
}