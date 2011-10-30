#include <AEvents.h>
#include <AEventHandler.h>
#include <BufferSegment.h>
#include <CommManagerInterface.h>
#include <CommTool.h>
#include <SerialOptions.h>
#include <CommOptions.h>

#include "THCILayer.h"
#include "TRFCOMMTool.h"
#include "TL2CAPLayer.h"
#include "TSDPLayer.h"
#include "TRFCOMMLayer.h"

#define PF					0x10
#define COMMAND			 	0x02
#define RESPONSE			0x00
#define EA					0x01

TRFCOMMLayer::TRFCOMMLayer (void)
{
	fLogLevel = 0;
	LOG ("TRFCOMMLayer::TRFCOMMLayer\n");

	fL2CAP = NULL;
	fTool = NULL;
	fInitiator = false;
	fOutputPacket = NULL;
	fInputPacket = NULL;
	fState = RFCOMM_IDLE;
	CreateCRCTable ();
}
	
TRFCOMMLayer::~TRFCOMMLayer (void)
{
}
	
int TRFCOMMLayer::ProcessRFCOMMEvent (UByte *data, Channel *c)
{
	UByte control;
	int r;
	Byte DLCI;
	
	LOG ("TRFCOMMLayer::ProcessRFCOMMEvent %08x\n", data);
	r = noErr;
	fChannel = c;
	if ((data[2] & 0x01) != 0x01) {
		fInputPacketLength = (data[2] >> 1) + (data[3] << 7);
		fInputPacket = data + 4;
	} else {
		fInputPacketLength = data[2] >> 1;
		fInputPacket = data + 3;
	}
	DLCI = data[0] >> 2;
	
	if ((data[0] & COMMAND) == COMMAND) fCR = true; else fCR = false;
	if ((data[1] & PF) == PF) fPF = true; else fPF = false;
	
	control = data[1] & 0xef;
	
	LOG (" Address: %d Control: %02x Length: %d\n",
		DLCI, control, fInputPacketLength);
		
	switch (control) {
		case CTRL_SABM:
			if (fPF) SetAsynchronousBalancedMode (DLCI);
			break;
		case CTRL_UA:
			UnnumberedAcknowlegdement (DLCI);
			break;
		case CTRL_DM:
			DisconnectedMode (DLCI);
			break;
		case CTRL_DISC:
			if (fPF) Disconnect (DLCI);
			break;
		case CTRL_UIH:
			r = UnnumberedInformation (DLCI);
			break;
	}
	
	fTool->ProcessRFCOMMEvent (control, data, DLCI);
	
	return r;
}

void TRFCOMMLayer::SetAsynchronousBalancedMode (Byte DLCI)
{
	LOG ("TRFCOMMLayer::SetAsynchronousBalancedMode\n");

	fNegotiationComplete = false;
	fInitiator = false;
	if (DLCI != 0) {
		fChannel->fDLCI = DLCI;
	} else {
		fState = RFCOMM_ACCEPT;
	}
	SndUnnumberedAcknowlegdement (DLCI);
}

void TRFCOMMLayer::UnnumberedAcknowlegdement (Byte DLCI)
{
	LOG ("TRFCOMMLayer::UnnumberedAcknowlegdement (%d)\n", DLCI);
	if (DLCI != 0) {
		fChannel->fDLCI = DLCI;
		fState = RFCOMM_CONNECTED;
	}
}

void TRFCOMMLayer::DisconnectedMode (Byte DLCI)
{
	LOG ("TRFCOMMLayer::DisconnectedMode\n");
}

void TRFCOMMLayer::Disconnect (Byte DLCI)
{
	LOG ("TRFCOMMLayer::Disconnect\n");

	SndUnnumberedAcknowlegdement (DLCI);
	fState = RFCOMM_IDLE;
}

int TRFCOMMLayer::UnnumberedInformation (Byte DLCI)
{
	int r;
	
	LOG ("TRFCOMMLayer::UnnumberedInformation\n");
	r = noErr;
	if (DLCI == 0) {
		ProcessMultiplexerCommands ();
	} else {
		r = ProcessRFCOMMData ();
	}
	return r;
}

void TRFCOMMLayer::SndSetAsynchronousBalancedMode (Byte DLCI)
{
	UByte response[4];

	LOG ("TRFCOMMLayer::SndSetAsynchronousBalancedMode (%d)\n", DLCI);
	fState = RFCOMM_CONNECT;

	fInitiator = true;
	response[0] = (DLCI << 2) | COMMAND | EA;
	response[1] = CTRL_SABM | PF;
	response[2] = (0 << 1) | EA;
	response[3] = CalculateCRC (response, 3);
	SndData (response, sizeof (response));
}

void TRFCOMMLayer::SndUnnumberedAcknowlegdement (Byte DLCI)
{
	UByte response[4];

	LOG ("TRFCOMMLayer::SndUnnumberedAcknowlegdement\n");
	
	if (DLCI != 0) {
		fState = RFCOMM_CONNECTED;
	}
	response[0] = (DLCI << 2) | COMMAND | EA;
	response[1] = CTRL_UA | PF;
	response[2] = (0 << 1) | EA;
	response[3] = CalculateCRC (response, 3);
	SndData (response, sizeof (response));
}

void TRFCOMMLayer::SndDisconnectedMode (Byte DLCI)
{
	LOG ("TRFCOMMLayer::SndDisconnectedMode\n");
}

void TRFCOMMLayer::SndDisconnect (Byte DLCI)
{
	UByte response[4];

	LOG ("TRFCOMMLayer::SndDisconnect\n");

	if (fState == RFCOMM_CONNECTED) {
		response[0] = (DLCI << 2) | COMMAND | EA;
		response[1] = CTRL_DISC | PF;
		response[2] = (0 << 1) | EA;
		response[3] = CalculateCRC (response, 3);
		SndData (response, sizeof (response));
	}
}

void TRFCOMMLayer::SndData (UByte *data, Short length, Boolean release)
{
	CBufferList *list;
	CBufferSegment *buffer;
	
	LOG ("TRFCOMMLayer::SndData\n");

	if (fChannel) {
		buffer = CBufferSegment::New ();
		buffer->Init (data, length, release);
		
		list = CBufferList::New ();
		list->Init ();
		list->Insert ((CBuffer *) buffer);
		
		fL2CAP->SndData (fChannel, list);
		
		list->Delete ();
	} else {
		LOGX ("  No channel available!\n");
	}
}

void TRFCOMMLayer::SndUnnumberedInformation (CBufferList *list)
{
	UByte header[4];
	UByte crc;
	CBufferSegment *buffer;
	Short len;

	len = list->GetSize ();

	LOG ("TRFCOMMLayer::SndUnnumberedInformation (%d %d)\n", fChannel->fDLCI, len);

	header[0] = (fChannel->fDLCI << 2) | COMMAND | EA; 
	header[1] = CTRL_UIH;

	buffer = CBufferSegment::New ();
	if (len > 127) {
		header[2] = (len & 0x007f) << 1;
		header[3] = (len & 0x7f80) >> 7;
		buffer->Init (header, 4, false);
	} else {
		header[2] = (len << 1) | EA;
		buffer->Init (header, 3, false);
	}
	
	list->InsertFirst ((CBuffer *) buffer);

	crc = CalculateCRC (header, 2);

	buffer = CBufferSegment::New ();
	buffer->Init (&crc, sizeof (crc), false);
	list->InsertLast ((CBuffer *) buffer);
	
	fL2CAP->SndData (fChannel, list);
}

void TRFCOMMLayer::ProcessMultiplexerCommands (void)
{
	Byte command;
	Short length;
	Boolean cr;
	
	LOG ("TRFCOMMLayer::ProcessMultiplexerCommands\n");
	
	if ((fInputPacket[0] & 0x01) == 0x01) {
		cr = (fInputPacket[0] & COMMAND) == COMMAND ? true : false;
		command = fInputPacket[0] >> 2;
		
		switch (command) {
			case MPX_PN:
				MPXParameterNegotiation (cr);
				break;
			case MPX_RPN:
				MPXRemotePortNegotiation (cr);
				break;
			case MPX_MSC:
				MPXModemStatusCommand (cr);
				break;
			case MPX_PSC:
			case MPX_CLD:
			case MPX_Test:
			case MPX_FCon:
			case MPX_FCOff:
			case MPX_NSC:
			case MPX_RLS:
			case MPX_SNC:
				LOG ("  %02x\n", command);
				break;
		}
	}
}

int TRFCOMMLayer::ProcessRFCOMMData (void)
{
	Short len;
	int r;

	LOG ("TRFCOMMLayer::ProcessRFCOMMData (%d)\n", fInputPacketLength);
	r = noErr;
	if (fInputPacketLength > 0) {
		LOG ("  %02x (%c)\n", fInputPacket[0], fInputPacket[0]);
		r = fL2CAP->fTool->HandleInputData (fInputPacket, fInputPacketLength);
	}
	
	return r;
}

void TRFCOMMLayer::MPXParameterNegotiation (Boolean command)
{
	Byte negotiationDLCI;
	Byte flowControl;
	UShort frameSize;
	Byte windowSize;
	
	LOG ("TRFCOMMLayer::MPXParameterNegotiation\n");
	
	negotiationDLCI = fInputPacket[2];
	flowControl = fInputPacket[3] >> 4;
	frameSize = fInputPacket[6] + (fInputPacket[7] << 8);
	windowSize = fInputPacket[9];
	LOG ("  DLCI: %d Flow: %02x Frame size: %d, Window: %d\n",
		negotiationDLCI, flowControl, frameSize, windowSize);

	if (command) {
		SndMPXParameterNegotiation (negotiationDLCI, 0, 667, 0);
		fNegotiationComplete = true;
	}
}

void TRFCOMMLayer::MPXRemotePortNegotiation (Boolean command)
{
	LOG ("TRFCOMMLayer::MPXRemotePortNegotiation\n");
}

void TRFCOMMLayer::MPXModemStatusCommand (Boolean command)
{
	Byte DLCI;

	DLCI = fInputPacket[2] >> 2;
	LOG ("TRFCOMMLayer::MPXModemStatusCommand (%d %d %d 0x%02x)\n",
		DLCI, command, fPF, fInputPacket[3]);
	if (command) {
		SndMPXModemStatusCommand (DLCI, false, fPF, fInputPacket[3]);
	}
}

void TRFCOMMLayer::SndMPXParameterNegotiation (Byte negotiationDLCI, Byte flowControl, Short frameSize, Byte windowSize)
{
	UByte response[14];

	LOG ("TRFCOMMLayer::SndMPXParameterNegotiation (%d %d)\n", negotiationDLCI, frameSize);

	response[0] = (0 << 2) | (fInitiator ? COMMAND : RESPONSE) | EA; 
//	response[1] = CTRL_UIH | PF; 
	response[1] = CTRL_UIH | 0; // T68i XXX
	response[2] = ((sizeof (response) - 4) << 1) | EA;
	
	response[3] = (MPX_PN << 2) | (fInitiator ? COMMAND : RESPONSE) | EA;
	response[4] = ((sizeof (response) - 6) << 1) | EA;
	response[5] = negotiationDLCI;
	response[6] = flowControl << 4 | 0; // CL flow control & I-Frames
	response[7] = 0; // Priority
	response[8] = 0; // ACK timer
	response[9] = frameSize & 0x00ff; // Frame size
	response[10] = frameSize >> 8;
	response[11] = 0; // Retransmissions;
	response[12] = windowSize; // Window size
	
	response[13] = CalculateCRC (response, 2);
	SndData (response, sizeof (response));
}

void TRFCOMMLayer::SndMPXRemotePortNegotiation (void)
{
	LOG ("TRFCOMMLayer::SndMPXRemotePortNegotiation\n");
}

void TRFCOMMLayer::SndMPXModemStatusCommand (Byte DLCI, Boolean isCommand, Boolean pf, Byte status)
{
	UByte response[8];

	LOG ("TRFCOMMLayer::SndMPXModemStatusCommand (%d %d %d)\n", DLCI, fInitiator, isCommand);

	response[0] = (0 << 2) | (fInitiator ? COMMAND : RESPONSE) | EA; 
	response[1] = CTRL_UIH | (pf ? PF : 0);
	response[2] = ((sizeof (response) - 4) << 1) | EA;
	
	response[3] = (MPX_MSC << 2) | (isCommand ? COMMAND : RESPONSE) | EA;
	response[4] = ((sizeof (response) - 6) << 1) | EA;
	response[5] = (DLCI << 2) | 0x02 | EA;
	response[6] = status | 0x01;
	
	response[7] = CalculateCRC (response, 2);
	SndData (response, sizeof (response));
}

void TRFCOMMLayer::CreateCRCTable ()
{
	int i ,j;
	UByte data;
	UByte code_word = 0xe0; // pol = x8+x2+x1+1
	UByte sr = 0;			// Shiftregister initiated to zero
	
	for (j = 0; j < 256; j++) {
		data = (UByte) j;
		for (i = 0; i < 8; i++) {
			if ((data & 0x1) ^ (sr & 0x1)) {
				sr >>= 1;
				sr ^= code_word;
			} else {
				sr >>= 1;
			}
			data >>= 1;
			sr &= 0xff;
		}
		fCRCTable[j] = sr;
		sr = 0;
	} 
}

UByte TRFCOMMLayer::CalculateCRC (UByte *data, Byte length)
{
	UByte fcs = 0xff;

	while (length--) {
		fcs = fCRCTable[fcs ^ *data++];
	}
	
	return 0xff-fcs;
} 