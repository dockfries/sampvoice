#pragma once

// Unified SA:MP API wrapper for sampvoice client.
// Provides version-independent access to sampapi types.
//
// Include path order resolves version-specific headers:
//   1. lib/sampapi/include/sampapi/{version}  → unqualified includes
//   2. lib/sampapi/include                     → common sampapi headers
//
// SAMPVOICE_NS compile-time define selects the right namespace
// (e.g. v037r1, v037r3, v037r5, v03dl).
//
// Singleton accessors provided by sampapi:
//   sv::RefNetGame()  sv::RefChat()  sv::RefInputBox()
//   sv::RefScoreboard()  sv::RefGame()  sv::RefCamera()

#include <sampapi/sampapi.h>

#include <CNetGame.h>
#include <CPlayerPool.h>
#include <CVehiclePool.h>
#include <CObjectPool.h>

#include <CLocalPlayer.h>
#include <CRemotePlayer.h>
#include <CPlayerInfo.h>

#include <CPed.h>
#include <CVehicle.h>
#include <CObject.h>

#include <CCamera.h>
#include <CEntity.h>

#include <CChat.h>
#include <CInput.h>
#include <CScoreboard.h>
#include <CGame.h>

#include <sampapi/CMatrix.h>
#include <sampapi/CVector.h>
#include <sampapi/CRect.h>

namespace sv = sampapi::SAMPVOICE_NS;

// Common sampapi helpers (GetAddress is in sampapi::, not in versioned namespaces)
inline unsigned long GetBase() { return sampapi::GetBase(); }
inline unsigned long GetAddress(long offset) { return sampapi::GetAddress(offset); }
