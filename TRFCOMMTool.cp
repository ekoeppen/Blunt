#include <AEvents.h>
#include <AEventHandler.h>
#include <BufferSegment.h>
#include <CommManagerInterface.h>
#include <CommTool.h>
#include <CommErrors.h>
#include <SerialOptions.h>
#include <CommOptions.h>
#include <Endpoint.h>
#include <UserTasks.h>
#include <NewtonScript.h>
#include <SerialChipV2.h>
#include "RelocHack.h"

#include "TRFCOMMTool.h"
#include "TSDPLayer.h"
#include "TL2CAPLayer.h"
#include "THCILayer.h"
#include "TRFCOMMLayer.h"

//-------------------------------
// 16450 register base addr offsets
//-------------------------------

#define kReceiveBufReg 3
#define kTransmitBufReg 3
#define kInterruptEnbReg 2
#define kInterruptIDReg 1
#define kLineControlReg 0
#define kModemControlReg 7
#define kLineStatusReg 6
#define kModemStatusReg 5
#define kScratchReg 4

static const ULong kQueriedServices[] = {
	UUID_SERIAL_PORT,
	UUID_PPP_LAN,
	UUID_DUN,
	UUID_OBEX_PUSH,
	UUID_OBEX_FILETRANSFER
};

class TGPIOInterface
{
public:
	NewtonErr ReadGPIOData (UByte, ULong *);
};

class TBIOInterface
{
public:
	NewtonErr ReadDIOPins (UByte pin, ULong *data);
	NewtonErr WriteDIODir (UByte pin, UByte dir, UByte *data);

	NewtonErr WriteDIOPins (UByte pin, UByte value, UByte *data);
};

class TVoyagerPlatform
{
public:
	char filler_0000[0x0010];
	
	TGPIOInterface *fGPIOInterface;
	TBIOInterface *fBIOInterface;
	
	char filler_0014[0x00f0];
};

extern TVoyagerPlatform *GetPlatformDriver (void);

UByte SetChannel3Selector (UByte value)
{
	TVoyagerPlatform *p;
	UByte data;
	
	p = GetPlatformDriver ();
	p->fBIOInterface->WriteDIOPins (0x22, value, &data);
}

TRFCOMMTimerEvent::TRFCOMMTimerEvent (void)
{
	fAEventClass = 'rfcm';
	fAEventID = 'timr';
	Init ();
}


TRFCOMMTool::TRFCOMMTool (unsigned long serviceId):
	TAsyncSerTool (serviceId)
{
	fState = TOOL_IDLE;
	fHCI = NULL;
	fL2CAP = NULL;
	fSDP = NULL;
	fRFCOMM = NULL;
	
	fDriver = DRIVER_GENERIC;

	fRemainingSendBytes = 0;
	fReceiveBufferList = NULL;
	fRemainingReceiveBytes = 0;
	fReceiveBufferSize = 0;
	fSavedDataSize = 0;
	fDiscoveredDevices = NULL;
	fNumDiscoveredDevices = 0;
	fMode = TOOL_MODE_NORMAL;
	memcpy (fPINCode, "1234", 4);
	memcpy (fPeerBdAddr, "\221\033\051\140\020\000", 6);
	fPeerRFCOMMPort = 1;
	fPINCodeLength = 4;
	fLinkKeyValid = false;
	fTimerEvent = new TRFCOMMTimerEvent ();
	
	fLogLevel = 0;
	
	LOG ("-------------------------\nTRFCOMMTool\n");
	
	RelocVTable (__VTABLE__11TRFCOMMTool);

	fQueriedServices = (ULong *) RelocFuncPtr (kQueriedServices);
	fNumQueriedServices = ArrayCount (kQueriedServices);
}

TRFCOMMTool::~TRFCOMMTool (void)
{
	LOG ("~TRFCOMMTool\n");

	if (fHCI) delete fHCI;
	if (fL2CAP) delete fL2CAP;
	if (fSDP) delete fSDP;
	if (fRFCOMM) delete fRFCOMM;
	if (fDiscoveredDevices) delete fDiscoveredDevices;
	if (fTimerEvent) delete fTimerEvent;
}

#pragma mark -

// ================================================================================
// ¥ Driver Specific Functions
// ================================================================================

void TRFCOMMTool::DriverReset (void)
{
	TSerialChip16450 *chip;
	Byte prescale;
	
	LOG ("TRFCOMMTool::DriverReset (%d)\n", fDriver);
	chip = (TSerialChip16450 *) fSerialChip;
	if (fDriver == DRIVER_BT2000) {
		// Enable Enhanced Mode to get larger FIFO buffers
		chip->WriteSerReg (kLineControlReg, 0x83);
		chip->WriteSerReg (kInterruptIDReg, 0x21);
		chip->WriteSerReg (kLineControlReg, 0x03);
	} else if (fDriver == DRIVER_BT2000E ||
		fDriver == DRIVER_950_7MHZ ||
		fDriver == DRIVER_950_14MHZ ||
		fDriver == DRIVER_950_18MHZ ||
		fDriver == DRIVER_950_32MHZ) {
		// Enable '950 Mode
		chip->WriteSerReg (kLineControlReg, 0xbf);
		chip->WriteSerReg (kInterruptIDReg, 0x10);
		chip->WriteSerReg (kLineControlReg, 0x03);

		// Enable Enhanced Mode to get larger FIFO buffers
		chip->WriteSerReg (kLineControlReg, 0x83);
		chip->WriteSerReg (kInterruptIDReg, 0x21);
		chip->WriteSerReg (kLineControlReg, 0x03);

		// Enable the prescaler
		chip->WriteSerReg (kModemControlReg, 0x80);
		chip->WriteSerReg (kScratchReg, 0x01);
		chip->WriteSerReg (kLineStatusReg, 0x20);

		// Set the prescale value
		if (fDriver == DRIVER_BT2000E || fDriver == DRIVER_950_7MHZ) {
			prescale = 0x20;
		} else if (fDriver == DRIVER_950_14MHZ) {
			prescale = 0x40;
		} else if (fDriver == DRIVER_950_18MHZ) {
			prescale = 0x50;
		} else if (fDriver == DRIVER_950_32MHZ) {
			prescale = 0x8b;
		}
		chip->WriteSerReg (kScratchReg, 0x01);
		chip->WriteSerReg (kLineStatusReg, prescale);
	} else if (fDriver == DRIVER_TAIYO_YUDEN) {
//		SetChannel3Selector (1);
//		Wait (100);
//		SetChannel3Selector (0);
	}
	Wait (100);
}

void TRFCOMMTool::DriverSendDelay (void)
{
	switch (fDriver) {
		case DRIVER_TAIYO_YUDEN:
			Wait (1);
			break;
		default:
			break;
	}
}

#pragma mark -

// ================================================================================
// ¥ Comm Tool Functions
// ================================================================================

NewtonErr TRFCOMMTool::HandleRequest (TUMsgToken& msgToken, ULong msgType)
{
	if (msgToken.GetMsgId () == fTimerEvent->GetMsgId ()) {
		TimerExpired ();
	}
	return TAsyncSerTool::HandleRequest (msgToken, msgType);
}

NewtonErr TRFCOMMTool::ProcessOptionStart(TOption* theOption, ULong label, ULong opcode)
{
	Char s[5];
	
	memset (s, 0, 5);
	memcpy (s, &label, 4);
	LOG ("ProcessOptionStart: %s\n", s);

	if (label == 'addr') {
		TRFCOMMAddressOption *o;
		o = (TRFCOMMAddressOption *) theOption;
		fPeerBdAddr[0] = o->fBdAddr[5]; fPeerBdAddr[1] = o->fBdAddr[4]; 
		fPeerBdAddr[2] = o->fBdAddr[3]; fPeerBdAddr[3] = o->fBdAddr[2]; 
		fPeerBdAddr[4] = o->fBdAddr[1]; fPeerBdAddr[5] = o->fBdAddr[0]; 
		fPeerRFCOMMPort = o->fPort;
		LOG ("  %02x:%02x:%02x:", o->fBdAddr[0], o->fBdAddr[1], o->fBdAddr[2]);
		LOG ("%02x:%02x:%02x ", o->fBdAddr[3], o->fBdAddr[4], o->fBdAddr[5]);
		LOG ("%d\n", o->fPort);
	} else if (label == 'pinc') {
		TRFCOMMPINCodeOption *o;
		o = (TRFCOMMPINCodeOption *) theOption;
		memcpy (fPINCode, o->fPINCode, sizeof (fPINCode));
		fPINCodeLength = o->fPINCodeLength;
		LOG ("  %*s (%d)\n", fPINCodeLength, fPINCode, fPINCodeLength);
	} else if (label == 'mode') {
		fMode = ((TRFCOMMModeOption *) theOption)->fMode;
	} else if (label == 'lnkk') {
		TRFCOMMLinkKeyOption *o;
		o = (TRFCOMMLinkKeyOption *) theOption;
		memcpy (fLinkKey, o->fLinkKey, sizeof (fLinkKey));
		fLinkKeyValid = true;
	}
	return TAsyncSerTool::ProcessOptionStart (theOption, label, opcode);
}

void TRFCOMMTool::BindStart ()
{
	TCMOSerialHWChipLoc *loc;
	TCMOSerialIOParms *parms;
	Ref globals, v;
	UShort *s;
	UByte *c;
	
	LOG ("TRFCOMMTool::BindStart\n");
	
	globals = NSCallGlobalFn (SYM (GetGlobals));
	v = GetVariable (globals, SYM (bluetoothSpeed));
	if (v != NILREF) {
		parms = new TCMOSerialIOParms ();
		parms->fSpeed = RefToInt (v);
		SetIOParms (parms);
	}
	
	v = GetVariable (globals, SYM (bluetoothLocation));
	if (v != NILREF) {
		loc = new TCMOSerialHWChipLoc ();
		WITH_LOCKED_BINARY (v, b);
		s = (UShort *) b; c = (UByte *) &loc->fHWLoc;
		*c++ = *s++; *c++ = *s++; *c++ = *s++; *c++ = *s++;
		END_WITH_LOCKED_BINARY (v);
		SetSerialChipLocation(loc);
	}
	
	v = GetVariable (globals, SYM (bluetoothName));
	if (v != NILREF) {
		WITH_LOCKED_BINARY (v, b);
		for (s = (UShort *) b, c = fName; *s != 0; c++, s++) *c = *s;
		*c = '\0';
		END_WITH_LOCKED_BINARY (v);
	}

	v = GetVariable (globals, SYM (bluetoothDriver));
	fDriver = DRIVER_BT2000E;
	if (v != NILREF) {
		if (v == SYM(TaiyoYuden)) fDriver = DRIVER_TAIYO_YUDEN;
		else if (SymbolCompareLex (v, SYM(PICO)) == 0) fDriver = DRIVER_PICO;
		else if (SymbolCompareLex (v, SYM(Generic)) == 0) fDriver = DRIVER_GENERIC;
		else if (SymbolCompareLex (v, SYM(BT2000)) == 0) fDriver = DRIVER_BT2000;
		else if (SymbolCompareLex (v, SYM(BT2000E)) == 0) fDriver = DRIVER_BT2000E;
		else if (SymbolCompareLex (v, SYM(OXC950_7Mhz)) == 0) fDriver = DRIVER_950_7MHZ;
		else if (SymbolCompareLex (v, SYM(OXC950_14MHz)) == 0) fDriver = DRIVER_950_14MHZ;
		else if (SymbolCompareLex (v, SYM(OXC950_18MHz)) == 0) fDriver = DRIVER_950_18MHZ;
		else if (SymbolCompareLex (v, SYM(OXC950_32MHz)) == 0) fDriver = DRIVER_950_32MHZ;
		else if (SymbolCompareLex (v, SYM(CBT100C)) == 0) fDriver = DRIVER_CBT100C;
	}
	
	fState = TOOL_BIND;
	if (ClaimSerialChip () || flag_01d2 == 0) {
		if (TurnOn () == noErr) {
			fHCI = new THCILayer;
			fL2CAP = new TL2CAPLayer;
			fSDP = new TSDPLayer;
			fRFCOMM = new TRFCOMMLayer;
		
			fHCI->fTool = this;
			fHCI->fL2CAP = fL2CAP;
			
			fL2CAP->fTool = this;
			fL2CAP->fHCI = fHCI;
			fL2CAP->fSDP = fSDP;
			fL2CAP->fRFCOMM = fRFCOMM;
			
			fSDP->fTool = this;
			fSDP->fL2CAP = fL2CAP;
		
			fRFCOMM->fTool = this;
			fRFCOMM->fL2CAP = fL2CAP;

//			fLogLevel = 1;
//			fHCI->fLogLevel = 1;
//			fL2CAP->fLogLevel = 1;
//			fSDP->fLogLevel = 1;
//			fRFCOMM->fLogLevel = 1;

			fTransferState |= TOOL_RECEIVING;

			DriverReset ();
			SyncInputBuffer ();
			
			fHCI->ReadBufferSize ();
		} else {
			BindComplete (kSerErr_ChannelInUse);
		}
	} else {
		BindComplete (kSerErr_ChannelInUse);
	}
}

void TRFCOMMTool::ConnectStart ()
{
	LOG ("TRFCOMMTool::ConnectStart (%d)\n", fMode);

	TurnOn ();

	if (fMode == TOOL_MODE_DISCOVER) {
		fState = TOOL_DISCOVER;
		fHCI->Inquiry ();
	} else if (fMode == TOOL_MODE_SDP) {
		fState = TOOL_SDP;
		fHCI->CreateConnection ((UByte *) fPeerBdAddr, 1, 0);
	} else if (fMode == TOOL_MODE_PAIR) {
		fState = TOOL_PAIR_OUTGOING;
		fHCI->CreateConnection ((UByte *) fPeerBdAddr, 1, 0);
	} else {
		fState = TOOL_CONNECT;
		fHCI->CreateConnection ((UByte *) fPeerBdAddr, 1, 0);
	}
}

void TRFCOMMTool::ListenStart (void)
{
	LOG ("TRFCOMMTool::ListenStart\n");
	
	TurnOn ();
	fState = TOOL_LISTEN;
}

void TRFCOMMTool::AcceptStart (void)
{
	LOG ("TRFCOMMTool::AcceptStart\n");

	if (fMode == TOOL_MODE_PAIR) {
		fState = TOOL_PAIR_INCOMING;
		LOG ("Accept complete\n");
		AcceptComplete (noErr);
	} else if (fMode == TOOL_MODE_SDP) {
		fState = TOOL_SDP;
		LOG ("Accept complete\n");
		AcceptComplete (noErr);
	} else {
		fState = TOOL_ACCEPT;
	}
	fHCI->AcceptConnectionRequest (fPeerBdAddr, 0x01);
}

void TRFCOMMTool::TerminateConnection (void)
{
	LOG ("TRFCOMMTool::TerminateConnection\n");
	
	if (fHCI->fConnectionHandle != -1) {
		if (fRFCOMM->fState == RFCOMM_CONNECTED) {
			fRFCOMM->SndDisconnect (fRFCOMM->fChannel->fDLCI);
		}
		
		if (fL2CAP->fState == L2CAP_CONNECTED && fRFCOMM->fChannel != NULL) {
			fL2CAP->SndDisconnectionRequest (fL2CAP->fChannel);
		}
		
		fHCI->Disconnect (fHCI->fConnectionHandle, 0x13);
	}
	
	StartTimer (TOOL_TERMINATE, 3000);
}

void TRFCOMMTool::DoTerminate (void)
{
	LOG ("TRFCOMMTool::DoTerminate\n");

	TAsyncSerTool::TerminateConnection ();
}

#pragma mark -

// ================================================================================
// ¥ Input Functions
// ================================================================================

void TRFCOMMTool::DoGetComplete (NewtonErr result, Boolean endOfMessage)
{
	LOG ("TRFCOMMTool::DoGetComplete\n");

	TAsyncSerTool::DoGetComplete (result, endOfMessage);
	fTransferState |= TOOL_RECEIVING;
}

void TRFCOMMTool::RxDataAvailable (void)
{
	LOG2 ("\nTRFCOMMTool::RxDataAvailable\n");

	if (fReceiveBufferList != NULL) DoInput ();
	else DoEvent ();
}

void TRFCOMMTool::DoEvent (void)
{
	int r;

	LOG2 ("TRFCOMMTool::DoEvent\n");
	SyncInputBuffer ();
	r = fHCI->ProcessToolBuffer ();
}

int TRFCOMMTool::EmptyInputBuffer (ULong *size)
{
	int r;
	
	LOG2 ("TRFCOMMTool::EmptyInputBuffer\n");
	
	r = fHCI->ProcessToolBuffer ();
	LOG2 ("--> %d\n", r);
	if (r == noErr) {
		r = HandleInputData (NULL, 0);
		LOG2 ("--> %d\n", r);
	}
	
	return r;
}

NewtonErr TRFCOMMTool::HandleInputData (UByte *data, Short length)
{
	Size bytesRead, totalBytesAvailable, totalBytesRead;
	Long r;
	
	LOG2 ("TRFCOMMTool::HandleInputData (%d)\n  RBuffer size: %d RRBytes: %d RBL: %08x\n",
		length, fReceiveBufferSize, fRemainingReceiveBytes, fReceiveBufferList);

	r = noErr;

	if (fReceiveBufferValid) {
		totalBytesAvailable = fSavedDataSize + length;
		totalBytesRead = 0;
		
		if (fReceiveBufferList) {
			LOG2 ("TRFCOMMTool::HandleInputData (%d %d)\n", length, fSavedDataSize);

			bytesRead = fReceiveBufferList->Putn (fSavedData, fSavedDataSize);
			fSavedDataSize -= bytesRead;
			totalBytesAvailable -= bytesRead;
			totalBytesRead += bytesRead;
			
			if (fSavedDataSize > 0) {
				LOG2 ("  Restored: %d + %d bytes\n", fSavedDataSize, bytesRead);
				memmove (fSavedData, fSavedData + bytesRead, fSavedDataSize);
				LOG2 ("  Done.\n");
			}
			LOG2 ("  1: %d %d %d\n", bytesRead, totalBytesRead, totalBytesAvailable);
			
			bytesRead = fReceiveBufferList->Putn (data, length);
			length -= bytesRead;
			totalBytesAvailable -= bytesRead;
			totalBytesRead += bytesRead;

			if (length > 0) {
				if (fSavedDataSize + length < sizeof (fSavedData)) {
					memcpy (fSavedData + fSavedDataSize, data + bytesRead, length);
					fSavedDataSize += length;
				} else {
					LOGX ("  Overflow1: %d + %d bytes\n", fSavedDataSize, length);
				}
			}
			
			LOG2 ("  2: %d %d %d\n", bytesRead, totalBytesRead, totalBytesAvailable);

		} else {
			LOG2 ("TRFCOMMTool::HandleInputData (%d)\n  RBuffer size: %d RRBytes: %d RBL: %08x\n",
				length, fReceiveBufferSize, fRemainingReceiveBytes, fReceiveBufferList);

			if (fSavedDataSize + length < sizeof (fSavedData)) {
				LOG2 ("  Saved: %d + %d bytes\n", fSavedDataSize, length);
				memcpy (fSavedData + fSavedDataSize, data, length);
				fSavedDataSize += length;
			} else {
				LOG2 ("  Overflow2: %d + %d bytes\n", fSavedDataSize, length);
			}
		}
		
		if (totalBytesRead < totalBytesAvailable) {
			r = kSerResult_InputDataPending;
		} else if (fReceiveBufferSize <= totalBytesRead) {
			r = kSerResult_EOM;
		}
		
		fRemainingReceiveBytes -= totalBytesRead;
		fReceiveBufferSize -= totalBytesRead;
		
		if (!fReceiveBufferList) {
			LOG2 ("  Done (%d %d).\n", totalBytesAvailable, totalBytesRead);
		}
	} else {
		if (fReceiveBufferList) {
			LOG2 ("  Data saved anyway.\n");
			fReceiveBufferList->Putn (data, length);
		} else {
			LOG2 ("  Data lost.\n");
		}
	}
	
	LOG2 ("  r = %d\n", r);

	return r;
}

#pragma mark -

// ================================================================================
// ¥ Output Functions
// ================================================================================

void TRFCOMMTool::DoOutput ()
{
	UByte packet[RFCOMM_MTU_LEN];
	CBufferList *list;
	CBufferSegment *buffer;
	Size length;
	
	if (fRemainingSendBytes > 0) {
		length = fSendBufferList->GetSize ();
		LOG ("TRFCOMMTool::DoOutput (%d)\n", length);
		
		if (length > sizeof (packet)) length = sizeof(packet);

		length = fSendBufferList->Getn (packet, length);
		fRemainingSendBytes -= length;

		buffer = CBufferSegment::New ();
		buffer->Init (packet, length, false);
		
		list = CBufferList::New ();
		list->Init ();
		list->Insert ((CBuffer *) buffer);
		
		fRFCOMM->SndUnnumberedInformation (list);
		
		list->Delete ();
	}
	
	if (fRemainingSendBytes == 0) {
		DoPutComplete (kSerResult_NoErr);
	}
}

#pragma mark -

// ================================================================================
// ¥ HCI Event Handling
// ================================================================================

NewtonErr TRFCOMMTool::ProcessHCIEvent (UByte *event)
{
	NewtonErr r;
	
	LOG2 ("TRFCOMMTool::ProcessHCIEvent (%d %d)\n", event[0], fState);
	
	switch (fState) {
		case TOOL_BIND:
			r = BindHCI (event);
			break;
		case TOOL_CONNECT:
			r = ConnectHCI (event);
			break;
		case TOOL_LISTEN:
			r = ListenHCI (event);
			break;
		case TOOL_ACCEPT:
			r = AcceptHCI (event);
			break;
		case TOOL_CONNECTED:
			r = ConnectedHCI (event);
			break;
		case TOOL_DISCOVER:
			r = DiscoverHCI (event);
			break;
		case TOOL_SDP:
			r = ServicesHCI (event);
			break;
		case TOOL_PAIR_INCOMING:
			r = PairIncomingHCI (event);
			break;
		case TOOL_PAIR_OUTGOING:
			r = PairOutgoingHCI (event);
			break;
		default:
			LOG ("  Unhandled event %02x\n", event[0]);
			break;
	}
	
	return r;
}

NewtonErr TRFCOMMTool::BindHCI (UByte *event)
{
	LOG ("TRFCOMMTool::BindHCI (%d)\n", event[0]);
	
	if (event[0] == EVT_HCI_COMMAND_COMPLETE) {
		if (event[5] == 0x00) {
			switch (fHCI->fState) {
				case HCI_RESET:
					fHCI->ReadBufferSize ();
					break;
				case HCI_READ_BUFFER_SIZE:
					fHCI->fHCIBufferSize = event[6] + (event[7] << 8);
					fHCI->fHCIWindowSize = event[9];
					LOG ("  Buffers: %d Size: %d\n",
						fHCI->fHCIWindowSize, fHCI->fHCIBufferSize);
					fHCI->HostBufferSize ();
					break;
				case HCI_HOST_BUFFER_SIZE:
					fHCI->ChangeLocalName (fName);
					break;
				case HCI_CHANGE_LOCAL_NAME:
					fHCI->WriteScanEnable (0x03);
					break;
				case HCI_WRITE_SCAN_ENABLE:
					fHCI->WriteClassofDevice ((UByte *) "\020\001\024");
					break;
				case HCI_WRITE_CLASS_OF_DEVICE:
					BindComplete (noErr);
					break;
				default:
					break;
			};
		} else {
			BindComplete (kRFCOMMToolErrBind);
		}
	} else {
	}
	
	return noErr;
}

NewtonErr TRFCOMMTool::ConnectHCI (UByte *event)
{
	Channel *c;
	
	LOG ("TRFCOMMTool::ConnectHCI (%d)\n", event[0]);
	
	switch (event[0]) {
		case EVT_HCI_COMMAND_STATUS:
			if (event[2] != 0x00) {
				ConnectComplete (kRFCOMMToolErrConnectStatus);
				fState = TOOL_IDLE;
			}
			break;
		case EVT_HCI_CONNECTION_COMPLETE:
			if (event[2] == 0x00) {
				fHCI->GetLinkQuality (fHCI->fConnectionHandle);
			} else {
				ConnectComplete (kRFCOMMToolErrConnect);
				TerminateConnection ();
				fState = TOOL_IDLE;
			}
			break;
		case EVT_HCI_PIN_CODE_REQUEST:
			fHCI->PINCodeRequestReply (fPeerBdAddr, fPINCode, fPINCodeLength);
			break;
		case EVT_HCI_LINK_KEY_NOTIFICATION:
			break;
		case EVT_HCI_LINK_KEY_REQUEST:
			if (fLinkKeyValid) fHCI->LinkKeyRequestReply (fPeerBdAddr, fLinkKey);
			else fHCI->LinkKeyRequestNegativeReply (fPeerBdAddr);
			break;
		case EVT_HCI_MAX_SLOTS_CHANGE:
			break;
		case EVT_HCI_COMMAND_COMPLETE:
			switch (fHCI->fState) {
				case HCI_GET_LINK_QUALITY:
					if (fL2CAP->fState == L2CAP_IDLE) {
						c = fL2CAP->SndConnectionRequest (PSM_RFCOMM, NULL);
						StartTimer (L2CAP_CONNECTION_REQUEST, 2000, c);
					}
					break;
				case HCI_PIN_CODE_REQUEST_REPLY:
				case HCI_LINK_KEY_REQUEST_REPLY:
				case HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY:
					break;
				default:
					LOG ("Warning: unhandled command complete\n");
					break;
			}
			break;
		default:
			break;
	}
	LOG ("\n");
	
	return noErr;
}

NewtonErr TRFCOMMTool::ListenHCI (UByte *event)
{
	LOG ("TRFCOMMTool::ListenHCI (0x%02x)\n", event[0]);

	switch (event[0]) {
		case EVT_HCI_CONNECTION_REQUEST:
			memcpy (fPeerBdAddr, &event[2], 6);
			ListenComplete (noErr);
			break;
		default:
			break;
	}
	
	return noErr;
}

NewtonErr TRFCOMMTool::AcceptHCI (UByte *event)
{
	LOG ("TRFCOMMTool::AcceptHCI (0x%02x)\n", event[0]);

	switch (event[0]) {
		case EVT_HCI_COMMAND_STATUS:
			if (event[2] != 0x00) {
				AcceptComplete (kRFCOMMToolErrAcceptStatus);
				fState = TOOL_IDLE;
			}
			break;
		case EVT_HCI_CONNECTION_COMPLETE:
			if (event[2] == 0x00) {
				memcpy (fPeerBdAddr, &event[5], 6);
				LOG ("Accept complete\n");
				AcceptComplete (noErr);
				fState = TOOL_LISTEN;
			} else {
				AcceptComplete (kRFCOMMToolErrAccept);
				TerminateConnection ();
				fState = TOOL_IDLE;
			}
			break;
		case EVT_HCI_PIN_CODE_REQUEST:
			fHCI->PINCodeRequestReply (fPeerBdAddr, fPINCode, fPINCodeLength);
			break;
		case EVT_HCI_LINK_KEY_NOTIFICATION:
			break;
		case EVT_HCI_LINK_KEY_REQUEST:
			if (fLinkKeyValid) fHCI->LinkKeyRequestReply (fPeerBdAddr, fLinkKey);
			else fHCI->LinkKeyRequestNegativeReply (fPeerBdAddr);
			break;
		case EVT_HCI_COMMAND_COMPLETE:
			if (fHCI->fState == HCI_PIN_CODE_REQUEST_REPLY) {
			} else {
				LOG ("Warning: unhandled command complete in accept state\n");
			}
			break;
		default:
			break;
	}
	
	return noErr;
}

NewtonErr TRFCOMMTool::ConnectedHCI (UByte *event)
{
	switch (event[0]) {
		case EVT_HCI_LINK_KEY_REQUEST:
			if (fLinkKeyValid) fHCI->LinkKeyRequestReply (fPeerBdAddr, fLinkKey);
			else fHCI->LinkKeyRequestNegativeReply (fPeerBdAddr);
			break;
		case EVT_HCI_DISCONNECTION_COMPLETE:
			fState = TOOL_IDLE;
			TerminateConnection();
			break;
		case EVT_HCI_PIN_CODE_REQUEST:
			fHCI->PINCodeRequestReply (fPeerBdAddr, fPINCode, fPINCodeLength);
			break;
		case EVT_HCI_COMMAND_COMPLETE:
			if (fHCI->fState != HCI_PIN_CODE_REQUEST_REPLY) {
				LOG ("Warning: unhandled command complete in connected state\n");
			}
			break;
	}
	
	return noErr;
}

NewtonErr TRFCOMMTool::DiscoverHCI (UByte *event)
{
	NewtonErr r;
	DiscoveredDevice *d;
	int i, j, n;
	
	r = noErr;
	switch (event[0]) {
		case EVT_HCI_COMMAND_STATUS:
			ConnectComplete (noErr);
			break;
		case EVT_HCI_INQUIRY_RESULT:
			n = event[2];
			for (i = 0; i < n; i++) {
				fDiscoveredDevices = (DiscoveredDevice *) realloc (fDiscoveredDevices,
					(fNumDiscoveredDevices + 1) * sizeof (DiscoveredDevice));
				d = &fDiscoveredDevices[fNumDiscoveredDevices];
				
				memcpy (&d->fBdAddr, &event[i * 6 + 3], 6);
				d->fPageScanRepetitionMode = event[i + n * 6 + 3];
				d->fPageScanPeriodMode = event[i + n * 7 + 3];
				d->fPageScanMode = event[i + n * 8 + 3];
				memcpy (&d->fClass, &event[i * 3 + n * 9 + 3], 3);
				
				fNumDiscoveredDevices++;
			}
			break;
		case EVT_HCI_INQUIRY_COMPLETE:
			if (fNumDiscoveredDevices > 0) {
				fCurrentDevice = 0;
				d = &fDiscoveredDevices[fCurrentDevice];
				fHCI->RemoteNameRequest (d->fBdAddr, d->fPageScanRepetitionMode, d->fPageScanMode);
			} else {
				fHCI->ReadBdAddr ();
			}
			break;
		case EVT_HCI_REMOTE_NAME_REQUEST_COMPLETE:
			d = &fDiscoveredDevices[fCurrentDevice];
			memcpy (d->fName, &event[9], sizeof (d->fName) - 1);
			d->fName[sizeof (d->fName) - 1] = 0;
			LOG ("==> %s %d %d\n", d->fName, fCurrentDevice, event[2]);
			HandleInputData ((UByte *) d, sizeof (DiscoveredDevice));
			
			if (fCurrentDevice < fNumDiscoveredDevices - 1) {
				fCurrentDevice++;
				d = &fDiscoveredDevices[fCurrentDevice];
				fHCI->RemoteNameRequest (d->fBdAddr, d->fPageScanRepetitionMode, d->fPageScanMode);
			} else {
				fHCI->ReadBdAddr ();
			}
			break;
		case EVT_HCI_COMMAND_COMPLETE:
			fState = TOOL_IDLE;
			TerminateConnection ();
			for (i = 0; i < fNumDiscoveredDevices; i++) {
				d = &fDiscoveredDevices[i];
				for (j = 0; j < 6; j++) {
					LOG ("%02x ", d->fBdAddr[5 - j]);
				}
				LOG (" - ");
				for (j = 0; j < 3; j++) {
					LOG ("%02x ", d->fClass[3 - j]);
				}
				LOG ("\n");
			}
			break;
	}
	
	return r;
}

NewtonErr TRFCOMMTool::ServicesHCI (UByte *event)
{
	Channel *c;
	
	LOG ("TRFCOMMTool::ServicesHCI (%d)\n", event[0]);
	
	switch (event[0]) {
		case EVT_HCI_COMMAND_STATUS:
			if (event[2] != 0x00) {
				ConnectComplete (kRFCOMMToolErrConnectStatus);
				fState = TOOL_IDLE;
			}
			break;
		case EVT_HCI_CONNECTION_COMPLETE:
			if (event[2] == 0x00) {
				LOG ("Connection complete\n");
				if (fL2CAP->fState == L2CAP_IDLE) {
					c = fL2CAP->SndConnectionRequest (PSM_SDP, NULL);
					StartTimer (L2CAP_SDP_CONNECTION_REQUEST, 2000, c);
				}
				ConnectComplete (noErr);
			}
			break;
		case EVT_HCI_PIN_CODE_REQUEST:
			fHCI->PINCodeRequestReply (fPeerBdAddr, fPINCode, fPINCodeLength);
			break;
		case EVT_HCI_LINK_KEY_REQUEST:
			if (fLinkKeyValid) fHCI->LinkKeyRequestReply (fPeerBdAddr, fLinkKey);
			else fHCI->LinkKeyRequestNegativeReply (fPeerBdAddr);
			break;
		default:
			break;
	}
	LOG ("\n");
	
	return noErr;
}

NewtonErr TRFCOMMTool::PairOutgoingHCI (UByte *event)
{
	LOG ("TRFCOMMTool::PairOutgoingHCI (%d %d)\n", event[0], fHCI->fState);
	
	switch (event[0]) {
		case EVT_HCI_COMMAND_STATUS:
			if (event[2] != 0x00) {
				ConnectComplete (kRFCOMMToolErrConnectStatus);
				fState = TOOL_IDLE;
			}
			break;
		case EVT_HCI_CONNECTION_COMPLETE:
			if (event[2] == 0x00) {
				ConnectComplete (noErr);
				StartTimer (TOOL_PAIR, 500, NULL);
			}
			break;
		case EVT_HCI_PIN_CODE_REQUEST:
			fHCI->PINCodeRequestReply (fPeerBdAddr, fPINCode, fPINCodeLength);
			break;
		case EVT_HCI_LINK_KEY_NOTIFICATION:
			LOG ("  Got Link Key\n");
			fLinkKeyValid = true;
			memcpy (fSavedData, &event[8], 16);
			fSavedDataSize = 16;
			break;
		case EVT_HCI_LINK_KEY_REQUEST:
			if (fLinkKeyValid) fHCI->LinkKeyRequestReply (fPeerBdAddr, fLinkKey);
			else fHCI->LinkKeyRequestNegativeReply (fPeerBdAddr);
			break;
		case EVT_HCI_AUTHENTICATION_COMPLETE:
			TerminateConnection ();
			break;
		default:
			break;
	}
	LOG ("\n");
	
	return noErr;
}

NewtonErr TRFCOMMTool::PairIncomingHCI (UByte *event)
{
	LOG ("TRFCOMMTool::PairIncomingHCI (%d)\n", event[0]);
	
	switch (event[0]) {
		case EVT_HCI_COMMAND_STATUS:
			if (event[2] != 0x00) {
				AcceptComplete (kRFCOMMToolErrAcceptStatus);
				fState = TOOL_IDLE;
			} else {
			}
			break;
		case EVT_HCI_CONNECTION_COMPLETE:
			if (event[2] == 0x00) {
				memcpy (fPeerBdAddr, &event[5], 6);
			} else {
				AcceptComplete (kRFCOMMToolErrAccept);
				TerminateConnection ();
				fState = TOOL_IDLE;
			}
			break;
		case EVT_HCI_PIN_CODE_REQUEST:
			fHCI->PINCodeRequestReply (fPeerBdAddr, fPINCode, fPINCodeLength);
			break;
		case EVT_HCI_LINK_KEY_NOTIFICATION:
			HandleInputData (&event[8], 16);
			break;
		case EVT_HCI_COMMAND_COMPLETE:
			if (fHCI->fState == HCI_PIN_CODE_REQUEST_REPLY) {
			} else {
				LOG ("Warning: unhandled command complete in accept state\n");
			}
			break;
		case EVT_HCI_DISCONNECTION_COMPLETE:
			TerminateConnection ();
			break;
		default:
			break;
	}
	LOG ("\n");
	
	return noErr;
}

#pragma mark -

// ================================================================================
// ¥ L2CAP Event Handling
// ================================================================================

NewtonErr TRFCOMMTool::ProcessL2CAPEvent (UByte *event)
{
	switch (fState) {
		case TOOL_BIND:
			BindL2CAP (event);
			break;
		case TOOL_CONNECT:
			ConnectL2CAP (event);
			break;
		case TOOL_LISTEN:
			ListenL2CAP (event);
			break;
		case TOOL_ACCEPT:
			AcceptL2CAP (event);
			break;
		case TOOL_CONNECTED:
			ConnectedL2CAP (event);
			break;
		case TOOL_SDP:
			ServicesL2CAP (event);
			break;
		default:
			LOG ("  Unhandled event %02x\n", event[0]);
			break;
	};
	
	return noErr;
}

void TRFCOMMTool::BindL2CAP (UByte *event)
{
}

void TRFCOMMTool::ConnectL2CAP (UByte *event)
{
	LOG ("TRFCOMMTool::ConnectL2CAP (0x%02d)\n", event[0]);
	Channel *c;

	switch (event[0]) {
		case EVT_L2CAP_CONNECTION_RESPONSE:
			StopTimer ();
			if (fL2CAP->fState == L2CAP_CONFIGURE &&
				(GET_SHORT (event, 8) == 0 || GET_SHORT (event, 8) == 1) &&
				fL2CAP->fConfigureState == CONFIGURE_TWO) {
				fRFCOMM->fChannel = fL2CAP->GetChannel (GET_SHORT (event, 6));
				fL2CAP->SndConfigureRequest (fRFCOMM->fChannel);
				StartTimer (L2CAP_CONFIGURE_REQUEST, 2000);
			}
			break;
		case EVT_L2CAP_CONFIGURE_RESPONSE:
			StopTimer ();
		case EVT_L2CAP_CONFIGURE_REQUEST:
			if (fL2CAP->fConfigureState == CONFIGURE_DONE &&
				fRFCOMM->fState == RFCOMM_IDLE) {
				LOG ("Connection complete\n");
				fL2CAP->fState = L2CAP_CONNECTED;
				fRFCOMM->SndSetAsynchronousBalancedMode (0);
				StartTimer (RFCOMM_SABM, 3000);
			}
			break;
	}
}

void TRFCOMMTool::ListenL2CAP (UByte *event)
{
	LOG ("TRFCOMMTool::ListenL2CAP (0x%02d)\n", event[0]);

	switch (event[0]) {
		case EVT_L2CAP_COMMAND_REJECT:
		break;
	}
}

void TRFCOMMTool::AcceptL2CAP (UByte *event)
{
}

void TRFCOMMTool::ConnectedL2CAP (UByte *event)
{
}

void TRFCOMMTool::ServicesL2CAP (UByte *event)
{
	LOG ("TRFCOMMTool::ServicesL2CAP (0x%02d)\n", event[0]);

	switch (event[0]) {
		case EVT_L2CAP_CONNECTION_RESPONSE:
			StopTimer ();
			if (fL2CAP->fState == L2CAP_CONFIGURE &&
				(GET_SHORT (event, 8) == 0 || GET_SHORT (event, 8) == 1) &&
				fL2CAP->fConfigureState == CONFIGURE_TWO) {
				fSDP->fChannel = fL2CAP->GetChannel (GET_SHORT (event, 6));
				fL2CAP->SndConfigureRequest (fSDP->fChannel);
				StartTimer (L2CAP_CONFIGURE_REQUEST, 2000);
			}
			break;
		case EVT_L2CAP_CONFIGURE_RESPONSE:
			StopTimer ();
		case EVT_L2CAP_CONFIGURE_REQUEST:
			if (fL2CAP->fConfigureState == CONFIGURE_DONE) {
				fCurrentService = 0;
				if (fCurrentService < fNumQueriedServices) {
					fSDP->SndServiceSearchAttributeRequest (
						fQueriedServices[fCurrentService]);
				} else {
					fL2CAP->SndDisconnectionRequest (fSDP->fChannel);
				}
			}
			break;
		case EVT_L2CAP_DISCONNECTION_RESPONSE:
			TerminateConnection ();
			break;
	}
}

#pragma mark -

// ================================================================================
// ¥ RFCOMM Event Handling
// ================================================================================

NewtonErr TRFCOMMTool::ProcessRFCOMMEvent (UByte event, UByte *data, Byte DLCI)
{
	LOG ("TRFCOMMTool::ProcessRFCOMMEvent (0x%02x)\n", event);
	
	switch (fState) {
		case TOOL_BIND:
			BindRFCOMM (event, data, DLCI);
			break;
		case TOOL_CONNECT:
			ConnectRFCOMM (event, data, DLCI);
			break;
		case TOOL_LISTEN:
			ListenRFCOMM (event, data, DLCI);
			break;
		case TOOL_ACCEPT:
			AcceptRFCOMM (event, data, DLCI);
			break;
		case TOOL_CONNECTED:
			ConnectedRFCOMM (event, data, DLCI);
			break;
		default:
			LOG ("  Unhandled event %02x\n", event);
			break;
	};
	
	return noErr;
}

void TRFCOMMTool::BindRFCOMM (UByte event, UByte *data, Byte DLCI)
{
	LOG ("TRFCOMMTool::BindRFCOMM\n");
}

void TRFCOMMTool::ConnectRFCOMM (UByte event, UByte *data, Byte DLCI)
{
	LOG ("TRFCOMMTool::ConnectRFCOMM (%d)\n", DLCI);
	Byte mtu;
	
	switch (event) {
		case CTRL_UA:
			if (DLCI == 0) {
				StopTimer ();
				if (fRFCOMM->fChannel->fRemoteMTU > 0) {
					mtu = MIN(RFCOMM_MTU_LEN, fRFCOMM->fChannel->fRemoteMTU - 8);
				} else {
					mtu = RFCOMM_MTU_LEN;
				}
				fRFCOMM->SndMPXParameterNegotiation (fPeerRFCOMMPort << 1, 0, mtu, 0);
			} else {
				fRFCOMM->SndMPXModemStatusCommand (DLCI, true, false, 0x8c);
				ConnectComplete (noErr);
				LOG ("Connection completed\n");
				fState = TOOL_CONNECTED;
			}
			break;
		case CTRL_UIH:
			if ((fRFCOMM->fInputPacket[0] >> 2) == MPX_PN && DLCI == 0) {
				fRFCOMM->SndSetAsynchronousBalancedMode (fPeerRFCOMMPort << 1);
			}
			break;
	}
}

void TRFCOMMTool::ListenRFCOMM (UByte event, UByte *data, Byte DLCI)
{
	LOG ("TRFCOMMTool::ListenRFCOMM (%d)\n", DLCI);

	switch (event) {
		case CTRL_SABM:
			if (DLCI != 0) {
				fRFCOMM->SndMPXModemStatusCommand (DLCI, true, false, 0x8c);
				fState = TOOL_CONNECTED;
			}
			break;
		case CTRL_UIH:
			LOG ("  %02x %d\n", data[3], DLCI);
			break;
	}
}

void TRFCOMMTool::AcceptRFCOMM (UByte event, UByte *data, Byte DLCI)
{
}

void TRFCOMMTool::ConnectedRFCOMM (UByte event, UByte *data, Byte DLCI)
{
}

#pragma mark -

// ================================================================================
// ¥ SDP Event Handling
// ================================================================================

NewtonErr TRFCOMMTool::ProcessSDPEvent (UByte *event)
{
	LOG ("TRFCOMMTool::ProcessSDPEvent\n");
	
	switch (fState) {
		case TOOL_SDP:
			ServicesSDP (event);
			break;
		default:
			switch (event[0]) {
				case SDP_PDU_SERVICE_SEARCH_REQUEST:
					break;
				case SDP_PDU_SERVICE_ATTRIBUTE_REQUEST:
					break;
				case SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_REQUEST:
					break;
			}
			break;
	};
	
	return noErr;
}

void TRFCOMMTool::ServicesSDP (UByte *event)
{
	LOG ("TRFCOMMTool::ServicesSDP (%d)\n", event[0]);

	switch (event[0]) {
		case SDP_PDU_ERROR_RESPONSE:
			TerminateConnection ();
			break;
		case SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_RESPONSE:
			fCurrentService++;
			if (fCurrentService < fNumQueriedServices) {
				fSDP->SndServiceSearchAttributeRequest (
					fQueriedServices[fCurrentService]);
			} else {
				fL2CAP->SndDisconnectionRequest (fSDP->fChannel);
			}
			break;
	}
}

#pragma mark -

// ================================================================================
// ¥ Timer functions
// ================================================================================

void TRFCOMMTool::StartTimer (TimerType type, ULong delay, void *userData)
{
	TUPort *p;
	TTime futureTime;

	LOG ("TRFCOMMTool::StartTimer\n");
	
	fTimerEvent->fTimerId = type;
	fTimerEvent->fUserData = userData;
	p = (TUPort *)(((char *) this) + 0x8c);
	futureTime = GetGlobalTime () + TTime (delay, kMilliseconds);
	p->Send (fTimerEvent, (void *) fTimerEvent, sizeof (TRFCOMMTimerEvent),
		kNoTimeout, &futureTime);
}

void TRFCOMMTool::TimerExpired (void)
{
	LOG ("TRFCOMMTool::TimerExpired (%d)\n", fTimerEvent->fTimerId);
	
	switch (fTimerEvent->fTimerId) {
		case L2CAP_CONFIGURE_REQUEST:
			fL2CAP->fTimeout = true;
			fL2CAP->SndConfigureRequest (fRFCOMM->fChannel);
			break;
		case L2CAP_CONNECTION_REQUEST:
			fL2CAP->fTimeout = true;
			fL2CAP->SndConnectionRequest (PSM_RFCOMM, (Channel *) fTimerEvent->fUserData);
			break;
		case L2CAP_SDP_CONNECTION_REQUEST:
			fL2CAP->fTimeout = true;
			fL2CAP->SndConnectionRequest (PSM_SDP, (Channel *) fTimerEvent->fUserData);
			break;
		case L2CAP_CONNECTION_RESPONSE:
			fL2CAP->SndConnectionResponse ((Channel *) fTimerEvent->fUserData, 0x0000, 0x0000);
			fL2CAP->SndConfigureRequest ((Channel *) fTimerEvent->fUserData);
			break;
		case RFCOMM_SABM:
			fRFCOMM->SndSetAsynchronousBalancedMode (0);
			break;
		case TOOL_PAIR:
			if (!fLinkKeyValid) {
				LOG ("  Requesting authentication\n");
				fHCI->AuthenticationRequested (fHCI->fConnectionHandle);
			} else {
				TerminateConnection ();
				fState = TOOL_IDLE;
			}
			break;
		case TOOL_CONNECT_FAILED:
			ConnectComplete (kRFCOMMToolErrConnect);
			break;
		case TOOL_TERMINATE:
			DoTerminate ();
			break;
	}
}

void TRFCOMMTool::StopTimer (void)
{
	LOG ("TRFCOMMTool::StopTimer\n");
	
	fTimerEvent->Abort ();
}
