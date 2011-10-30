#include <AEvents.h>
#include <AEventHandler.h>
#include <BufferSegment.h>
#include <CommManagerInterface.h>
#include <CommTool.h>
#include <SerialOptions.h>
#include <CommOptions.h>
#include <Endpoint.h>
#include <UserTasks.h>
#include <NewtonScript.h>
#include <stdarg.h>

#include <CMService.h>
#include <CommServices.h>
#include <OptionArray.h>
#include <Protocols.h>

#include "TRFCOMMTool.h"
#include "TRFCOMMService.impl.h"

PROTOCOL_IMPL_SOURCE_MACRO(TRFCOMMService);

TCMService *TRFCOMMService::New ()
{
	return this;
}

void TRFCOMMService::Delete (void)
{
}

NewtonErr TRFCOMMService::Start(TOptionArray* options, ULong serviceId, TServiceInfo* serviceInfo)
{
	TUPort *port;
	TRFCOMMTool *tool;
	NewtonErr r;
	int fLogLevel = 0;
	
	r = ServiceToPort (serviceId, port);
	if (r == -10067) {
		tool = new TRFCOMMTool (serviceId);
		r = StartCommTool (tool, serviceId, serviceInfo);
		r = OpenCommTool (serviceInfo->GetPortId (), options, this);
	} else {
		serviceInfo->SetPortId (port->fId);
		serviceInfo->SetServiceId (serviceId);
	}
	return r;
}

NewtonErr TRFCOMMService::DoneStarting(TAEvent* event, ULong size, TServiceInfo* serviceInfo)
{
	return noErr;
}
