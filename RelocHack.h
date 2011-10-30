// =========== Header ===========
// File:				RelocHack.h
// Project:				(Library)
// Written by:			Paul Guyot (pguyot@kallisys.net)
//
// Created on:			06/29/2001
// Internal version:	1
//
// Copyright (c) 2001, Paul Guyot.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// * Redistributions of source code must retain the above copyright
//   notice and the following disclaimer.
// * Redistributions in binary form must reproduce the above
//   copyright notice and the following disclaimer in the
//   documentation and/or other materials provided with the
//   distribution.
// * Neither the name of Kallisys nor the names of its contributors
//   may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
// HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ===========

// =========== Change History ===========
// 06/29/2001	v1	[PG]	Creation of the file
// ===========

#ifndef __RELOCHACK__
#define __RELOCHACK__

#ifndef __NEWTON_H
	#include <Newton.h>
#endif

// ----------------	//
// RelocVTableHack	//
// ----------------	//

// This is a big hack to fix the problem of copy of code (to avoid page fault mechanism) and virtual functions.
// Cf the technote on the subject (Virtual functions and page fault mechanism)

// Prototype for the function:

typedef void (*VTableFuncPtr)( void );

// Umm. This is what I would like to code:
// typedef void (*RelocVTableHackFuncPtr)( ULong, RelocVTableHackFuncPtr, VTableFuncPtr );
// extern void RelocVTableHack( ULong inObject, RelocVTableHackFuncPtr inRelocVTableHackPtr, VTableFuncPtr inVTablePtr );

extern void RelocVTableHack( ULong inObject, ULong inRelocVTableHackPtr, VTableFuncPtr inVTablePtr );

// You can use this macro for your convenience (this way you're sure that the parameters will be passed in the correct order)

#define RelocVTable( inVTablePtr )	RelocVTableHack( (ULong) this, (ULong) &RelocVTableHack, inVTablePtr )

// ----------------	//
// RelocFuncPtrHack	//
// ----------------	//

// This is another hack, although less dirty :)
// It works like the previous one and is useful if you need to pass a function pointer to the copy of the function.

extern ULong RelocFuncPtrHack( ULong inRelocFuncPtrHack, ULong inFuncPtr );

// You can use a template in this case, too (beware, you'll have to cast the result to a func pointer)

#define RelocFuncPtr( inFuncPtr )	RelocFuncPtrHack( (ULong) &RelocFuncPtrHack, (ULong) inFuncPtr );

#endif
		// __RELOCHACK__

// ====================================================================== //
// Thus spake the master programmer:                                      //
//         "Without the wind, the grass does not move.  Without software, //
//         hardware is useless."                                          //
//                 -- Geoffrey James, "The Tao of Programming"            //
// ====================================================================== //