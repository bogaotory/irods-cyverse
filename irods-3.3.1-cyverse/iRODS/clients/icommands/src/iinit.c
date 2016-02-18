/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "rods.h"
#include "parseCommandLine.h"
#include "rcMisc.h"

void usage (char *prog);
void usageTTL ();

/* Uncomment the line below if you want TTL to be required for all
   users; i.e. all credentials will be time-limited.  This is only
   enforced on the client side so users can bypass this restriction by
   building their own iinit but it would strongly encourage the use of
   time-limited credentials. */
/* #define TIME_TO_LIVE_REQUIRED 1 */
/* Uncomment the line below if you also want a default TTL if none
   is specified by the user. This TTL is specified in hours. */
/* #define TIME_TO_LIVE_DEFAULT 8 */

#define TTYBUF_LEN 100
#define UPDATE_TEXT_LEN NAME_LEN*10

/* 
 Attempt to make the ~/.irods directory in case it doesn't exist (may
 be needed to write the .irodsA file and perhaps the .irodsEnv file).
 */
int
mkrodsdir() {
   char dirName[NAME_LEN];
   int status, mode;
   char *getVar;
#ifdef windows_platform
   getVar = iRODSNt_gethome();
#else
   getVar = getenv("HOME");
#endif
   rstrcpy(dirName, getVar, NAME_LEN);
   rstrcat(dirName, "/.irods", NAME_LEN);
   mode = 0700;
#ifdef _WIN32
   status = iRODSNt_mkdir (dirName, mode);
#else
   status = mkdir (dirName, mode);
#endif
   return(0); /* no error messages as it normally fails */
}

void
printUpdateMsg()
{
#if 0
   printf("One or more fields in your iRODS environment file (.irodsEnv) are\n");
   printf("missing.  This program will ask you for them and if they work (the\n");
   printf("connection/login succeeds), it will update (or create if needed) your\n");
   printf("irods environment file with those values for use by the i-commands.\n");
   printf("You can also edit the environment file with a text editor.\n");
#endif
   printf("One or more fields in your iRODS environment file (.irodsEnv) are\n");
   printf("missing; please enter them.\n");
}
int
main(int argc, char **argv)
{
    int i, ix, status;
    int echoFlag=0;
    char *password;
    rodsEnv myEnv;
    rcComm_t *Conn;
    rErrMsg_t errMsg;
    rodsArguments_t myRodsArgs;
    int useGsi=0;
    int doPassword;
    int pamMode=0;
    char ttybuf[TTYBUF_LEN];
    int doingEnvFileUpdate=0;
    char updateText[UPDATE_TEXT_LEN]="";
    int ttl=0;

    status = parseCmdLineOpt(argc, argv, "ehvVlZ", 1, &myRodsArgs);
    if (status != 0) {
       printf("Use -h for help.\n");
       exit(1);
    }

    if (myRodsArgs.echo==True) {
       echoFlag=1;
    }

    if (myRodsArgs.help==True && myRodsArgs.ttl==True) {
       usageTTL();
       exit(0);
    }
    if (myRodsArgs.help==True) {
       usage(argv[0]);
       exit(0);
    }

    if (myRodsArgs.longOption==True) {
	rodsLogLevel(LOG_NOTICE);
    }
 
    status = getRodsEnv (&myEnv);
    if (status < 0) {
       rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
		status);
       exit (1);
    }

    if (myRodsArgs.ttl==True) {
      ttl = myRodsArgs.ttlValue;
      if (ttl < 1) {
	printf("Time To Live value needs to be a positive integer\n");
	exit(1);
      }
    }

#ifdef TIME_TO_LIVE_REQUIRED 
    if (myRodsArgs.ttl!=True) {
#ifdef TIME_TO_LIVE_DEFAULT
       ttl=TIME_TO_LIVE_DEFAULT;
       printf("Notice: using default TTL (time to live) value of %d hours\n",
	      ttl);
#else
       printf("--ttl (Time To Live) is required, please try again\n");
       exit(2);
#endif /* TIME_TO_LIVE_DEFAULT */
    }
#endif /* TIME_TO_LIVE_REQUIRED */

    ix = myRodsArgs.optind;

    password="";
    if (ix < argc) {
       password = argv[ix];
    }

    if (myRodsArgs.longOption==True) {
	/* just list the env */
	exit (0);
    }

/*
 Create ~/.irods/ if it does not exist
 */
    mkrodsdir(); 

 /*
    Check on the key Environment values, prompt and save
    them if not already available.
  */
    if (myEnv.rodsHost == NULL || strlen(myEnv.rodsHost)==0) {
       int i;
       if (doingEnvFileUpdate==0) {
	  doingEnvFileUpdate=1;
	  printUpdateMsg();
       }
       printf("Enter the host name (DNS) of the server to connect to:");
       memset(ttybuf, 0, TTYBUF_LEN);
       fgets(ttybuf, TTYBUF_LEN, stdin);
       rstrcat(updateText,"irodsHost ", UPDATE_TEXT_LEN);
       rstrcat(updateText,ttybuf, UPDATE_TEXT_LEN);
       i = strlen(ttybuf);
       if (i>0) ttybuf[i-1]='\0'; /* chop off trailing \n */
       strncpy(myEnv.rodsHost, ttybuf, NAME_LEN);
    }
    if (myEnv.rodsPort == 0) {
       int i;
       if (doingEnvFileUpdate==0) {
	  doingEnvFileUpdate=1;
	  printUpdateMsg();
       }
       printf("Enter the port number:");
       memset(ttybuf, 0, TTYBUF_LEN);
       fgets(ttybuf, TTYBUF_LEN, stdin);
       rstrcat(updateText,"irodsPort ", UPDATE_TEXT_LEN);
       rstrcat(updateText,ttybuf, UPDATE_TEXT_LEN);
       i = strlen(ttybuf);
       if (i>0) ttybuf[i-1]='\0';
       myEnv.rodsPort = atoi(ttybuf);
    }
    if (myEnv.rodsUserName == NULL || strlen(myEnv.rodsUserName)==0) {
       int i;
       if (doingEnvFileUpdate==0) {
	  doingEnvFileUpdate=1;
	  printUpdateMsg();
       }
       printf("Enter your irods user name:");
       memset(ttybuf, 0, TTYBUF_LEN);
       fgets(ttybuf, TTYBUF_LEN, stdin);
       rstrcat(updateText,"irodsUserName ", UPDATE_TEXT_LEN);
       rstrcat(updateText,ttybuf, UPDATE_TEXT_LEN);
       i = strlen(ttybuf);
       if (i>0) ttybuf[i-1]='\0';
       strncpy(myEnv.rodsUserName, ttybuf, NAME_LEN);
    }
    if (myEnv.rodsZone == NULL || strlen(myEnv.rodsZone)==0) {
       int i;
       if (doingEnvFileUpdate==0) {
	  doingEnvFileUpdate=1;
	  printUpdateMsg();
       }
       printf("Enter your irods zone:");
       memset(ttybuf, 0, TTYBUF_LEN);
       fgets(ttybuf, TTYBUF_LEN, stdin);
       rstrcat(updateText,"irodsZone ", UPDATE_TEXT_LEN);
       rstrcat(updateText,ttybuf, UPDATE_TEXT_LEN);
       i = strlen(ttybuf);
       if (i>0) ttybuf[i-1]='\0';
       strncpy(myEnv.rodsZone, ttybuf, NAME_LEN);
    }
    #if defined(PAM_AUTH) || defined(GSI_AUTH)
    if (myEnv.rodsAuthScheme == NULL || strlen(myEnv.rodsAuthScheme)==0) {
       int authTypes=0,OK;
       if (doingEnvFileUpdate==1) { /* don't bother if authScheme is
                                       the only one missing; assume
                                       that password is what they want */
#if defined(PAM_AUTH)
       authTypes=1;    /* PAM */
#endif
#if defined(GSI_AUTH)
       if (authTypes==1) {
          authTypes=3; /* PAM and GSI */
       }
       else {
          authTypes=2; /* GSI */
       }
#endif
          if (authTypes==1) {
             printf("Enter your authentication method (password or PAM):");
          }
          if (authTypes==2) {
             printf("Enter your authentication method (password or GSI):");
          }
          if (authTypes==3) {
             printf("Enter your authentication method (password, GSI, or PAM):");
          }
          memset(ttybuf, 0, TTYBUF_LEN);
          fgets(ttybuf, TTYBUF_LEN, stdin);
          OK=-1;
          if (strncmp("GSI",ttybuf,3)==0) {OK=1; ttybuf[3]='\0';}
          if (strncmp("PAM",ttybuf,3)==0) {OK=2; ttybuf[3]='\0';}
          if (strncmp("password",ttybuf,8)==0) {OK=3; ttybuf[8]='\0';}
          if (OK < 0) {
             printf("Warning, invalid authentication type, ignoring\n");
          }
          if (OK > 0) {
             rstrcat(updateText,"irodsAuthScheme ", UPDATE_TEXT_LEN);
             rstrcat(updateText,ttybuf, UPDATE_TEXT_LEN);
             strncpy(myEnv.rodsAuthScheme, ttybuf, NAME_LEN);
          }
       }
    }
#endif
    if (doingEnvFileUpdate) {
       printf("Those values will be added to your environment file (for use by\n");
       printf("other i-commands) if the login succeeds.\n\n");
    }

    /*
      Now, get the password
     */
    doPassword=1;
#if defined(GSI_AUTH)
    if (strncmp("GSI",myEnv.rodsAuthScheme,3)==0) {
       useGsi=1;
       doPassword=0;
    }
#endif
    if (strncmp("PAM",myEnv.rodsAuthScheme,3)==0 ||
	strncmp("pam",myEnv.rodsAuthScheme,3)==0) {
#if defined(PAM_AUTH)
       doPassword=0;
#else 
       rodsLog (LOG_ERROR,
	    "PAM_AUTH_NOT_BUILT_INTO_CLIENT, will try iRODS password",
	     status);
#endif
    }

    if (strcmp(myEnv.rodsUserName, ANONYMOUS_USER)==0) {
       doPassword=0;
    }
    if (useGsi==1) {
       printf("Using GSI, attempting connection/authentication\n");
    }
    if (doPassword==1) {
       if (myRodsArgs.verbose==True) {
	  i = obfSavePw(echoFlag, 1, 1, password);
       }
       else {
	  i = obfSavePw(echoFlag, 0, 0, password);
       }

       if (i != 0) {
	  rodsLogError(LOG_ERROR, i, "Save Password failure");
	  exit(1);
       }
    }

    /* Connect... */ 
    Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 0, &errMsg);
    if (Conn == NULL) {
       rodsLog(LOG_ERROR, 
		    "Saved password, but failed to connect to server %s",
	       myEnv.rodsHost);
       exit(2);
    }

#ifdef PAM_AUTH
    if (strncmp("PAM",myEnv.rodsAuthScheme,3)==0 ||
	strncmp("pam",myEnv.rodsAuthScheme,3)==0) {
       status = clientLoginPam(Conn, password, ttl);
       if (status != 0) exit(8);
       /* if this succeeded, do the regular login below to check that the
	generated password works properly.  */
       pamMode=1;
    }
#endif
    /* and check that the user/password is OK */
    status = clientLogin(Conn);
    if (status != 0) {
       rcDisconnect(Conn);
       exit (7);
    }

    printErrorStack(Conn->rError);

    if (ttl>0 && pamMode==0) {
       /* if doing non-PAM TTL, now get the 
	short-term password (after initial login) */
       status = clientLoginTTL(Conn, ttl);
       if (status != 0) {
	  rcDisconnect(Conn);
	  exit(8);
       }

       /* And check that it works */
       status = clientLogin(Conn);
       if (status != 0) {
	  rcDisconnect(Conn);
	  exit (7);
       }
    }

    rcDisconnect(Conn);

    /* Save updates to .irodsEnv. */
    if (doingEnvFileUpdate==1) {
       appendRodsEnv(updateText);
    }

    exit (0);
}


void usage (char *prog)
{
  printf("Creates a file containing your iRODS password in a scrambled form,\n");
  printf("to be used automatically by the icommands.\n");
  printf("Usage: %s [-ehvVl] [--ttl TimeToLive]\n", prog);
  printf(" -e  echo the password as you enter it (normally there is no echo)\n");
  printf(" -l  list the iRODS environment variables (only)\n");
  printf(" -v  verbose\n");
  printf(" -V  Very verbose\n");
  printf("--ttl ttl  set the password Time To Live (specified in hours)\n");
  printf("           Run 'iinit -h --ttl' for more\n");
  printf(" -h  this help\n");
  printReleaseInfo("iinit");
}

void usageTTL() {
  printf("When using regular iRODS passwords you can use --ttl (Time To Live)\n");
  printf("to request a credential (a temporary password) that will be valid\n");
  printf("for only the number of hours you specify (up to a limit set by the\n");
  printf("administrator).  This is more secure, as this temporary password\n");
  printf("(not your permanent one) will be stored in the obfuscated\n");
  printf("credential file (.irodsA) for use by the other i-commands.\n");
  printf("\n");
  printf("When using PAM, iinit always generates a temporary iRODS password\n");
  printf("for use by the other i-commands, using a time-limit set by the\n");
  printf("administrator (usually a few days).  With the --ttl option, you can\n");
  printf("specify how long this derived password will be valid, within the\n");
  printf("limits set by the administrator.\n");
}
