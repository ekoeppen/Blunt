# This makefile was produced at 12:36:27 on Sun, Oct 30, 2011# by (PPC) MakeMake 2.0d48 (3/12/1996) by Rick Holzgrafe, scott douglass, Jeff Holcomb.MAKEFILE     = MakefileObjects-dir  = :{NCT-ObjectOut}:LIB          = {NCT-lib} {NCT-lib-options} {LocalLibOptions} LINK         = {NCT-link}LINKOPTS     = {NCT-link-options} {LocalLinkOptions} Asm          = {NCT-asm} {NCT-asm-options} {LocalAsmOptions}CFront       = {NCT-cfront} {NCT-cfront-options} {LocalCFrontOptions}CFrontC      = {NCT-cfront-c} {NCT-cfront-c-options} {LocalCfrontCOptions}C            = {NCT-ARMc} {NCT-ARMc-options} ARMCPlus     = {NCT-ARMCpp} {NCT-ARMCpp-options} {LocalARMCppOptions}ProtocolOptions = -packagePram         = {NCT-pram} {NCT-pram-options} {LocalPRAMOptions} SETFILE      = {NCT-setfile-cmd}SETFILEOPTS  = LocalLinkOptions = -dupok LocalARMCppOptions = -cfront -W LocalCfronttOptions = LocalCfrontCOptions = -W LocalCOptions = -d forARM LocalPackerOptions =  -packageid 'rfcm' -copyright 'Copyright (c) 2003 Eckhart K�ppen'COUNT        = CountCOUNTOPTS    = CTAGS        = CTagsCTAGSOPTS    = -local -updateDELETE       = DeleteDELETEOPTS   = -iFILES        = FilesFILESOPTS    = -lLIBOPTS      = PRINT        = PrintPRINTOPTS    = REZ          = Rez"{Objects-dir}"	� :TARGETS = "{Objects-dir}TRFCOMMToolMP2x00US.bin" BluntMP2x00US.pkg �	"{Objects-dir}TRFCOMMToolMP2100D.bin" BluntMP2100D.pkg �	"{Objects-dir}TRFCOMMTooleMate.bin" BlunteMate.pkg# For "{Objects-dir}TRFCOMMToolMP2x00US.bin"OBJS_00 = "{Objects-dir}RelocHack.a.o" "{Objects-dir}THCILayer.cp.o" �	"{Objects-dir}TL2CAPLayer.cp.o" "{Objects-dir}TSDPLayer.cp.o" �	"{Objects-dir}TRFCOMMLayer.cp.o" "{Objects-dir}TRFCOMMTool.cp.o" �	"{Objects-dir}TRFCOMMService.cp.o" "{Objects-dir}Logger.cp.o" �	"{Objects-dir}TRFCOMMService.impl.h.o"# For "{Objects-dir}TRFCOMMToolMP2100D.bin"OBJS_02 = "{Objects-dir}RelocHack.a.o" "{Objects-dir}THCILayer.cp.o" �	"{Objects-dir}TL2CAPLayer.cp.o" "{Objects-dir}TSDPLayer.cp.o" �	"{Objects-dir}TRFCOMMLayer.cp.o" "{Objects-dir}TRFCOMMTool.cp.o" �	"{Objects-dir}TRFCOMMService.cp.o" "{Objects-dir}Logger.cp.o" �	"{Objects-dir}TRFCOMMService.impl.h.o"# For "{Objects-dir}TRFCOMMTooleMate.bin"OBJS_04 = "{Objects-dir}RelocHack.a.o" "{Objects-dir}THCILayer.cp.o" �	"{Objects-dir}TL2CAPLayer.cp.o" "{Objects-dir}TSDPLayer.cp.o" �	"{Objects-dir}TRFCOMMLayer.cp.o" "{Objects-dir}TRFCOMMTool.cp.o" �	"{Objects-dir}TRFCOMMService.cp.o" "{Objects-dir}Logger.cp.o" �	"{Objects-dir}TRFCOMMService.impl.h.o"# For "{Objects-dir}TRFCOMMToolMP2x00US.bin"LIBS_00 = "{NCT_Libraries}MP2x00US.a.o"# For "{Objects-dir}TRFCOMMToolMP2100D.bin"LIBS_02 = "{NCT_Libraries}MP2100D.a.o"# For "{Objects-dir}TRFCOMMTooleMate.bin"LIBS_04 = "{NCT_Libraries}eMate.a.o"# For "{Objects-dir}TRFCOMMToolMP2x00US.bin"EXDEPS_00 = Makefile# For BluntMP2x00US.pkgEXDEPS_01 = "{Objects-dir}TRFCOMMToolMP2x00US.bin"# For "{Objects-dir}TRFCOMMToolMP2100D.bin"EXDEPS_02 = Makefile# For BluntMP2100D.pkgEXDEPS_03 = "{Objects-dir}TRFCOMMToolMP2x00US.bin" �	"{Objects-dir}TRFCOMMToolMP2100D.bin"# For "{Objects-dir}TRFCOMMTooleMate.bin"EXDEPS_04 = Makefile# For BlunteMate.pkgEXDEPS_05 = "{Objects-dir}TRFCOMMToolMP2x00US.bin" �	"{Objects-dir}TRFCOMMToolMP2100D.bin" "{Objects-dir}TRFCOMMTooleMate.bin"AOptions = -i "{DDK_Includes-dir}"COptions = -i "{DDK_Includes-dir}" -i "{DDK_Includes-dir}Bootstrap" �	-i "{DDK_Includes-dir}CLibrary" -i "{DDK_Includes-dir}CommAPI" �	-i "{DDK_Includes-dir}Communications" -i "{DDK_Includes-dir}Frames" �	-i "{DDK_Includes-dir}HAL" -i "{DDK_Includes-dir}OS600" �	-i "{DDK_Includes-dir}Packages" -i "{DDK_Includes-dir}Power" �	-i "{DDK_Includes-dir}PSS" -i "{DDK_Includes-dir}QD" �	-i "{DDK_Includes-dir}Toolbox" -i "{DDK_Includes-dir}UtilityClasses" �	-i "{NCT_Includes}Frames" {NCT_DebugSwitch} {LocalCOptions}POptions = -i "{DDK_Includes-dir}"ROptions = -i "{DDK_Includes-dir}" -aall	� {TARGETS}"{Objects-dir}TRFCOMMToolMP2x00US.bin"	� {OBJS_00} {LIBS_00} {EXDEPS_00}	{LINK} {LINKOPTS} -o {Targ} {OBJS_00} {LIBS_00}BluntMP2x00US.pkg	�� {EXDEPS_01}	"{NCT-packer}" -o {Targ} "Blunt" {NCT-packer-options} {LocalPackerOptions} -version 01 �	 -protocol -aif "{Objects-dir}TRFCOMMToolMP2x00US.bin" -autoload -autoRemoveBluntMP2x00US.pkg	�� {EXDEPS_01}	{SETFILE} -t "pkg " -c {NCT-package-creator} {Targ}"{Objects-dir}TRFCOMMToolMP2100D.bin"	� {OBJS_02} {LIBS_02} {EXDEPS_02}	{LINK} {LINKOPTS} -o {Targ} {OBJS_02} {LIBS_02}BluntMP2100D.pkg	�� {EXDEPS_03}	"{NCT-packer}" -o {Targ} "Blunt" {NCT-packer-options} {LocalPackerOptions} -version 01 �	 -protocol -aif "{Objects-dir}TRFCOMMToolMP2x00US.bin" -autoload -autoRemove�	 -protocol -aif "{Objects-dir}TRFCOMMToolMP2100D.bin" -autoload -autoRemoveBluntMP2100D.pkg	�� {EXDEPS_03}	{SETFILE} -t "pkg " -c {NCT-package-creator} {Targ}"{Objects-dir}TRFCOMMTooleMate.bin"	� {OBJS_04} {LIBS_04} {EXDEPS_04}	{LINK} {LINKOPTS} -o {Targ} {OBJS_04} {LIBS_04}BlunteMate.pkg	�� {EXDEPS_05}	"{NCT-packer}" -o {Targ} "Blunt" {NCT-packer-options} {LocalPackerOptions} -version 01 �	 -protocol -aif "{Objects-dir}TRFCOMMToolMP2x00US.bin" -autoload -autoRemove�	 -protocol -aif "{Objects-dir}TRFCOMMToolMP2100D.bin" -autoload -autoRemove�	 -protocol -aif "{Objects-dir}TRFCOMMTooleMate.bin" -autoload -autoRemoveBlunteMate.pkg	�� {EXDEPS_05}	{SETFILE} -t "pkg " -c {NCT-package-creator} {Targ}clean	�	{DELETE} {DELETEOPTS} {OBJS_00} {OBJS_02} {OBJS_04}clobber	� clean	{DELETE} {DELETEOPTS} {TARGETS}files	�	{FILES} {FILESOPTS} {TARGETS} {MAKEFILE} "{NCTTools}DDKBuildMakefile.Post" �		{OBJS_00} {OBJS_02} {OBJS_04}print	�	{PRINT} {PRINTOPTS} {MAKEFILE} "{NCTTools}DDKBuildMakefile.Post"tags	�	{CTAGS} {CTAGSOPTS} {NewerDeps} -i "{DDK_Includes-dir}" �		-i "{DDK_Includes-dir}Bootstrap" -i "{DDK_Includes-dir}CLibrary" �		-i "{DDK_Includes-dir}CommAPI" -i "{DDK_Includes-dir}Communications" �		-i "{DDK_Includes-dir}Frames" -i "{DDK_Includes-dir}HAL" �		-i "{DDK_Includes-dir}OS600" -i "{DDK_Includes-dir}Packages" �		-i "{DDK_Includes-dir}Power" -i "{DDK_Includes-dir}PSS" �		-i "{DDK_Includes-dir}QD" -i "{DDK_Includes-dir}Toolbox" �		-i "{DDK_Includes-dir}UtilityClasses" -i "{NCT_Includes}Frames""{Objects-dir}RelocHack.a.o" � �		RelocHack.a"{Objects-dir}THCILayer.cp.o" � �		THCILayer.cp "{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}UtilityClasses:AEventHandler.h" �		"{DDK_Includes-dir}UtilityClasses:TimerQueue.h" �		"{DDK_Includes-dir}OS600:LongTime.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}UtilityClasses:BufferSegment.h" �		"{DDK_Includes-dir}CommAPI:CommManagerInterface.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}CommAPI:CommOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h" �		"{DDK_Includes-dir}Communications:CommTool.h" �		"{DDK_Includes-dir}Communications:ConfigCommunications.h" �		"{DDK_Includes-dir}Communications:CommErrors.h" �		"{DDK_Includes-dir}OS600:UserTasks.h" �		"{DDK_Includes-dir}UtilityClasses:BufferList.h" �		"{DDK_Includes-dir}Communications:CommToolOptions.h" �		"{DDK_Includes-dir}CommAPI:CommAddresses.h" �		"{DDK_Includes-dir}CommAPI:Transport.h" �		"{DDK_Includes-dir}UtilityClasses:TraceEvents.h" �		"{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Communications:SerialOptions.h" �		"{DDK_Includes-dir}HAL:SerialChipV2.h" "{DDK_Includes-dir}NewtTypes.h" �		THCILayer.h TRFCOMMTool.h CircleBuf.h TL2CAPLayer.h"{Objects-dir}TL2CAPLayer.cp.o" � �		TL2CAPLayer.cp "{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}UtilityClasses:AEventHandler.h" �		"{DDK_Includes-dir}UtilityClasses:TimerQueue.h" �		"{DDK_Includes-dir}OS600:LongTime.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}UtilityClasses:BufferSegment.h" �		"{DDK_Includes-dir}CommAPI:CommManagerInterface.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}CommAPI:CommOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h" �		"{DDK_Includes-dir}Communications:CommTool.h" �		"{DDK_Includes-dir}Communications:ConfigCommunications.h" �		"{DDK_Includes-dir}Communications:CommErrors.h" �		"{DDK_Includes-dir}OS600:UserTasks.h" �		"{DDK_Includes-dir}UtilityClasses:BufferList.h" �		"{DDK_Includes-dir}Communications:CommToolOptions.h" �		"{DDK_Includes-dir}CommAPI:CommAddresses.h" �		"{DDK_Includes-dir}CommAPI:Transport.h" �		"{DDK_Includes-dir}UtilityClasses:TraceEvents.h" �		"{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Communications:SerialOptions.h" TRFCOMMTool.h �		CircleBuf.h THCILayer.h TSDPLayer.h TL2CAPLayer.h TRFCOMMLayer.h"{Objects-dir}TSDPLayer.cp.o" � �		TSDPLayer.cp "{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}UtilityClasses:AEventHandler.h" �		"{DDK_Includes-dir}UtilityClasses:TimerQueue.h" �		"{DDK_Includes-dir}OS600:LongTime.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}UtilityClasses:BufferSegment.h" �		"{DDK_Includes-dir}CommAPI:CommManagerInterface.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}CommAPI:CommOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h" �		"{DDK_Includes-dir}Communications:CommTool.h" �		"{DDK_Includes-dir}Communications:ConfigCommunications.h" �		"{DDK_Includes-dir}Communications:CommErrors.h" �		"{DDK_Includes-dir}OS600:UserTasks.h" �		"{DDK_Includes-dir}UtilityClasses:BufferList.h" �		"{DDK_Includes-dir}Communications:CommToolOptions.h" �		"{DDK_Includes-dir}CommAPI:CommAddresses.h" �		"{DDK_Includes-dir}CommAPI:Transport.h" �		"{DDK_Includes-dir}UtilityClasses:TraceEvents.h" �		"{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Communications:SerialOptions.h" THCILayer.h �		TRFCOMMTool.h CircleBuf.h TL2CAPLayer.h TSDPLayer.h"{Objects-dir}TRFCOMMLayer.cp.o" � �		TRFCOMMLayer.cp "{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}UtilityClasses:AEventHandler.h" �		"{DDK_Includes-dir}UtilityClasses:TimerQueue.h" �		"{DDK_Includes-dir}OS600:LongTime.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}UtilityClasses:BufferSegment.h" �		"{DDK_Includes-dir}CommAPI:CommManagerInterface.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}CommAPI:CommOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h" �		"{DDK_Includes-dir}Communications:CommTool.h" �		"{DDK_Includes-dir}Communications:ConfigCommunications.h" �		"{DDK_Includes-dir}Communications:CommErrors.h" �		"{DDK_Includes-dir}OS600:UserTasks.h" �		"{DDK_Includes-dir}UtilityClasses:BufferList.h" �		"{DDK_Includes-dir}Communications:CommToolOptions.h" �		"{DDK_Includes-dir}CommAPI:CommAddresses.h" �		"{DDK_Includes-dir}CommAPI:Transport.h" �		"{DDK_Includes-dir}UtilityClasses:TraceEvents.h" �		"{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Communications:SerialOptions.h" THCILayer.h �		TRFCOMMTool.h CircleBuf.h TL2CAPLayer.h TSDPLayer.h TRFCOMMLayer.h"{Objects-dir}TRFCOMMTool.cp.o" � �		TRFCOMMTool.cp "{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}UtilityClasses:AEventHandler.h" �		"{DDK_Includes-dir}UtilityClasses:TimerQueue.h" �		"{DDK_Includes-dir}OS600:LongTime.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}UtilityClasses:BufferSegment.h" �		"{DDK_Includes-dir}CommAPI:CommManagerInterface.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}CommAPI:CommOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h" �		"{DDK_Includes-dir}Communications:CommTool.h" �		"{DDK_Includes-dir}Communications:ConfigCommunications.h" �		"{DDK_Includes-dir}Communications:CommErrors.h" �		"{DDK_Includes-dir}OS600:UserTasks.h" �		"{DDK_Includes-dir}UtilityClasses:BufferList.h" �		"{DDK_Includes-dir}Communications:CommToolOptions.h" �		"{DDK_Includes-dir}CommAPI:CommAddresses.h" �		"{DDK_Includes-dir}CommAPI:Transport.h" �		"{DDK_Includes-dir}UtilityClasses:TraceEvents.h" �		"{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Communications:SerialOptions.h" �		"{DDK_Includes-dir}CommAPI:Endpoint.h" �		"{NCT_Includes}Frames:NewtonScript.h" "{NCT_Includes}Frames:objects.h" �		"{DDK_Includes-dir}HAL:SerialChipV2.h" "{DDK_Includes-dir}NewtTypes.h" �		"{DDK_Includes-dir}CLibrary:stdarg.h" RelocHack.h TRFCOMMTool.h �		CircleBuf.h TSDPLayer.h TL2CAPLayer.h THCILayer.h TRFCOMMLayer.h �		Logger.h"{Objects-dir}TRFCOMMService.cp.o" � �		TRFCOMMService.cp "{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}UtilityClasses:AEventHandler.h" �		"{DDK_Includes-dir}UtilityClasses:TimerQueue.h" �		"{DDK_Includes-dir}OS600:LongTime.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}UtilityClasses:BufferSegment.h" �		"{DDK_Includes-dir}CommAPI:CommManagerInterface.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}CommAPI:CommOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h" �		"{DDK_Includes-dir}Communications:CommTool.h" �		"{DDK_Includes-dir}Communications:ConfigCommunications.h" �		"{DDK_Includes-dir}Communications:CommErrors.h" �		"{DDK_Includes-dir}OS600:UserTasks.h" �		"{DDK_Includes-dir}UtilityClasses:BufferList.h" �		"{DDK_Includes-dir}Communications:CommToolOptions.h" �		"{DDK_Includes-dir}CommAPI:CommAddresses.h" �		"{DDK_Includes-dir}CommAPI:Transport.h" �		"{DDK_Includes-dir}UtilityClasses:TraceEvents.h" �		"{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Communications:SerialOptions.h" �		"{DDK_Includes-dir}CommAPI:Endpoint.h" �		"{NCT_Includes}Frames:NewtonScript.h" "{NCT_Includes}Frames:objects.h" �		"{DDK_Includes-dir}CLibrary:stdarg.h" �		"{DDK_Includes-dir}CommAPI:CMService.h" TRFCOMMTool.h CircleBuf.h �		TRFCOMMService.impl.h"{Objects-dir}Logger.cp.o" � �		Logger.cp "{DDK_Includes-dir}OS600:UserTasks.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:LongTime.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserSemaphore.h" �		"{DDK_Includes-dir}NewtTypes.h" �		"{DDK_Includes-dir}HAL:SerialChipRegistry.h" �		"{DDK_Includes-dir}HAL:SerialChipV2.h" �		"{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Communications:SerialOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}HAL:HALOptions.h" �		"{DDK_Includes-dir}CLibrary:stdarg.h" CircleBuf.h �		"{DDK_Includes-dir}UtilityClasses:BufferList.h" �		"{DDK_Includes-dir}UtilityClasses:BufferSegment.h" Logger.h"{Objects-dir}TRFCOMMService.impl.h.o" � �		TRFCOMMService.impl.h "{DDK_Includes-dir}OS600:Protocols.h" �		"{DDK_Includes-dir}Newton.h" "{DDK_Includes-dir}ConfigGlobal.h" �		"{DDK_Includes-dir}CLibrary:stdlib.h" �		"{DDK_Includes-dir}CLibrary:string.h" �		"{DDK_Includes-dir}CLibrary:stddef.h" "{DDK_Includes-dir}NewtonTypes.h" �		"{DDK_Includes-dir}NewtonWidgets.h" "{DDK_Includes-dir}NewtonTime.h" �		"{DDK_Includes-dir}Toolbox:CompMath.h" �		"{DDK_Includes-dir}Toolbox:ConfigToolbox.h" �		"{DDK_Includes-dir}NewtonMemory.h" �		"{DDK_Includes-dir}NewtonExceptions.h" �		"{DDK_Includes-dir}CLibrary:setjmp.h" "{DDK_Includes-dir}NewtonDebug.h" �		"{DDK_Includes-dir}CLibrary:stdio.h" �		"{DDK_Includes-dir}CommAPI:CMService.h" �		"{DDK_Includes-dir}UtilityClasses:AEvents.h" �		"{DDK_Includes-dir}UtilityClasses:ItemComparer.h" �		"{DDK_Includes-dir}UtilityClasses:ItemTester.h" �		"{DDK_Includes-dir}OS600:UserPorts.h" "{DDK_Includes-dir}NewtErrors.h" �		"{DDK_Includes-dir}OS600:UserObjects.h" �		"{DDK_Includes-dir}OS600:KernelTypes.h" �		"{DDK_Includes-dir}OS600:ConfigOS600.h" �		"{DDK_Includes-dir}OS600:SharedTypes.h" �		"{DDK_Includes-dir}OS600:UserSharedMem.h" �		"{DDK_Includes-dir}OS600:UserGlobals.h" �		"{DDK_Includes-dir}OS600:LongTime.h" "{DDK_Includes-dir}NewtConfig.h" �		"{DDK_Includes-dir}HammerConfigBits.h" �		"{DDK_Includes-dir}CommAPI:CommManagerInterface.h" �		"{DDK_Includes-dir}OS600:SystemEvents.h" �		"{DDK_Includes-dir}OS600:NameServer.h" �		"{DDK_Includes-dir}OS600:OSErrors.h" �		"{DDK_Includes-dir}CommAPI:OptionArray.h" �		"{DDK_Includes-dir}Toolbox:NewtMemory.h" �		"{DDK_Includes-dir}CommAPI:CommOptions.h" �		"{DDK_Includes-dir}CommAPI:CommServices.h"	.cp.o		�		.cp	{ARMCPlus}	{depDir}{Default}.cp {COptions} -o {targDir}{Default}.cp.o.cf.o		�		.cf	{CFront} {depDir}{Default}.cf {COptions} {NCT-cfront-redirection} "{{CPlusScratch}}"X{Default}.cf -o {targDir}{Default}.cf.o	{CFrontC} "{{CPlusScratch}}"X{Default}.cf -o {targDir}{Default}.cf.o  ; Delete -i "{{CPlusScratch}}"X{Default}.cf.c.o	    �		.c	{C} {depDir}{Default}.c -o {targDir}{Default}.c.o {COptions}.exp.o		�		.exp	"{NCTTools}"NCTBuildMain	{depDir}{Default}.exp "{{CPlusScratch}}"	{Asm}	"{{CplusScratch}}"{Default}.exp.main.a -o {targDir}{Default}.exp.o ; Delete -i "{{CPlusScratch}}"{Default}.exp.main.a.a.o		�		.a	{Asm}	{depDir}{Default}.a  -o {targDir}{Default}.a.o {AOptions}# defualt rule for Pram tool build of Printer DDK form part containing UI strings for System printer selection dialogs.pf.part 	�		.pf	{Pram}	{depDir}{Default}.pf -o {targDir}{Default}.pf.part ##	Rules for building protocol glue##		.h.o		caller glue (what callers link against)#		.impl.h.o	implementation glue (linked into provider)#.h.o		�		.h	ProtocolGen -InterfaceGlue {depDir}{Default}.h {COptions} -stdout > "{{CPlusScratch}}"{Default}.glue.a	{Asm} "{{CPlusScratch}}"{Default}.glue.a -o {targDir}{Default}.h.o ; Delete -i "{{CPlusScratch}}"{Default}.glue.a.impl.h.o	�		.impl.h	ProtocolGen -ImplementationGlue {depDir}{Default}.impl.h {ProtocolOptions} {COptions} -stdout >"{{CPlusScratch}}"{Default}.impl.a	{Asm} "{{CPlusScratch}}"{Default}.impl.a -o {targDir}{Default}.impl.h.o ; Delete -i "{{CPlusScratch}}"{Default}.impl.a# backward compatability rule for old apple DDK's.j.c.o	    �		.j.c	{C} {depDir}{Default}.j.c -o {targDir}{Default}.j.c.o {COptions}	