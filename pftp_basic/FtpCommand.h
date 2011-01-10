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

class FtpCmdCwd : public FtpCmdCustom
{
public:
	FtpCmdCwd(char* pPath);
};


class FtpCmdMkDir : public FtpCmdCustom
{
public:
	FtpCmdMkDir(char* pPath);
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

class FtpCmdStat : public FtpCmdCustom
{
public:
	FtpCmdStat(char* pFlags);	
};

class FtpCmdAuthTls : public FtpCommand
{
public:
	char* AsString(){return "AUTH TLS\r\n";};
};

class FtpCmdAsciMode : public FtpCommand
{
public:
	char* AsString(){return "TYPE A\r\n";};
};

class FtpCmdBinaryMode : public FtpCommand
{	
public:
	char* AsString(){return "TYPE I\r\n";};
	
};

class FtpCmdPwd : public FtpCommand
{	
public:
	char* AsString(){return "PWD\r\n";};
	
};

class FtpCmdXDUPE : public FtpCommand
{
public:
	char* AsString(){return "SITE XDUPE 3\r\n";};	
};

class FtpCmdQuit : public FtpCommand
{
public:
	char* AsString(){return "QUIT\r\n";};	
};


#endif