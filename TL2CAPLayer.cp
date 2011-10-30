#include <AEvents.h>
#include <AEventHandler.h>
#include <BufferSegment.h>
#include <CommManagerInterface.h>
#include <CommTool.h>
#include <SerialOptions.h>
#include <CommOptions.h>

#include "TRFCOMMTool.h"
#include "THCILayer.h"
#include "TSDPLayer.h"
#include "TL2CAPLayer.h"
#include "TRFCOMMLayer.h"

TL2CAPLayer::TL2CAPLayer (void)
{
	fLogLevel = 0;
	
	LOG ("TL2CAPLayer::TL2CAPLayer\n");

	fTool = NULL;
	fHCI = NULL;
	fSDP = NULL;
	fRFCOMM = NULL;
	fLocalIdentifier = 1;
	fRemoteIdentifier = 0;
	fState = L2CAP_IDLE;
	fTimeout = false;
	memset (fChannel, 0, sizeof (fChannel));
}
	
int TL2CAPLayer::ProcessACLData (UByte *data)
{
	Long len;
	UByte continuation;
	UByte broadcast;
	int r;
	
	LOG ("TL2CAPLayer::ProcessACLData\n");
	
	r = noErr;
	
	len = GET_SHORT (data, 2);
	continuation = (data[1] & 0x30) >> 4;
	broadcast = (data[1] & 0xc0) >> 6;
	
	LOG ("  Flags: %d %d", broadcast, continuation);

	if (continuation == CONT_FIRST) {
		if (len - 4 > sizeof (fInputPacket)) {
			printf ("*** Overflow in ProcessACLData: %d ***\n", len - 4);
			len = sizeof (fInputPacket) - 4;
		}
		memcpy (fInputPacket, &data[8], len - 4);
		fCurrentInputPacketLength = len - 4;
		fInputPacketLength = GET_SHORT (data, 4);
		fInputPacketCID = GET_SHORT (data, 6);
		LOG (" Length: %d CID: %04x\n", fInputPacketLength, fInputPacketCID);
	} else if (continuation == CONT_FOLLOW) {
		if (fCurrentInputPacketLength + len > sizeof (fInputPacket)) {
			printf ("*** Overflow in ProcessACLData (2): %d ***\n", fCurrentInputPacketLength + len);
			fCurrentInputPacketLength = sizeof (fInputPacket) - len;
		}
		memcpy (&fInputPacket[fCurrentInputPacketLength], &data[4], len);
		fCurrentInputPacketLength += len;
		LOG ("\n");
	} else {
		LOG ("\n");
	}
	
	if (fCurrentInputPacketLength == fInputPacketLength) {
		switch (fInputPacketCID) {
			case CID_NULL:
				break;
			case CID_SIGNAL:
				ProcessACLEvent ();
				break;
			case CID_CL_RCV:
				break;
			default:
				r = ProcessL2CAPData ();
				break;
		}
	}
	
	return r;
}

void TL2CAPLayer::ProcessACLEvent (void)
{
	LOG ("TL2CAPLayer::ProcessACLEvent (%d %d %d)\n", fRemoteIdentifier, fOutstandingRequestID, fLocalIdentifier);
	
	switch (fInputPacket[0]) {
		case EVT_L2CAP_COMMAND_REJECT:
			CommandReject ();
			break;
		case EVT_L2CAP_CONNECTION_REQUEST:
			ConnectionRequest ();
			break;
		case EVT_L2CAP_CONNECTION_RESPONSE:
			ConnectionResponse ();
			break;
		case EVT_L2CAP_CONFIGURE_REQUEST:
			ConfigureRequest ();
			break;
		case EVT_L2CAP_CONFIGURE_RESPONSE:
			ConfigureResponse ();
			break;
		case EVT_L2CAP_DISCONNECTION_REQUEST:
			DisconnectionRequest ();
			break;
		case EVT_L2CAP_DISCONNECTION_RESPONSE:
			DisconnectionResponse ();
			break;
		case EVT_L2CAP_ECHO_REQUEST:
			EchoRequest ();
			break;
		case EVT_L2CAP_ECHO_RESPONSE:
			EchoResponse ();
			break;
		case EVT_L2CAP_INFORMATION_REQUEST:
			InformationRequest ();
			break;
		case EVT_L2CAP_INFORMATION_RESPONSE:
			InformationResponse ();
			break;
	}
	
	fTool->ProcessL2CAPEvent (fInputPacket);
}

Channel *TL2CAPLayer::OpenChannel (Short prot, Short remote, Short handle)
{
	int i;
	Channel *c;
	
	i = 0;
	while (i < MAX_L2CAP_CHANNELS && fChannel[i].fInUse) i++;
	
	if (i < MAX_L2CAP_CHANNELS) {
		c = &fChannel[i];
		c->fProtocol = prot;
		c->fLocalCID = L2CAP_CID_BASE + i;
		c->fRemoteCID = remote;
		c->fHCIHandle = handle;
		c->fInUse = true;
		c->fRemoteMTU = 0;
	} else {
		c = NULL;
	}
	
	return c;
}

void TL2CAPLayer::CloseChannel (Channel *c)
{
	c->fInUse = false;
}

Channel *TL2CAPLayer::GetChannel (Short local)
{
	int i;
	Channel *c;
	
/*	
	LOG (">> %d ", local);
	for (i = 0; i < 2; i++) {
		LOG ("(%d %d %d) ",
			fChannel[i].fInUse, fChannel[i].fLocalCID, fChannel[i].fRemoteCID);
	}
*/
	
	i = 0;
	while (i < MAX_L2CAP_CHANNELS &&
		!(fChannel[i].fInUse && fChannel[i].fLocalCID == local)) i++;
	
	if (i < MAX_L2CAP_CHANNELS) {
		c = &fChannel[i];
	} else {
		c = NULL;
	}
	
	return c;
}

Byte TL2CAPLayer::NextLocalIdentifier (void)
{
	LOG ("TL2CAPLayer::NextLocalIdentifier\n  %d %d", fTimeout, fLocalIdentifier);
	if (!fTimeout) {
		fLocalIdentifier++;
		if (fLocalIdentifier == 0) fLocalIdentifier = 1;
		fOutstandingRequestID = fLocalIdentifier;
	} else {
		fTimeout = false;
	}
	LOG (" %d\n", fLocalIdentifier);
	
	return fLocalIdentifier;
}

int TL2CAPLayer::ProcessL2CAPData (void)
{
	Channel *c;
	int r;
	
	LOG ("TL2CAPLayer::ProcessL2CAPData\n");
	
	r = noErr;
	c = GetChannel (fInputPacketCID);
	if (c != NULL) {
		switch (c->fProtocol) {
			case PSM_SDP:
				fSDP->ProcessSDPEvent (fInputPacket, c);
				break;
			case PSM_RFCOMM:
				r = fRFCOMM->ProcessRFCOMMEvent (fInputPacket, c);
				break;
			default:
				break;
		}
	} else {
		LOG ("  Channel %d not found\n", GET_SHORT (fInputPacket, 2));
	}
	
	return r;
}

void TL2CAPLayer::CommandReject (void)
{
	LOG ("TL2CAPLayer::CommandReject (%d)\n", GET_SHORT (fInputPacket, 4));
}

void TL2CAPLayer::ConnectionRequest (void)
{
	Short prot;
	Short CID;
	Channel *c;
	
	LOG ("TL2CAPLayer::ConnectionRequest\n");

	CID = GET_SHORT (fInputPacket, 6);
	prot = GET_SHORT (fInputPacket, 4);
	LOG ("  Protocol: %d Source: %d\n", prot, CID);
	
	c = OpenChannel (prot, CID, fHCI->fConnectionHandle);
	if (c != NULL) {
		fRemoteIdentifier = fInputPacket[1];
		
		LOG ("  New channel: %08x, %d %d %d\n",
			c, c->fProtocol, c->fLocalCID, c->fRemoteCID);
		
		fState = L2CAP_CONFIGURE;
		fConfigureState = CONFIGURE_TWO;
		
		if (prot == PSM_SDP || prot == PSM_RFCOMM) {
			SndConnectionResponse (c, 0x0001, 0x0000);
			fTool->StartTimer (L2CAP_CONNECTION_RESPONSE, 500, c);
		} else {
			SndConnectionResponse (c, 0x0002, 0x0000);
		}
	} else {
		LOGX ("  ERROR: all channels busy!\n");
	}
}

void TL2CAPLayer::ConnectionResponse (void)
{
	Channel *c;
	Short result;

	result = GET_SHORT (fInputPacket, 8);
	fRemoteIdentifier = fInputPacket[1];
	LOG ("TL2CAPLayer::ConnectionResponse (%d %d)\n", result, fInputPacket[1]);
	LOG (" Remote CID: %d, Local CID: %d\n", GET_SHORT (fInputPacket, 4), GET_SHORT (fInputPacket, 6));
	if (result == 0 || result == 1) {
		c = GetChannel (GET_SHORT (fInputPacket, 6));
		if (c != NULL) {
			if (fRemoteIdentifier == fOutstandingRequestID) {
				fOutstandingRequestID = 0;
				c->fRemoteCID = GET_SHORT (fInputPacket, 4);
				LOG ("Channel: %08x Remote CID: %d Local CID: %d\n", c, c->fRemoteCID, c->fLocalCID);
				fConfigureState = CONFIGURE_TWO;
				fState = L2CAP_CONFIGURE;
			} else {
				LOG ("  Unmatched response (%d %d)\n",
					fOutstandingRequestID, fRemoteIdentifier);
			}
		} else {
			LOG ("  Channel %d not found\n", GET_SHORT (fInputPacket, 6));
		}
	}
}

void TL2CAPLayer::ConfigureRequest (void)
{
	int i;
	Short len;
	Short flags;
	Channel *c;
	
	LOG ("TL2CAPLayer::ConfigureRequest (%d)\n", GET_SHORT (fInputPacket, 4));

	c = GetChannel (GET_SHORT (fInputPacket, 4));
	if (c != NULL) {
		LOG ("Channel: %08x Remote CID: %d Local CID: %d\n", c, c->fRemoteCID, c->fLocalCID);
		fRemoteIdentifier = fInputPacket[1];
		
		len = GET_SHORT (fInputPacket, 2) - 4;
		flags = GET_SHORT (fInputPacket, 6);
		fTempCID = c->fRemoteCID;
	
		LOG ("  ID: %d Length: %d ", fRemoteIdentifier, len);
		
		i = 8;
		while (i - 8 < len) {
			switch (fInputPacket[i]) {
				case CONF_MTU:
					c->fRemoteMTU = GET_SHORT (fInputPacket, i + 2);
					LOG ("MTU: %d ", c->fRemoteMTU);
					break;
				case CONF_FLUSH_TIMEOUT:
					LOG ("FTO: %d", GET_SHORT (fInputPacket, i + 2));
					break;
				case CONF_QOS:
					LOG ("QoS: ");
					break;
			}
			i += fInputPacket[i + 1] + 2;
		};
		LOG ("\n");
		
		fConfigureState--;
		SndConfigureResponse (c);
	} else {
		LOG ("  Channel %d not found\n", GET_SHORT (fInputPacket, 4));
	}
}

void TL2CAPLayer::ConfigureResponse (void)
{
	int i;
	Short len;
	Short flags;
	Channel *c;
	
	LOG ("TL2CAPLayer::ConfigureResponse\n");
	c = GetChannel (GET_SHORT (fInputPacket, 4));
	if (c != NULL) {
		fRemoteIdentifier = fInputPacket[1];
		LOG ("  Length %d Flags: %04x Result: %d\n",
			GET_SHORT (fInputPacket, 2),
			GET_SHORT (fInputPacket, 6),
			GET_SHORT (fInputPacket, 8));
			
		if (fRemoteIdentifier == fOutstandingRequestID) {
			fOutstandingRequestID = 0;
			fConfigureState--;
			if (fConfigureState == 0) {
				fState = L2CAP_CONNECTED;
			}
		} else {
			LOG ("  Unmatched response (%d %d)\n",
				fOutstandingRequestID, fRemoteIdentifier);
		}
	} else {
		LOG ("  Channel %d not found\n", GET_SHORT (fInputPacket, 4));
	}
}

void TL2CAPLayer::DisconnectionRequest (void)
{
	Short len;
	Short flags;
	Channel *c;

	LOG ("TL2CAPLayer::DisconnectionRequest\n");
	c = GetChannel (GET_SHORT (fInputPacket, 4));	
	if (c != NULL) {
		LOG ("  DCID: %d SCID: %d\n", c->fLocalCID, c->fRemoteCID);
		
		fRemoteIdentifier = fInputPacket[1];
		if (c->fDLCI> 0) fState = L2CAP_IDLE;
		SndDisconnectionResponse (c);
		CloseChannel (c);
	} else {
		LOG ("  Channel %d not found\n", GET_SHORT (fInputPacket, 4));
	}
}

void TL2CAPLayer::DisconnectionResponse (void)
{
	LOG ("TL2CAPLayer::DisconnectionResponse\n");
}

void TL2CAPLayer::EchoRequest (void)
{
	LOG ("TL2CAPLayer::EchoRequest\n");
}

void TL2CAPLayer::EchoResponse (void)
{
	LOG ("TL2CAPLayer::EchoResponse\n");
}

void TL2CAPLayer::InformationRequest (void)
{
	Channel *c;

	LOG ("TL2CAPLayer::InformationRequest\n");
	SndInformationResponse (GET_SHORT (fInputPacket, 4), 1, 0);
}

void TL2CAPLayer::InformationResponse (void)
{
	LOG ("TL2CAPLayer::InformationResponse\n");
}


void TL2CAPLayer::SndData (Channel *c, CBufferList *list)
{
	UByte header[4];
	CBufferSegment *buffer;

	LOG ("TL2CAPLayer::SndData (cid: 0x%02x)\n", c->fRemoteCID);

	SET_SHORT (header, 0, list->GetSize ());
	SET_SHORT (header, 2, c->fRemoteCID);
	
	buffer = CBufferSegment::New ();
	buffer->Init (header, sizeof (header), false);
	list->InsertFirst ((CBuffer *) buffer);
	fHCI->Data (CONT_FIRST, list);
}

void TL2CAPLayer::SndCommandReject (Channel *c)
{
}

Channel *TL2CAPLayer::SndConnectionRequest (Short prot, Channel *c)
{
	UByte data[12];
	
	fState = L2CAP_CONNECT;
	fRemoteIdentifier = 0;

	if (c == NULL) c = OpenChannel (prot, 0, fHCI->fConnectionHandle);
	
	SET_SHORT (data, 0, sizeof (data) - 4);
	SET_SHORT (data, 2, CID_SIGNAL);
	
	data[4] = EVT_L2CAP_CONNECTION_REQUEST;
	data[5] = NextLocalIdentifier ();
	SET_SHORT (data, 6, sizeof (data) - 8);
	
	SET_SHORT (data, 8, prot);
	SET_SHORT (data, 10, c->fLocalCID);

	LOG ("TL2CAPLayer::SndConnectionRequest\n  ID: %d CID: %d\n",
		fLocalIdentifier, c->fLocalCID);
	
	fHCI->Data (CONT_FIRST, data, sizeof (data));
	
	return c;
}

void TL2CAPLayer::SndConnectionResponse (Channel *c, Short result, Short status)
{
	UByte data[16];
	
	LOG ("TL2CAPLayer::SndConnectionResponse (%d to %d)\n",
		c->fLocalCID, c->fRemoteCID);

	SET_SHORT (data, 0, 12);
	SET_SHORT (data, 2, CID_SIGNAL);
	
	data[4] = EVT_L2CAP_CONNECTION_RESPONSE;
	data[5] = fRemoteIdentifier;
	SET_SHORT (data, 6, 8);
	
	SET_SHORT (data, 8, c->fLocalCID);
	SET_SHORT (data, 10, c->fRemoteCID);
	SET_SHORT (data, 12, result);
	SET_SHORT (data, 14, status);
	
	fHCI->Data (CONT_FIRST, data, 16);
}

void TL2CAPLayer::SndConfigureRequest (Channel *c)
{
	UByte data[16];
	
	fRemoteIdentifier = 0;

	SET_SHORT (data, 0, sizeof (data) - 4);
	SET_SHORT (data, 2, CID_SIGNAL);
	
	data[4] = EVT_L2CAP_CONFIGURE_REQUEST;
	data[5] = NextLocalIdentifier ();
	SET_SHORT (data, 6, sizeof (data) - 8);
	
	SET_SHORT (data, 8, c->fRemoteCID);
	SET_SHORT (data, 10, 0);
	data[12] = CONF_MTU;
	data[13] = 2;
	SET_SHORT (data, 14, L2CAP_IN_MTU_LEN);
	
	LOG ("TL2CAPLayer::SndConfigureRequest (Id %d, %d to %d)\n",
		fLocalIdentifier, c->fLocalCID, c->fRemoteCID);

	fHCI->Data (CONT_FIRST, data, sizeof (data));
}

void TL2CAPLayer::SndConfigureResponse (Channel *c)
{
	UByte data[18];
	Short len;

	if (c->fRemoteMTU != 0) { 
		len = 18;
	} else {
		len = 14;
	}
	
	LOG ("TL2CAPLayer::SndConfigureResponse (Id %d, %d to %d)\n",
		fRemoteIdentifier, c->fLocalCID, c->fRemoteCID);

	SET_SHORT (data, 0, len - 4);
	SET_SHORT (data, 2, CID_SIGNAL);
	
	data[4] = EVT_L2CAP_CONFIGURE_RESPONSE;
	data[5] = fRemoteIdentifier;
	SET_SHORT (data, 6, len - 8);

	SET_SHORT (data, 8, c->fRemoteCID);
	SET_SHORT (data, 10, 0);
	SET_SHORT (data, 12, 0);

	if (c->fRemoteMTU != 0) {
		LOG ("  Accepting MTU %d\n", c->fRemoteMTU);
		data[14] = CONF_MTU;
		data[15] = 2;
		SET_SHORT (data, 16, c->fRemoteMTU);
	} else {
//		c->fRemoteMTU = 672;
	}
	
	fHCI->Data (CONT_FIRST, data, len);
}

void TL2CAPLayer::SndDisconnectionRequest (Channel *c)
{
	UByte data[12];
	
	SET_SHORT (data, 0, sizeof (data) - 4);
	SET_SHORT (data, 2, CID_SIGNAL);
	
	data[4] = EVT_L2CAP_DISCONNECTION_REQUEST;
	data[5] = NextLocalIdentifier ();
	SET_SHORT (data, 6, sizeof (data) - 8);
	
	SET_SHORT (data, 8, c->fRemoteCID);
	SET_SHORT (data, 10, c->fLocalCID);
	
	LOG ("TL2CAPLayer::SndDisconnectionRequest (%d to %d)\n",
		c->fLocalCID, c->fRemoteCID);

	fHCI->Data (CONT_FIRST, data, sizeof (data));
}

void TL2CAPLayer::SndDisconnectionResponse (Channel *c)
{
	UByte data[12];
	
	LOG ("TL2CAPLayer::SndDisconnectionResponse\n");

	SET_SHORT (data, 0, 8);
	SET_SHORT (data, 2, CID_SIGNAL);
	
	data[4] = EVT_L2CAP_DISCONNECTION_RESPONSE;
	data[5] = fRemoteIdentifier;
	SET_SHORT (data, 6, 4);
	
	SET_SHORT (data, 8, c->fLocalCID);	
	SET_SHORT (data, 10, c->fRemoteCID);
	
	fHCI->Data (CONT_FIRST, data, 12);
}

void TL2CAPLayer::SndEchoRequest (Channel *c)
{
	LOG ("TL2CAPLayer::SndEchoRequest\n");
}

void TL2CAPLayer::SndEchoResponse (Channel *c)
{
	LOG ("TL2CAPLayer::SndEchoResponse\n");
}

void TL2CAPLayer::SndInformationRequest (Channel *c)
{
	LOG ("TL2CAPLayer::SndInformationRequest\n");
}

void TL2CAPLayer::SndInformationResponse (Short infoType, Short result, Short respLength)
{
	UByte data[12];

	LOG ("TL2CAPLayer::SndInformationResponse\n");
	LOG (" Type: %d, result: %d\n", infoType, result);

	SET_SHORT (data, 0, 8);
	SET_SHORT (data, 2, CID_SIGNAL);
	
	data[4] = EVT_L2CAP_INFORMATION_RESPONSE;
	data[5] = fRemoteIdentifier;
	SET_SHORT (data, 6, 4 + respLength);
	
	SET_SHORT (data, 8, infoType);	
	SET_SHORT (data, 10, result);
	
	fHCI->Data (CONT_FIRST, data, sizeof (data));
}
