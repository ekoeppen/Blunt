gConfigSymbol changed to 00008200 (was 00000000). "Erase Internal Store" ignored.
TL2CAPLayer::TL2CAPLayer
TSDPLayer::TSDPLayer
TL2CAPLayer::NextLocalIdentifier
  0 1 2
TL2CAPLayer::SndConnectionRequest
  ID: 2 CID: 200
TL2CAPLayer::ProcessACLData
  Flags: 0 2 Length: 6 CID: 0001
TL2CAPLayer::ProcessACLEvent (0 2 2)
TL2CAPLayer::InformationRequest
TL2CAPLayer::SndInformationResponse
 Type: 2, result: 1
TL2CAPLayer::ProcessACLData
  Flags: 0 2 Length: 24 CID: 0001
TL2CAPLayer::ProcessACLEvent (0 2 2)
TL2CAPLayer::ConnectionResponse (1 2)
 Remote CID: 64, Local CID: 200
Channel: 0cce19a8 Remote CID: 64 Local CID: 200
TL2CAPLayer::ProcessACLData
  Flags: 0 2 Length: 6 CID: 0001
TL2CAPLayer::ProcessACLEvent (2 0 2)
TL2CAPLayer::InformationRequest
TL2CAPLayer::SndInformationResponse
 Type: 2, result: 1
TL2CAPLayer::ProcessACLData
  Flags: 0 2 Length: 8 CID: 0001
TL2CAPLayer::ProcessACLEvent (2 0 2)
TL2CAPLayer::ConfigureRequest (200)
Channel: 0cce19a8 Remote CID: 64 Local CID: 200
  ID: 2 Length: 0 
TL2CAPLayer::SndConfigureResponse (Id 2, 200 to 64)


	if (fState == 3) {
		UByte data[4];
		Size length;
		CBufferSegment *segment;

		length = 4;
		data[0] = 0x01;
		data[1] = 0x03;
		data[2] = 0x0c;
		data[3] = 0x00;
	
		fSendBufferList = CBufferList::New ();
		fSendBufferList->Init ();
		segment = CBufferSegment::New ();
		segment->Init (data, 4, true);
		fSendBufferList->Insert ((CBuffer *) segment);
		
		PutBytes (fSendBufferList);
		
		fReceiveBufferList = CBufferList::New ();
		fReceiveBufferList->Init ();
		segment = CBufferSegment::New ();
		segment->Init (7);
		fReceiveBufferList->Insert ((CBuffer *) segment);
		
		GetBytes (fReceiveBufferList);
	} else if (fState == 4) {
	}

void TRFCOMMTool::DoOutput ()
{
	int r;
	
	printf ("TRFCOMMTool::DoOutput\n");

	if (fSerialMiscConfig.txdOffUntilSend == true) {
		SetTxDTransceiverEnable (true);
		fSerialMiscConfig.txdOffUntilSend = false;
	}
	
	r = FillOutputBuffer ();
	
	if (r != kSerResult_NoErr) {
		if (r == 5) r = kSerResult_NoErr;
		printf ("  Put complete\n");
		DoPutComplete (r);
	} else {
		printf ("  Put started\n");
		fTransferState |= TOOL_SENDING;
		if (fIdle == false) {
			ContinueOutputST (true);
		}
		
	
		fIdle = false;
		if (fChipIdle) {
			fSerialChip->ConfigureForOutput (true);
		}
		
		StartOutputST ();
	}
}

NewtonErr TRFCOMMTool::HandleInputData (UByte *data, Short length)
{
	Short i;
	Long r;
	
	LOG ("TRFCOMMTool::HandleInputData (%d)\n  RBuffer size: %d RRBytes: %d RBL: %08x\n",
		length, fReceiveBufferSize, fRemainingReceiveBytes, fReceiveBufferList);

	r = noErr;
	i = 0;
	
	if (fReceiveBufferList != NULL) {
		while (i < length && fReceiveBufferSize > 0) {
			fReceiveBufferList->Put (data[i]);
			i++;
			
			fRemainingReceiveBytes--;
			fReceiveBufferSize--;
		}
	}
	
	LOG ("  %d %d %d\n", i, length, fSavedInputDataLength);

	if (i < length) {
		if (fSavedInputDataLength + length - i > sizeof (fSavedInputData)) {
			printf ("*** Overflow in HandleInputData: %d ***\n", fSavedInputDataLength + length - i);
			length = sizeof (fSavedInputData) - fSavedInputDataLength;
		}
		memcpy (&fSavedInputData[fSavedInputDataLength], &data[i], length - i);
		fSavedInputDataLength += (length - i);
		r = kSerResult_InputDataPending;
	} else {
		if (fRemainingReceiveBytes == 0) {
			r = kSerResult_EOM;
		}
	}
	
	return r;
}

NewtonErr TRFCOMMTool::ReturnSavedInputData (void)
{
	int i;
	Long r;
	
	r = noErr;
	if (fSavedInputDataLength > 0) {
		LOG ("TRFCOMMTool::ReturnSavedInputData (%d)\n  RBuffer size: %d RRBytes: %d\n",
			fSavedInputDataLength, fReceiveBufferSize, fRemainingReceiveBytes);

		i = 0;
		if (fReceiveBufferList != NULL) {
			while (i < fSavedInputDataLength && fReceiveBufferSize > 0) {
				fReceiveBufferList->Put (fSavedInputData[i]);
				i++;
				
				fRemainingReceiveBytes--;
				fReceiveBufferSize--;
			}
		}
		fSavedInputDataLength -= i;
		
		if (fSavedInputDataLength >= 0) {
			if (fSavedInputDataLength + i > sizeof (fSavedInputData)) {
				printf ("*** Overflow in ReturnSavedInputData %d ***\n", fSavedInputDataLength + i);
				fSavedInputDataLength = sizeof (fSavedInputData) - i;
			}
			memmove (fSavedInputData, &fSavedInputData[i], fSavedInputDataLength);
			r = kSerResult_InputDataPending;
		} else {
			if (fRemainingReceiveBytes == 0) {
				r = kSerResult_EOM;
			}
		}
	}
	LOG ("  %d\n", r);
	
	return r;
}

void TRFCOMMTool::EmptyInputBuffer (Size *n)
{
	Size bytesLeft, bytesRead, s, r;
	
	if (fReceiveBufferValid) {
		bytesLeft = fRemainingReceiveBytes;
		s = fInputBuffer->CopyOut (fReceiveBufferList, &bytesLeft, n);
		bytesRead = fRemainingReceiveBytes - bytesLeft;

		if (s != noErr) {
			if (fReceiveBufferSize < bytesRead) {
				r = kSerResult_EOM;
			}
		}
		
		fRemainingReceiveBytes = bytesLeft;
		fReceiveBufferSize -= bytesRead;
		
	} else {
		s = fInputBuffer->CopyOut (fReceiveBufferList, &fRemainingReceiveBytes, n);
	}

	if (s != 1) return s;
	
	r = *n;
	if (r != kSerResult_EOM) r = -18002;
	return r;
}

void TRFCOMMTool::DoInput ()
{
	ULong n;
	int r;
	Boolean eom;
	Boolean end;

	printf ("TRFCOMMTool::DoInput\n");
	fState++;

	printf ("  %d\n", fState);
	
	if (fDoInputActive == false) {
		fDoInputActive = true;
		
		if (fReceiveBufferValid == false && fReceiveBufferSize == 0) {
			SyncInputBuffer ();
			if (fInputBuffer.BufferCount () == 0) {
				GetComplete (kCommErrNoDataAvailable);
			}
		}

		end = false;
		do {
			fTransferState |= TOOL_RECEIVING;
			SyncInputBuffer ();
			r = EmptyInputBuffer (&n);
			if (r != kSerResult_NoErr) {
				eom = false;
				if (r == kSerResult_InputDataPending) {
					r = kSerResult_NoErr;
				} else if (r == kSerResult_EOM) {
					r = kSerResult_NoErr;
					eom = true;
				}
				DoGetComplete (r, eom);
				
				if (fReceiveBufferList != NULL) {
					end = true;
				}
			}
		} while (end == false);
		
		if (fIFCParms.useSoftFlowControl == true ||
			fIFCParms.useHardFlowControl == true) {
		}
		
		fDoInputActive = false;
	}
}

void TRFCOMMTool::DoOutput ()
{
	int r;
	
	printf ("TRFCOMMTool::DoOutput\n");

	if (fSerialMiscConfig.txdOffUntilSend == true) {
		SetTxDTransceiverEnable (true);
		fSerialMiscConfig.txdOffUntilSend = false;
	}
	
	r = FillOutputBuffer ();
	
	if (r != kSerResult_NoErr) {
		if (r == 5) r = kSerResult_NoErr;

		printf ("  Put complete\n");
		
		DoPutComplete (r);
	} else {
		printf ("  Put started\n");

		fTransferState |= TOOL_SENDING;

		if (fIdle == false) {
			ContinueOutputST (true);
		}
		
	
		fIdle = false;
		if (fChipIdle) {
			fSerialChip->ConfigureForOutput (true);
		}
		
		StartOutputST ();
	}
}

void TRFCOMMTool::DoInput ()
{
	ULong n;
	int r;
	Boolean eom;
	Boolean end;

	printf ("TRFCOMMTool::DoInput\n");
	fState++;

	printf ("  %d\n", fState);
	
	TAsyncSerTool::DoInput ();
}	

void TRFCOMMTool::RxDataAvailable ()
{
	printf ("TRFCOMMTool::RxDataAvailable\n");
	
	if (fReceiveBufferList != NULL) {
		DoInput ();
	}
}

void TRFCOMMTool::IHReqHandler (void)
{
	int state;
	
	printf ("TAsyncSerTool::IHReqHandler\n");
	
	if ((fToolState & TOOL_GET_STATUS) == TOOL_GET_STATUS) {
		DebugStr ("IHReqHandler");
	} else {
		state = fTransferState;
	
		if ((state & TOOL_SENDING) == TOOL_SENDING) {
			TxDataSent ();
		}


		if ((state & TOOL_RECEIVING) == TOOL_RECEIVING) {
			RxDataAvailable ();
		} else {
			DoEvent ();
		}
		
		state &=  0x00003fff;
		state &= ~0x00003080;
		
		if (state) {
			SerialEvents (state);
		}
	}
}

int TRFCOMMTool::FillOutputBuffer ()
{
	printf ("TRFCOMMTool::FillOutputBuffer\n");
	
	return TAsyncSerTool::FillOutputBuffer ();
}

	void					SndData (UByte *data, Short length);
void TRFCOMMLayer::SndData (UByte *data, Short length)
{
	LOG ("TRFCOMMLayer::SndData (%d)\n", fChannel->fDLCI);
	
	fOutputPacketLength = length + 3;
	fOutputPacket = &fL2CAP->fOutputPacket[L2CAP_OUT_MTU_LEN - length - 4);
	if (length > 256) {
		fOutputPacket--;
		fOutputPacketLength++;
	}
	
	fOutputPacket[0] = (fChannel->fDLCI << 2) | 0x02 | 0x01;
	fOutputPacket[1] = CTRL_UIH;
	if (length > 256) {
		fOutputPacket[2] = (length && 0x00ff) << 1;
		fOutputPacket[3] = length >> 8;
		memcpy (&fOutputPacket[4], data, length);
	} else {
		fOutputPacket[2] = (length << 1) | 0x01;
		memcpy (&fOutputPacket[3], data, length);
	}
	fOutputPacket[fOutputPacketLength] = CalculateCRC (fOutputPacket, 2);

	fL2CAP->SndData (fOutputPacket, fOutputPacketLength);
}

	void					SndData (Channel *c, UByte *data, Short length);
void TL2CAPLayer::SndData (Channel *c, UByte *data, Short length)
{
	LOG ("TL2CAPLayer::SndData\n");

	data -= 4;
	SET_SHORT (data, 0, length);
	SET_SHORT (data, 2, c->fRemoteCID);
	
	fHCI->Data (CONT_FIRST, data, length + 4);
}

	{
		CBufferList *l;
		CBufferSegment *s;
		UByte b1[3], b2[3], b3[3];
		memset (b1, 'a', 3);
		memset (b2, 'b', 3);
		memset (b3, 'b', 3);
	
		l = CBufferList::New ();
		l->Init ();
	
		s = CBufferSegment::New ();
		s->Init (b1, 3, false);
		l->Insert ((CBuffer *) s);
	
		/*
		s = CBufferSegment::New ();
		s->Init (b2, 3, false);
		l->InsertLast ((CBuffer *) s);
	
		s = CBufferSegment::New ();
		s->Init (b3, 3, false);
		l->InsertLast ((CBuffer *) s);
		
		for (int i = 0; i < l->GetSize (); i++) {
			UByte b;
			Size n;
			n = 1;
			l->CopyOut (&b, n);
			putchar (b);
		}
		*/
		putchar (10);
	}

int TRFCOMMTool::FillOutputBuffer ()
{
	int r;
	
	LOG ("TRFCOMMTool::FillOutputBuffer (%d)\n", fRemainingSendBytes);
	
	r = noErr;
	if (fSendingControlData) {
		r = TAsyncSerTool::FillOutputBuffer ();
	} else {
		if (fRemainingSendBytes > 0) {
			fRFCOMM->SndData ((UByte *) "ABC", 3);
		} else {
			r = kSerResult_OutputComplete;
		}
	}

	LOG ("  %d\n", r);

	return r;
}

NewtonErr TRFCOMMTool::AddDefaultOptions(TOptionArray* options)
{
	Ref globals, v;
	UShort *s;
	UByte *c;
	TCMOSerialIOParms *ioParms;
	TCMOSerialHWChipLoc *loc;
	
	LOG ("TRFCOMMTool::AddDefaultOptions\n");
	
	ioParms = new TCMOSerialIOParms ();
	loc = new TCMOSerialHWChipLoc ();
	
	globals = NSCallGlobalFn (SYM (GetGlobals));
	v = GetVariable (globals, SYM (bluetoothSpeed));
	if (v != NILREF) ioParms->fSpeed = RefToInt (v);
	
	v = GetVariable (globals, SYM (bluetoothLocation));
	if (v != NILREF) {
		WITH_LOCKED_BINARY (v, b);
		s = (UShort *) b; c = (UByte *) &loc->fHWLoc;
		*c++ = *s++; *c++ = *s++; *c++ = *s++; *c++ = *s++;
		END_WITH_LOCKED_BINARY (v);
	}
	
	v = GetVariable (globals, SYM (bluetoothName));
	if (v != NILREF) {
		WITH_LOCKED_BINARY (v, b);
		for (s = (UShort *) b, c = fName; *s != 0; c++, s++) *c = *s;
		*c = '\0';
		END_WITH_LOCKED_BINARY (v);
	}

	options->AppendOption (ioParms);
	options->AppendOption (loc);
	options->AppendOption (new TCMOSerialHalfDuplex ());
	options->AppendOption (new TCMOOutputFlowControlParms ());
	options->AppendOption (new TCMOInputFlowControlParms ());

	return noErr;
}

