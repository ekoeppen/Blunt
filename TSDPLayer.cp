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

#define GET_DATA_TYPE(b) 	(((b)[0] & 0xf8) >> 3)
#define GET_DATA_WIDTH(b) 	((b)[0] & 0x07)
#define GET_DATA_BYTE(b) 	((b)[1])
#define GET_DATA_SHORT(b) 	(((b)[1] << 8) + (b)[2])
#define GET_DATA_LONG(b)	(*(Long *) ((b) + 1))
#define GET_DATA_ULONG(b)	(*(ULong *) ((b) + 1))

#define TYPE_NIL			0
#define TYPE_UINT			1
#define TYPE_INT			2
#define TYPE_UUID			3
#define TYPE_STRING			4
#define TYPE_BOOLEAN		5
#define TYPE_SEQUENCE		6
#define TYPE_ALTERNATIVE	7
#define TYPE_URL			8

#define SIZE_BYTE			0
#define SIZE_SHORT			1
#define SIZE_LONG			2
#define SIZE_8BYTE			3
#define SIZE_16BYTE			4
#define SIZE_BYTE_LEN		5
#define SIZE_SHORT_LEN		6
#define SIZE_LONG_LEN		7

#define MAKE_HEADER(type, width) ((type << 3) | width)

#define ATTR_SERVICE_RECORD_HANDLE 					0x0000
#define ATTR_SERVICE_CLASS_ID_LIST 					0x0001 
#define ATTR_SERVICE_RECORD_STATE 					0x0002 
#define ATTR_SERVICE_ID 							0x0003 
#define ATTR_PROTOCOL_DESCRIPTOR_LIST 				0x0004 
#define ATTR_BROWSE_GROUP_LIST 						0x0005 
#define ATTR_LANGUAGE_BASE_ATTRIBUTE_ID_LIST 		0x0006 
#define ATTR_SERVICE_INFO_TIME_TO_LIVE 				0x0007 
#define ATTR_SERVICE_AVAILABILITY 					0x0008 
#define ATTR_BLUETOOTH_PROFILE_DESCRIPTOR_LIST 		0x0009 
#define ATTR_DOCUMENTATION_URL 						0x000A 
#define ATTR_CLIENT_EXECUTABLE_URL 					0x000B 
#define ATTR_ICON_URL 								0x000C 
#define ATTR_ADDITIONAL_PROTOCOL_DESCRIPTOR_LISTS	0x000D 
#define ATTR_SERVICE_NAME							0x0100
#define ATTR_GROUP_ID 								0x0200 
#define ATTR_IP_SUBNET 								0x0200
#define ATTR_VERSION_NUMBER_LIST 					0x0200 
#define ATTR_SERVICE_DATABASE_STATE 				0x0201

#define UUID_SDP									0x0001
#define UUID_RFCOMM									0x0003
#define UUID_OBEX									0x0008
#define UUID_L2CAP									0x0100
#define UUID_SERIAL_PORT							0x1101
#define UUID_PPP_LAN								0x1102
#define UUID_DUN									0x1103
#define UUID_SYNC									0x1104
#define UUID_OBEX_PUSH								0x1105
#define UUID_OBEX_FILETRANSFER						0x1106
#define UUID_PNP_INFO								0x1200

#define SRH_SERIAL_PORT							0x00010000
#define SRH_OBEX_PUSH							0x00010001
#define SRH_OBEX_FILETRANSFER					0x00010003
#define SRH_NEWTON_DOCK							0x00010004

#define EVT_SDP_ERROR_RESPONSE 						0x01
#define EVT_SDP_SERVICE_SEARCH_REQUEST 				0x02
#define EVT_SDP_SERVICE_SEARCH_RESPONSE 			0x03
#define EVT_SDP_SERVICE_ATTRIBUTE_REQUEST 			0x04
#define EVT_SDP_SERVICE_ATTRIBUTE_RESPONSE 			0x05
#define EVT_SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST 	0x06
#define EVT_SDP_SERVICE_SEARCH_ATTRIBUTE_RESPONSE 	0x07
#define EVT_SDP_TIMER								0xff

#define RFCOMM_SERIAL_PORT							4
#define RFCOMM_OBEX_PUSH							5
#define RFCOMM_OBEX_FILETRANSFER					6

static const ULong kQueriedServices[] = {
	UUID_SERIAL_PORT,
	UUID_PPP_LAN,
	UUID_DUN,
	UUID_OBEX_PUSH,
	UUID_OBEX_FILETRANSFER
};

static const kElementSize[] = {
	1, 2, 4, 8, 16
};

TSDPLayer::TSDPLayer (void)
{
	fLogLevel = 0;

	LOG ("TSDPLayer::TSDPLayer\n");

	fL2CAP = NULL;
	fTool = NULL;
	fLocalTID = 0;
	fRemoteTID = 0;
}
	
void TSDPLayer::ProcessSDPEvent (UByte *data, Channel *c)
{
	Byte PDU;
	Short TID;
	Short length;
	
	fRemoteTID = data[1] * 256 + data[2];
	fPacketLength = data[3] * 256 + data[4];
	fPacket = data + 5;
	fChannel = c;
	
	switch (data[0]) {
		case SDP_PDU_ERROR_RESPONSE:
			ErrorResponse ();
			break;
		case SDP_PDU_SERVICE_SEARCH_REQUEST:
			ServiceSearchRequest ();
			break;
		case SDP_PDU_SERVICE_SEARCH_RESPONSE:
			ServiceSearchResponse ();
			break;
		case SDP_PDU_SERVICE_ATTRIBUTE_REQUEST:
			ServiceAttributeRequest ();
			break;
		case SDP_PDU_SERVICE_ATTRIBUTE_RESPONSE:
			ServiceAttributeResponse ();
			break;
		case SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_REQUEST:
			ServiceSearchAttributeRequest ();
			break;
		case SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_RESPONSE:
			ServiceSearchAttributeResponse ();
			break;
	}
	
	fTool->ProcessSDPEvent (data);
}

void TSDPLayer::GetDataElementLength (UByte *data, ULong& length, Byte& fieldLength)
{
	fieldLength = 1;
	switch (GET_DATA_WIDTH (data)) {
		case 0: length = 1; break;
		case 1: length = 2; break;
		case 2: length = 4; break;
		case 3: length = 8; break;
		case 4: length = 16; break;
		case 5: fieldLength += 1; length = GET_DATA_BYTE (data); break;
		case 6: fieldLength += 2; length = GET_DATA_SHORT (data); break;
		case 7: fieldLength += 4; length = GET_DATA_LONG (data); break;
	}
}

void TSDPLayer::GetSequenceLength (UByte *data, ULong& length, Byte& fieldLength)
{
	fieldLength = -1;
	
	if (GET_DATA_TYPE (data) == 6) {
		switch (GET_DATA_WIDTH (data)) {
			case 5:
				length = GET_DATA_BYTE (data);
				fieldLength = 2;
				break;
			case 6:
				length = GET_DATA_SHORT (data);
				fieldLength = 3;
				break;
			case 7:
				length = GET_DATA_LONG (data);
				fieldLength = 5;
				break;
		}
	}

	LOG2 ("  SL: %d FL: %d\n", length, fieldLength);
}

void TSDPLayer::GetInteger (UByte *data, Long& value, Byte& fieldLength)
{
	switch (GET_DATA_WIDTH (data)) {
		case 0: value = GET_DATA_BYTE (data); fieldLength = 2; break;
		case 1: value = GET_DATA_SHORT (data); fieldLength = 3; break;
		case 2: value = GET_DATA_LONG (data); fieldLength = 5; break;
	}
}

void TSDPLayer::GetUUIDFromSequence (UByte *data, ULong& UUID, Byte& fieldLength)
{
	fieldLength = -1;
	
	if (GET_DATA_TYPE (data) == 3) {
		switch (GET_DATA_WIDTH (data)) {
			case 1:
				UUID = GET_DATA_SHORT (data);
				fieldLength = 2;
				break;
			case 2:
				UUID = GET_DATA_LONG (data);
				fieldLength = 4;
				break;
			case 4:
				UUID = 0;
				fieldLength = 16;
				break;
		}
	}

	if (fieldLength > 0) LOG2 ("  UUID: %04x FL: %d\n", UUID, fieldLength);
}

void TSDPLayer::GetAttrRangeFromSequence (UByte *data, ULong& attr, Byte& fieldLength)
{
	fieldLength = -1;
	
	attr = 0;
	if (GET_DATA_TYPE (data) == 1) {
		switch (GET_DATA_WIDTH (data)) {
			case 2:
				attr = (data[1] << 24) + (data[2] << 16) + (data[4] << 8) + data[4];
				fieldLength = 5;
				break;
		}
	}

	if (fieldLength > 0) LOG2 ("  {Attr: %08x FL: %d}", attr, fieldLength);
}

void TSDPLayer::ErrorResponse (void)
{
	LOG ("TSDPLayer::ErrorResponse\n");
}

void TSDPLayer::ServiceSearchRequest (void)
{
	int i, j;
	Byte fieldLength;
	ULong sequenceLength;
	ULong UUID[12];
	int UUIDCount;
	
	LOG ("TSDPLayer::ServiceSearchRequest\n  ");

	i = 0; j = 0;
	UUIDCount = 0;
	GetSequenceLength (fPacket + i, sequenceLength, fieldLength);
	i += fieldLength;
	
	while (fieldLength != -1 && j < sequenceLength && UUIDCount < 12) {
		GetUUIDFromSequence (fPacket + i, UUID[UUIDCount], fieldLength);
		if (fieldLength > 0) {
			LOG ("%04x ", UUID[UUIDCount]);
			i += fieldLength; j += fieldLength; UUIDCount++;
		}
	}
	LOG ("\n");
	
	SndServiceSearchResponse (UUID, UUIDCount);
}

void TSDPLayer::ServiceSearchResponse (void)
{
	LOG ("TSDPLayer::ServiceSearchResponse\n");
}

void TSDPLayer::ServiceAttributeRequest (void)
{

	int i, j;
	Byte fieldLength;
	ULong sequenceLength;
	ULong UUID[12];
	int UUIDCount;
	ULong attr[12];
	int attrCount;
	Short maxReturnedAttrData;
	ULong handle;
	
	LOG ("TSDPLayer::ServiceAttributeRequest\n");

//	handle = *(ULong *) (&fPacket[0]);
	handle = (fPacket[0] << 24) + (fPacket[1] << 16) + (fPacket[2] << 8) + fPacket[3];
	LOG ("  %08x  ", handle);

	i = 6;
	GetSequenceLength (fPacket + i, sequenceLength, fieldLength);
	i += fieldLength;
	
	j = 0;
	attrCount = 0;
	while (fieldLength != -1 && j < sequenceLength && attrCount < 12) {
		GetAttrRangeFromSequence (fPacket + i, attr[attrCount], fieldLength);
		if (fieldLength > 0) {
			LOG ("%08x ", attr[attrCount]);
			i += fieldLength; j += fieldLength;
			attrCount++;
		}
	}
	LOG ("\n");

	SndServiceAttributeResponse (handle);
}

void TSDPLayer::ServiceAttributeResponse (void)
{
	LOG ("TSDPLayer::ServiceAttributeResponse\n");
}

void TSDPLayer::ServiceSearchAttributeRequest (void)
{
	int i, j;
	Byte fieldLength;
	ULong sequenceLength;
	ULong UUID[12];
	int UUIDCount;
	ULong attr[12];
	int attrCount;
	Short maxReturnedAttrData;
	
	LOG ("TSDPLayer::ServiceSearchAttributeRequest\n");

	i = 0;
	GetSequenceLength (fPacket + i, sequenceLength, fieldLength);
	i += fieldLength;
	
	j = 0;
	UUIDCount = 0;
	while (fieldLength != -1 && j < sequenceLength && UUIDCount < 12) {
		GetUUIDFromSequence (fPacket + i, UUID[UUIDCount], fieldLength);
		if (fieldLength > 0) {
			LOG ("%04x ", UUID[UUIDCount]);
			i += fieldLength; j += fieldLength;
			UUIDCount++;
		}
	}
	LOG ("\n");

	i++; // XXXX
	
	maxReturnedAttrData = GET_SHORT_N (fPacket, i);
	i += 2;
	
	GetSequenceLength (fPacket + i, sequenceLength, fieldLength);
	i += fieldLength;
	
	j = 0;
	attrCount = 0;
	while (fieldLength != -1 && j < sequenceLength && attrCount < 12) {
		GetAttrRangeFromSequence (fPacket + i, attr[attrCount], fieldLength);
		if (fieldLength > 0) {
			LOG ("%08x ", attr[attrCount]);
			i += fieldLength; j += fieldLength;
			attrCount++;
		}
	}

	SndServiceSearchAttributeResponse (UUID, UUIDCount, attr, attrCount);
}

void TSDPLayer::ServiceSearchAttributeResponse (void)
{
	int j, k, l, m;
	ULong serviceSequenceLength, attrSequenceLength, attrValueLength;
	ULong protSequenceLength, elementLength;
	Byte fieldLength, length;
	ULong attributeID;
	ULong UUID;
	Long value;
	UByte *packet;
	UByte result[5];
	Boolean serviceValid;
	
	LOG ("TSDPLayer::ServiceSearchAttributeResponse\n");
	
	packet = fPacket + 2;
	GetSequenceLength (packet, serviceSequenceLength, fieldLength);
	packet += fieldLength;
	
	j = 0;
	while (j < serviceSequenceLength) {
		GetSequenceLength (packet + j, attrSequenceLength, fieldLength);
		LOG ("  Service: %d bytes\n", attrSequenceLength);
		j += fieldLength;
		
		k = 0;
		serviceValid = false;
		while (k < attrSequenceLength) {
			// Attribute ID

			attributeID = GET_DATA_SHORT (packet + j + k);
			k += 3;
			
			// Attribute Value

			if (GET_DATA_TYPE (packet + j + k) == 6) {
				GetSequenceLength (packet + j + k, attrValueLength, fieldLength);
				k += fieldLength;
				
				if (attributeID == ATTR_SERVICE_CLASS_ID_LIST) {
					l = 0;
					while (l < attrValueLength) {
						GetUUIDFromSequence (packet + j + k + l, UUID, fieldLength);
						LOG ("    Service UUID: %04x\n", UUID);
						if (UUID == UUID_SERIAL_PORT ||
							UUID == UUID_OBEX_PUSH ||
							UUID == UUID_PPP_LAN ||
							UUID == UUID_DUN ||
							UUID == UUID_OBEX_FILETRANSFER) {
							serviceValid = true;
							*(ULong *) &result[0] = UUID;
						}
						l += fieldLength + 1;
					}
				} else if (attributeID == ATTR_PROTOCOL_DESCRIPTOR_LIST) {
					l = 0;
					while (l < attrValueLength) {
						GetSequenceLength (packet + j + k + l, protSequenceLength, fieldLength);
						l += fieldLength;
	
						m = 0;
						GetUUIDFromSequence (packet + j + k + l, UUID, fieldLength);
						LOG ("    Protocol UUID: %04x\n", UUID);
						m += fieldLength + 1;
						
						while (m < protSequenceLength) {
							LOG ("      Parameter: %d\n", GET_DATA_TYPE (packet + j + k + l + m));
							GetDataElementLength (packet + j + k + l + m, elementLength, fieldLength);
							if (serviceValid && UUID == UUID_RFCOMM) {
								GetInteger (packet + j + k + l + m, value, length);
								result[4] = value;
							}
							m += elementLength + fieldLength;
						}
						
						l += protSequenceLength;
					}
				}
			} else {
				GetDataElementLength (packet + j + k, attrValueLength, fieldLength);
				k += fieldLength;
			}

			k += attrValueLength;
		}
		
		if (serviceValid) {
			fTool->HandleInputData (result, sizeof (result));
		}
		
		j += attrSequenceLength;
	}
}

void TSDPLayer::SndErrorResponse (void)
{
}

void TSDPLayer::SndServiceSearchRequest (void)
{
	UByte response[14];
	
	LOG ("TSDPLayer::SndServiceSearchRequest\n");

	response[0] = SDP_PDU_SERVICE_SEARCH_REQUEST;
	
	/*
	SET_SHORT_N (response, 1, fRemoteTID);
	SET_SHORT_N (response, 3, 9);
	
	SET_SHORT_N (response, 5, 1);
	SET_SHORT_N (response, 7, 1);
	SET_ULONG_N (response, 9, SRH_OBEX_PUSH);
	*/
	
	response[1] = (fRemoteTID & 0xff00) >> 8; response[2] = fRemoteTID & 0x00ff;
	
	response[3] = 0; response[4] = 9;
	
	response[5] = 0; response[6] = 1;
	response[7] = 0; response[8] = 1;
	response[9] = 0x00; response[10] = 0x01; response[11] = 0x00; response[12] = 0x00;
	
	response[13] = 0;
	
	SndData (response, 14);
}

void TSDPLayer::SndServiceSearchResponse (ULong *UUID, Byte count)
{
	UByte response[5 + 4 * 12];
	int i;
	Byte r;
	ULong handle;
	
	LOG ("TSDPLayer::SndServiceSearchResponse\n");

	response[0] = SDP_PDU_SERVICE_SEARCH_RESPONSE;
	
	/*
	SET_SHORT_N (response, 1, fRemoteTID);
	SET_SHORT_N (response, 3, 9);
	
	SET_SHORT_N (response, 5, 1);
	SET_SHORT_N (response, 7, 1);
	SET_ULONG_N (response, 9, SRH_OBEX_PUSH);
	*/
	
	response[1] = (fRemoteTID & 0xff00) >> 8; response[2] = fRemoteTID & 0x00ff;
	
	r = 0;

	for (i = 0; i < count; i++)	{
		handle = 0;
		switch (UUID[i]) {
			case UUID_SERIAL_PORT:
				handle = SRH_SERIAL_PORT;
				break;
			case UUID_OBEX_PUSH:
				handle = SRH_OBEX_PUSH;
				break;
			case UUID_OBEX_FILETRANSFER:
				handle = SRH_OBEX_FILETRANSFER;
				break;
		}
		
		if (handle != 0) {
			response[r * 4 + 9] = (handle & 0xff000000) >> 24;
			response[r * 4 + 10] = (handle & 0x00ff0000) >> 16;
			response[r * 4 + 11] = (handle & 0x0000ff00) >> 8;
			response[r * 4 + 12] = (handle & 0x000000ff);
			r++;
		}
	}

	response[3] = 0; response[4] = r * 4 + 5;
	
	response[5] = 0; response[6] = r;
	response[7] = 0; response[8] = r;
	
	response[r * 4 + 5 + 4] = 0;
	
	LOG ("  %08x %d\n", response, r);
	
	SndData (response, r * 4 + 5 + 5);
}

void TSDPLayer::SndServiceAttributeRequest (void)
{
}

void TSDPLayer::SndServiceAttributeResponse (ULong handle)
{
	UByte response[48];
	Short c;
	Byte p;
	Byte len;
	
	LOG ("TSDPLayer::SndServiceAttributeResponse\n");

	len = sizeof (response);
	switch (handle) {
		case SRH_SERIAL_PORT: c = UUID_SERIAL_PORT; p = 2; len -= 5; break;
		case SRH_OBEX_PUSH: c = UUID_OBEX_PUSH; p = 3; break;
		case SRH_OBEX_FILETRANSFER: c = UUID_OBEX_FILETRANSFER; p = 4; break;
	}
	
	response[0] = SDP_PDU_SERVICE_ATTRIBUTE_RESPONSE;
	SET_SHORT_N (response, 1, fRemoteTID);
	SET_SHORT_N (response, 3, len - 5);
	
	SET_SHORT_N (response, 5, len - 7 - 1);
	
	response[7] = (6 << 3) | 5;
	response[8] = len - 9 - 1;

		// Attribute 1: Service Record Handle
	
		response[9] = (1 << 3) | 1; response[10] = 0x00; response[11] = 0x00;
		response[12] = (1 << 3) | 2;
		response[13] = (handle & 0xff000000) >> 24; 
		response[14] = (handle & 0x00ff0000) >> 16;
		response[15] = (handle & 0x0000ff00) >> 8;
		response[16] = (handle & 0x000000ff);
		
		// Attribute 2: Service Class ID List

		response[17] = (1 << 3) | 1; response[18] = 0x00; response[19] = 0x01;
		response[20] = (6 << 3) | 5; response[21] = 3;
		
			response[22] = (3 << 3) | 1;
			response[23] = (c & 0xff00) >> 8; response[24] = (c & 0x00ff);

		// Attribute 3: Protocol Descriptor List
		response[25] = (1 << 3) | 1; response[26] = 0x00; response[27] = 0x04;
		
		response[28] = (6 << 3) | 5; 
		if (handle == SRH_SERIAL_PORT) {
			response[29] = 12;
		} else {
			response[29] = 17;
		}
			// L2CAP
			response[30] = (6 << 3) | 5; response[31] = 3;
			
				response[32] = (3 << 3) | 1; response[33] = 0x01; response[34] = 0x00;
				
			// RFCOMM
			response[35] = (6 << 3) | 5; response[36] = 5;
			
				response[37] = (3 << 3) | 1; response[38] = 0x00; response[39] = 0x03;
				response[40] = (1 << 3) | 0; response[41] = p;
	
		if (handle != SRH_SERIAL_PORT) {
			// OBEX
			response[42] = (6 << 3) | 5; response[43] = 3;
			
				response[44] = (3 << 3) | 1; response[45] = 0x00; response[46] = 0x08;
						
			response[47] = 0;
		} else {
			response[42] = 0;
		}
	
	SndData (response, len);
}

void TSDPLayer::SndServiceSearchAttributeRequest (ULong UUID)
{
	UByte request[21];
	
	LOG ("TSDPLayer::SndServiceSearchAttributeRequest (0x%08x)\n", UUID);

	request[0] = SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_REQUEST;
	
	fLocalTID++;
	request[1] = (fLocalTID & 0xff00) >> 8; request[2] = fLocalTID & 0x00ff;
	
	request[3] = 0; request[4] = sizeof (request) - 5;
	
	// Service UUID List
	
	request[5] = (6 << 3) | 5; request[6] = 3;
	
		// Service UUID
	
		request[7] = (3 << 3) | 1; request[8] = (UUID & 0xff00) >> 8; request[9] = (UUID & 0x00ff);
			
	// Maximum Return Bytes		
			
	request[10] = ((fTool->fHCI->fHCIBufferSize - 8) & 0xff00) >> 8;
	request[11] = (fTool->fHCI->fHCIBufferSize - 8) & 0x00ff;
	
	// Attribute UUID List
	
	request[12] = (6 << 3) | 5; request[13] = 6;
	
		// Protocol descriptor and service class attributes
	
		request[14] = (1 << 3) | 1;	request[15] = 0x00; request[16] = 0x01;
		request[17] = (1 << 3) | 1;	request[18] = 0x00; request[19] = 0x04;

	request[20] = 0;
			
	SndData (request, sizeof (request));
}

void TSDPLayer::SndServiceSearchAttributeResponse (ULong *UUID, Byte UUIDCount, ULong *attr, Byte attrCount)
{
	UByte buffer[200];
	UByte *b;
	int responses;
	
	LOG ("TSDPLayer::SndServiceSearchAttributeResponse\n");

	b = buffer + sizeof (buffer) - 1;

	// Continuation state
	*b = 0;

	responses = 0;
	while (UUIDCount-- > 0) {
		switch (UUID[UUIDCount]) {
			case UUID_SERIAL_PORT:
				{
					// Attribute 5: Service Name
					b = MakeString (b, "Serial Port");
					b = MakeAttribute (b, ATTR_SERVICE_NAME);
					
					// Attribute 4: Bluetooth Profile Descriptor List
					{
						// Serial Port Profile
						{
							b = MakeUInt (b, 0x0100);
							b = MakeUUID (b, UUID_SERIAL_PORT);
						}
						b = MakeSequence (b, 2);
					}
					b = MakeSequence (b, 1);
					b = MakeAttribute (b, ATTR_BLUETOOTH_PROFILE_DESCRIPTOR_LIST);
					
					// Attribute 3: Protocol Descriptor List
					{
						// RFCOMM
						{
							b = MakeUInt (b, RFCOMM_SERIAL_PORT);
							b = MakeUUID (b, UUID_RFCOMM);
						}
						b = MakeSequence (b, 2);

						// L2CAP
						{ 
							b = MakeUUID (b, UUID_L2CAP);
						}
						b = MakeSequence (b, 1);
					}
					b = MakeSequence (b, 2);
					b = MakeAttribute (b, ATTR_PROTOCOL_DESCRIPTOR_LIST);
					
					// Attribute 2: Service Class ID List
					{
						{
							b = MakeUUID (b, UUID_SERIAL_PORT);
						}
						b = MakeSequence (b, 1);
					}
					b = MakeSequence (b, 1);
					b = MakeAttribute (b, ATTR_SERVICE_CLASS_ID_LIST);
		
					// Attribute 1: Service Record Handle
					b = MakeUInt (b, SRH_SERIAL_PORT);
					b = MakeAttribute (b, ATTR_SERVICE_RECORD_HANDLE);
				}
				b = MakeSequence (b, 10);
				responses++;
				break;
			case UUID_OBEX_PUSH:
				{
					// Attribute 5: Service Name
					b = MakeString (b, "OBEX Push");
					b = MakeAttribute (b, ATTR_SERVICE_NAME);
					
					// Attribute 4: Bluetooth Profile Descriptor List
					{
						// Serial Port Profile
						{
							b = MakeUInt (b, 0x0100);
							b = MakeUUID (b, UUID_OBEX_PUSH);
						}
						b = MakeSequence (b, 2);
					}
					b = MakeSequence (b, 1);
					b = MakeAttribute (b, ATTR_BLUETOOTH_PROFILE_DESCRIPTOR_LIST);
					
					// Attribute 3: Protocol Descriptor List
					{
						// RFCOMM
						{
							b = MakeUInt (b, RFCOMM_OBEX_PUSH);
							b = MakeUUID (b, UUID_RFCOMM);
						}
						b = MakeSequence (b, 2);

						// L2CAP
						{ 
							b = MakeUUID (b, UUID_L2CAP);
						}
						b = MakeSequence (b, 1);
					}
					b = MakeSequence (b, 2);
					b = MakeAttribute (b, ATTR_PROTOCOL_DESCRIPTOR_LIST);
					
					// Attribute 2: Service Class ID List
					{
						{
							b = MakeUUID (b, UUID_OBEX_PUSH);
						}
						b = MakeSequence (b, 1);
					}
					b = MakeSequence (b, 1);
					b = MakeAttribute (b, ATTR_SERVICE_CLASS_ID_LIST);
		
					// Attribute 1: Service Record Handle
					b = MakeUInt (b, SRH_OBEX_PUSH);
					b = MakeAttribute (b, ATTR_SERVICE_RECORD_HANDLE);
				}
				b = MakeSequence (b, 10);
				responses++;
				break;
			case UUID_OBEX_FILETRANSFER:
				{
					// Attribute 5: Service Name
					b = MakeString (b, "OBEX File Transfer");
					b = MakeAttribute (b, ATTR_SERVICE_NAME);
					
					// Attribute 4: Bluetooth Profile Descriptor List
					{
						// Serial Port Profile
						{
							b = MakeUInt (b, 0x0100);
							b = MakeUUID (b, UUID_OBEX_FILETRANSFER);
						}
						b = MakeSequence (b, 2);
					}
					b = MakeSequence (b, 1);
					b = MakeAttribute (b, ATTR_BLUETOOTH_PROFILE_DESCRIPTOR_LIST);
					
					// Attribute 3: Protocol Descriptor List
					{
						// RFCOMM
						{
							b = MakeUInt (b, RFCOMM_OBEX_FILETRANSFER);
							b = MakeUUID (b, UUID_RFCOMM);
						}
						b = MakeSequence (b, 2);

						// L2CAP
						{ 
							b = MakeUUID (b, UUID_L2CAP);
						}
						b = MakeSequence (b, 1);
					}
					b = MakeSequence (b, 2);
					b = MakeAttribute (b, ATTR_PROTOCOL_DESCRIPTOR_LIST);
					
					// Attribute 2: Service Class ID List
					{
						{
							b = MakeUUID (b, UUID_OBEX_FILETRANSFER);
						}
						b = MakeSequence (b, 1);
					}
					b = MakeSequence (b, 1);
					b = MakeAttribute (b, ATTR_SERVICE_CLASS_ID_LIST);
		
					// Attribute 1: Service Record Handle
					b = MakeUInt (b, SRH_SERIAL_PORT);
					b = MakeAttribute (b, ATTR_SERVICE_RECORD_HANDLE);
				}
				b = MakeSequence (b, 10);
				responses++;
				break;
		}
	}
	b = MakeSequence (b, responses);

	b -= 7;
	b[0] = EVT_SDP_SERVICE_SEARCH_ATTRIBUTE_RESPONSE;
	SET_SHORT_N (b, 1, fRemoteTID);

	// Packet length
	SET_SHORT_N (b, 3, sizeof (buffer) - (b - buffer) - 5);
	
	// Attribute data length
	SET_SHORT_N (b, 5, sizeof (buffer) - (b - buffer) - 7 - 1);

	SndData (b, sizeof (buffer) - (b - buffer));

#if 0
	UByte response[110];
	
	LOG ("TSDPLayer::SndServiceSearchAttributeResponse\n");

	response[0] = SDP_PDU_SERVICE_SEARCH_ATTRIBUTE_RESPONSE;
	SET_SHORT_N (response, 1, fRemoteTID);
	SET_SHORT_N (response, 3, sizeof (response) - 5);
	
	SET_SHORT_N (response, 5, sizeof (response) - 7 - 1);
	
	response[7] = (6 << 3) | 5;
	response[8] = sizeof (response) - 9 - 1;
	
		// Serial Port Service
		
		response[9] = (6 << 3) | 5;
		response[10] = 33;

			// Attribute 1: Service Record Handle
		
			response[11] = (1 << 3) | 1; response[12] = 0x00; response[13] = 0x00;
			response[14] = (1 << 3) | 2; response[15] = 0x00; response[16] = 0x01; response[17] = 0x00; response[18] = 0x00;
			
			// Attribute 2: Service Class ID List

			response[19] = (1 << 3) | 1; response[20] = 0x00; response[21] = 0x01;
			response[22] = (6 << 3) | 5; response[23] = 3;
			
				response[24] = (3 << 3) | 1; response[25] = 0x11; response[26] = 0x01;

			// Attribute 3: Protocol Descriptor List
			
			response[27] = (1 << 3) | 1; response[28] = 0x00; response[29] = 0x04;
			response[30] = (6 << 3) | 5; response[31] = 12;
			
				// L2CAP

				response[32] = (6 << 3) | 5; response[33] = 3;
				
					response[34] = (3 << 3) | 1; response[35] = 0x01; response[36] = 0x00;
					
				// RFCOMM
	
				response[37] = (6 << 3) | 5; response[38] = 5;
				
					response[39] = (3 << 3) | 1; response[40] = 0x00; response[41] = 0x03;
					response[42] = (1 << 3) | 0; response[43] = 0x04;

		// OBEX Push Service
		
		response[44] = (6 << 3) | 5;
		response[45] = 63;

			// Attribute 1: Service Record Handle
		
			response[46] = (1 << 3) | 1; response[47] = 0x00; response[48] = 0x00;
			response[49] = (1 << 3) | 2; response[50] = 0x00; response[51] = 0x01; response[52] = 0x00; response[53] = 0x01;
			
			// Attribute 2: Service Class ID List

			response[54] = (1 << 3) | 1; response[55] = 0x00; response[56] = 0x01;
			response[57] = (6 << 3) | 5; response[58] = 3;
			
				response[59] = (3 << 3) | 1; response[60] = 0x11; response[61] = 0x05;

			// Attribute 3: Protocol Descriptor List
			
			response[62] = (1 << 3) | 1; response[63] = 0x00; response[64] = 0x04;
			response[65] = (6 << 3) | 5; response[66] = 17;
			
				// L2CAP

				response[67] = (6 << 3) | 5; response[68] = 3;
				
					response[69] = (3 << 3) | 1; response[70] = 0x01; response[71] = 0x00;
					
				// RFCOMM
	
				response[72] = (6 << 3) | 5; response[73] = 5;
				
					response[74] = (3 << 3) | 1; response[75] = 0x00; response[76] = 0x03;
					response[77] = (1 << 3) | 0; response[78] = 0x05;

				// OBEX

				response[79] = (6 << 3) | 5; response[80] = 3;
				
					response[81] = (3 << 3) | 1; response[82] = 0x00; response[83] = 0x08;
					
			// Attribute 4: Bluetooth Profile Descriptor List
			
			response[84] = (1 << 3) | 1; response[85] = 0x00; response[86] = 0x09;
			response[87] = (6 << 3) | 5; response[88] = 8;
			
				// OBEX Push Profile
	
				response[89] = (6 << 3) | 5; response[90] = 5;
				
					response[91] = (3 << 3) | 1; response[92] = 0x11; response[93] = 0x05;
					response[94] = (1 << 3) | 1; response[95] = 0x01; response[96] = 0x00;

			// Attribute 5: Service Name
			
			response[97] = (1 << 3) | 1; response[98] = 0x01; response[99] = 0x00;
			response[100] = (6 << 3) | 5; response[101] = 7;
			
				// "OPUSH"
	
				response[102] = (4 << 3) | 5; response[103] = 5;
				
					response[104] = 'O';
					response[105] = 'P';
					response[106] = 'U';
					response[107] = 'S';
					response[108] = 'H';

		response[109] = 0;
	
	SndData (response, sizeof (response));
#endif	
}

void TSDPLayer::SndData (UByte *data, Short length, Boolean release)
{
	CBufferList *list;
	CBufferSegment *buffer;
	
	buffer = CBufferSegment::New ();
	buffer->Init (data, length, release);
	
	list = CBufferList::New ();
	list->Init ();
	list->Insert ((CBuffer *) buffer);
	
	fL2CAP->SndData (fChannel, list);
	
	list->Delete ();
}

#pragma mark -

int TSDPLayer::GetElementSize (UByte* data)
{
	int r;
	
	r = 0;
	switch (*data & 0x07) {
		case SIZE_BYTE_LEN:
			r = data[1] + 2;
			break;
		case SIZE_SHORT_LEN:
			r = (data[1] << 8) + data[2] + 3;
			break;
		case SIZE_LONG_LEN:
			r = (((((data[1] << 8) + data[2]) << 8) + data[3]) << 8) + data[4] + 5;
			break;
		default:
			r = kElementSize[*data & 0x07] + 1;
			break;
	}
	return r;
}

UByte* TSDPLayer::MakeUInt (UByte* buffer, unsigned int data)
{
	if (data < 0x100) {
		buffer -= 2;
		buffer[0] = MAKE_HEADER (TYPE_UINT, SIZE_BYTE);
		buffer[1] = data & 0x000000ff;
	} else if (data < 0x10000) {
		buffer -= 3;
		buffer[0] = MAKE_HEADER (TYPE_UINT, SIZE_SHORT);
		SET_USHORT_N (buffer, 1, data);
	} else {
		buffer -= 5;
		buffer[0] = MAKE_HEADER (TYPE_UINT, SIZE_LONG);
		SET_ULONG_N (buffer, 1, data);
	}
	return buffer;
}

UByte* TSDPLayer::MakeUUID (UByte* buffer, unsigned int uuid)
{
	if (uuid < 0x10000) {
		buffer -= 3;
		buffer[0] = MAKE_HEADER (TYPE_UUID, SIZE_SHORT);
		SET_USHORT_N (buffer, 1, uuid);
	} else {
		buffer -= 5;
		buffer[0] = MAKE_HEADER (TYPE_UUID, SIZE_LONG);
		SET_ULONG_N (buffer, 1, uuid);
	}
	return buffer;
}

UByte* TSDPLayer::MakeAttribute (UByte* buffer, unsigned short attr)
{
	buffer -= 3;
	buffer[0] = MAKE_HEADER (TYPE_UINT, SIZE_SHORT);
	SET_USHORT_N (buffer, 1, attr);
	return buffer;
}

UByte* TSDPLayer::MakeString (UByte* buffer, char *data)
{
	int len;
	
	len = strlen (data);
	LOG2 ("TSDPLayer::MakeString %08x %d", buffer, len);
	if (len < 0x100) {
		buffer = buffer - len - 2;
		buffer[0] =  MAKE_HEADER (TYPE_STRING, SIZE_BYTE_LEN);
		buffer[1] = len;
		memcpy (buffer + 2, data, len);
	} else {
		buffer -= (len + 3);
		buffer[0] =  MAKE_HEADER (TYPE_STRING, SIZE_SHORT_LEN);
		SET_USHORT_N (buffer, 1, len);
		memcpy (buffer + 3, data, len);
	}
	LOG2 (" %08x\n", buffer);
	return buffer;
}

UByte* TSDPLayer::MakeSequence (UByte* buffer, int numElements)
{
	int len, n;
	UByte *b;
	
	LOG2 ("TSDPLayer::MakeSequence %08x %d\n", buffer, numElements);
	b = buffer;
	len = 0;
	while (numElements-- > 0) {
		n = GetElementSize (b);
		LOG2 ("  %08x %d (%d)\n", b, n, len);
		len += n;
		b += n;
	}
	if (len < 0x100) {
		buffer -= 2;
		buffer[0] =  MAKE_HEADER (TYPE_SEQUENCE, SIZE_BYTE_LEN);
		buffer[1] = len;
	} else {
		buffer -= 3;
		buffer[0] =  MAKE_HEADER (TYPE_SEQUENCE, SIZE_SHORT_LEN);
		SET_USHORT_N (buffer, 1, len);
	}
	return buffer;
}

