/*
 *  FtpSession.cpp
 *  pftp_basic
 *
 *  Created by living room on 09/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "FtpSession.h"


FtpSession::FtpSession(BOOKMARK* pBm) {
	
	lpBm= pBm;
	tcpClient = new CTCP();
}
FtpSession::~FtpSession() {
	
}

int FtpSession::Login() {
	
	if (!tcpClient->OpenControl(lpBm)) {
		std::cout<<tcpClient->GetError();
		return -1;
	}
	if (!tcpClient->WaitForMessage()) {
		std::cout<<"Failed reading reply message";
		return -2;
	}
	

	int result = authTls();
	if(result<0)return result;
	
	result = sendLoginDetails();
	if(result<0)return result;
	
	result = setBufferProtectionSize();
	if(result<0)return result;
	
	result = sendXDupe();
	if(result<0)return result;
	
	SetAsciTransfer();

}

int FtpSession::setBufferProtectionSize()
{
	
#ifdef TLS
	SendCommand(new FtpCmdPBSZ(0)); 
	std::cout << tcpClient->GetControlBuffer() << '\n';
    
#endif
	return 1;	
}

int FtpSession::authTls()
{
#ifdef TLS
	SendCommand(new FtpCmdAuthTls());
	
	if (!tcpClient->SecureControl()) 
	{
		char temp_string[90000];
		sprintf(temp_string, "[%-10.10s] %s", lpBm->label, E_MSG_BAD_TLSCONN);
		std::cout << temp_string << '\n';
		return -4;
	}
#endif
	return 1;
	
}

int FtpSession::sendLoginDetails()
{
	int result = SendCommand( new FtpCmdUser(lpBm->user));
	std::cout << tcpClient->GetControlBuffer() << '\n';
	
	if(result<0)return result;
	
	result = SendCommand( new FtpCmdPass(lpBm->pass));
	
	return result;
	
}

int FtpSession::SendCommand(FtpCommand* ftpCommand)
{
	if (!tcpClient->SendData(ftpCommand->AsString())) {
		std::cout << "Sending " << ftpCommand->AsString();
        return -1;
    }
	
	if (!tcpClient->WaitForMessage()) {
		std::cout<<"Failed reading reply message";
		return -2;		
    }
	return 1;
}

int FtpSession::Kill()
{
	return SendCommand(new FtpCmdQuit());
}

int FtpSession::SetAsciTransfer()
{
	return SendCommand(new FtpCmdAsciMode());
}
int FtpSession::SetBinaryTransfer()
{
	return SendCommand(new FtpCmdBinaryMode());
}


int FtpSession::sendXDupe()
{
	return SendCommand(new FtpCmdXDUPE());
}