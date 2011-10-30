#ifndef __TRFCOMMTOOL_H
#define __TRFCOMMTOOL_H

class SCCChannelInts;
class TCMOSerialIOParms;
class TCMOSerialHardware;
class TCMOSerialHWParms;
class TCMOSerialHWChipLoc;
class TCMOSerialChipSpec;
class TCMService;
class TSerialChip;
class TOption;

class THCILayer;
class TL2CAPLayer;
class TSDPLayer;
class TRFCOMMLayer;

#define ArrayCount(x) (sizeof(x)/sizeof(x[0]))

#ifdef forDebug
#define LOG if (fLogLevel) printf
#define LOG2 if (fLogLevel >= 2) printf
#define LOG3 if (fLogLevel >= 3) printf
#define LOGX printf
#else
#define LOG
#define LOG2
#define LOG3
#define LOGX printf
#endif

#define TOOL_GET_STATUS		0x10000000
#define TOOL_SENDING 		0x20000000
#define TOOL_RECEIVING 		0x40000000
#define TOOL_XYZ 			0x80000000

#define TOOL_MODE_NORMAL	0
#define TOOL_MODE_DISCOVER	1
#define TOOL_MODE_PAIR		2
#define TOOL_MODE_SDP		3

typedef enum {
	L2CAP_CONNECTION_REQUEST,
	L2CAP_SDP_CONNECTION_REQUEST,
	L2CAP_CONFIGURE_REQUEST,
	L2CAP_ACCEPT_CONFIGURE_REQUEST,
	L2CAP_CONNECTION_RESPONSE,
	RFCOMM_SABM,
	RFCOMM_PN,
	TOOL_PAIR,
	TOOL_TERMINATE,
	TOOL_CONNECT_FAILED,
	TOOL_SERVICES
} TimerType;

typedef enum {
	DRIVER_GENERIC,
	DRIVER_PICO,
	DRIVER_TAIYO_YUDEN,
	DRIVER_BT2000,
	DRIVER_BT2000E,
	DRIVER_950_7MHZ,
	DRIVER_950_14MHZ,
	DRIVER_950_18MHZ,
	DRIVER_950_32MHZ,
	DRIVER_CBT100C
} DriverType;

#define kSerResult_InputDataPending		6
#define kSerResult_OutputComplete		5

#define kRFCOMMToolErrBase				-19000
#define kRFCOMMToolErrBind				(-10078)
#define kRFCOMMToolErrConnectStatus		(kRFCOMMToolErrBase - 1)
#define kRFCOMMToolErrConnect			(kRFCOMMToolErrBase - 2)
#define kRFCOMMToolErrAcceptStatus		(kRFCOMMToolErrBase - 3)
#define kRFCOMMToolErrAccept			(kRFCOMMToolErrBase - 4)

typedef enum {
	TOOL_IDLE,
	TOOL_BIND,
	TOOL_CONNECT,
	TOOL_LISTEN,
	TOOL_ACCEPT,
	TOOL_CONNECTED,
	TOOL_CLOSE,
	TOOL_PAIR_INCOMING,
	TOOL_PAIR_OUTGOING,
	TOOL_DISCOVER,
	TOOL_SDP
} ToolState;

struct DiscoveredDevice {
	UByte fBdAddr[6];
	UByte fPageScanRepetitionMode;
	UByte fPageScanPeriodMode;
	UByte fPageScanMode;
	UByte fClass[3];
	UByte fName[249];
};

#define SET_SHORT(addr, offset, value) \
	addr[offset] = value & 0x00ff; addr[offset + 1] = (value & 0xff00) >> 8
#define GET_SHORT(addr, offset) \
	 (addr[(offset) + 1] * 256 + addr[(offset)])

#define SET_LONG(addr, offset, value) \
	addr[offset] = value & 0x000000ff;             addr[offset + 1] = (value & 0x0000ff00) >> 8; \
	addr[offset + 2] = (value & 0x00ff0000) >> 16; addr[offset + 3] = (value & 0xff000000) >> 24;
#define GET_LONG(addr, offset) \
	 ((addr[(offset) + 3] << 24) + (addr[(offset) + 2] << 16) + (addr[(offset) + 1] << 8) + addr[(offset)])

#define SET_SHORT_N(addr, offset, value) \
	addr[offset + 1] =  value & 0x000000ff;       addr[offset] = (value & 0x0000ff00) >> 8;
#define SET_USHORT_N(addr, offset, value) \
	addr[offset + 1] =  value & 0x000000ff;       addr[offset] = (value & 0x0000ff00) >> 8;
#define SET_ULONG_N(addr, offset, value) \
	addr[offset + 3] =  value & 0x000000ff;       addr[offset + 2] = (value & 0x0000ff00) >> 8; \
	addr[offset + 1] = (value & 0x00ff0000) >> 16;addr[offset] = (value & 0xff000000) >> 24;

#define GET_USHORT_N(addr, offset) \
	* (UShort *) (&addr[offset])
#define GET_SHORT_N(addr, offset) \
	* (Short *) (&addr[offset])
#define GET_ULONG_N(addr, offset) \
	* (ULong *) (&addr[offset])
	
#define NUM_ELEMENTS(x) (sizeof (x)/sizeof (x[0]))	

// ================================================================================
// ¥ CBuffer classes
// ================================================================================

class CMinBuffer
{
public:
	ULong dummy_00;
};

class CBuffer: public CMinBuffer
{
public:
};

class CShadowBufferSegment: public CBuffer
{
private:
	Byte filler_00[0x1C - sizeof (CBuffer)];

public:
												/* 0000 - 0000 */
												/* 0008 - 0018 */
};

class CBufferSegmentPublic
{
public:
	void 					*fJumpTable;
	UByte *dummy_04;
	ULong dummy_08;
	ULong					fPhysicalSize;
	UByte					*fBuffer;
	UByte					*fCurrent;
	UByte					*fBufferLast;
	ULong dummy_1c;
	ULong dummy_20;
	ULong dummy_24;
};

// ================================================================================
// ¥ Utility classes
// ================================================================================

class TSerToolReply: public TCommToolReply
{
private:
	Byte filler_00[0x30 - sizeof (TCommToolReply)];
};

class TDelayTimer
{
private:
	Byte filler_00[0x0c];
};

class TRFCOMMTimerEvent: public TAEvent, public TUAsyncMessage
{
public:
	TimerType				fTimerId;
	void					*fUserData;
							TRFCOMMTimerEvent (void);
};

class TSerialChip16450
{
public:
	void WriteSerReg (ULong, UByte);
};

// ================================================================================
// ¥ TCircleBuf
// ================================================================================

class TCircleBuf
{
public:
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
	Long Allocate (Size);
	Long Allocate (Size, int, Boolean, Boolean);
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

// ================================================================================
// ¥ TRFCOMMTool option classes
// ================================================================================

class TRFCOMMAddressOption: public TOption
{
public:
	UByte					fBdAddr[6];
	Byte					fPort;
};

class TRFCOMMPINCodeOption: public TOption
{
public:
	Byte					fPINCodeLength;
	UByte					fPINCode[16];
};

class TRFCOMMModeOption: public TOption
{
public:
	Byte					fMode;
};

class TRFCOMMNameOption: public TOption
{
public:
	UByte					fNameLength;
	UChar					fName[64];
};

class TRFCOMMLinkKeyOption: public TOption
{
public:
	UChar					fLinkKey[16];
};

// ================================================================================
// ¥ TCommTool
// ================================================================================

class TCommTool: public TUTaskWorld					/* 0018 - 026b */
{

public:
	Byte 					filler_0030[0x30 - 0x18];

	TCMOCTConnectInfo 		fConnectInfo;			/* 0030 - 0044 */

	Byte					filler_0044[0x100];  	/* 0044 */
	

#if 0
	Byte					filler_0044[0x48];  	/* 0044 */
	
	TUPort					fPort;					/* 008c */
	
	Byte					filler_0094[0xb4];  	/* 0094 - 0148 */
#endif

public:
	TCMOTransportInfo		fTransportInfo;			/* 0148 */
	ULong					filler_0174;
	TCommToolOptionInfo 	fOptionInfo_1;			/* 0178 */
	TCommToolOptionInfo 	fOptionInfo_2;			/* 0190 */
	TCommToolOptionInfo 	fOptionInfo_3;			/* 01a8 */
	
	ULong					filler_01c0;
	ULong					filler_01c4;
	ULong					filler_01c8;
	
	Size					fTCommToolInputBufferSize;
	
	UByte					filler_01d0;
	UByte					filler_01d1;
	
	UByte					flag_01d2;
	
	UByte					filler_01d3;
	ULong					filler_01d4;
	
	TCommToolGetEventReply	fGetEventReply;			/* 01d8 */
	
	ULong					filler_01fc;
	ULong					filler_0200;
	ULong					filler_0204;
	ULong					filler_0208;
	ULong					filler_020c;
	ULong					filler_0210;
	ULong					filler_0214;
	ULong					filler_0218;
	ULong					filler_021c;
	
	CShadowBufferSegment	fBufferSegment_1;		/* 0220 */
	CShadowBufferSegment	fBufferSegment_2;  		/* 023c */
													/* 0248 */
												
	Byte 					filler_0248[0x14];

public:
							TCommTool (unsigned long);
							TCommTool (unsigned long, long);
	virtual					~TCommTool (void);
	
	virtual	ULong			GetSizeOf (void) = 0;

	virtual	NewtonErr		TaskConstructor();
	virtual	void			TaskDestructor();
	virtual	void			TaskMain();

	virtual UChar*			GetToolName() = 0;

	virtual void			HandleInternalEvent(void);
	virtual NewtonErr		HandleRequest(TUMsgToken& msgToken, ULong msgType);
	virtual void			HandleReply(ULong userRefCon, ULong msgType);
	virtual void			HandleTimerTick(void);

	virtual void			DoControl(ULong opCode, ULong msgType);
	virtual void			DoKillControl(ULong msgType);
	virtual void			DoStatus(unsigned long, unsigned long);
	
	virtual void			GetCommEvent();
	virtual void			DoKillGetCommEvent();
	virtual NewtonErr		PostCommEvent(TCommToolGetEventReply& theEvent, NewtonErr result);

	virtual NewtonErr		OpenStart(TOptionArray* options);
	virtual NewtonErr		OpenComplete();
	virtual int				Close (void);
	virtual void			CloseComplete(NewtonErr result);

	virtual void			ConnectStart();
	virtual void			ConnectComplete(NewtonErr result);

	virtual void			ListenStart();
	virtual void			ListenComplete(NewtonErr result);

	virtual void			AcceptStart();
	virtual void			AcceptComplete(NewtonErr result);

			void			Disconnect();
	virtual void			DisconnectComplete(NewtonErr result);

	virtual void			ReleaseStart();
	virtual void			ReleaseComplete(NewtonErr result);

	virtual void			BindStart();
	virtual void			BindComplete(NewtonErr result);

	virtual void			UnbindStart();
	virtual void			UnbindComplete(NewtonErr result);

	virtual void			GetProtAddr();
	
	virtual void		 	OptionMgmt(TCommToolOptionMgmtRequest *);
	virtual void		 	OptionMgmtComplete(NewtonErr result);

	virtual ULong			ProcessOptions(TCommToolOptionInfo*);
	virtual ULong			ProcessOptionsContinue(TCommToolOptionInfo*);
	virtual ULong			ProcessOptionsComplete(NewtonErr, TCommToolOptionInfo*);
	virtual ULong			ProcessOptionsCleanUp(NewtonErr, TCommToolOptionInfo*);

	virtual void			ProcessCommOptionComplete(unsigned long, TCommToolOptionInfo* theOption);
	virtual NewtonErr		ProcessOptionStart(TOption* theOption, ULong label, ULong opcode);
	virtual void			ProcessOptionComplete(unsigned long);
	virtual void			ProcessOption(TOption* theOption, ULong label, ULong opcode);
	
	virtual void			ForwardOptions();
	virtual NewtonErr		AddDefaultOptions(TOptionArray* options);
	virtual NewtonErr		AddCurrentOptions(TOptionArray* options);


	virtual void			ProcessPutBytesOptionStart(TOption* theOption, ULong label, ULong opcode);
	virtual void			ProcessPutBytesOptionComplete(unsigned long);
	virtual void			ProcessGetBytesOptionStart(TOption* theOption, ULong label, ULong opcode);
	virtual void			ProcessGetBytesOptionComplete(unsigned long);
	
	virtual	NewtonErr		PutBytes (CBufferList *) = 0;
	virtual	NewtonErr		PutFramedBytes (CBufferList *, Boolean) = 0;

	virtual void			PutComplete(NewtonErr result, ULong putBytesCount);
	virtual	void			KillPut (void) = 0;
	virtual void			KillPutComplete(NewtonErr result);
	virtual	NewtonErr		GetBytes (CBufferList *) = 0;
	virtual	NewtonErr		GetFramedBytes (CBufferList *) = 0;

	virtual void			GetBytesImmediate(CBufferList* clientBuffer, Size threshold);
	virtual void			GetComplete(NewtonErr result, Boolean endOfFrame = false, ULong getBytesCount = 0);
	virtual	void			KillGet (void) = 0;
	virtual void			KillGetComplete(NewtonErr result);

	virtual void			PrepGetRequest();
	virtual void			GetOptionsComplete(NewtonErr);

	virtual void			PrepPutRequest();
	virtual void			PutOptionsComplete(NewtonErr);
	
	virtual void			ResArbRelease(unsigned char*, unsigned char*);
	virtual void			ResArbReleaseStart(unsigned char*, unsigned char*);
	virtual void			ResArbReleaseComplete(NewtonErr);
	virtual void			ResArbClaimNotification(unsigned char*, unsigned char*);
	
	virtual void			TerminateConnection();
	virtual void			TerminateComplete();
	virtual void			GetNextTermProc(ULong terminationPhase,ULong& terminationFlag,TerminateProcPtr& TerminationProc);
	virtual void			SetChannelFilter(CommToolRequestType, Boolean);
};

// ================================================================================
// ¥ TSerTool
// ================================================================================

class TSerTool: public TCommTool					/* 026c - 037f */
{
public:
	ULong 					filler_026c;

	CBufferList				*fSendBufferList;		/* 0270 */
	ULong					fRemainingSendBytes;

	UByte 					filler_0278;
	UByte 					filler_0279;
	Boolean					fIdle;

	UByte 					filler_027b;

	CBufferList				*fReceiveBufferList;	/* 027c */
	Size 					fRemainingReceiveBytes;
	Size					fReceiveBufferSize;

	UByte 					filler_0288;
	UByte 					fReceiveBufferValid;	/* 0289 */
	UByte 					filler_028a;
	UByte 					filler_028b;
	
	ULong 					filler_028c;
	
	UByte 					filler_0290;
	UByte 					filler_0291;

	Boolean					fChipIdle;

	UByte 					filler_0293;

	TSerToolReply 			fReply;					/* 0294 - */

private:
	Byte 					filler_00[0x14];

public:
	TUAsyncMessage			fMessage1;				/* 02d8 - */

	ULong 					filler_02e8;

	TAEvent					fEvent1;				/* 02ec - */

	ULong 					filler_02f4;
	ULong 					filler_02f8;
	ULong 					filler_02fc;
	ULong 					filler_0300;
	
	TSerialChip				*fSerialChip;

	ULong 					filler_0308;
	ULong 					filler_030c;

public:
	TCMOSerialChipSpec		fChipSpec;				/* 0310 - */

	ULong 					filler_0330;
	ULong 					filler_0334;

	TDelayTimer				fDelayTimer;			/* 0338 - */
	TCMOSerialIOParms		fSerialIOParms;			/* 0344 - */
	TUAsyncMessage			fMessage2;				/* 0360 - */

	ULong 					filler_0370;

	TAEvent					fEvent2;				/* 0374 - 037b */
												
	ULong 					filler_037c;
	
public:
							TSerTool (unsigned long);
	virtual					~TSerTool (void);
	
	virtual	NewtonErr		TaskConstructor();
	virtual	void			TaskDestructor();

	virtual NewtonErr		HandleRequest(TUMsgToken& msgToken, ULong msgType);

	virtual void			DoControl(ULong opCode, ULong msgType);
	virtual void			DoKillControl(ULong msgType);

	virtual void			GetCommEvent();

	virtual void			ConnectStart();
	virtual void			ListenStart();
	virtual void			BindStart();
	virtual void			UnbindStart();

	virtual NewtonErr		ProcessOptionStart(TOption* theOption, ULong label, ULong opcode);

	virtual NewtonErr		AddDefaultOptions(TOptionArray* options);
	virtual NewtonErr		AddCurrentOptions(TOptionArray* options);

	virtual	NewtonErr		PutBytes (CBufferList *) ;
	virtual	NewtonErr		PutFramedBytes (CBufferList *, Boolean);

	virtual void			PutComplete(NewtonErr result, ULong putBytesCount);
	virtual	NewtonErr		GetBytes (CBufferList *);
	virtual	NewtonErr		GetFramedBytes (CBufferList *);

	virtual void			GetBytesImmediate(CBufferList* clientBuffer, Size threshold);
	virtual void			GetComplete(NewtonErr result, Boolean endOfFrame = false, ULong getBytesCount = 0);

	virtual void			ResArbReleaseStart(unsigned char*, unsigned char*);
	virtual void			ResArbClaimNotification(unsigned char*, unsigned char*);
	
	virtual void			TerminateComplete();

	virtual void			PowerOnEvent (ULong);
	virtual void			PowerOffEvent(ULong);
	virtual Boolean			ClaimSerialChip();
	virtual void			AllocateBuffers () = 0;
	virtual void			BindToSerChip();
	virtual void			TurnOnSerChip () = 0;
	virtual void			TurnOffSerChip () = 0;
	virtual void			UnbindToSerChip();
	virtual void			DeallocateBuffers () = 0;
	virtual NewtonErr		UnclaimSerialChip();
	virtual NewtonErr		TurnOn();
	virtual NewtonErr		TurnOff();
	virtual void			SetIOParms(TCMOSerialIOParms *);
	virtual void			SetSerialChipSelect(TCMOSerialHardware *);
	virtual void			SetSerialChipLocation(TCMOSerialHWChipLoc *);
	virtual void			SetSerialChipSpec(TCMOSerialChipSpec *);
	virtual void			BytesAvailable (ULong &) = 0;
	virtual void			StartOutput(CBufferList *);
	virtual void			DoOutput () = 0;
	virtual void			StartInput(CBufferList *);
	virtual void			DoInput () = 0;
	virtual void			GetChannelIntHandlers (SCCChannelInts *) = 0;
	virtual void			IHRequest(ULong);
	virtual void			IHReqHandler () = 0;
	virtual void			WakeUpHandler();
	
	void					SetTxDTransceiverEnable (Boolean);
};

// ================================================================================
// ¥ TAsyncSerTool
// ================================================================================

class TAsyncSerTool: public TSerTool
{
public:
	Boolean					fDoInputActive;
	UByte 					filler_0381;
	UByte 					filler_0382;
	UByte 					filler_0383;
	
	TCircleBuf				fOutputBuffer;				/* 0384 */
	TCircleBuf				fInputBuffer;				/* 03ac */
	TCMOSerialBuffers		fSerialBuffers;				/* 03d4 */

	ULong 					filler_03ec;
	ULong 					filler_03f0;

	TCMOOutputFlowControlParms	fOFCParms;				/* 03f4 */
	TCMOInputFlowControlParms	fIFCParms;				/* 0408 */
	TCMOSerialIOStats		fSerialIOStats;				/* 041c */
	TCMOBreakFraming		fBreakFraming;				/* 043c */
	TCMOSerialEventEnables	fSerialEventEnables;		/* 0458 */

	ULong 					filler_046c;

	TCMOSerialMiscConfig	fSerialMiscConfig;			/* 0470 */
	
	ULong					filler_0488;
	ULong					filler_048c;
	ULong					filler_0490;

	ULong					fToolState;
	ULong					fTransferState;				/* 0498 */
	
	ULong					filler_049c;
	ULong					filler_04a0;
	ULong					filler_04a4;
	ULong					filler_04a8;
	ULong					filler_04ac;

public:
							TAsyncSerTool (unsigned long);
	virtual					~TAsyncSerTool (void);
	
	virtual	ULong			GetSizeOf (void);

	virtual	NewtonErr		TaskConstructor();
	virtual	void			TaskDestructor();

	virtual UChar*			GetToolName();

	virtual NewtonErr		ProcessOptionStart(TOption* theOption, ULong label, ULong opcode);

	virtual NewtonErr		AddDefaultOptions(TOptionArray* options);
	virtual NewtonErr		AddCurrentOptions(TOptionArray* options);

	virtual	void			KillPut (void);
	virtual	void			KillGet (void);
	
	virtual void			AllocateBuffers ();
	virtual void			TurnOnSerChip ();
	virtual void			TurnOffSerChip ();
	virtual void			DeallocateBuffers ();

	virtual void			BytesAvailable (ULong &);
	virtual void			DoOutput ();
	virtual void			DoInput ();
	virtual void			GetChannelIntHandlers (SCCChannelInts *);
	virtual void			IHRequest (unsigned long);
	virtual void			IHReqHandler ();
	virtual void			DoPutComplete (long);
	virtual int				FillOutputBuffer (void);
	virtual void			DoGetComplete (long, unsigned char);
	virtual int 			EmptyInputBuffer (unsigned long *);
	virtual void			TxDataSent (void);
	virtual void			RxDataAvailable (void);
	virtual void			SerialEvents (unsigned long);
	virtual void			DataInObserver (Boolean);
	virtual void			SetInputSendForIntDelay (unsigned long);
	virtual void			RestoreInputSendForIntDelay ();
	
	void					SyncInputBuffer (void);
	void					ContinueOutputST (UByte);
	void					StartOutputST (void);
};

// ================================================================================
// ¥ TRFCOMMTool
// ================================================================================

class TRFCOMMTool: public TAsyncSerTool					/* 0690 */
{
public:
	ToolState				fState;

	Byte					fMode;
	
	Byte					fLogLevel;
	
	DriverType				fDriver;
	
	THCILayer				*fHCI;
	TL2CAPLayer				*fL2CAP;
	TSDPLayer				*fSDP;
	TRFCOMMLayer			*fRFCOMM;
	
	TRFCOMMTimerEvent		*fTimerEvent;
	
	// Device addresses
	
	UByte					fBdAddr[6];
	UByte					fPeerBdAddr[6];
	Byte					fPeerRFCOMMPort;
	
	// Link key and PIN code data

	Boolean					fLinkKeyValid;
	UByte					fLinkKey[16];	
	UByte					fPINCode[16];
	Byte					fPINCodeLength;
	
	UByte					fName[65];
	
	// Data buffer
	
	ULong					fSavedDataSize;
	UByte					fSavedData[2064];
	
	// Discovery data
	
	DiscoveredDevice		*fDiscoveredDevices;
	Byte					fNumDiscoveredDevices;
	Byte					fCurrentDevice;
	
	// Services data
	
	ULong					*fQueriedServices;
	Byte					fCurrentService;
	Byte					fNumQueriedServices;
	
public:
							TRFCOMMTool (unsigned long serviceId);
	virtual					~TRFCOMMTool (void);
	virtual	ULong			GetSizeOf (void) { return sizeof (TRFCOMMTool); }

	virtual NewtonErr		HandleRequest(TUMsgToken& msgToken, ULong msgType);

	virtual NewtonErr		ProcessOptionStart(TOption* theOption, ULong label, ULong opcode);

	virtual void			BindStart (void);
	virtual void			ConnectStart (void);
	virtual void			ListenStart (void);
	virtual void			AcceptStart (void);

	virtual void			TerminateConnection (void);
			void			DoTerminate (void);

	virtual void			DoGetComplete (long, unsigned char);

	virtual void			RxDataAvailable (void);
	virtual int 			EmptyInputBuffer (unsigned long *);

	virtual void			DoOutput (void);
	
	void					DoEvent (void);
	
	void					DriverReset (void);
	void					DriverSendDelay (void);
	
	NewtonErr				HandleInputData (UByte *data, Short length);

	NewtonErr				ProcessHCIEvent (UByte *event);
	NewtonErr				BindHCI (UByte *event);
	NewtonErr				ConnectHCI (UByte *event);
	NewtonErr				ListenHCI (UByte *event);
	NewtonErr				AcceptHCI (UByte *event);
	NewtonErr				ConnectedHCI (UByte *event);
	NewtonErr				DiscoverHCI (UByte *event);
	NewtonErr				ServicesHCI (UByte *event);
	NewtonErr				PairIncomingHCI (UByte *event);
	NewtonErr				PairOutgoingHCI (UByte *event);

	NewtonErr				ProcessL2CAPEvent (UByte *event);
	void					BindL2CAP (UByte *event);
	void					ConnectL2CAP (UByte *event);
	void					ListenL2CAP (UByte *event);
	void					AcceptL2CAP (UByte *event);
	void					ConnectedL2CAP (UByte *event);
	void					ServicesL2CAP (UByte *event);

	NewtonErr				ProcessRFCOMMEvent (UByte event, UByte *data, Byte DLCI);
	void					BindRFCOMM (UByte event, UByte *data, Byte DLCI);
	void					ConnectRFCOMM (UByte event, UByte *data, Byte DLCI);
	void					ListenRFCOMM (UByte event, UByte *data, Byte DLCI);
	void					AcceptRFCOMM (UByte event, UByte *data, Byte DLCI);
	void					ConnectedRFCOMM (UByte event, UByte *data, Byte DLCI);

	NewtonErr				ProcessSDPEvent (UByte *event);
	void					ServicesSDP (UByte *event);
	
	void					StartTimer (TimerType type, ULong milliSecondsDelay, void *userData = NULL);
	void					TimerExpired (void);
	void					StopTimer (void);
};

extern StartCommTool (TCommTool *, ULong, TServiceInfo *);
extern OpenCommTool (ULong, TOptionArray *, TCMService *);

#endif