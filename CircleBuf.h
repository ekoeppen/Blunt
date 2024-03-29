#ifndef __CIRCLEBUF_H
#define __CIRCLEBUF_H

#include <BufferList.h>
#include <BufferSegment.h>

#define kBufferUnderrun 2
#define kBufferOverrun 3

class TCircleBuf
{
public:
	TCircleBuf();
	~TCircleBuf(void);
	
	Long BufferCountToNextMarker (ULong *);
	Long FlushBytes ();
	Long FlushToNextMarker (ULong *);
	Long Reset ();
	Long ResetStart ();
	Long CopyOut (CBufferList *, ULong *, ULong *);
	Long CopyOut (UByte *, ULong *, ULong *);
	Long CopyIn (CBufferList *, ULong *);
	Long CopyIn (UByte *, ULong *, Boolean, ULong);
	Long GetNextByte (UByte *);
	Long Allocate (ULong);
	Long Allocate (ULong, int, UChar, UChar);
	Long GetNextByte (UByte *, ULong *);
	Long PeekNextByte (UByte *);
	Long PeekNextByte (UByte *, ULong *);
	Long PutNextByte (UByte);
	Long PutNextByte (UByte, ULong *);
	Long PutEOM (ULong);
	Long PutNextStart ();
	Long PutFirstPossible (UByte);
	Long PutNextPossible (UByte);
	Long PutNextEOM (ULong);
	Long PutNextCommit (UByte);
	Long GetBytes (TCircleBuf *);
	Long DMABufInfo (ULong *, ULong *, UByte *, UByte *);
	Long DMAGetInfo (ULong *);
	Long DMAGetUpdate (ULong);
	Long DMAPutInfo (ULong *, ULong *);
	Long DMAPutUpdate (ULong, UByte, ULong);
	Long UpdateStart ();
	Long UpdateEnd ();
	Long Deallocate ();
	Long GetAlignLong ();
	Long PutAlignLong ();
	Long GetEOMMark (ULong *);
	Long PutEOMMark (ULong, ULong);
	Long PeekNextEOMIndex ();
	Long PeekNextEOMIndex (ULong *);
	Long BufferSpace ();
	Long MarkerSpace ();
	Long MarkerCount ();
	Long BufferSpace (ULong);
	Long BufferCount ();
	Long UpdateStart (ULong);
	Long UpdateEnd (ULong);
	UByte PeekByte (ULong offset);
	
public:
	ULong fSize;									/* 0000 */
	UByte *fBuffer;									/* 0004 */
	ULong fStart;									/* 0008 */
	ULong fNext;									/* 000c */
	
	ULong filler_0010;
	
	UByte filler_0014;
	UByte filler_0015;
	
	ULong filler_0018;
	
	ULong fCount;									/* 001c */
	
	ULong filler_0020;
	ULong filler_0024;
};

class TExtendedCircleBuf: public TCircleBuf
{
public:
	Long PeekNextBytes (UByte*, ULong);
	UByte PeekByte (ULong offset);
};

#endif
