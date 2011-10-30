#ifndef __TSDPPLAYER_H
#define __TSDPPLAYER_H

class TRFCOMMTool;
class THCILayer;
class TL2CAPLayer;
class Channel;

#define SDP_PDU_ERROR_RESPONSE 						0x01
#define SDP_PDU_SERVICE_SEARCH_REQUEST 				0x02
#define SDP_PDU_SERVICE_SEARCH_RESPONSE 			0x03
#define SDP_PDU_SERVICE_ATTRIBUTE_REQUEST 			0x04
#define SDP_PDU_SERVICE_ATTRIBUTE_RESPONSE 			0x05
#define SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_REQUEST 	0x06
#define SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_RESPONSE 	0x07

#define UUID_SDP									0x0001
#define UUID_RFCOMM									0x0003
#define UUID_OBEX									0x0008
#define UUID_L2CAP									0x0100
#define UUID_SERIAL_PORT							0x1101
#define UUID_PPP_LAN								0x1102
#define UUID_DUN									0x1103
#define UUID_OBEX_PUSH								0x1105
#define UUID_OBEX_FILETRANSFER						0x1106

class TSDPLayer
{
public:
	TL2CAPLayer 			*fL2CAP;
	TRFCOMMTool				*fTool;

	Byte					fLogLevel;
	
	UByte					*fPacket;
	Short					fPacketLength;
	Short					fLocalTID;
	Short					fRemoteTID;
	Channel					*fChannel;

							TSDPLayer (void);
							
	void					ProcessSDPEvent (UByte *data, Channel *c);
	
	// Utility functions

	void					GetDataElementLength (UByte *data, ULong& length, Byte& fieldLength);	
	void					GetSequenceLength (UByte *data, ULong& length, Byte& fieldLength);
	void					GetUUIDFromSequence (UByte *data, ULong& UUID, Byte& fieldLength);
	void					GetAttrRangeFromSequence (UByte *data, ULong& attr, Byte& fieldLength);
	void					GetInteger (UByte *data, Long& value, Byte& fieldLength);
	
	int 					GetElementSize (UByte* data);
	UByte* 					MakeUInt (UByte* buffer, unsigned int data);
	UByte* 					MakeUUID (UByte* buffer, unsigned int uuid);
	UByte* 					MakeAttribute (UByte* buffer, unsigned short attr);
	UByte* 					MakeString (UByte* buffer, char *data);
	UByte* 					MakeSequence (UByte* buffer, int numElements);
	// Events

	void					ErrorResponse (void);
	void					ServiceSearchRequest (void);
	void					ServiceSearchResponse (void);
	void					ServiceAttributeRequest (void);
	void					ServiceAttributeResponse (void);
	void					ServiceSearchAttributeRequest (void);
	void					ServiceSearchAttributeResponse (void);

	// Actions

	void					SndErrorResponse (void);
	void					SndServiceSearchRequest (void);
	void					SndServiceSearchResponse (ULong *UUID, Byte count);
	void					SndServiceAttributeRequest (void);
	void					SndServiceAttributeResponse (ULong handle);
	void					SndServiceSearchAttributeRequest (ULong UUID);
	void					SndServiceSearchAttributeResponse (ULong *UUID, Byte UUIDCount, ULong *attr, Byte attrCount);
	
	void					SndData (UByte *data, Short length, Boolean release = false);
};

#endif