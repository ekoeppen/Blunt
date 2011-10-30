#ifndef __TL2CAPLAYER_H
#define __TL2CAPLAYER_H

class TRFCOMMTool;
class THCILayer;
class TSDPLayer;
class TRFCOMMLayer;

#define L2CAP_IN_MTU_LEN	 672
#define L2CAP_OUT_MTU_LEN	 490

#define CID_NULL 		0x0000
#define CID_SIGNAL		0x0001
#define CID_CL_RCV		0x0002

#define CONT_FOLLOW		0x01
#define CONT_FIRST		0x02

#define BC_P2P			0x00
#define BC_PARK			0x01
#define BC_NONPARK		0x02

#define PSM_SDP			0x0001
#define PSM_RFCOMM		0x0003
#define PSM_TELEPHONY	0x0005

#define CONF_MTU			0x01
#define CONF_FLUSH_TIMEOUT	0x02
#define CONF_QOS			0x03

#define EVT_L2CAP_COMMAND_REJECT 0x01
#define EVT_L2CAP_CONNECTION_REQUEST 0x02
#define EVT_L2CAP_CONNECTION_RESPONSE 0x03
#define EVT_L2CAP_CONFIGURE_REQUEST 0x04
#define EVT_L2CAP_CONFIGURE_RESPONSE 0x05
#define EVT_L2CAP_DISCONNECTION_REQUEST 0x06
#define EVT_L2CAP_DISCONNECTION_RESPONSE 0x07
#define EVT_L2CAP_ECHO_REQUEST 0x08
#define EVT_L2CAP_ECHO_RESPONSE 0x09
#define EVT_L2CAP_INFORMATION_REQUEST 0x0a
#define EVT_L2CAP_INFORMATION_RESPONSE 0x0b

#define MAX_L2CAP_CHANNELS	10
#define L2CAP_CID_BASE		200

#define CONFIGURE_DONE		0
#define CONFIGURE_ONE		1
#define CONFIGURE_TWO		2

typedef enum {
	L2CAP_IDLE,
	L2CAP_CONNECT,
	L2CAP_CONFIGURE,
	L2CAP_ACCEPT,
	L2CAP_CONNECTED,
} L2CAPState;

class Channel
{
public:
	Boolean 				fInUse;
	Short					fProtocol;
	Short					fLocalCID;
	Short					fRemoteCID;
	Short					fHCIHandle;
	Short					fRemoteMTU;
	Byte					fDLCI;
};

class TL2CAPLayer
{
public:
	TRFCOMMTool 			*fTool;
	THCILayer 				*fHCI;
	TSDPLayer				*fSDP;
	TRFCOMMLayer			*fRFCOMM;
	
	Byte					fLogLevel;
	
	L2CAPState				fState;
	
	// Packet identifiers
	
	Boolean					fTimeout;
	Byte					fLocalIdentifier;
	Byte					fRemoteIdentifier;
	Byte					fOutstandingRequestID;
	
	// Information about the current L2CAP input packet
	
	UByte					fInputPacket[L2CAP_IN_MTU_LEN * 2];
	Long					fInputPacketLength;
	Long					fCurrentInputPacketLength;
	Short					fInputPacketCID;
	
	// Information about the current L2CAP output packet
	
	UByte					fOutputPacket[L2CAP_OUT_MTU_LEN];
	Long					fOutputPacketLength;

	// Temporary CID (used in config phase)

	Short					fTempCID;
	
	// Configuration state
	
	Byte					fConfigureState;
	
	// All channels
	
	Channel					fChannel[MAX_L2CAP_CHANNELS + 1];

							TL2CAPLayer ();
	
	int						ProcessACLData (UByte *data);
	
	void					ProcessACLEvent (void);
	int						ProcessL2CAPData (void);
	
	Channel					*OpenChannel (Short prot, Short remote, Short handle);
	void					CloseChannel (Channel *c);
	Channel					*GetChannel (Short remote);
	
	Byte					NextLocalIdentifier (void);
	
	// Commands
	
	void					CommandReject (void);
	void					ConnectionRequest (void);
	void					ConnectionResponse (void);
	void					ConfigureRequest (void);
	void					ConfigureResponse (void);
	void					DisconnectionRequest (void);
	void					DisconnectionResponse (void);
	void					EchoRequest (void);
	void					EchoResponse (void);
	void					InformationRequest (void);
	void					InformationResponse (void);
	
	// Actions
	
	void					SndData (Channel *c, CBufferList *list);
	
	void					SndCommandReject (Channel *c);
	Channel					*SndConnectionRequest (Short prot, Channel *c);
	void					SndConnectionResponse (Channel *c, Short result, Short status);
	void					SndConfigureRequest (Channel *c);
	void					SndConfigureResponse (Channel *c);
	void					SndDisconnectionRequest (Channel *c);
	void					SndDisconnectionResponse (Channel *c);
	void					SndEchoRequest (Channel *c);
	void					SndEchoResponse (Channel *c);
	void					SndInformationRequest (Channel *c);
	void					SndInformationResponse (Short infoType, Short result, Short respLength);
};

#endif