#ifndef __FTP_COMMAND_H
#define __FTP_COMMAND_H

#include <stdio.h>
#include <time.h>
#include "defines.h"
/*
 *  FtpCommand.h
 *  pftp_basic
 *
 *  Created by living room on 09/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

class FtpCommand
{
public:
	virtual char* AsString()=0;	
};

class FtpCmdCustom : public FtpCommand
{
public:
	FtpCmdCustom();
	FtpCmdCustom(char* pCustomeCmdText);

	char* AsString(){return commandBuffer;};
	
protected:
	char commandBuffer[250];
};

class FtpCmdUser : public FtpCmdCustom
{
public:
	FtpCmdUser(char* pUserName);
};

class FtpCmdPass : public FtpCmdCustom
{
public:
	FtpCmdPass(char* pPassword);
};

class FtpCmdPBSZ : public FtpCmdCustom
{
public:
	FtpCmdPBSZ(int protectionLevel);	
};

class FtpCmdAuthTls : public FtpCommand
{
public:
	char* AsString(){return "AUTH TLS\r\n";};
};

class FtpCmdAsciMode : public FtpCommand
{
public:
	char* AsString(){return "TYPE I\r\n";};
};

class FtpCmdBinaryMode : public FtpCommand
{	
public:
	char* AsString(){return "TYPE B\r\n";};
	
};

class FtpCmdPwd : public FtpCommand
{	
public:
	char* AsString(){return "PWD\r\n";};
	
};

class FtpCmdXDUPE : public FtpCommand
{
public:
	char* AsString(){return "XDUPE \r\n";};	
};

class FtpCmdQuit : public FtpCommand
{
public:
	char* AsString(){return "QUIT\r\n";};	
};

#endif