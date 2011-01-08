#include <iostream>
#include "defines.h"
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


int retry_counter = 10;
bool isretryin;
bool use_ssl = true;
char temp_string[512];
int error;
int nossl;
int retry_delay;
int type;
bool use_retry;
bool no_login;
char buffer[255];

bool Login(BOOKMARK *bme ,CTCP *tcp) {
	no_login = false;
    int n;
    for (n = 0; n < retry_counter + 1; n++) {
		isretryin = false;
        //this->PostBusy("CONN"); (show status init)
        if (use_ssl)
            sprintf(temp_string, "[%-10.10s] logging in ... (TLS mode)",bme->label);
        else
            sprintf(temp_string, "[%-10.10s] logging in ... (NO-TLS mode)",
                    bme->label);
		
       // display->PostStatusLine(temp_string, false); (show log line)
        std::cout << temp_string << '\n';
		if (!tcp->OpenControl(bme)) {
			error = tcp->GetError();
            //PostBusy(NULL);
            return (false);
        }
		//PostBusy("WLCM");
		
        // wait for welcome message
        if (!tcp->WaitForMessage()) {
            error = E_BAD_WELCOME;
            //PostBusy(NULL);
            return (false);
        }
		
#ifdef TLS
        if (use_ssl) {
            //PostBusy("AUTH");
            if (!tcp->SendData("AUTH TLS\r\n")) {
                error = E_CONTROL_RESET;
                //PostBusy(NULL);
                return (false);
            }
			std::cout << tcp->GetControlBuffer() << '\n';
            if (!tcp->WaitForMessage()) {
                error = E_BAD_AUTH;
                //EvalError();
                nossl = 1;
            } else {
                if (!tcp->SecureControl()) {
                    //error = E_BAD_TLSCONN;
                    sprintf(temp_string, "[%-10.10s] %s", bme->label,
                            E_MSG_BAD_TLSCONN);
					std::cout << temp_string << '\n';
                    //display->PostStatusLine(temp_string, TRUE);
                } else {
                    /*
					 sprintf(this->temp_string,
					 "[%-10.10s] switched to TLS mode",
					 this->prefs.label);
					 */
					
                   // display->PostMessageFromServer(SERVER_MSG_NEWLABEL,this->magic, prefs.label);
                    //display->PostStatusLine(temp_string, FALSE);
                }
            }
        } else {
            /*
			 sprintf(this->temp_string, "[%-10.10s] NO-TLS forced",
			 this->prefs.label);
			 display->PostStatusLine(temp_string, FALSE);
			 */
            nossl = 1;
        }
#endif
		
        // send USER
        //PostBusy("USER");
        sprintf(temp_string, "USER %s\r\n", bme->pass);
        if (!tcp->SendData(temp_string)) {
            error = E_CONTROL_RESET;
            //PostBusy(NULL);
			std::cout << " BADUSER 1\n";
            return (false);
        }
		
        if (!tcp->WaitForMessage()) {
            error = E_BAD_USER;
            //PostBusy(NULL);
			std::cout << " BADUSER 2\n";
            return (false);
        }
		std::cout << tcp->GetControlBuffer() << '\n';
        // send PASS
        //PostBusy("PASS");
        no_login = false;
		
        sprintf(temp_string, "PASS %s\r\n", bme->pass);
        if (!tcp->SendData(temp_string)) {
            error = E_CONTROL_RESET;
            //PostBusy(NULL);
			std::cout << " ERRROR BADPASSa\n";
            return (false);
        }
		std::cout << tcp->GetControlBuffer() << '\n';
        if (!tcp->WaitForMessage()) {
		//PostBusy(NULL);
            if (!use_retry || (n == retry_counter)) {
                error = E_BAD_PASS;
				std::cout << " ERRROR BADPASSb\n";
                return (false);
            } else {
                no_login = true;
            }
        } else {
            break;
        }
		
        if (use_retry && no_login) {
            sprintf(temp_string, "[%-10.10s] login failed, retry in %ds ... (%s)",
                    bme->label, retry_delay, tcp->GetControlBuffer());
            //display->PostStatusLine(this->temp_string, FALSE);
			std::cout << temp_string << '\n';
            tcp->CloseControl();
            //SetRetrying();
            sleep(retry_delay);
        }
    }
	
#ifdef TLS
    if (nossl != 1) {
        //PostBusy("PBSZ");
        /* now i dunno if this is needed but lets do it anyway : */
        sprintf(temp_string, "PBSZ 0\r\n");
        if (!tcp->SendData(temp_string)) {
            error = E_CONTROL_RESET;
            //PostBusy(NULL);
			std::cout << " ERRROR 1\n";
            return (false);
        }
		
        if (!tcp->WaitForMessage()) {
            error = E_BAD_PBSZ;
            //PostBusy(NULL);
			std::cout << " ERRROR 2\n";
            return (false);
        }
        /*
		 this->PostBusy("PROT");
		 
		 if (prefs.use_ssl_list)
		 if (!SetProt(1)) return (FALSE); //secure data
		 else if (!SetProt(0)) return (FALSE); //secure data
		 */
		std::cout << tcp->GetControlBuffer() << '\n';
    }
#endif
	
    //PostBusy("XDUP");
    sprintf(temp_string, "SITE XDUPE 3\r\n");
    if (!tcp->SendData(temp_string)) {
        error = E_CONTROL_RESET;
        //PostBusy(NULL);
		std::cout << " ERRROR 3\n";
        return (false);
    }
	std::cout << tcp->GetControlBuffer() << '\n';
    /*
	 if (this->tcp.WaitForMessage()) {
	 sprintf(this->temp_string, "[%-10.10s] XDUPE turned on",
	 this->prefs.label);
	 display->PostStatusLine(temp_string, FALSE);
	 } else {
	 */
	
    if (!tcp->WaitForMessage()) {
        sprintf(temp_string, "[%-10.10s] XDUPE not supported",
                bme->label);
		std::cout << temp_string << '\n';
        //display->PostStatusLine(temp_string, FALSE);
    }
	
    //PostBusy("TYPE");
    // TYPE with error checking
    sprintf(temp_string, "TYPE I\r\n");
    if (!tcp->SendData(temp_string)) {
        error = E_CONTROL_RESET;
        std::cout << " ERRROR 5\n";
		return (false);
    }
	
    if (!tcp->WaitForMessage()) {
        sprintf(temp_string, "[%-10.10s] failed   - TYPE command",
                bme->label);
        //display->PostStatusLine(temp_string, TRUE);
		std::cout << temp_string << '\n';
        //PostBusy(NULL);
		std::cout << " ERRROR 6\n";
        return (false); //this is major error, we bail out
    }
    type = TYPE_I;
	
       //PostBusy(NULL);
	
	std::cout << tcp->GetControlBuffer() << '\n';
	
	std::cout << " THE END W00t!\n";
	sprintf(temp_string, "site stat\r\n");
	if (!tcp->SendData(temp_string)) {
        error = E_CONTROL_RESET;
        std::cout << " ERRROR MKDIRTEST\n";
		return (false);
    }
	
	std::cout << tcp->GetControlBuffer() << '\n';
	std::cout << tcp->GetControlBuffer() << '\n';
	
    return (true);
}








int main (int argc, char * const argv[]) {
	
	
	    if (tls_init())
		{
		std::cout << "TLS FAILED\n ";
			exit(1);
		}
	// insert code here...
    std::cout << "pftpHook!\n";
	CTCP *tcp = new CTCP();
	BOOKMARK *bmk = new BOOKMARK();
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
	//
	DetermineOwnIP("en1");
	std::cout << " Local ip: " << own_ip << "\n";
	
    Login(bmk, tcp);
	std::cout << "DIVERTED LOGIN FINISHED\n";
	
	
	/*
	std::cin >> buffer;

	
	tcp->OpenControl(bmk);
	std::cout << "OPEN STD LOGIN\n";

	std::cout << tcp->GetControlBuffer() << '\n';
	//tcp->msg_stack;
	tcp->WaitForMessage();
	sleep(8);
	std::cout << "sending login details\n";
	tcp->SendData("USER glftpd\r\n");
    sleep(1);
	tcp->SendData("PASS glftpd\r\n");
	
	char temp_string[50];
	std::cout << tcp->GetControlBuffer() << '\n';
	
	
	sprintf(temp_string, "SITE XDUPE 3\r\n");                                                                                                                                                                                         
    if (!tcp->SendData(temp_string)) {                                                                                                                                                                                           
    
    }                               
	if (tcp->WaitForMessage()) {                                                                                                                                                                                                       
		sprintf(temp_string, "[%-10.10s] XDUPE turned on",bmk->label);                                                                                                                                                                                                                     
		std::cout << temp_string<< "\n"; 
	}
*/
	 //lets try get dir
	std::cout << tcp->GetControlBuffer() << '\n';
	if (!tcp->SendData("stat -la")) {                                                                                                                                                                                           
		
    }  
	tcp->WaitForMessage();
    std::cout << tcp->GetControlBuffer() << '\n';
	char cmdbuf[512];
	while (true) {
		tcp->FlushStack();
		std::cout << ":> ";
		std::cin >> cmdbuf;
		sprintf(temp_string, "%s\r\n", cmdbuf);
		printf("sent %s\n",temp_string);
		tcp->SendData(temp_string);
		
		std::cout << tcp->GetControlBuffer() << '\n';
		
		std::cout << tcp->GetControlBuffer() << '\n';
		std::cout << tcp->GetControlBuffer() << '\n';
        tcp->WaitForMessage();
	}
	
	std::cin >> buffer;

    return 0;
}
