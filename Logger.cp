#define USE_SYS_WRITE 0

#include <UserTasks.h>
#include <UserSemaphore.h>
#include <SerialChipRegistry.h>
#include <SerialChipV2.h>
#include <HALOptions.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "CircleBuf.h"
#include "Logger.h"


extern "C" _sys_write (int fd, char* data, int len);

void hammer_log (int currentLevel, int desiredLevel, char *format, ... )
{
    va_list args;
    char buffer[128];
    
    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);
    _sys_write (1, buffer, strlen (buffer));
}

#define MAX_LOGGER_OUTPUT 32768

extern "C" void EnterFIQAtomic (void);
extern "C" void ExitFIQAtomic (void);
extern "C" void _EnterFIQAtomic (void);
extern "C" void _ExitFIQAtomicFast (void);
extern "C" long LockStack(TULockStack* lockRef, ULong additionalSpace);	// Lock the entire stack plus additionalSpace # of bytes
extern "C" long UnlockStack(TULockStack* lockRef);
extern "C" long LockHeapRange(VAddr start, VAddr end, Boolean wire);	// Lock range from start up to but not including end.  Wire will prevent v->p mappings from changing.
extern "C" long UnlockHeapRange(VAddr start, VAddr end);

// ================================================================================
// � Interrupt Handlers
// ================================================================================

static void __StartOfInterruptHandlers ()
{
}

static void TxBEmptyIntHandler (void* logger)
{
	((Logger*)logger)->TxBEmptyIntHandler ();
}

static void RxCAvailIntHandler (void* logger)
{
	 ((Logger*)logger)->RxCAvailIntHandler ();
}

static void RxCSpecialIntHandler (void* logger)
{
	((Logger*)logger)->RxCSpecialIntHandler ();
}

static void ExtStsIntHandler (void* logger)
{
	 ((Logger*)logger)->ExtStsIntHandler ();
}

static void __EndOfInterruptHandlers ()
{
}

#pragma mark -

// ================================================================================
// � Server Functions
// ================================================================================

#if USE_SYS_WRITE

NewtonErr Logger::Initialize ()
{
	NewtonErr r = noErr;
	return r;
}

long Logger::Main ()
{
	return noErr;
}

void Logger::Close ()
{
}

void Logger::BufferOutput (Boolean buffer)
{
}

void Logger::StartOutput ()
{
}

void Logger::Output (UByte* data, ULong size)
{
	_sys_write (1, (char *) data, size);
}

#else

NewtonErr Logger::Initialize ()
{
	PSerialChipRegistry* reg;
	SerialChipID id;
	NewtonErr r = noErr;

	fSemaphore = new TULockingSemaphore ();
	fSemaphore->Init ();
	fLogLevel = 3;
	fBufferOutput = false;
	fBuffer = new UByte[MAX_LOGGER_OUTPUT];
	fHead = 0;
	fTail = 0;
	fLocation = kHWLocExternalSerial;
	fSpeed = k9600bps;
	fChip = NULL;
	reg = GetSerialChipRegistry ();
	id = reg->FindByLocation (fLocation);
	fChip = reg->GetChipPtr (id);
	if (fChip == NULL) {
		r = -10005;
	} else {
		* (Long *) ((Byte*) (fChip) + 0x10) = 0;
	}
	return r;
}

void Logger::TxBEmptyIntHandler (void)
{
	if (fHead != fTail) {
		EnterFIQAtomic ();
		fChip->SetTxDTransceiverEnable (true);
		while (fChip->TxBufEmpty () && fTail != fHead) {
			fChip->PutByte (fBuffer[fTail]);
			fTail = (fTail + 1) % MAX_LOGGER_OUTPUT;
		}
		ExitFIQAtomic ();
	} else {
		EnterFIQAtomic ();
		fChip->ResetTxBEmpty ();
		ExitFIQAtomic ();
	}
}

void Logger::ExtStsIntHandler (void)
{
}

void Logger::RxCAvailIntHandler (void)
{
	UByte c;
	
	while (fChip->RxBufFull ()) {
		c = fChip->GetByte ();
	}
}

void Logger::RxCSpecialIntHandler (void)
{
	UByte c;
	
	while (fChip->RxBufFull ()) {
		c = fChip->GetByte ();
	}
}

long Logger::Main ()
{
	TCMOSerialIOParms parms;
	SCCChannelInts handlers;
	NewtonErr r;
	ULong n;
	TULockStack lock;
	
	LockHeapRange((VAddr) &::__StartOfInterruptHandlers, (VAddr) &::__EndOfInterruptHandlers, true);

 	fChip->SetSerialMode (kSerModeAsync);

	if (!fChip->PowerIsOn ()) fChip->PowerOn ();

	fChip->SetInterruptEnable (false);
	LockStack (&lock, 64);
	_EnterFIQAtomic ();
	
	handlers.TxBEmptyIntHandler = ::TxBEmptyIntHandler;
	handlers.ExtStsIntHandler = ::ExtStsIntHandler;
	handlers.RxCAvailIntHandler = ::RxCAvailIntHandler;
	handlers.RxCSpecialIntHandler = ::RxCSpecialIntHandler;
	
	r = fChip->InstallChipHandler (this, &handlers);

	fChip->SetIntSourceEnable (kSerIntSrcTxBufEmpty, true);
	parms.fStopBits = k1StopBits;
	parms.fParity = kNoParity;
	parms.fDataBits = k8DataBits;
	parms.fSpeed = fSpeed;
	fChip->SetIOParms (&parms);
	fChip->SetSpeed (fSpeed);
	fChip->Reconfigure ();
	fChip->SetTxDTransceiverEnable (true);
	fChip->SetInterruptEnable (true); 
	_ExitFIQAtomicFast ();
	UnlockStack (&lock);
	
	return noErr;
}

void Logger::Close ()
{
	fChip->SetInterruptEnable (false);
	fChip->RemoveChipHandler (this);
	fChip->SetTxDTransceiverEnable (false);
	fChip->PowerOff ();
	delete fSemaphore;
	delete fBuffer;
	UnlockHeapRange((VAddr) &::TxBEmptyIntHandler, (VAddr) &::__EndOfInterruptHandlers);
}

void Logger::BufferOutput (Boolean buffer)
{
	fBufferOutput = buffer;
}

void Logger::StartOutput ()
{
	UByte c;
	
	fBufferOutput = false;
	fChip->SetTxDTransceiverEnable (true);
	while (fChip->TxBufEmpty () && fTail != fHead) {
		fChip->PutByte (fBuffer[fTail]);
		fTail = (fTail + 1) % MAX_LOGGER_OUTPUT;
	}
}

void Logger::Output (UByte* data, ULong size)
{
	fSemaphore->Acquire ();
	while (size > 0) {
		if (*data == 13) fBuffer[fHead] = 10;
		else if (*data == 10) fBuffer[fHead] = 13;
		else fBuffer[fHead] = *data;
		data++;
		size--;
		fHead = (fHead + 1) % MAX_LOGGER_OUTPUT;
	}
	fSemaphore->Release ();

	if (!fBufferOutput) {
		StartOutput ();
	}
}

#endif