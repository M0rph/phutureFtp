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
	
	result = SetAsciTransfer();
	
	return result;

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
		std::cout<<tcpClient->GetError();
		return -2;		
    }
	return 1;
}

int FtpSession::FtpStat(char* pFlags)
{
	char* temp_file = "/Users/livingroom/osx_Dev/phutureFtp/tempfile";
	
	tcpClient->FlushStack();
	tcpClient->SetStealth(true, temp_file);
	int result = SendCommand(new FtpCmdStat(pFlags));
	tcpClient->SetStealth(false, temp_file);

	FILELIST* fl = parseStatFile(temp_file);

		
	tcpClient->CloseData();
	result = SetBinaryTransfer();
	
	}


FILELIST* FtpSession::parseStatFile(char *filename)
{
	char temp_string2[1024];
	char temp_string[1024];
    FILE *file_in;
    FILELIST *fl_temp, *fl_new;
    char *start, name[PATHLEN], dummy[12], date1[10], date2[10], date3[10];
    char owntemp[9], grptemp[9];
    int magic = 0, n, blocks;
    char *start_fix, *end_fix;
    char month_name[][5] = {"Jan ", "Feb ", "Mar ", "Apr ", "May ", "Jun ",
		"Jul ", "Aug ", "Sep ", "Oct ", "Nov ", "Dec "};
    bool use_fix, space;
	
    if ((file_in = fopen(filename, "r"))) {
        // build up new list
        fl_temp = NULL;
		
        do {
            fgets(temp_string2, PATHLEN, file_in);
            if (!feof(file_in)) {
                if (strrchr(temp_string, '\r'))
                    *strrchr(temp_string, '\r') = '\0';
				
                if (strrchr(temp_string, '\n'))
                    *strrchr(temp_string, '\n') = '\0';
				
                StripANSI(temp_string2);
				
                if (!strncmp(temp_string2, "total", 5)) continue;
				
                fl_new = new(FILELIST);
                fl_new->next = NULL;
				
                // determine if we have all the needed fields
                // this one is tricky. first we need to determine the beginning of the date, since this is
                // the only stop-point that keeps beeing the same for all sites/dirs/dates/...
                // we cant rely on counting spaces since it could be a symlink or a file with a space
                n = 0;
                end_fix = NULL;
                use_fix = 0;
                while (!end_fix && (n < 12)) {
                    end_fix = strstr(temp_string2, month_name[n]);
                    n++;
                }
				
                if (end_fix) {
                    // now we have the beginning of the date
                    // here we can count blocks to see if we have groups or not
                    start_fix = temp_string2;
                    space = true;
                    blocks = 0;
                    while (start_fix < end_fix) {
                        if (space) {
                            // look for a space
                            if (*start_fix == ' ') {
                                blocks++;
                                space = !space;
                            }
                        } else {
                            // look for a non-space
                            if (*start_fix != ' ') {
                                space = !space;
                            }
                        }
                        start_fix += 1;
                    }
					
                    switch (blocks) {
							// normal
                        case 5:
                            use_fix = 0;
                            break;
							
							// just no user
                        case 4:
                            use_fix = 1;
                            break;
							
							// not even groups
                        case 3:
                            use_fix = 2;
                            break;
							
                        default:
                            // unknown dirlist line
                            debuglog("weird dirlist line, cant parse");
                            debuglog("line : '%s'", temp_string2);
                                                       continue;
                            break;
                    }
                } else {
                    debuglog("uhm, no month entry found in the dirlist line");
                    debuglog("line : '%s'", temp_string2);
                                     continue;
                }
				
                // break line into correct parts
                if (!use_fix) {
                    if (temp_string2[0] != 'l') {
                        sscanf(temp_string2,
                               "%s %s %s %s %ld %s %s %s",
                               fl_new->mode, dummy, owntemp, grptemp,
                               &(fl_new->size), date1, date2, date3);
                        strncpy(fl_new->owner, owntemp, 8);
                        fl_new->owner[8] = '\0';
                        strncpy(fl_new->group, grptemp, 8);
                        fl_new->group[8] = '\0';
                        start = strstr(temp_string2, date3);
                        start += strlen(date3) + 1;
                        strcpy(name, start);
                        /*
						 if (strrchr(name, '\r'))
						 *strrchr(name, '\r') = '\0';
						 if (strrchr(name, '\n'))
						 *strrchr(name, '\n') = '\0';
						 */
                        fl_new->name = new(char[strlen(name) + 1]);
                        strcpy(fl_new->name, name);
						
                        if (temp_string2[0] == 'd')
                            fl_new->is_dir = true;
                        else
                            fl_new->is_dir = false;
						
                        if (strlen(date2) > 1)
                            sprintf(fl_new->date, "%s %s %s", date1, date2, date3);
                        else
                            sprintf(fl_new->date, "%s  %s %s", date1, date2, date3);
						
                        fl_new->magic = magic;
                    } else {
                        sscanf(temp_string2,
                               "%s %s %s %s %ld %s %s %s",
                               fl_new->mode, dummy, owntemp, grptemp,
                               &(fl_new->size), date1, date2, date3);
                        strncpy(fl_new->owner, owntemp, 8);
                        fl_new->owner[8] = '\0';
                        strncpy(fl_new->group, grptemp, 8);
                        fl_new->group[8] = '\0';
                        start = strstr(temp_string2, " -> ");
                        if (start)
                            *start = '\0';
						
                        start = strstr(temp_string2, date3);
                        start += strlen(date3) + 1;
                        strcpy(name, start);
                        if (strrchr(name, '\r'))
                            *strrchr(name, '\r') = '\0';
						
                        if (strrchr(name, '\n'))
                            *strrchr(name, '\n') = '\0';
						
                        fl_new->name = new(char[strlen(name) + 1]);
                        strcpy(fl_new->name, name);
						
                        if (temp_string2[0] == 'l')
                            fl_new->is_dir = true;
                        else
                            fl_new->is_dir = false;
						
                        if (strlen(date2) > 1)
                            sprintf(fl_new->date, "%s %s %s", date1, date2, date3);
                        else
                            sprintf(fl_new->date, "%s  %s %s", date1, date2, date3);
						
                        fl_new->magic = magic;
                    }
                } else if (use_fix == 1) {
                    // use fix for sitez like STH (gftpd)
                    if (temp_string2[0] != 'l') {
                        sscanf(temp_string2,
                               "%s %s %s %ld %s %s %s", fl_new->mode,
                               dummy, owntemp, &(fl_new->size), date1,
                               date2, date3);
                        strncpy(fl_new->owner, owntemp, 8);
                        fl_new->owner[8] = '\0';
                        strcpy(fl_new->group, "none");
                        start = strstr(temp_string2, date3);
                        start += strlen(date3) + 1;
                        strcpy(name, start);
                        if (strrchr(name, '\r'))
                            *strrchr(name, '\r') = '\0';
						
                        if (strrchr(name, '\n'))
                            *strrchr(name, '\n') = '\0';
						
                        fl_new->name = new(char[strlen(name) + 1]);
                        strcpy(fl_new->name, name);
						
                        if (temp_string2[0] == 'd')
                            fl_new->is_dir = true;
                        else
                            fl_new->is_dir = false;
						
                        if (strlen(date2) > 1)
                            sprintf(fl_new->date, "%s %s %s", date1, date2, date3);
                        else
                            sprintf(fl_new->date, "%s  %s %s", date1, date2, date3);
						
                        fl_new->magic = magic;
                    } else {
                        sscanf(temp_string2,
                               "%s %s %s %ld %s %s %s", fl_new->mode,
                               dummy, owntemp, &(fl_new->size), dummy,
                               dummy, dummy);
                        strncpy(fl_new->owner, owntemp, 8);
                        fl_new->owner[8] = '\0';
                        strcpy(fl_new->group, "none");
						
                        start = strstr(temp_string2, " -> ");
                        if (start)
                            *start = '\0';
						
                        start = strstr(temp_string2, dummy);
						
                        start += strlen(dummy) + 1;
                        strcpy(name, start);
                        if (strrchr(name, '\r'))
                            *strrchr(name, '\r') = '\0';
						
                        if (strrchr(name, '\n'))
                            *strrchr(name, '\n') = '\0';
						
                        fl_new->name = new(char[strlen(name) + 1]);
                        strcpy(fl_new->name, name);
						
                        if (temp_string2[0] == 'l')
                            fl_new->is_dir = true;
                        else
                            fl_new->is_dir = false;
						
                        if (strlen(date2) > 1)
                            sprintf(fl_new->date, "%s %s %s", date1, date2, date3);
                        else
                            sprintf(fl_new->date, "%s  %s %s", date1, date2, date3);
						
                        fl_new->magic = magic;
                    }
                } else {
                    // use fix for entries with no user, no group (FUCK YOU)
                    strcpy(fl_new->owner, "none");
                    strcpy(fl_new->group, "none");
					
                    if (temp_string2[0] != 'l') {
                        sscanf(temp_string2, "%s %s %ld %s %s %s",
                               fl_new->mode, dummy, &(fl_new->size),
                               date1, date2, date3);
                        start = strstr(temp_string2, date3);
                        start += strlen(date3) + 1;
                        strcpy(name, start);
                        if (strrchr(name, '\r'))
                            *strrchr(name, '\r') = '\0';
						
                        if (strrchr(name, '\n'))
                            *strrchr(name, '\n') = '\0';
						
                        fl_new->name = new(char[strlen(name) + 1]);
                        strcpy(fl_new->name, name);
						
                        if (temp_string2[0] == 'd')
                            fl_new->is_dir = true;
                        else
                            fl_new->is_dir = false;
						
                        if (strlen(date2) > 1)
                            sprintf(fl_new->date, "%s %s %s", date1, date2, date3);
                        else
                            sprintf(fl_new->date, "%s  %s %s", date1, date2, date3);
						
                        fl_new->magic = magic;
                    } else {
                        sscanf(temp_string2, "%s %s %ld %s %s %s",
                               fl_new->mode, dummy, &(fl_new->size),
                               dummy, dummy, dummy);
                        start = strstr(temp_string2, " -> ");
                        if (start)
                            *start = '\0';
						
                        start = strstr(temp_string2, dummy);
                        start += strlen(dummy) + 1;
                        strcpy(name, start);
                        if (strrchr(name, '\r'))
                            *strrchr(name, '\r') = '\0';
						
                        if (strrchr(name, '\n'))
                            *strrchr(name, '\n') = '\0';
						
                        fl_new->name = new(char[strlen(name) + 1]);
                        strcpy(fl_new->name, name);
						
                        if (temp_string2[0] == 'l')
                            fl_new->is_dir = true;
                        else
                            fl_new->is_dir = false;
						
                        if (strlen(date2) > 1)
                            sprintf(fl_new->date, "%s %s %s", date1, date2, date3);
                        else
                            sprintf(fl_new->date, "%s  %s %s", date1, date2, date3);
						
                        fl_new->magic = magic;
                    }
                }
				
                if (strcmp(fl_new->name, ".")
                    && strcmp(fl_new->name, "..")) {
                    magic++;
					
                    if (fl_temp)
                        fl_temp->next = fl_new;
                    else
						;					
                    fl_temp = fl_new;
                } else {
                    delete[](fl_new->name);
                    delete(fl_new);
                }
            }
        } while (!feof(file_in));
		
        fclose(file_in);
      
    }
    remove(filename);
	return fl_new;

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