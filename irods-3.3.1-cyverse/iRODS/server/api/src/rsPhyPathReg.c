/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See phyPathReg.h for a description of this API call.*/

#include "phyPathReg.h"
#include "rodsLog.h"
#include "icatDefines.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "collection.h"
#include "specColl.h"
#include "resource.h"
#include "physPath.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"
#include "miscServerFunct.h"
#include "apiHeaderAll.h"

/* holds a struct that describes pathname match patterns
   to exclude from registration. Needs to be global due
   to the recursive dirPathReg() calls. */
static pathnamePatterns_t *ExcludePatterns = NULL;

/* function to read pattern file from a data server */
pathnamePatterns_t *
readPathnamePatternsFromFile(rsComm_t *rsComm, char *filename, rescInfo_t *rescInfo);

/* phyPathRegNoChkPerm - Wrapper internal function to allow phyPathReg with 
 * no checking for path Perm.
 */
int
phyPathRegNoChkPerm (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;

    addKeyVal (&phyPathRegInp->condInput, NO_CHK_FILE_PERM_KW, "");

    status = irsPhyPathReg (rsComm, phyPathRegInp);
    return (status);
}

int
rsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;

    if (getValByKey (&phyPathRegInp->condInput, NO_CHK_FILE_PERM_KW) != NULL &&
      rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	return SYS_NO_API_PRIV;
    }

    status = irsPhyPathReg (rsComm, phyPathRegInp);
    return (status);
}

int
irsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;
    rescGrpInfo_t *rescGrpInfo = NULL;
    rodsServerHost_t *rodsServerHost = NULL;
    int remoteFlag;
    int rescCnt;
    rodsHostAddr_t addr;
    char *tmpStr = NULL;
    char *rescGroupName = NULL;
    rescInfo_t *tmpRescInfo = NULL;

    if ((tmpStr = getValByKey (&phyPathRegInp->condInput,
      COLLECTION_TYPE_KW)) != NULL && strcmp (tmpStr, UNMOUNT_STR) == 0) {
        status = unmountFileDir (rsComm, phyPathRegInp);
        return (status);
    } else if (tmpStr != NULL && (strcmp (tmpStr, HAAW_STRUCT_FILE_STR) == 0 ||
      strcmp (tmpStr, TAR_STRUCT_FILE_STR) == 0 ||
      strcmp (tmpStr, MSSO_STRUCT_FILE_STR) == 0 )) {
	status = structFileReg (rsComm, phyPathRegInp);
        return (status);
    } else if (tmpStr != NULL && strcmp (tmpStr, LINK_POINT_STR) == 0) {
        status = linkCollReg (rsComm, phyPathRegInp);
        return (status);
    }

    status = getRescInfo (rsComm, NULL, &phyPathRegInp->condInput,
      &rescGrpInfo);

    if (status < 0) {
	rodsLog (LOG_ERROR,
	  "rsPhyPathReg: getRescInfo error for %s, status = %d",
	  phyPathRegInp->objPath, status);
	return (status);
    }

    rescCnt = getRescCnt (rescGrpInfo);

    if (rescCnt != 1) {
        rodsLog (LOG_ERROR,
          "rsPhyPathReg: The input resource is not unique for %s",
          phyPathRegInp->objPath);
        return (SYS_INVALID_RESC_TYPE);
    }

    if ((rescGroupName = getValByKey (&phyPathRegInp->condInput,
      RESC_GROUP_NAME_KW)) != NULL) {
        status = getRescInGrp (rsComm, rescGrpInfo->rescInfo->rescName, 
	  rescGroupName, &tmpRescInfo);
	if (status < 0) {
            rodsLog (LOG_ERROR,
              "rsPhyPathReg: resc %s not in rescGrp %s for %s",
              rescGrpInfo->rescInfo->rescName, rescGroupName,
	      phyPathRegInp->objPath);
            return SYS_UNMATCHED_RESC_IN_RESC_GRP;
	}
    }

    memset (&addr, 0, sizeof (addr));
    
    rstrcpy (addr.hostAddr, rescGrpInfo->rescInfo->rescLoc, LONG_NAME_LEN);
    remoteFlag = resolveHost (&addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsPhyPathReg (rsComm, phyPathRegInp, rescGrpInfo, 
	  rodsServerHost);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remotePhyPathReg (rsComm, phyPathRegInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_ERROR,
              "rsPhyPathReg: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remotePhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, 
rodsServerHost_t *rodsServerHost)
{
   int status;

   if (rodsServerHost == NULL) {
        rodsLog (LOG_ERROR,
          "remotePhyPathReg: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcPhyPathReg (rodsServerHost->conn, phyPathRegInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
         "remotePhyPathReg: rcPhyPathReg failed for %s",
          phyPathRegInp->objPath);
    }

    return (status);
}

int
_rsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, 
rescGrpInfo_t *rescGrpInfo, rodsServerHost_t *rodsServerHost)
{
    int status;
    fileOpenInp_t chkNVPathPermInp;
    int rescTypeInx;
    char *tmpFilePath;
    char filePath[MAX_NAME_LEN];
    dataObjInfo_t dataObjInfo;
    char *tmpStr = NULL;
    int chkType;
    char *excludePatternFile;

    if ((tmpFilePath = getValByKey (&phyPathRegInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
	  "_rsPhyPathReg: No filePath input for %s",
	  phyPathRegInp->objPath);
	return (SYS_INVALID_FILE_PATH);
    } else {
	/* have to do this since it will be over written later */
	rstrcpy (filePath, tmpFilePath, MAX_NAME_LEN);
    }

    /* check if we need to chk permission */

    memset (&dataObjInfo, 0, sizeof (dataObjInfo));
    rstrcpy (dataObjInfo.objPath, phyPathRegInp->objPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.filePath, filePath, MAX_NAME_LEN);
    dataObjInfo.rescInfo = rescGrpInfo->rescInfo;
    rstrcpy (dataObjInfo.rescName, rescGrpInfo->rescInfo->rescName, 
      LONG_NAME_LEN);

 
    if (getValByKey (&phyPathRegInp->condInput, NO_CHK_FILE_PERM_KW) == NULL &&
      (chkType = getchkPathPerm (rsComm, phyPathRegInp, &dataObjInfo)) != 
      NO_CHK_PATH_PERM) { 
        memset (&chkNVPathPermInp, 0, sizeof (chkNVPathPermInp));

        rescTypeInx = rescGrpInfo->rescInfo->rescTypeInx;
        rstrcpy (chkNVPathPermInp.fileName, filePath, MAX_NAME_LEN);
        chkNVPathPermInp.fileType = (fileDriverType_t)RescTypeDef[rescTypeInx].driverType;
        rstrcpy (chkNVPathPermInp.addr.hostAddr,  
	  rescGrpInfo->rescInfo->rescLoc, NAME_LEN);

        status = chkFilePathPerm (rsComm, &chkNVPathPermInp, rodsServerHost,
	 chkType);
    
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsPhyPathReg: chkFilePathPerm error for %s",
              phyPathRegInp->objPath);
            return (SYS_NO_PATH_PERMISSION);
        }
    } else {
	status = 0;
    }

    if (getValByKey (&phyPathRegInp->condInput, COLLECTION_KW) != NULL) {
        excludePatternFile = getValByKey(&phyPathRegInp->condInput, EXCLUDE_FILE_KW);
        if (excludePatternFile != NULL) {
            ExcludePatterns = readPathnamePatternsFromFile(rsComm, 
                                                           excludePatternFile, 
                                                           rescGrpInfo->rescInfo);
        }
	status = dirPathReg (rsComm, phyPathRegInp, filePath, 
	  rescGrpInfo->rescInfo);
        if (excludePatternFile != NULL) {
            freePathnamePatterns(ExcludePatterns);
            ExcludePatterns = NULL;
        }
    } else if ((tmpStr = getValByKey (&phyPathRegInp->condInput, 
      COLLECTION_TYPE_KW)) != NULL && strcmp (tmpStr, MOUNT_POINT_STR) == 0) {
        status = mountFileDir (rsComm, phyPathRegInp, filePath,
          rescGrpInfo->rescInfo);
    } else {
        if (getValByKey (&phyPathRegInp->condInput, REG_REPL_KW) != NULL) {
	    status = filePathRegRepl (rsComm, phyPathRegInp, filePath,
	      rescGrpInfo->rescInfo); 
	} else {
	    status = filePathReg (rsComm, phyPathRegInp, filePath,
	      rescGrpInfo->rescInfo); 
	}
    }

    return (status);
}

int
filePathRegRepl (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath,
rescInfo_t *rescInfo)
{
    dataObjInfo_t destDataObjInfo, *dataObjInfoHead = NULL;
    regReplica_t regReplicaInp;
    char *rescGroupName = NULL;
    int status;

    status = getDataObjInfo (rsComm, phyPathRegInp, &dataObjInfoHead,
      ACCESS_READ_OBJECT, 0);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "filePathRegRepl: getDataObjInfo for %s", phyPathRegInp->objPath);
        return (status);
    }
    status = sortObjInfoForOpen (rsComm, &dataObjInfoHead, NULL, 0);
    if (status < 0) return status;

    destDataObjInfo = *dataObjInfoHead;
    rstrcpy (destDataObjInfo.filePath, filePath, MAX_NAME_LEN);
    destDataObjInfo.rescInfo = rescInfo;
    rstrcpy (destDataObjInfo.rescName, rescInfo->rescName, NAME_LEN);
    if ((rescGroupName = getValByKey (&phyPathRegInp->condInput,
      RESC_GROUP_NAME_KW)) != NULL) {
        rstrcpy (destDataObjInfo.rescGroupName, rescGroupName, NAME_LEN);
    }
    memset (&regReplicaInp, 0, sizeof (regReplicaInp));
    regReplicaInp.srcDataObjInfo = dataObjInfoHead;
    regReplicaInp.destDataObjInfo = &destDataObjInfo;
    if (getValByKey (&phyPathRegInp->condInput, SU_CLIENT_USER_KW) != NULL) {
        addKeyVal (&regReplicaInp.condInput, SU_CLIENT_USER_KW, "");
        addKeyVal (&regReplicaInp.condInput, IRODS_ADMIN_KW, "");
    } else if (getValByKey (&phyPathRegInp->condInput,
      IRODS_ADMIN_KW) != NULL) {
        addKeyVal (&regReplicaInp.condInput, IRODS_ADMIN_KW, "");
    }
    status = rsRegReplica (rsComm, &regReplicaInp);
    clearKeyVal (&regReplicaInp.condInput);
    freeAllDataObjInfo (dataObjInfoHead);

    return status;
}

int
filePathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath, 
rescInfo_t *rescInfo)
{
    dataObjInfo_t dataObjInfo;
    int status;
    char *rescGroupName = NULL;
    char *chksum;

    initDataObjInfoWithInp (&dataObjInfo, phyPathRegInp);
    if ((rescGroupName = getValByKey (&phyPathRegInp->condInput, 
      RESC_GROUP_NAME_KW)) != NULL) {
	rstrcpy (dataObjInfo.rescGroupName, rescGroupName, NAME_LEN);
    }      
    dataObjInfo.replStatus = NEWLY_CREATED_COPY;
    dataObjInfo.rescInfo = rescInfo;
    rstrcpy (dataObjInfo.rescName, rescInfo->rescName, NAME_LEN);

    if (dataObjInfo.dataSize <= 0 && 
#ifdef FILESYSTEM_META
      (dataObjInfo.dataSize = getFileMetadataFromVault (rsComm, &dataObjInfo)) < 0 &&
#else
      (dataObjInfo.dataSize = getSizeInVault (rsComm, &dataObjInfo)) < 0 &&
#endif
      dataObjInfo.dataSize != UNKNOWN_FILE_SZ) {
	status = (int) dataObjInfo.dataSize;
        rodsLog (LOG_ERROR,
#ifdef FILESYSTEM_META
         "filePathReg: getFileMetadataFromVault for %s failed, status = %d",
#else
         "filePathReg: getSizeInVault for %s failed, status = %d",
#endif
          dataObjInfo.objPath, status);
	return (status);
    }
#ifdef FILESYSTEM_META
    addKeyVal(&dataObjInfo.condInput, FILE_SOURCE_PATH_KW, filePath);
#endif

    if ((chksum = getValByKey (&phyPathRegInp->condInput, 
      REG_CHKSUM_KW)) != NULL) {
        if((status = verifyHashUse(chksum)) == 0) {
            rstrcpy (dataObjInfo.chksum, chksum, CHKSUM_LEN);
        } else {
            rodsLog (LOG_ERROR, 
                "rodsPathReg: unsupported file hash for %s, status = %d",
                dataObjInfo.objPath, status);
            return (status);
        }
    }
    else if ((chksum = getValByKey (&phyPathRegInp->condInput, 
           VERIFY_CHKSUM_KW)) != NULL) {
        status = _dataObjChksum (rsComm, &dataObjInfo, &chksum);
        if (status < 0) {
            rodsLog (LOG_ERROR, 
             "rodsPathReg: _dataObjChksum for %s failed, status = %d",
             dataObjInfo.objPath, status);
            return (status);
        }
        rstrcpy (dataObjInfo.chksum, chksum, CHKSUM_LEN);
    }

    setDataTypeByResc (&dataObjInfo);

    status = svrRegDataObj (rsComm, &dataObjInfo);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "filePathReg: rsRegDataObj for %s failed, status = %d",
          dataObjInfo.objPath, status);
    } else {
        ruleExecInfo_t rei;
        initReiWithDataObjInp (&rei, rsComm, phyPathRegInp);
	rei.doi = &dataObjInfo;
	rei.status = status;
        rei.status = applyRule ("acPostProcForFilePathReg", NULL, &rei,
          NO_SAVE_REI);
    }

    return (status);
}

int
dirPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath,
rescInfo_t *rescInfo)
{
    collInp_t collCreateInp;
    fileOpendirInp_t fileOpendirInp;
    fileClosedirInp_t fileClosedirInp;
    int rescTypeInx;
    int status;
    int dirFd;
    dataObjInp_t subPhyPathRegInp;
    fileReaddirInp_t fileReaddirInp;
    rodsDirent_t *rodsDirent = NULL;
    rodsObjStat_t *rodsObjStatOut = NULL;
    int forceFlag;
    fileStatInp_t fileStatInp;
    rodsStat_t *myStat = NULL;
    char curcoll[MAX_NAME_LEN];

    *curcoll = '\0';
    rescTypeInx = rescInfo->rescTypeInx;

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) {
        memset (&collCreateInp, 0, sizeof (collCreateInp));
        rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, 
	  MAX_NAME_LEN);
	/* no need to resolve sym link */
	addKeyVal (&collCreateInp.condInput, TRANSLATED_PATH_KW, "");
#ifdef FILESYSTEM_META
        /* stat the source directory to track the         */
        /* original directory meta-data                   */
        memset (&fileStatInp, 0, sizeof (fileStatInp));
        rstrcpy (fileStatInp.fileName, filePath, MAX_NAME_LEN);
        fileStatInp.fileType = (fileDriverType_t)RescTypeDef[rescTypeInx].driverType;
        rstrcpy (fileStatInp.addr.hostAddr, rescInfo->rescLoc, NAME_LEN);

        status = rsFileStat (rsComm, &fileStatInp, &myStat);
        if (status != 0) {
            rodsLog (LOG_ERROR,
	      "dirPathReg: rsFileStat failed for %s, status = %d",
	      filePath, status);
	    return (status);
        }
        getFileMetaFromStat (myStat, &collCreateInp.condInput);
        addKeyVal(&collCreateInp.condInput, FILE_SOURCE_PATH_KW, filePath);
        free (myStat);
#endif /* FILESYSTEM_META */
        /* create the coll just in case it does not exist */
        status = rsCollCreate (rsComm, &collCreateInp);
	clearKeyVal (&collCreateInp.condInput);
	if (status < 0) {
            rodsLog (LOG_ERROR,
              "dirPathReg: rsCollCreate %s error. status = %d", 
              phyPathRegInp->objPath, status);
            return status;
        }
    } else if (rodsObjStatOut->specColl != NULL) {
        freeRodsObjStat (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "dirPathReg: %s already mounted", phyPathRegInp->objPath);
        return (SYS_MOUNT_MOUNTED_COLL_ERR);
    }
    freeRodsObjStat (rodsObjStatOut);

    memset (&fileOpendirInp, 0, sizeof (fileOpendirInp));

    rstrcpy (fileOpendirInp.dirName, filePath, MAX_NAME_LEN);
    fileOpendirInp.fileType = (fileDriverType_t)RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileOpendirInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);

    dirFd = rsFileOpendir (rsComm, &fileOpendirInp);
    if (dirFd < 0) {
       rodsLog (LOG_ERROR,
          "dirPathReg: rsFileOpendir for %s error, status = %d",
          filePath, dirFd);
        return (dirFd);
    }

    fileReaddirInp.fileInx = dirFd;

    if (getValByKey (&phyPathRegInp->condInput, FORCE_FLAG_KW) != NULL) {
	forceFlag = 1;
    } else {
	forceFlag = 0;
    }

    while ((status = rsFileReaddir (rsComm, &fileReaddirInp, &rodsDirent))
      >= 0) {
	int len;

        if (strlen (rodsDirent->d_name) == 0) break;

        if (strcmp (rodsDirent->d_name, ".") == 0 ||
          strcmp (rodsDirent->d_name, "..") == 0) {
	    free (rodsDirent);
            continue;
        }

        if (matchPathname(ExcludePatterns, rodsDirent->d_name, filePath)) {
            continue;
        }

	memset (&fileStatInp, 0, sizeof (fileStatInp));

        if (RescTypeDef[rescTypeInx].incParentDir == NO_INC_PARENT_DIR) {
	    /* don't include parent path */
            snprintf (fileStatInp.fileName, MAX_NAME_LEN, "%s",
             rodsDirent->d_name);
        } else if (RescTypeDef[rescTypeInx].incParentDir == 
          PHYPATH_IN_DIR_PTR) {
            /* we can do this locally because this API is executed at the
             * resource server */
            getPhyPathInOpenedDir (dirFd, rodsDirent->d_ino, 
              fileStatInp.fileName);
        } else {
	    len = strlen (filePath);
	    if (filePath[len - 1] == '/') {
                /* already has a '/' */
                snprintf (fileStatInp.fileName, MAX_NAME_LEN, "%s%s",
                 filePath, rodsDirent->d_name);
            } else {
                snprintf (fileStatInp.fileName, MAX_NAME_LEN, "%s/%s",
                  filePath, rodsDirent->d_name);
            }
        }
        fileStatInp.fileType = fileOpendirInp.fileType; 
	fileStatInp.addr = fileOpendirInp.addr;
        myStat = NULL;
        status = rsFileStat (rsComm, &fileStatInp, &myStat);

	if (status != 0) {
            rodsLog (LOG_ERROR,
	      "dirPathReg: rsFileStat failed for %s, status = %d",
	      fileStatInp.fileName, status);
	    free (rodsDirent);
	    return (status);
	}

        if (RescTypeDef[rescTypeInx].driverType == TDS_FILE_TYPE &&
          strchr (rodsDirent->d_name, '/') != NULL) {
            /* TDS may contain '/' in the file path */
            char *tmpPtr = rodsDirent->d_name;
            /* replace '/' with '.' */
            while (*tmpPtr != '\0') {
               if (*tmpPtr == '/') *tmpPtr = '.';
               tmpPtr ++;
            }
        }
        if (RescTypeDef[rescTypeInx].incParentDir == PHYPATH_IN_DIR_PTR) {
            /* the st_mode is stored in the opened dir */
            myStat->st_mode = getStModeInOpenedDir (dirFd, rodsDirent->d_ino);
            freePhyPathInOpenedDir (dirFd, rodsDirent->d_ino);
        }
	subPhyPathRegInp = *phyPathRegInp;
        if (RescTypeDef[rescTypeInx].incParentDir == NO_INC_PARENT_DIR) {
	    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
	    /* d_name is a full path, need to split it */
            if ((status = splitPathByKey (rodsDirent->d_name, myDir, myFile, 
              '/')) < 0) {
                rodsLog (LOG_ERROR,
                  "dirPathReg: splitPathByKey error for %s ", 
                  rodsDirent->d_name);
                continue;
            }
	    snprintf (subPhyPathRegInp.objPath, MAX_NAME_LEN, "%s/%s",
	      phyPathRegInp->objPath, myFile);
        } else if (RescTypeDef[rescTypeInx].incParentDir == 
            PHYPATH_IN_DIR_PTR) {
	  /**** RAJA Mar 2013 changed for TDS  to take care of doubling of collection names as found by Howard Lander 
            char curdir[MAX_NAME_LEN];
            *curdir = '\0';
            getCurDirInOpenedDir (dirFd, curdir);
            if (strlen (curdir) > 0) {
                / * see if we have done it already *     /
                int len = strlen (subPhyPathRegInp.objPath);
                if (*curcoll == '\0'  || 
                  strcmp (&curcoll[len + 1], curdir) != 0) {
                    snprintf (curcoll, MAX_NAME_LEN, "%s/%s",
                      phyPathRegInp->objPath, curdir);
                    rsMkCollR (rsComm, phyPathRegInp->objPath, curcoll);
                }
	        snprintf (subPhyPathRegInp.objPath, MAX_NAME_LEN, "%s/%s",
	          curcoll, rodsDirent->d_name);
            } else {
                snprintf (subPhyPathRegInp.objPath, MAX_NAME_LEN, "%s/%s",
                  phyPathRegInp->objPath, rodsDirent->d_name);
            }
	  *****/
	  snprintf (subPhyPathRegInp.objPath, MAX_NAME_LEN, "%s/%s",                                                  
		    phyPathRegInp->objPath, rodsDirent->d_name); 
          /**** RAJA Mar 2013 changed above ****/
        } else {
	    snprintf (subPhyPathRegInp.objPath, MAX_NAME_LEN, "%s/%s",
	      phyPathRegInp->objPath, rodsDirent->d_name);
        }
	if ((myStat->st_mode & S_IFREG) != 0) {     /* a file */
	    if (forceFlag > 0) {
		/* check if it already exists */
	        if (isData (rsComm, subPhyPathRegInp.objPath, NULL) >= 0) {
		    free (myStat);
	            free (rodsDirent);
		    continue;
		}
	    }
	    subPhyPathRegInp.dataSize = myStat->st_size;
	    if (getValByKey (&phyPathRegInp->condInput, REG_REPL_KW) != NULL) {
                status = filePathRegRepl (rsComm, &subPhyPathRegInp,
                  fileStatInp.fileName, rescInfo);
	    } else {
                addKeyVal (&subPhyPathRegInp.condInput, FILE_PATH_KW, 
	          fileStatInp.fileName);
	        status = filePathReg (rsComm, &subPhyPathRegInp, 
	          fileStatInp.fileName, rescInfo);
	    }
        } else if ((myStat->st_mode & S_IFDIR) != 0) {      /* a directory */
            len = strlen (subPhyPathRegInp.objPath);
            if (subPhyPathRegInp.objPath[len - 1] == '/') {
                /* take out the end '/' for objPath */
                subPhyPathRegInp.objPath[len - 1] = '\0';
            }
            status = dirPathReg (rsComm, &subPhyPathRegInp,
              fileStatInp.fileName, rescInfo);
	}
        if (status < 0) return status;
	free (myStat);
	free (rodsDirent);
    }
    if (status == -1) {         /* just EOF */
        status = 0;
    }

    fileClosedirInp.fileInx = dirFd;
    rsFileClosedir (rsComm, &fileClosedirInp);

    return (status);
}

int
mountFileDir (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath,
rescInfo_t *rescInfo)
{
    collInp_t collCreateInp;
    int rescTypeInx;
    int status;
    fileStatInp_t fileStatInp;
    rodsStat_t *myStat = NULL;
    rodsObjStat_t *rodsObjStatOut = NULL;

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH)
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) return status;

    if (rodsObjStatOut->specColl != NULL) {
        freeRodsObjStat (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "mountFileDir: %s already mounted", phyPathRegInp->objPath);
        return (SYS_MOUNT_MOUNTED_COLL_ERR);
    }
    freeRodsObjStat (rodsObjStatOut);

    if (isCollEmpty (rsComm, phyPathRegInp->objPath) == False) {
        rodsLog (LOG_ERROR,
          "mountFileDir: collection %s not empty", phyPathRegInp->objPath);
        return (SYS_COLLECTION_NOT_EMPTY);
    }

    memset (&fileStatInp, 0, sizeof (fileStatInp));

    rstrcpy (fileStatInp.fileName, filePath, MAX_NAME_LEN);

    rescTypeInx = rescInfo->rescTypeInx;
    fileStatInp.fileType = (fileDriverType_t)RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileStatInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);
    status = rsFileStat (rsComm, &fileStatInp, &myStat);

    if (status < 0) {
	fileMkdirInp_t fileMkdirInp;

        rodsLog (LOG_NOTICE,
          "mountFileDir: rsFileStat failed for %s, status = %d, create it",
          fileStatInp.fileName, status);
	memset (&fileMkdirInp, 0, sizeof (fileMkdirInp));
	rstrcpy (fileMkdirInp.dirName, filePath, MAX_NAME_LEN);
        fileMkdirInp.fileType = (fileDriverType_t)RescTypeDef[rescTypeInx].driverType;
	fileMkdirInp.mode = getDefDirMode ();
        rstrcpy (fileMkdirInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);
	status = rsFileMkdir (rsComm, &fileMkdirInp);
	if (status < 0) {
            return (status);
	}
    } else if ((myStat->st_mode & S_IFDIR) == 0) {
        rodsLog (LOG_ERROR,
          "mountFileDir: phyPath %s is not a directory",
          fileStatInp.fileName);
	free (myStat);
        return (USER_FILE_DOES_NOT_EXIST);
    }

    free (myStat);
    /* mk the collection */

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&collCreateInp.condInput, COLLECTION_TYPE_KW, MOUNT_POINT_STR);

    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO1_KW, filePath);
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO2_KW, 
      rescInfo->rescName);

    /* try to mod the coll first */
    status = rsModColl (rsComm, &collCreateInp);

    if (status < 0) {	/* try to create it */
       status = rsRegColl (rsComm, &collCreateInp);
    }

    if (status >= 0) {
        char outLogPath[MAX_NAME_LEN];
	int status1;
	/* see if the phyPath is mapped into a real collection */
	if (getLogPathFromPhyPath (filePath, rescInfo, outLogPath) >= 0 &&
	  strcmp (outLogPath, phyPathRegInp->objPath) != 0) {
	    /* log path not the same as input objPath */
	    if (isColl (rsComm, outLogPath, NULL) >= 0) {
		modAccessControlInp_t modAccessControl;
		/* it is a real collection. better set the collection
		 * to read-only mode because any modification to files
		 * through this mounted collection can be trouble */
		bzero (&modAccessControl, sizeof (modAccessControl));
		modAccessControl.accessLevel = "read";
		modAccessControl.userName = rsComm->clientUser.userName;
		modAccessControl.zone = rsComm->clientUser.rodsZone;
		modAccessControl.path = phyPathRegInp->objPath;
                status1 = rsModAccessControl(rsComm, &modAccessControl);
                if (status1 < 0) {
                    rodsLog (LOG_NOTICE, 
		      "mountFileDir: rsModAccessControl err for %s, stat = %d",
                      phyPathRegInp->objPath, status1);
		}
	    }
	}
    }
    return (status);
}

int
unmountFileDir (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;
    collInp_t modCollInp;
    rodsObjStat_t *rodsObjStatOut = NULL;

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) {
        return status;
    } else if (rodsObjStatOut->specColl == NULL) {
        freeRodsObjStat (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "unmountFileDir: %s not mounted", phyPathRegInp->objPath);
        return (SYS_COLL_NOT_MOUNTED_ERR);
    }

    if (getStructFileType (rodsObjStatOut->specColl) >= 0) {    
	/* a struct file */
        status = _rsSyncMountedColl (rsComm, rodsObjStatOut->specColl,
          PURGE_STRUCT_FILE_CACHE);
#if 0
	if (status < 0) {
	    freeRodsObjStat (rodsObjStatOut);
	    return (status);
	}
#endif
    }

    freeRodsObjStat (rodsObjStatOut);

    memset (&modCollInp, 0, sizeof (modCollInp));
    rstrcpy (modCollInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&modCollInp.condInput, COLLECTION_TYPE_KW, 
      "NULL_SPECIAL_VALUE");
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO1_KW, "NULL_SPECIAL_VALUE");
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO2_KW, "NULL_SPECIAL_VALUE");

    status = rsModColl (rsComm, &modCollInp);

    return (status);
}

int
structFileReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    collInp_t collCreateInp;
    int status;
    dataObjInfo_t *dataObjInfo = NULL;
    char *structFilePath = NULL;
    dataObjInp_t dataObjInp;
    char *collType;
    int len;
    rodsObjStat_t *rodsObjStatOut = NULL;
    specCollCache_t *specCollCache = NULL;
    rescInfo_t *rescInfo = NULL;

#if 0	/* fixed */
    /* make it a privileged call for now */
    if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH)
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
#endif

    if ((structFilePath = getValByKey (&phyPathRegInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
          "structFileReg: No structFilePath input for %s",
          phyPathRegInp->objPath);
        return (SYS_INVALID_FILE_PATH);
    }

    collType = getValByKey (&phyPathRegInp->condInput, COLLECTION_TYPE_KW);
    if (collType == NULL) {
        rodsLog (LOG_ERROR,
          "structFileReg: Bad COLLECTION_TYPE_KW for structFilePath %s",
              dataObjInp.objPath);
            return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    len = strlen (phyPathRegInp->objPath);
    if (strncmp (structFilePath, phyPathRegInp->objPath, len) == 0 &&
     (structFilePath[len] == '\0' || structFilePath[len] == '/')) {
        rodsLog (LOG_ERROR,
          "structFileReg: structFilePath %s inside collection %s",
          structFilePath, phyPathRegInp->objPath);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }

    /* see if the struct file is in spec coll */

    if (getSpecCollCache (rsComm, structFilePath, 0,  &specCollCache) >= 0) {
        rodsLog (LOG_ERROR,
          "structFileReg: structFilePath %s is in a mounted path",
          structFilePath);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) return status;
 
    if (rodsObjStatOut->specColl != NULL) {
	freeRodsObjStat (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "structFileReg: %s already mounted", phyPathRegInp->objPath);
	return (SYS_MOUNT_MOUNTED_COLL_ERR);
    }

    freeRodsObjStat (rodsObjStatOut);

    if (isCollEmpty (rsComm, phyPathRegInp->objPath) == False) {
        rodsLog (LOG_ERROR,
          "structFileReg: collection %s not empty", phyPathRegInp->objPath);
        return (SYS_COLLECTION_NOT_EMPTY);
    }

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, structFilePath, sizeof (dataObjInp));
    /* user need to have write permission */
    dataObjInp.openFlags = O_WRONLY;
    status = getDataObjInfoIncSpecColl (rsComm, &dataObjInp, &dataObjInfo);
    if (status < 0) {
	int myStatus;
	/* try to make one */
	dataObjInp.condInput = phyPathRegInp->condInput;
	/* have to remove FILE_PATH_KW because getFullPathName will use it */
	rmKeyVal (&dataObjInp.condInput, FILE_PATH_KW);
	myStatus = rsDataObjCreate (rsComm, &dataObjInp);
	if (myStatus < 0) {
            rodsLog (LOG_ERROR,
              "structFileReg: Problem with open/create structFilePath %s, status = %d",
              dataObjInp.objPath, status);
            return (status);
	} else {
	    openedDataObjInp_t dataObjCloseInp;
	    bzero (&dataObjCloseInp, sizeof (dataObjCloseInp));
	    rescInfo = L1desc[myStatus].dataObjInfo->rescInfo;
	    dataObjCloseInp.l1descInx = myStatus;
	    rsDataObjClose (rsComm, &dataObjCloseInp);
	}
    } else {
	rescInfo = dataObjInfo->rescInfo;
    }

    if (!structFileSupport (rsComm, phyPathRegInp->objPath, 
      collType, rescInfo)) {
        rodsLog (LOG_ERROR,
          "structFileReg: structFileDriver type %s does not exist for %s",
          collType, dataObjInp.objPath);
        return (SYS_NOT_SUPPORTED);
    }

    /* mk the collection */

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&collCreateInp.condInput, COLLECTION_TYPE_KW, collType);

    /* have to use dataObjInp.objPath because structFile path was removed */ 
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO1_KW, 
     dataObjInp.objPath);

    char collInfo2[MAX_NAME_LEN];
    snprintf(collInfo2, MAX_NAME_LEN, ";;;%s;;;0", rescInfo->rescName);
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO2_KW, collInfo2);

    /* try to mod the coll first */
    status = rsModColl (rsComm, &collCreateInp);

    if (status < 0) {	/* try to create it */
       status = rsRegColl (rsComm, &collCreateInp);
    }

    return (status);
}

int
structFileSupport (rsComm_t *rsComm, char *collection, char *collType, 
rescInfo_t *rescInfo)
{
    rodsStat_t *myStat = NULL;
    int status;
    subFile_t subFile;
    specColl_t specColl;

    if (rsComm == NULL || collection == NULL || collType == NULL || 
      rescInfo == NULL) return 0;

    memset (&subFile, 0, sizeof (subFile));
    memset (&specColl, 0, sizeof (specColl));
    /* put in some fake path */
    subFile.specColl = &specColl;
    rstrcpy (specColl.collection, collection, MAX_NAME_LEN);
    specColl.collClass = STRUCT_FILE_COLL;
    if (strcmp (collType, HAAW_STRUCT_FILE_STR) == 0) { 
        specColl.type = HAAW_STRUCT_FILE_T;
    } else if (strcmp (collType, TAR_STRUCT_FILE_STR) == 0) {
	specColl.type = TAR_STRUCT_FILE_T;
    } else if (strcmp (collType, MSSO_STRUCT_FILE_STR) == 0) {
	specColl.type = MSSO_STRUCT_FILE_T;
    } else {
	return (0);
    }
    snprintf (specColl.objPath, MAX_NAME_LEN, "%s/myFakeFile",
      collection);
    rstrcpy (specColl.resource, rescInfo->rescName, NAME_LEN);
    rstrcpy (specColl.phyPath, "/fakeDir1/fakeDir2/myFakeStructFile",
      MAX_NAME_LEN);
    rstrcpy (subFile.subFilePath, "/fakeDir1/fakeDir2/myFakeFile",
      MAX_NAME_LEN);
    rstrcpy (subFile.addr.hostAddr, rescInfo->rescLoc, NAME_LEN);

    status = rsSubStructFileStat (rsComm, &subFile, &myStat);

    if (status == SYS_NOT_SUPPORTED)
	return (0);
    else
	return (1);
}

int
linkCollReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    collInp_t collCreateInp;
    int status;
    char *linkPath = NULL;
    char *collType;
    int len;
    rodsObjStat_t *rodsObjStatOut = NULL;
    specCollCache_t *specCollCache = NULL;

    if ((linkPath = getValByKey (&phyPathRegInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
          "linkCollReg: No linkPath input for %s",
          phyPathRegInp->objPath);
        return (SYS_INVALID_FILE_PATH);
    }

    collType = getValByKey (&phyPathRegInp->condInput, COLLECTION_TYPE_KW);
    if (collType == NULL || strcmp (collType, LINK_POINT_STR) != 0) {
        rodsLog (LOG_ERROR,
          "linkCollReg: Bad COLLECTION_TYPE_KW for linkPath %s",
              phyPathRegInp->objPath);
            return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (phyPathRegInp->objPath[0] != '/' || linkPath[0] != '/') {
        rodsLog (LOG_ERROR,
          "linkCollReg: linkPath %s or collection %s not absolute path",
          linkPath, phyPathRegInp->objPath);
        return (SYS_COLL_LINK_PATH_ERR);
    }

    len = strlen (phyPathRegInp->objPath);
    if (strncmp (linkPath, phyPathRegInp->objPath, len) == 0 && 
      linkPath[len] == '/') { 
        rodsLog (LOG_ERROR,
          "linkCollReg: linkPath %s inside collection %s",
          linkPath, phyPathRegInp->objPath);
        return (SYS_COLL_LINK_PATH_ERR);
    }

    len = strlen (linkPath);
    if (strncmp (phyPathRegInp->objPath, linkPath, len) == 0 &&
      phyPathRegInp->objPath[len] == '/') {
        rodsLog (LOG_ERROR,
          "linkCollReg: collection %s inside linkPath %s",
          linkPath, phyPathRegInp->objPath);
        return (SYS_COLL_LINK_PATH_ERR);
    }

    if (getSpecCollCache (rsComm, linkPath, 0,  &specCollCache) >= 0 &&
     specCollCache->specColl.collClass != LINKED_COLL) {
        rodsLog (LOG_ERROR,
          "linkCollReg: linkPath %s is in a spec coll path",
          linkPath);
        return (SYS_COLL_LINK_PATH_ERR);
    }

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) {
	/* does not exist. make one */
	collInp_t collCreateInp;
        memset (&collCreateInp, 0, sizeof (collCreateInp));
        rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
        status = rsRegColl (rsComm, &collCreateInp);
	if (status < 0) {
            rodsLog (LOG_ERROR,
               "linkCollReg: rsRegColl error for  %s, status = %d",
               collCreateInp.collName, status);
             return status;
        }
        status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
	if (status < 0) return status;

    }

    if (rodsObjStatOut->specColl != NULL && 
      rodsObjStatOut->specColl->collClass != LINKED_COLL) {
        freeRodsObjStat (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "linkCollReg: link collection %s in a spec coll path", 
	  phyPathRegInp->objPath);
        return (SYS_COLL_LINK_PATH_ERR);
    }

    freeRodsObjStat (rodsObjStatOut);

    if (isCollEmpty (rsComm, phyPathRegInp->objPath) == False) {
        rodsLog (LOG_ERROR,
          "linkCollReg: collection %s not empty", phyPathRegInp->objPath);
        return (SYS_COLLECTION_NOT_EMPTY);
    }

    /* mk the collection */

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&collCreateInp.condInput, COLLECTION_TYPE_KW, collType);

    /* have to use dataObjInp.objPath because structFile path was removed */
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO1_KW,
     linkPath);

    /* try to mod the coll first */
    status = rsModColl (rsComm, &collCreateInp);

    if (status < 0) {   /* try to create it */
       status = rsRegColl (rsComm, &collCreateInp);
    }

    return (status);
}

pathnamePatterns_t *
readPathnamePatternsFromFile(rsComm_t *rsComm, char *filename, rescInfo_t *rescInfo)
{
    int status;
    rodsStat_t *stbuf;
    fileStatInp_t fileStatInp;
    bytesBuf_t fileReadBuf;
    fileOpenInp_t fileOpenInp;
    fileReadInp_t fileReadInp;
    fileCloseInp_t fileCloseInp;
    int buf_len, fd;
    pathnamePatterns_t *pp;

    if (rsComm == NULL || filename == NULL || rescInfo == NULL) {
        return NULL;
    }

    memset(&fileStatInp, 0, sizeof(fileStatInp));
    rstrcpy(fileStatInp.fileName, filename, MAX_NAME_LEN);
    fileStatInp.fileType = (fileDriverType_t)RescTypeDef[rescInfo->rescTypeInx].driverType;
    rstrcpy(fileStatInp.addr.hostAddr, rescInfo->rescLoc, NAME_LEN);
    status = rsFileStat(rsComm, &fileStatInp, &stbuf);
    if (status != 0) {
        if (status != UNIX_FILE_STAT_ERR - ENOENT) {
            rodsLog(LOG_DEBUG, "readPathnamePatternsFromFile: can't stat %s. status = %d",
                    fileStatInp.fileName, status);
        }
        return NULL;
    }
    buf_len = stbuf->st_size;
    free(stbuf);
    
    memset(&fileOpenInp, 0, sizeof(fileOpenInp));
    rstrcpy(fileOpenInp.fileName, filename, MAX_NAME_LEN);
    fileOpenInp.fileType = (fileDriverType_t)RescTypeDef[rescInfo->rescTypeInx].driverType;
    rstrcpy(fileOpenInp.addr.hostAddr, rescInfo->rescLoc, NAME_LEN);
    fileOpenInp.flags = O_RDONLY;
    fd = rsFileOpen(rsComm, &fileOpenInp);
    if (fd < 0) {
        rodsLog(LOG_NOTICE, 
                "readPathnamePatternsFromFile: can't open %s for reading. status = %d",
                fileOpenInp.fileName, fd);
        return NULL;
    }

    memset(&fileReadBuf, 0, sizeof(fileReadBuf));
    fileReadBuf.buf = malloc(buf_len);
    if (fileReadBuf.buf == NULL) {
        rodsLog(LOG_NOTICE, "readPathnamePatternsFromFile: could not malloc buffer");
        return NULL;
    }
    
    memset(&fileReadInp, 0, sizeof(fileReadInp));
    fileReadInp.fileInx = fd;
    fileReadInp.len = buf_len;
    status = rsFileRead(rsComm, &fileReadInp, &fileReadBuf);
    
    memset(&fileCloseInp, 0, sizeof(fileCloseInp));
    fileCloseInp.fileInx = fd;
    rsFileClose(rsComm, &fileCloseInp);
    
    if (status < 0) {
        rodsLog(LOG_NOTICE, "readPathnamePatternsFromFile: could not read %s. status = %d",
                fileOpenInp.fileName, status);
        free(fileReadBuf.buf);
        return NULL;
    }

    pp = readPathnamePatterns((char*)fileReadBuf.buf, buf_len);

    return pp;
}
