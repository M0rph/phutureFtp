#ifndef __FXP_SYSTEM_H
#define __FXP_SYSTEM_H

/*
 *  FxpSystem.h
 *  pftp_basic
 *
 *  Created by living room on 09/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <time.h>
#include "defines.h"
#include "FtpCommand.h"

class FxpSystem
{
	
public:
	void CreateSession(BOOKMARK* pBm);
	void ExecuteSiteCommand(FtpCommand* pFtpCommand, int bookmarkId);
	
};
#endif