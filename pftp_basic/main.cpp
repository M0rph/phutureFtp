#include <iostream>
#include <time.h>

#include "FxpSystem.h"
#include "FtpSession.h"

#include "tcp.h"

#include "tlsutil.h"
/*
 typedef struct _BOOKMARK {
 char *label, *host, *user, *pass, *startdir, *exclude_source, *exclude_target;
 char *section[NUM_SECTIONS], *pre_cmd[NUM_PRECOMMANDS];
 char *site_rules, *site_user, *site_wkup, *noop_cmd, *first_cmd, *nuke_prefix,
 *msg_string;
 int magic, retry_counter, refresh_rate, noop_rate, file_sorting, dir_sorting,
 retry_delay; 
 bool use_refresh, use_noop, use_startdir, use_exclude_source,
 use_exclude_target, use_autologin, use_chaining, use_jump, use_track,
 use_sections;
 bool use_pasv, use_ssl, use_ssl_list, use_ssl_data, use_stealth_mode,
 use_retry, use_firstcmd, use_rndrefr;
 bool use_ssl_fxp, use_alt_fxp, use_rev_file, use_rev_dir;
 
 struct _BOOKMARK *next;
 }
*/ 

extern void DetermineOwnIP(char *DEVICE);
extern char own_ip[256];

void Initialise();
bool parseCommand(char* pCmd); 

BOOKMARK *bmk = new BOOKMARK();
FtpSession* ftpSession;

int main (int argc, char * const argv[]) 
{
	printf("PftpOShit");
	
	Initialise();

	ftpSession = new FtpSession(bmk);
	int result = ftpSession->Login();
	
	char cmd[256];

	std::cout << "\n";
	
	do 
	{
		printf("@:>");
		scanf("%s",cmd);
		
	}while (parseCommand(cmd));
	
	ftpSession->Kill();
}
bool parseCommand(char* pCmd)
{
	if( strcmp(pCmd, "quit")==0)
		return false;	
	   
	if( strcmp(pCmd,"pwd")==0)
	   ftpSession->SendCommand(new FtpCmdPwd());
	else if( strcmp(pCmd,"cmd")==0)
	{
		char cmd[512];
		printf(":>\n");
		scanf("%s",cmd);
		ftpSession->SendCommand(new FtpCmdCustom(cmd));
	}
	else if( strcmp(pCmd,"cmd")==0)
	{
		//ftpSession->ReadBuffer();
	}
	

	   
	return true;
}

void Initialise()
{
	if (tls_init())
	{
		std::cout << "System Tls Initialise failed";
		exit(1);
	}
	
	
	DetermineOwnIP("en1");
	
	//Build test bookmark
	bmk->label = "test1";
	bmk->host = "192.168.1.75:21";
	bmk->user = "glftpd";
	bmk->pass = "glftpd";
	bmk->startdir = "/";
	bmk->exclude_source = "";
	bmk->exclude_target = "";
	bmk->magic = 0;
	for (int i = 1 ; i > 21; i++) {
		bmk->section[i] = new(char[strlen("TESTVALUE") + 1]);
		strcpy(bmk->section[i],"TESTVALUE");
	}
	bmk->site_rules = "";
	bmk->site_user = "";
	bmk->site_wkup = "";
	bmk->use_pasv = 1;
	
}