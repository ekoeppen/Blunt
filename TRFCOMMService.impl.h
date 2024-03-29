#ifndef __TRFCOMMSERVICE_H
#define __TRFCOMMSERVICE_H

#include <Protocols.h>
#include <CMService.h>
#include <CommServices.h>
#include <CommManagerInterface.h>
#include <OptionArray.h>

PROTOCOL TRFCOMMService: public TCMService
	PROTOCOLVERSION(1.0)
{
public:
		PROTOCOL_IMPL_HEADER_MACRO(TRFCOMMService);
		
		CAPABILITIES((kCMS_CapabilityType_ServiceId 'rfcm'))

		TCMService*	New(void);
		void		Delete(void);

		NewtonErr	Start(TOptionArray* options, ULong serviceId, TServiceInfo* serviceInfo);		// start the service
		NewtonErr	DoneStarting(TAEvent* event, ULong size, TServiceInfo* serviceInfo);			// called back when done starting

};

#endif