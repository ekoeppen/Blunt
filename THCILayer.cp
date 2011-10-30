#include <AEvents.h>
#include <AEventHandler.h>
#include <BufferSegment.h>
#include <CommManagerInterface.h>
#include <CommTool.h>
#include <SerialOptions.h>
#include <SerialChipV2.h>
#include <CommOptions.h>

#include "THCILayer.h"
#include "TRFCOMMTool.h"
#include "TL2CAPLayer.h"

THCILayer::THCILayer (void)
{
	fTool = NULL;
	fLogLevel = 0;
	fOutstandingPackets = -1;
	fHCIWindowSize = 0;
	fPacketLength = 0;
	fSavedPacketLength = 0;
	fConnectionHandle = -1;
	
	LOG ("THCILayer::THCILayer\n");
}
	
int THCILayer::ProcessToolBuffer ()
{
	UByte type;
	Long r;
	Boolean c;
	
	LOG ("THCILayer::ProcessToolBuffer (%d %d)\n",
		fTool->fInputBuffer.BufferCount (),
		fSavedPacketLength);

	LOG3 ("(%d %d) - ",
		fTool->fInputBuffer.BufferCount (),
		fSavedPacketLength);

	r = noErr;
	do {
		c = CheckPacketComplete ();
		LOG3 ("%d ", c);
		if (c) {
			switch (fPacket[0]) {
				case 0x02: r = ProcessACLData (); break;
				case 0x04: r = ProcessHCIEvent (); break;
				default:
					LOG ("  unsupported type: 0x%02x, length: %d\n",
						fPacket[0], fTool->fInputBuffer.BufferCount ());
					r = -20001;
					break;
			}
		}
	} while (c && r == noErr);
	
	LOG3 ("(%d %d) %d\n",
		fTool->fInputBuffer.BufferCount (),
		fSavedPacketLength, r);

	return r;
}

Boolean THCILayer::CheckPacketComplete (void)
{
	int i;
	ULong len;
	ULong needed;
	ULong total;
	Boolean complete;
	
	complete = false;
	needed = 0;
	
	fTool->fInputBuffer.BufferCountToNextMarker (&len);
	LOG ("THCILayer::CheckPacketComplete (%d)\n", len);
	if (len > 0) {
		UByte *buffer = fTool->fInputBuffer.fBuffer;
		i = fTool->fInputBuffer.fStart;
		total = fTool->fInputBuffer.fSize;
		switch (buffer[i]) {
			case 0x02:
				needed = 5;
				if (len >= 5) {
					needed += buffer[(i + 3) % total] + (buffer[(i + 4) % total] << 8);
				} else {
					LOG2 ("  Incomplete data packet header\n");
				}
				break;
			case 0x04:
				needed = 3;
				if (len >= 3) {
					needed += buffer[(i + 2) % total];
				} else {
					LOG2 ("  Incomplete event packet header\n");
				}
				break;
			default:
				break;
		}
		
		LOG2 ("  Needed: %d\n", needed);
		if (needed <= len) {
			complete = true;
			fPacketLength = 0;
			while (needed > 0) {
				fTool->fInputBuffer.GetNextByte (&fPacket[fPacketLength]);
				fPacketLength++;
				needed--;
			}
		}
	} else {
		LOG2 ("  Incomplete header\n");
	}
	LOG ("   done\n");
	
	return complete;
}

NewtonErr THCILayer::ProcessACLData (void)
{
	Long r;
	
	LOG ("THCILayer::ProcessACLData\n");
	
	fConnectionHandle = (fPacket[2] & 0x0f) * 256 + fPacket[1];

	LOG ("  Handle: 0x%04x, flags: %d, length: %d\n",
		fConnectionHandle,
		(fPacket[2] & 0xf0) >> 4,
		fPacket[3] + fPacket[4]  * 256);
		
	r = fL2CAP->ProcessACLData (&fPacket[1]);
	
	return r;
}

NewtonErr THCILayer::ProcessHCIEvent (void)
{
	LOG ("THCILayer::ProcessHCIEvent\n  Event code: 0x%02x, length %d, got: %d\n",
		fPacket[1],
		fPacket[2],
		fPacketLength);
		
	switch (fPacket[1]) {
		case 0x01: InquiryComplete (); break;
		case 0x02: InquiryResult (); break;
		case 0x03: ConnectionComplete (); break;
		case 0x04: ConnectionRequest (); break;
		case 0x05: DisconnectionComplete (); break;
		case 0x06: AuthenticationComplete (); break;
		case 0x07: RemoteNameRequestComplete (); break;
		case 0x08: EncryptionChange (); break;
		case 0x09: ChangeConnectionLinkKeyComplete (); break;
		case 0x0a: MasterLinkKeyComplete (); break;
		case 0x0b: ReadRemoteSupportedFeaturesComplete (); break;
		case 0x0c: ReadRemoteVersionInformationComplete (); break;
		case 0x0d: QoSSetupComplete (); break;
		case 0x0e: CommandComplete (); break;
		case 0x0f: CommandStatus (); break;
		case 0x10: HardwareError (); break;
		case 0x11: FlushOccurred (); break;
		case 0x12: RoleChange (); break;
		case 0x13: NumberOfCompletedPackets (); break;
		case 0x14: ModeChange (); break;
		case 0x15: ReturnLinkKeys (); break;
		case 0x16: PINCodeRequest (); break;
		case 0x17: LinkKeyRequest (); break;
		case 0x18: LinkKeyNotification (); break;
		case 0x19: LoopbackCommand (); break;
		case 0x1a: DataBufferOverflow (); break;
		case 0x1b: MaxSlotsChange (); break;
		case 0x1c: ReadClockOffsetComplete (); break;
		case 0x1d: ConnectionPacketTypeChanged (); break;
		case 0x1e: QoSViolation (); break;
		case 0x1f: PageScanModeChange (); break;
		case 0x20: PageScanRepetitionModeChange (); break;
	}
	
	return fTool->ProcessHCIEvent (&fPacket[1]);
}

NewtonErr THCILayer::SendData (UByte *data, Size length, Boolean release)
{
	CBufferList *list;
	CBufferSegment *buffer;
	NewtonErr r;
	ULong n;
	
	LOG ("THCILayer::SendData\n %d (%d)\n", length, fOutstandingPackets);
	if (fOutstandingPackets > fHCIWindowSize) LOGX ("*** Overflow ***\n");

	buffer = CBufferSegment::New ();
	buffer->Init (data, length, release);
	
	list = CBufferList::New ();
	list->Init ();
	list->Insert ((CBuffer *) buffer);
	
	fTool->fOutputBuffer.CopyIn (list, &n);
	
	list->Delete ();
	
	if (fTool->fSerialMiscConfig.txdOffUntilSend == true) {
		fTool->SetTxDTransceiverEnable (true);
		fTool->fSerialMiscConfig.txdOffUntilSend = false;
	}
	
	fTool->fTransferState |= TOOL_SENDING;

	if (fTool->fIdle == false) {
		fTool->ContinueOutputST (true);
	}

	fTool->fIdle = false;
	if (fTool->fChipIdle) {
		fTool->fSerialChip->ConfigureForOutput (true);
	}
	
	fTool->StartOutputST ();
	
	fTool->DriverSendDelay ();
	
	if (fOutstandingPackets != -1) fOutstandingPackets++;	
//	return fTool->PutBytes (list);
	return noErr;
}

//-----------------------------------------------------------------------------
// Data packet
//-----------------------------------------------------------------------------

void THCILayer::Data (Byte flags, UByte *data, Size length, Boolean release)
{
	CBufferList *list;
	CBufferSegment *buffer;
	NewtonErr r;
	Byte header[5];
	ULong n;

	LOG ("THCILayer::Data\n %d (%d)\n", length, fOutstandingPackets);
	if (fOutstandingPackets > fHCIWindowSize) LOGX ("*** Overflow in THCILayer::Data ***\n");

	list = CBufferList::New ();
	list->Init ();

	buffer = CBufferSegment::New ();
	buffer->Init (data, length, release);
	list->InsertFirst ((CBuffer *) buffer);

	header[0] = 0x02;
	header[1] = fConnectionHandle & 0x00ff;
	header[2] = ((fConnectionHandle & 0xff00) >> 8) | (flags << 4);
	header[3] = length & 0x00ff;
	header[4] = (length & 0xff00) >> 8;
	buffer = CBufferSegment::New ();
	buffer->Init (header, 5, false);
	list->InsertFirst ((CBuffer *) buffer);
	
	fTool->fOutputBuffer.CopyIn (list, &n);
	
	list->Delete ();
	
	if (fTool->fSerialMiscConfig.txdOffUntilSend == true) {
		fTool->SetTxDTransceiverEnable (true);
		fTool->fSerialMiscConfig.txdOffUntilSend = false;
	}
	
	fTool->fTransferState |= TOOL_SENDING;

	if (fTool->fIdle == false) {
		fTool->ContinueOutputST (true);
	}

	fTool->fIdle = false;
	if (fTool->fChipIdle) {
		fTool->fSerialChip->ConfigureForOutput (true);
	}
	
	fTool->StartOutputST ();
	if (fOutstandingPackets != -1) fOutstandingPackets++;	

	fTool->DriverSendDelay ();
}

void THCILayer::Data (Byte flags, CBufferList *list, Boolean release)
{
	CBufferSegment *buffer;
	NewtonErr r;
	Byte header[5];
	ULong n;
	Short len;
	
	len = list->GetSize ();

	header[0] = 0x02;
	header[1] = fConnectionHandle & 0x00ff;
	header[2] = ((fConnectionHandle & 0xff00) >> 8) | (flags << 4);
	header[3] = len & 0x00ff;
	header[4] = (len & 0xff00) >> 8;
	
	buffer = CBufferSegment::New ();
	buffer->Init (header, sizeof (header), false);
	list->InsertFirst ((CBuffer *) buffer);
	
	n = len + 5;
	fTool->fOutputBuffer.CopyIn (list, &n);
	
	LOG ("THCILayer::Data %d %d (%d)\n", len, n, fOutstandingPackets);
	if (fOutstandingPackets > fHCIWindowSize) LOGX ("*** Too many outstanding packets THCILayer::Data ***\n");
	if (list->GetSize () > fHCIBufferSize) LOGX ("*** HCI Packet too large (%d > %d) in THCILayer::Data ***\n", list->GetSize (), fHCIBufferSize);

	if (fTool->fSerialMiscConfig.txdOffUntilSend == true) {
		fTool->SetTxDTransceiverEnable (true);
		fTool->fSerialMiscConfig.txdOffUntilSend = false;
	}
	
	fTool->fTransferState |= TOOL_SENDING;

	if (fTool->fIdle == false) {
		fTool->ContinueOutputST (true);
	}

	fTool->fIdle = false;
	if (fTool->fChipIdle) {
		fTool->fSerialChip->ConfigureForOutput (true);
	}

	fTool->StartOutputST ();
	if (fOutstandingPackets != -1) fOutstandingPackets++;

	fTool->DriverSendDelay ();
}

#pragma mark -

//-----------------------------------------------------------------------------
// Link Control Commands, OGF = 0x01
//-----------------------------------------------------------------------------

void THCILayer::Inquiry (void)
{
	LOG ("THCILayer::Inquiry\n");
	fState = HCI_INQUIRY;
	SendData ((UByte *) "\001\001\004\005\063\213\236\014\020", 9);
}

void THCILayer::InquiryCancel (void)
{
}

void THCILayer::PeriodicInquiryMode (void)
{
}

void THCILayer::ExitPeriodicInquiryMode (void)
{
}

void THCILayer::CreateConnection (UByte *bd_addr, Byte psRep, Byte psMode)
{
	UByte packet[17];
	
	LOG ("THCILayer::CreateConnection\n");
	fState = HCI_CREATE_CONNECTION;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\005\004\015", 4);
	memcpy (&packet[4], bd_addr, 6);
	packet[10] = 0x18;
	packet[11] = 0xcc;
	packet[12] = psRep;
	packet[13] = psMode;
	packet[14] = 0x00;
	packet[15] = 0x00;
	packet[16] = 0x00;
	SendData (packet, sizeof (packet));
}

void THCILayer::Disconnect (Short handle, Byte reason)
{
	UByte packet[7];
	
	LOG ("THCILayer::Disconnect %04x\n", handle);
	fState = HCI_DISCONNECT;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\006\004\003", 4);
	SET_SHORT (packet, 4, handle);
	packet[6] = reason;
	SendData (packet, sizeof (packet));
}

void THCILayer::AddSCOConnection (void)
{
}

void THCILayer::AcceptConnectionRequest (UByte *bd_addr, Byte role)
{
	UByte packet[11];
	
	LOG ("THCILayer::AcceptConnectionRequest\n");
	fState = HCI_ACCEPT_CONNECTION_REQUEST;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\011\004\007", 4);
	memcpy (&packet[4], bd_addr, 6);
	packet[10] = role;
	SendData (packet, sizeof (packet));
}

void THCILayer::RejectConnectionRequest (void)
{
}

void THCILayer::LinkKeyRequestReply (UByte *bd_addr, UByte *linkKey)
{
	UByte packet[26];
	
	LOG ("THCILayer::LinkKeyRequestReply\n");
	fState = HCI_LINK_KEY_REQUEST_REPLY;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\013\004\026", 4);
	memcpy (&packet[4], bd_addr, 6);
	memcpy (&packet[10], linkKey, 16);
	for (int i = 0; i < 16; i++) LOG ("%02x ", linkKey[i]); LOG ("\n");
	SendData (packet, sizeof (packet));
}

void THCILayer::LinkKeyRequestNegativeReply (UByte *bd_addr)
{
	UByte packet[10];
	
	LOG ("THCILayer::LinkKeyRequestNegativeReply\n");
	fState = HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\014\004\006", 4);
	memcpy (&packet[4], bd_addr, 6);
	SendData (packet, sizeof (packet));
}

void THCILayer::PINCodeRequestReply (UByte *bd_addr, UByte *PIN, Byte pinLength)
{
	UByte packet[27];
	
	LOG ("THCILayer::PINCodeRequestReply\n");
	fState = HCI_PIN_CODE_REQUEST_REPLY;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\015\004\027", 4);
	memcpy (&packet[4], bd_addr, 6);
	packet[10] = pinLength;
	memcpy (&packet[11], PIN, pinLength);
	SendData (packet, sizeof (packet));
}

void THCILayer::PINCodeRequestNegativeReply (void)
{
}

void THCILayer::ChangeConnectionPacketType (void)
{
}

void THCILayer::AuthenticationRequested (Short handle)
{
	UByte packet[6];
	
	LOG ("THCILayer::AuthenticationRequested\n");
	fState = HCI_AUTHENTICATION_REQUESTED;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\021\004\002", 4);
	SET_SHORT (packet, 4, handle);
	SendData (packet, sizeof (packet));
}

void THCILayer::SetConnectionEncryption (void)
{
}

void THCILayer::ChangeConnectionLinkKey (void)
{
}

void THCILayer::MasterLinkKey (void)
{
}

void THCILayer::RemoteNameRequest (UByte *bdAddr, Byte psRep, Byte psMode)
{
	UByte packet[14];
	
	LOG ("THCILayer::RemoteNameRequest (%d %d)\n", psRep, psMode);
	fState = HCI_PIN_CODE_REQUEST_REPLY;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\031\004\012", 4);
	memcpy (&packet[4], bdAddr, 6);
	packet[10] = psRep;
	packet[11] = psMode;
	packet[12] = 0;
	packet[13] = 0;
	SendData (packet, sizeof (packet));
}

void THCILayer::ReadRemoteSupportedFeatures (void)
{
}

void THCILayer::ReadRemoteVersionInformation (void)
{
}

void THCILayer::ReadClockOffset (void)
{
}

#pragma mark -

//-----------------------------------------------------------------------------
// Link Policy Commands, OGF = 0x02 
//-----------------------------------------------------------------------------

void THCILayer::HoldMode (void)
{
}

void THCILayer::SniffMode (void)
{
}

void THCILayer::ExitSniffMode (void)
{
}

void THCILayer::ParkMode (void)
{
}

void THCILayer::ExitParkMode (void)
{
}

void THCILayer::QoSSetup (void)
{
}

void THCILayer::RoleDiscovery (void)
{
}

void THCILayer::SwitchRole (void)
{
}

void THCILayer::ReadLinkPolicySettings (void)
{
}

void THCILayer::WriteLinkPolicySettings (void)
{
}

#pragma mark -

//-----------------------------------------------------------------------------
// Host Controller & Baseband Commands, OGF = 0x03
//-----------------------------------------------------------------------------

void THCILayer::SetEventMask (void)
{
}

void THCILayer::Reset (void)
{
	LOG ("THCILayer::Reset\n");
	fState = HCI_RESET;
	SendData ((UByte *) "\001\003\014\000", 4);
}

void THCILayer::SetEventFilter (void)
{
}

void THCILayer::Flush (void)
{
}

void THCILayer::ReadPINType (void)
{
}

void THCILayer::WritePINType (void)
{
}

void THCILayer::CreateNewUnitKey (void)
{
}

void THCILayer::ReadStoredLinkKey (void)
{
}

void THCILayer::WriteStoredLinkKey (void)
{
}

void THCILayer::DeleteStoredLinkKey (void)
{
}

void THCILayer::ChangeLocalName (UByte *name)
{
	UByte packet[4 + 248];
	
	LOG ("THCILayer::ChangeLocalName\n");
	fState = HCI_CHANGE_LOCAL_NAME;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, (UByte *) "\001\023\014\370", 4);
	memcpy (&packet[4], name, strlen ((char *) name));
	SendData (packet, sizeof (packet));
}

void THCILayer::ReadLocalName (void)
{
}

void THCILayer::ReadConnectionAcceptTimeout (void)
{
}

void THCILayer::WriteConnectionAcceptTimeout (void)
{
}

void THCILayer::ReadPageTimeout (void)
{
}

void THCILayer::WritePageTimeout (void)
{
}

void THCILayer::ReadScanEnable (void)
{
}

void THCILayer::WriteScanEnable (UByte enable)
{
	LOG ("THCILayer::WriteScanEnable\n");
	fState = HCI_WRITE_SCAN_ENABLE;
	SendData ((UByte *) "\001\032\014\001\003", 5);
}

void THCILayer::ReadPageScanActivity (void)
{
}

void THCILayer::WritePageScanActivity (UShort interval, UShort window)
{
}

void THCILayer::ReadInquiryScanActivity (void)
{
}

void THCILayer::WriteInquiryScanActivity (UShort interval, UShort window)
{
	LOG ("THCILayer::WriteInquiryScanActivity\n");
	fState = HCI_WRITE_INQ_SCAN;
	SendData ((UByte *) "\001\036\014\004\000\004\022\000", 8);
}

void THCILayer::ReadAuthenticationEnable (void)
{
}

void THCILayer::WriteAuthenticationEnable (void)
{
}

void THCILayer::ReadEncryptionMode (void)
{
}

void THCILayer::WriteEncryptionMode (void)
{
}

void THCILayer::ReadClassofDevice (void)
{
}

void THCILayer::WriteClassofDevice (UByte *deviceClass)
{
	UByte packet[7];

	LOG ("THCILayer::WriteClassofDevice\n");
	fState = HCI_WRITE_CLASS_OF_DEVICE;
	memcpy (packet, (UByte *) (UByte *) "\001\044\014\003", 4);
	memcpy (&packet[4], deviceClass, 3);
	SendData (packet, sizeof (packet));
}

void THCILayer::ReadVoiceSetting (void)
{
}

void THCILayer::WriteVoiceSetting (void)
{
}

void THCILayer::ReadAutomaticFlushTimeout (void)
{
}

void THCILayer::WriteAutomaticFlushTimeout (void)
{
}

void THCILayer::ReadNumBroadcastRetransmissions (void)
{
}

void THCILayer::WriteNumBroadcastRetransmissions (void)
{
}

void THCILayer::ReadHoldModeActivity (void)
{
}

void THCILayer::WriteHoldModeActivity (void)
{
}

void THCILayer::ReadTransmitPowerLevel (void)
{
}

void THCILayer::ReadSCOFlowControlEnable (void)
{
}

void THCILayer::WriteSCOFlowControlEnable (void)
{
}

void THCILayer::SetHostControllerToHostFlowControl (void)
{
}

void THCILayer::HostBufferSize (void)
{
	LOG ("THCILayer::HostBufferSize\n");
	fState = HCI_HOST_BUFFER_SIZE;
	SendData ((UByte *) "\001\063\014\007\000\002\000\001\000\000\000", 11);
}

void THCILayer::HostNumberOfCompletedPackets (void)
{
}

void THCILayer::ReadLinkSupervisionTimeout (void)
{
}

void THCILayer::WriteLinkSupervisionTimeout (void)
{
}

void THCILayer::ReadNumberOfSupportedIAC (void)
{
}

void THCILayer::ReadCurrentIACLAP (void)
{
}

void THCILayer::WriteCurrentIACLAP (void)
{
}

void THCILayer::ReadPageScanPeriodMode (void)
{
}

void THCILayer::WritePageScanPeriodMode (void)
{
}

void THCILayer::ReadPageScanMode (void)
{
}

void THCILayer::WritePageScanMode (void)
{
}

#pragma mark -

//-----------------------------------------------------------------------------
// Informational Parameters
//-----------------------------------------------------------------------------

void THCILayer::ReadLocalVersionInformation (void)
{
}

void THCILayer::ReadLocalSupportedFeatures (void)
{
}

void THCILayer::ReadBufferSize (void)
{
	LOG ("THCILayer::ReadBufferSize\n");
	fState = HCI_READ_BUFFER_SIZE;
	SendData ((UByte *) "\001\005\020\000", 4);
}

void THCILayer::ReadCountryCode (void)
{
}

void THCILayer::ReadBdAddr (void)
{
	LOG ("THCILayer::ReadBdAddr\n");
	fState = HCI_READ_BDADDR;
	SendData ((UByte *) "\001\011\020\000", 4);
}

#pragma mark -

//-----------------------------------------------------------------------------
// Status Parameters 
//-----------------------------------------------------------------------------

void THCILayer::ReadFailedContactCounter (void)
{
}

void THCILayer::ResetFailedContactCounter (void)
{
}

void THCILayer::GetLinkQuality (Short handle)
{
	UByte packet[6];
	
	LOG ("THCILayer::GetLinkQuality (0x%04x)\n", handle);
	fState = HCI_GET_LINK_QUALITY;
	memset (packet, 0, sizeof (packet));
	memcpy (packet, "\001\005\024\002", 4);
	SET_SHORT (packet, 4, handle);
	SendData (packet, sizeof (packet));
}

void THCILayer::ReadRSSI (void)
{
}

#pragma mark -

//-----------------------------------------------------------------------------
// Testing Commands 
//-----------------------------------------------------------------------------

void THCILayer::ReadLoopbackMode (void)
{
}

void THCILayer::WriteLoopbackMode (void)
{
}

void THCILayer::EnableDeviceUnderTestMode (void)
{
}

#pragma mark -

//-----------------------------------------------------------------------------
// Possible Events 
//-----------------------------------------------------------------------------

void THCILayer::InquiryComplete (void)
{
	LOG ("THCILayer::InquiryComplete\n");
}

void THCILayer::InquiryResult (void)
{
	int n, i, j;
	
	LOG ("THCILayer::InquiryResult\n");
	
	n = fPacket[3];
	LOG ("  Responses: %d\n", n);
	for (i = 0; i < 1; i++) {
		LOG ("  BD_ADDR: ");
		for (j = 0; j < 6; j++) {
			LOG ("%02x ", fPacket[4 + j + i]);
		}
		
		LOG ("PS: %d %d %d ",
			fPacket[4 + (6 + 0 + i)],
			fPacket[4 + (6 + 1 + i)],
			fPacket[4 + (6 + 2 + i)]);
			
		LOG ("Class: %02x%02x%02x",
			fPacket[4 + 0 + (6 + 3 + i)],
			fPacket[4 + 1 + (6 + 3 + i)],
			fPacket[4 + 2 + (6 + 3 + i)]);
			
		LOG ("\n");
	}
}

void THCILayer::ConnectionComplete (void)
{
	LOG ("THCILayer::ConnectionComplete: 0x%02x\n", fPacket[3]);
	fOutstandingPackets = 0;
	fConnectionHandle = fPacket[4] + (fPacket[5] << 8);
}

void THCILayer::ConnectionRequest (void)
{
	int i;
	
	LOG ("THCILayer::ConnectionRequest\n");
	
	LOG ("  BD_ADDR: ");
	for (i = 0; i < 6; i++) {
		LOG ("%02x ", fPacket[3 + i]);
	}
	LOG ("Class: %02x%02x%02x ",
		fPacket[9],
		fPacket[10],
		fPacket[11]);
	LOG ("Link type: %02x\n",
		fPacket[12]);
}

void THCILayer::DisconnectionComplete (void)
{
	LOG ("THCILayer::DisconnectionComplete\n");
	fConnectionHandle = -1;
	fOutstandingPackets = -1;
}

void THCILayer::AuthenticationComplete (void)
{
	LOG ("THCILayer::AuthenticationComplete\n");
}

void THCILayer::RemoteNameRequestComplete (void)
{
	LOG ("THCILayer::RemoteNameRequestComplete\n");
}

void THCILayer::EncryptionChange (void)
{
	LOG ("THCILayer::EncryptionChange\n");
}

void THCILayer::ChangeConnectionLinkKeyComplete (void)
{
	LOG ("THCILayer::ChangeConnectionLinkKeyComplete\n");
}

void THCILayer::MasterLinkKeyComplete (void)
{
	LOG ("THCILayer::MasterLinkKeyComplete\n");
}

void THCILayer::ReadRemoteSupportedFeaturesComplete (void)
{
	LOG ("THCILayer::ReadRemoteSupportedFeaturesComplete\n");
}

void THCILayer::ReadRemoteVersionInformationComplete (void)
{
	LOG ("THCILayer::ReadRemoteVersionInformationComplete\n");
}

void THCILayer::QoSSetupComplete (void)
{
	LOG ("THCILayer::QoSSetupComplete\n");
}

void THCILayer::CommandComplete (void)
{
	LOG ("THCILayer::CommandComplete: %02x %02d %02d\n",
		fPacket[6], fPacket[4], fPacket[5]);
}

void THCILayer::CommandStatus (void)
{
	LOG ("THCILayer::CommandStatus: 0x%02x\n", fPacket[3]);
}

void THCILayer::HardwareError (void)
{
	LOGX ("THCILayer::HardwareError\n  Code: 0x%02x Packets: %d\n", fPacket[3], fOutstandingPackets);
}

void THCILayer::FlushOccurred (void)
{
	LOG ("THCILayer::FlushOccurred\n");
}

void THCILayer::RoleChange (void)
{
	LOG ("THCILayer::RoleChange\n");
}

void THCILayer::NumberOfCompletedPackets (void)
{
	int i, n;
	Short o;
	
	LOG ("THCILayer::NumberOfCompletedPackets\n");

	n = fPacket[3];
	o = fPacket[4 + (n * 2)] + fPacket[4 + (n * 2) + 1] * 256;

	LOG ("%02x%02x ",
		fPacket[4 + 1],
		fPacket[4]);
	LOG ("%d\n", o);
	
	fOutstandingPackets -= o;
}

void THCILayer::ModeChange (void)
{
	LOG ("THCILayer::ModeChange\n");
}

void THCILayer::ReturnLinkKeys (void)
{
	LOG ("THCILayer::ReturnLinkKeys\n");
}

void THCILayer::PINCodeRequest (void)
{
	LOG ("THCILayer::PINCodeRequest\n");
}

void THCILayer::LinkKeyRequest (void)
{
	LOG ("THCILayer::LinkKeyRequest\n");
}

void THCILayer::LinkKeyNotification (void)
{
	LOG ("THCILayer::LinkKeyNotification\n");
	for (int i = 0; i < 16; i++) LOG ("%02x ", fPacket[9 + i]); LOG ("\n");
}

void THCILayer::LoopbackCommand (void)
{
	LOG ("THCILayer::LoopbackCommand\n");
}

void THCILayer::DataBufferOverflow (void)
{
	LOG ("THCILayer::DataBufferOverflow\n");
}

void THCILayer::MaxSlotsChange (void)
{
	LOG ("THCILayer::MaxSlotsChange\n");
}

void THCILayer::ReadClockOffsetComplete (void)
{
	LOG ("THCILayer::ReadClockOffsetComplete\n");
}

void THCILayer::ConnectionPacketTypeChanged (void)
{
	LOG ("THCILayer::ConnectionPacketTypeChanged\n");
}

void THCILayer::QoSViolation (void)
{
	LOG ("THCILayer::QoSViolation\n");
}

void THCILayer::PageScanModeChange (void)
{
	LOG ("THCILayer::PageScanModeChange\n");
}

void THCILayer::PageScanRepetitionModeChange (void)
{
	LOG ("THCILayer::PageScanRepetitionModeChange\n");
}

#if 0

Boolean THCILayer::CheckPacketComplete (void)
{
	int i;
	ULong len;
	ULong total;
	Boolean complete;
	ULong temp;
	
	complete = false;
	
	LOG2 ("THCILayer::CheckPacketComplete\n  %08x %08x %08x\n", fPacket, fSavedPacket, &fTool->fInputBuffer);
	temp = fSavedPacketLength;
	if (fSavedPacketLength > 0) {
		LOG2 ("  Saved length: %d\n", fSavedPacketLength);
		if (fSavedPacketLength > sizeof (fPacket)) {
			LOGX ("*** Overflow in CheckPacketComplete: %d ***\n", fSavedPacketLength);
			fSavedPacketLength = sizeof (fPacket);
		}
		memcpy (fPacket, fSavedPacket, fSavedPacketLength);
		fPacketLength = fSavedPacketLength;
		fSavedPacketLength = 0;
		LOG2 ("  Packet Length %d\n", fPacketLength);
	} else if (fTool->fInputBuffer.BufferCount () > 0) {
		fTool->fInputBuffer.GetNextByte (&fPacket[0]);
		LOG2 ("  No saved data. Type: 0x%02x\n", fPacket[0]);
		fPacketLength = 1;
	} else {
		return false;
	}
	
	len = fPacketLength;

/*	
	while ((i = fTool->fInputBuffer.GetNextByte (&fPacket[fPacketLength])) == noErr) {
		fPacketLength++;
	}
*/	

	while (fTool->fInputBuffer.BufferCount () > 0) {
		i = fTool->fInputBuffer.GetNextByte (&fPacket[fPacketLength]);
		if (i == 1) {
			LOG3 ("  X: %d\n", fPacketLength);
		}
		fPacketLength++;
	}

	LOG2 ("  Buffer Size: %d\n", fPacketLength);
	
	switch (fPacket[0]) {
		case 0x02: len = 5; break;
		case 0x04: len = 3; break;
		default:
			LOG3 ("xxx: %d %08x %08x\n",
				temp, fPacket, fSavedPacket);
			return false;
	}

	if (fPacketLength >= len) {
		switch (fPacket[0]) {
			case 0x02: len += fPacket[3] + (fPacket[4] << 8); break;
			case 0x04: len += fPacket[2]; break;
		}
		
		LOG2 ("  HCI Length: %d\n", len);
		
		if (fPacketLength > len) {
			LOG2 ("  Saving: %d (%02x)\n", fPacketLength - len, fPacket[len]);
			LOG2 ("  %08x %08x\n", &fSavedPacket[fSavedPacketLength], &fPacket[len]);
			if (fSavedPacketLength + fPacketLength - len > sizeof (fPacket)) {
				printf ("*** Overflow in CheckPacketComplete (2): %d %d %d ***\n", fSavedPacketLength, fPacketLength, len);
				fPacketLength = sizeof (fPacket) - fSavedPacketLength + len;
			}
			memcpy (&fSavedPacket[fSavedPacketLength], &fPacket[len], fPacketLength - len);
			fSavedPacketLength += fPacketLength - len;
			return true;
		} else if (fPacketLength == len) {
			return true;
		}		
	}
	
	LOG2 ("  Incomplete, saving %d\n", fPacketLength);
	
	if (fSavedPacketLength + fPacketLength > sizeof (fPacket)) {
		LOGX ("*** Overflow in CheckPacketComplete (3): %d ***\n", fSavedPacketLength + fPacketLength);
		fPacketLength = sizeof (fPacket) - fSavedPacketLength;
	}
	memcpy (&fSavedPacket[fSavedPacketLength], fPacket, fPacketLength);
	fSavedPacketLength += fPacketLength;
	
	LOG2 ("  %08x\n", &fSavedPacket[fSavedPacketLength]);
	
	return false;
}

#endif