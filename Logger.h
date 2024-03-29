#ifndef __LOGGER_H
#define __LOGGER_H

void hammer_log (int desiredLevel, int currentLevel, char *format, ...);

class TSerialChip;
class TCircleBuf;
class TULockingSemaphore;

// ================================================================================
// � Logger
// ================================================================================

class Logger
{
public:
	ULong 					fLogLevel;
	TULockingSemaphore*		fSemaphore;
	TSerialChip* 			fChip;
	
	Boolean					fBufferOutput;
	
	UByte*					fBuffer;
	ULong					fHead;
	ULong					fTail;

	ULong					fLocation;
	ULong					fSpeed;
	
	NewtonErr				Initialize ();
	long					Main ();
	void					Close ();
	
	void 					TxBEmptyIntHandler (void);
	void 					ExtStsIntHandler (void);
	void 					RxCAvailIntHandler (void);
	void 					RxCSpecialIntHandler (void);
	
	void					BufferOutput (Boolean buffer);
	void					StartOutput ();
	void 					Output (UByte*, ULong);
};

#endif
