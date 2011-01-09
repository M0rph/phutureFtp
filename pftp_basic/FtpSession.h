#ifndef __FTP_SESSION_H
#define __FTP_SESSION_H

#include <iostream>
#include <stdio.h>
#include <time.h>
#include "FtpCommand.h"
#include "defines.h"
#include "tcp.h"
/*
 *  FtpSession.h
 *  pftp_basic
 *
 *  Created by living room on 09/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
extern void StripANSI(char *line);

class FtpSession
{
public:
	FtpSession(BOOKMARK* pBm);
	~FtpSession();
	int Login();
	
	int SetAsciTransfer();
	int SetBinaryTransfer();
	int FtpStat(char* pFlags);
	int Kill();
		
	int SendCommand(FtpCommand* ftpCommand);

private:
	CTCP* tcpClient;
	BOOKMARK* lpBm;

	int sendLoginDetails();
	int authTls();
	int setBufferProtectionSize();
	int sendXDupe();
	
		
	
		
	FILELIST* parseStatFile(char *filename);
};

#endif