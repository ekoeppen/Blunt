#include <NewtonGestalt.h>typedef long Ref;const Ref	NILREF = 0x02;const Ref	TRUEREF = 0x1A;const Ref	FALSEREF = NILREF;	class TObjectIterator;class RefVar;typedef const RefVar& RefArg;typedef Ref (*MapSlotsFunction)(RefArg tag, RefArg value, ULong anything);extern Ref 	MakeInt(long i);extern Ref 	MakeChar(unsigned char c);extern Ref	MakeBoolean(int val);extern Boolean IsInt(RefArg r);	extern Boolean IsChar(RefArg r);	extern Boolean IsPtr(RefArg r);		extern Boolean IsMagicPtr(RefArg r);    extern Boolean IsRealPtr(RefArg r);   					extern long 	RefToInt(RefArg r);			extern UniChar RefToUniChar(RefArg r);		class TGPIOInterface{public:	NewtonErr ReadGPIOData (UByte, ULong *);};class TBIOInterface{public:	NewtonErr ReadDIOPins (UByte pin, ULong *data);	NewtonErr WriteDIODir (UByte pin, UByte dir, UByte *data);	NewtonErr WriteDIOPinsD (UByte pin, UByte value, UByte *data);	NewtonErr WriteDIOPinsUS (UByte pin, UByte value, UByte *data);};class TVoyagerPlatform{public:	char filler_0000[0x0010];		TGPIOInterface *fGPIOInterface;	TBIOInterface *fBIOInterface;		char filler_0014[0x00f0];};extern TVoyagerPlatform *GetPlatformDriverD (void);extern TVoyagerPlatform *GetPlatformDriverUS (void);extern "C" Ref SetChannel3Selector (RefArg rcvr, RefArg value){	TVoyagerPlatform *p;	UByte data;	TUGestalt gestalt;	TGestaltSystemInfo info;		gestalt.Gestalt (kGestalt_SystemInfo, &info, sizeof (info));	if ((info.fROMStage & 0x30000) == 0x30000) {		p = GetPlatformDriverD ();		p->fBIOInterface->WriteDIOPinsD (0x22, RefToInt (value), &data);	} else {		p = GetPlatformDriverUS ();		p->fBIOInterface->WriteDIOPinsUS (0x22, RefToInt (value), &data);	}	return MakeInt (data);}