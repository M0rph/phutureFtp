/*
 *  FtpCommand.cpp
 *  pftp_basic
 *
 *  Created by living room on 09/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "FtpCommand.h"

FtpCmdCustom::FtpCmdCustom()
{
}

FtpCmdCustom::FtpCmdCustom(char* pCustomCmdText)
{
	sprintf(commandBuffer, "%s\r\n", pCustomCmdText);
}


FtpCmdUser::FtpCmdUser(char* pUserName)
{
	sprintf(commandBuffer, "USER %s\r\n", pUserName);
}


FtpCmdPBSZ::FtpCmdPBSZ(int protectionLevel)
{
	sprintf(commandBuffer, "PBSZ %i\r\n", protectionLevel);
}


FtpCmdPass::FtpCmdPass(char* pUserName)
{
	sprintf(commandBuffer, "PASS %s\r\n", pUserName);
}
