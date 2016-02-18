/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* definitions for parseCommandLine routine */

#ifndef RODS_PARSE_COMMAND_LINE
#define RODS_PARSE_COMMAND_LINE

#if _WIN32
#include "irodsntutil.h"
#include "IRodsLib3.h"
#include "rodsType.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#define True 1
#define False 0

typedef struct {
   int add;
   int age;
   int agevalue;
   int all;
   int accessControl;
   int admin;
   int agginfo;
   int ascitime;
   int attr;
   int noattr;
   char *attrStr;
   int bulk;
   int backupMode; 
   int condition;
   char *conditionString;
   int collection;
   char *collectionString;
   int dataObjects;
   int dim;
   int dryrun;
   int echo;
   int empty;
   int force;
   int file;
   char *fileString;
   int global;
   int rescGroup;
   char *rescGroupString;
   int hash;
   char *hashValue;
   int header;
   int help;
   int hostAddr;
   char *hostAddrString;
   int input;
   int redirectConn;
   int checksum;
   int verifyChecksum;
   int dataType;
   char *dataTypeString; 
   int longOption;
   int link;
   int rlock;
   int wlock;
   int veryLongOption;
   int mountCollection;
   char *mountType; 
   int replNum;
   char *replNumValue;
   int newFlag;
   char *startTimeInxStr;
   int noPage;
   int number;
   int numberValue;
   int physicalPath;
   char *physicalPathString;
   int logicalPath;
   char *logicalPathString;
   int progressFlag;
   int option;
   char *optionString;
   int orphan;
   int purgeCache;
   int bundle;
   int prompt;
   int query;
   char *queryStr;
   int rbudp;
   int reg;
   int recursive;
   int resource;
   char *resourceString;
   int remove;
   int showFirstLine;
   int sizeFlag;
   rodsLong_t size;
   int srcResc;
   char *srcRescString;
   int subset;
   int subsetByVal;
   char *subsetStr;
   int test;
   int ticket;
   char *ticketString;
   int ttl;
   int ttlValue;
   int reconnect;
   int user;
   char *userString;
   int unmount;
   int verbose;
   int veryVerbose;
   int writeFlag;
   int zone;
   char *zoneName;
   int verify;
   int var;
   char *varStr;
   int extract; 
   int restart;
   char *restartFileString;
   int lfrestart;
   char *lfrestartFileString;
   int version;
   int retries;
   int retriesValue;
   int regRepl;
   int excludeFile;
   char *excludeFileString;

   int parallel;
   int serial;
   int masterIcat;
   int silent;
   int sql;
   int optind;  /* index into argv where non-recognized options begin */
} rodsArguments_t;

int
parseCmdLineOpt(int argc, char **argv, char *optString, int includeLong,
		 rodsArguments_t *rodsArgs);

#ifdef  __cplusplus
}
#endif

#endif /* RODS_PARSE_COMMAND_LINE */
