#ifndef __TRFCOMMLAYER_H
#define __TRFCOMMLAYER_H

class TRFCOMMTool;
class THCILayer;
class TL2CAPLayer;
class Channel;

#define CTRL_SABM	0x2f
#define CTRL_UA		0x63
#define CTRL_DM		0x0f
#define CTRL_DISC	0x43
#define CTRL_UIH	0xef

#define MPX_PN		0x20
#define MPX_PSC		0x10
#define MPX_CLD		0x30
#define MPX_Test	0x08
#define MPX_FCon	0x28
#define MPX_FCOff	0x18
#define MPX_MSC		0x38
#define MPX_NSC		0x04
#define MPX_RPN		0x24
#define MPX_RLS		0x14
#define MPX_SNC		0x34

#define RFCOMM_MTU_LEN 128

typedef enum {
	RFCOMM_IDLE,
	RFCOMM_CONNECT,
	RFCOMM_ACCEPT,
	RFCOMM_CONNECTED,
} RFCOMMState;

class TRFCOMMLayer
{
public:
	TL2CAPLayer 			*fL2CAP;
	TRFCOMMTool				*fTool;
		
	Byte					fLogLevel;
	
	RFCOMMState				fState;
	
	UByte					*fInputPacket;
	UShort					fInputPacketLength;
	
	UByte					*fOutputPacket;
	UShort					fOutputPacketLength;
	
	Channel					*fChannel;
	Boolean					fCR;
	Boolean					fPF;
	Boolean					fInitiator;
	
	Boolean					fConfigured;
	
	Boolean					fNegotiationComplete;
	
	UByte					fCRCTable[256];

							TRFCOMMLayer (void);
							~TRFCOMMLayer (void);
							
	int						ProcessRFCOMMEvent (UByte *data, Channel *c);
	
	void					SetAsynchronousBalancedMode (Byte DLCI);
	void					UnnumberedAcknowlegdement (Byte DLCI);
	void					DisconnectedMode (Byte DLCI);
	void					Disconnect (Byte DLCI);
	int						UnnumberedInformation (Byte DLCI);

	void					SndSetAsynchronousBalancedMode (Byte DLCI);
	void					SndUnnumberedAcknowlegdement (Byte DLCI);
	void					SndDisconnectedMode (Byte DLCI);
	void					SndDisconnect (Byte DLCI);
	void					SndUnnumberedInformation (CBufferList *data);
	
	void					SndData (UByte *data, Short length, Boolean release = false);

	void					ProcessMultiplexerCommands (void);
	int						ProcessRFCOMMData (void);

	void					MPXParameterNegotiation (Boolean command);
	void					MPXRemotePortNegotiation (Boolean command);
	void					MPXModemStatusCommand (Boolean command);

	void					SndMPXParameterNegotiation (Byte negotiationDLCI, Byte flowControl, Short frameSize, Byte windowSize);
	void					SndMPXRemotePortNegotiation (void);
	void					SndMPXModemStatusCommand (Byte DLCI, Boolean isCommand, Boolean pf, Byte status);
	
	void					CreateCRCTable (void);
	UByte					CalculateCRC (UByte *data, Byte Length);
};

#endif