/*
 ============================================================================
 Name		: HTTPClient.cpp
 Author	  : artem78
 Version	 : 1.0
 Copyright   : 
 Description : CHTTPClient implementation
 ============================================================================
 */

#include "HTTPClient.h"

CHTTPClient::CHTTPClient(MHTTPClientObserver* aObserver) :
		iObserver(aObserver),
		iIsRequestActive(EFalse)
	{
	iObserver->SetHTTPClient(this);
	}

CHTTPClient::~CHTTPClient()
	{
	//CancelRequest(); // Not neccesary
	iSession.Close();
	}

CHTTPClient* CHTTPClient::NewLC(MHTTPClientObserver* aObserver)
	{
	CHTTPClient* self = new (ELeave) CHTTPClient(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CHTTPClient* CHTTPClient::NewL(MHTTPClientObserver* aObserver)
	{
	CHTTPClient* self = CHTTPClient::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CHTTPClient::ConstructL()
	{
	// Open http session with default protocol HTTP/TCP
	iSession.OpenL();
	}

void CHTTPClient::GetL(const TDesC8 &aUrl)
	{
	SendRequestL(/*THTTPMethod::*/EGet, aUrl);
	}

void CHTTPClient::SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField,
		const TDesC8 &aHdrValue)
	{
	RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
	CleanupClosePushL(valStr);
	THTTPHdrVal val(valStr);
	//valStr.Close();
	aHeaders.SetFieldL(iSession.StringPool().StringF(aHdrField,
			RHTTPSession::GetTable()), val);
	CleanupStack::PopAndDestroy(&valStr);
	}

void CHTTPClient::SetHeaderL(TInt aHdrField, const TDesC8 &aHdrValue)
	{
	RHTTPHeaders headers = iSession.RequestSessionHeadersL();
	SetHeaderL(headers, aHdrField, aHdrValue);
	}

void CHTTPClient::SetUserAgentL(const TDesC8 &aDes)
	{
	SetHeaderL(HTTP::EUserAgent, aDes);
	}

void CHTTPClient::SendRequestL(THTTPMethod aMethod, const TDesC8 &aUrl)
	{
	// Method
	TInt method;
	switch (aMethod)
		{
		case /*THTTPMethod::*/EGet:
			{
			method = HTTP::EGET;
			break;
			}
			
		default:
			// Other methods not yet implemented
			{
			User::Leave(KErrNotSupported);
			break;
			}
		}
	RStringF methodStr = iSession.StringPool().StringF(
			method, RHTTPSession::GetTable());
	// It`s not needed to call methodStr.Close() after use
	
	// Parse URL
	TUriParser8 uri;
	TInt r = uri.Parse(aUrl);
	User::LeaveIfError(r);

	// Prepare and submit transaction
	CancelRequest(); // Cancel previous transation if opened
	iTransaction = iSession.OpenTransactionL(uri, *iObserver, methodStr);
	iTransaction.SubmitL();
	iIsRequestActive = ETrue;
	}

void CHTTPClient::CancelRequest()
	{
	if (IsRequestActive())
		{
		iObserver->OnHTTPError(/*KErrCancel*/ KErrAbort, iTransaction);
		//iTransaction.Cancel();
		CloseOwnTransaction(); // Note: After this MHTTPClientObserver::MHFRunL won`t be called
		iIsRequestActive = EFalse;
		}
	}

void CHTTPClient::CloseOwnTransaction()
	{
	CloseTransaction(iTransaction);
	}

void CHTTPClient::CloseTransaction(RHTTPTransaction &aTransaction)
	{
	aTransaction.Close();
	}

void MHTTPClientObserver::MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent &aEvent)
	{
	switch (aEvent.iStatus)
		{
		case THTTPEvent::EGotResponseHeaders:
			{
			OnHTTPHeadersRecieved(aTransaction);
			} 
			break;
			
		case THTTPEvent::EGotResponseBodyData:
			{			
			MHTTPDataSupplier* body = aTransaction.Response().Body();
			
			TPtrC8 dataChunk;
			TBool isLast = body->GetNextDataPart(dataChunk);

			OnHTTPResponseDataChunkRecieved(aTransaction, dataChunk,
					body->OverallDataSize(), isLast);
			
			// Done with that bit of body data
			body->ReleaseData();
			} 
			break;
			
		case THTTPEvent::EResponseComplete:
			{
			//CloseTransaction(aTransaction);
			//OnHTTPResponse(aTransaction);
			} 
			break;
			
		case THTTPEvent::ESucceeded:
			{
			OnHTTPResponse(aTransaction);
			CHTTPClient::CloseTransaction(aTransaction);
			iHTTPClient->iIsRequestActive = EFalse;
			} 
			break;
			
		case THTTPEvent::EFailed:
			{
			OnHTTPError(iLastError, aTransaction);
			iLastError = 0; // Reset last error code
			CHTTPClient::CloseTransaction(aTransaction);
			iHTTPClient->iIsRequestActive = EFalse;
			} 
			break;
			
		case THTTPEvent::ERedirectedPermanently:
			break;
			
		case THTTPEvent::ERedirectedTemporarily:
			break;
			
		default:
			{
			iLastError = aEvent.iStatus;
			
			if (aEvent.iStatus < 0) // Any error
				{
				//OnHTTPError(aEvent.iStatus, aTransaction);
				// Closing transaction not needed because THTTPEvent::EFailed will
				// be sent at final
				//CloseTransaction(aTransaction);
				//iHTTPClient->iIsRequestActive = EFalse;
				}
			else // Any warning
				{
			
				}
			}
			break;
		}
	}

TInt MHTTPClientObserver::MHFRunError(TInt /*aError*/, RHTTPTransaction aTransaction,
		const THTTPEvent& /*aEvent*/)
	{
	// Cleanup any resources in case MHFRunL() leaves
	CHTTPClient::CloseTransaction(aTransaction);
	iHTTPClient->iIsRequestActive = EFalse;
	
	return KErrNone;
	}


// MHTTPClientObserver

void MHTTPClientObserver::OnHTTPError(TInt /*aError*/, RHTTPTransaction /*aTransaction*/)
	{
	// No implementation by default
	}

void MHTTPClientObserver::SetHTTPClient(CHTTPClient *aClient)
	{
	iHTTPClient = aClient;
	}
