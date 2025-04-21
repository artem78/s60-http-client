/*
 ============================================================================
 Name		: HTTPClient.h
 Author	  : artem78
 Version	 : 1.0.2
 Copyright   : 
 Description : CHTTPClient declaration
 ============================================================================
 */

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <http.h>

// FORWARD DECLARATION
class MHTTPClientObserver; 

// CLASS DECLARATION

/**
 *  CHTTPClient - high-level class for making HTTP-requests.
 *  
 *  Note: Do not make new request from OnHTTPResponse or OnHTTPError
 *  callbacks. This will cause panic. (FixMe)
 * 
 */
class CHTTPClient : public CBase
	{
	// Constructors and destructor
public:
	~CHTTPClient();
	static CHTTPClient* NewL(MHTTPClientObserver* aObserver);
	static CHTTPClient* NewLC(MHTTPClientObserver* aObserver);

protected:
	CHTTPClient(MHTTPClientObserver* aObserver);
	void ConstructL();
	
	// Custom properties and methods
public:
	// ToDo: Add other methods (POST, HEAD, etc...)
	void GetL(const TDesC8 &aUrl);
	void SetUserAgentL(const TDesC8 &aDes);
	void SetHeaderL(TInt aHdrField, const TDesC8 &aHdrValue); // For session
	void CancelRequest();
	inline TBool IsRequestActive()
		{ return iIsRequestActive; };

protected:
	// Enum
	enum THTTPMethod
		{
		// ToDo: Add other methods (POST, HEAD, etc...)
		EGet
		};
	
	RHTTPSession iSession;
	MHTTPClientObserver* iObserver;
	RHTTPTransaction iTransaction;
	TBool iIsRequestActive;
	
	virtual void SendRequestL(THTTPMethod aMethod, const TDesC8 &aUrl);
	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8 &aHdrValue);
	void CloseOwnTransaction();
	void CloseTransaction(RHTTPTransaction &aTransaction);
	TBool IsTransactionOpened(RHTTPTransaction &aTransaction);
	
	// Friends
	/*friend void MHTTPClientObserver::MHFRunL(RHTTPTransaction aTransaction,
			const THTTPEvent& aEvent);
	friend TInt MHTTPClientObserver::MHFRunError(TInt aError, RHTTPTransaction aTransaction,
			const THTTPEvent& aEvent);*/
	friend class MHTTPClientObserver;
	};


class MHTTPClientObserver : public MHTTPTransactionCallback
	{
	// Inherited from MHTTPTransactionCallback
private:
	virtual void MHFRunL(RHTTPTransaction aTransaction,
			const THTTPEvent& aEvent);
	virtual TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction,
			const THTTPEvent& aEvent);
	
	// Custom properties and methods
private:
	TInt iLastError;
	CHTTPClient* iHTTPClient;
	
	void SetHTTPClient(CHTTPClient *aClient);
	
public:
	virtual void OnHTTPResponseDataChunkRecieved(const RHTTPTransaction aTransaction,
			const TDesC8 &aDataChunk, TInt anOverallDataSize, TBool anIsLastChunk) = 0;
	virtual void OnHTTPResponse(const RHTTPTransaction aTransaction) = 0;
	// @param aError Code of the last error. Equals to 0 if it is HTTP error (ex: 404 Not Found).
	virtual void OnHTTPError(TInt aError, const RHTTPTransaction aTransaction) /*= 0*/;
	virtual void OnHTTPHeadersRecieved(const RHTTPTransaction aTransaction) = 0;
	
	// Friends
	friend class CHTTPClient;
	};



#endif // HTTPCLIENT_H
