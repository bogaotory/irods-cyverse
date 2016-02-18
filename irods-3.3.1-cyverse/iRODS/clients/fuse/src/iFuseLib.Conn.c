/*** For more information please refer to files in the COPYRIGHT directory ***/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "irodsFs.h"
#include "iFuseLib.h"
#include "iFuseOper.h"
#include "hashtable.h"
#include "list.h"
#include "iFuseLib.Lock.h"

/*
 * An ifuseconn->inuseLock is locked when before it goes from free to inuse and during the whole time it is in use, it is only unlocked when it is unused.
 */
concurrentList_t *ConnectedConn;
concurrentList_t *FreeConn;
concurrentList_t *ConnReqWaitQue;

static int ConnManagerStarted = 0;

void initConn() {
	ConnectedConn = newConcurrentList();
	FreeConn = newConcurrentList();
	ConnReqWaitQue = newConcurrentList();
}
/* getIFuseConnByPath - try to use the same conn as opened desc of the
 * same path */
iFuseConn_t *getAndUseConnByPath (char *localPath, rodsEnv *myRodsEnv, int *status)
{
    iFuseConn_t *iFuseConn;
    /* make sure iFuseConn is not released after getAndLockIFuseDescByPath finishes */
    pathCache_t *tmpPathCache;
    matchAndLockPathCache(localPath, &tmpPathCache);

    if(tmpPathCache!=NULL) {
        *status = _getAndUseConnForPathCache (&iFuseConn, tmpPathCache);
    	UNLOCK_STRUCT (*tmpPathCache);
	} else {
		/* no match. just assign one */
		*status = getAndUseIFuseConn (&iFuseConn, myRodsEnv);

	}

    return iFuseConn;
}

/* precond: lock paca */
int _getAndUseConnForPathCache(iFuseConn_t **iFuseConn, pathCache_t *paca) {
	int status;
	/* if connection already closed by connection manager, get new ifuseconn */
    if(paca->iFuseConn != NULL) {
    	LOCK_STRUCT(*(paca->iFuseConn));
    	if(paca->iFuseConn->conn != NULL) {
    		if(paca->iFuseConn->inuseCnt == 0) {
				_useIFuseConn(paca->iFuseConn);
				UNLOCK_STRUCT(*(paca->iFuseConn));
				/* ifuseReconnect(paca->iFuseConn); */
				*iFuseConn = paca->iFuseConn;
				return 0;
    		} else {
				UNLOCK_STRUCT(*(paca->iFuseConn));
    		}
    	} else {
    		UNLOCK_STRUCT(*(paca->iFuseConn));
    		/* disconnect by not freed yet */
			UNREF(paca->iFuseConn, IFuseConn);
    	}
    }

    iFuseConn_t *tmpIFuseConn;
	UNLOCK_STRUCT(*paca);
	status = getAndUseIFuseConn(&tmpIFuseConn, &MyRodsEnv);
	LOCK_STRUCT(*paca);
	if(status < 0) {
		rodsLog (LOG_ERROR,
			  "ifuseClose: cannot get ifuse connection for %s error, status = %d",
			   paca->localPath, status);
		return status;
	}
	if(paca->iFuseConn != NULL) {
		/* has been changed by other threads, or current paca->ifuseconn inuse,
		 * return new ifuseconn without setting paca->ifuseconn */
		*iFuseConn = tmpIFuseConn;
	} else {
		/* conn in use, cannot be deleted by conn manager
		 * therefore, it is safe to do the following without locking conn */
		REF(paca->iFuseConn, tmpIFuseConn);
		*iFuseConn = paca->iFuseConn;
	}
    return 0;
}

int getAndUseIFuseConn (iFuseConn_t **iFuseConn, rodsEnv *myRodsEnv) {
	int ret = _getAndUseIFuseConn(iFuseConn, myRodsEnv);
	return ret;

}

void _waitForConn() {
    connReqWait_t myConnReqWait;
	bzero (&myConnReqWait, sizeof (myConnReqWait));
	initConnReqWaitMutex(&myConnReqWait);
	addToConcurrentList(ConnReqWaitQue, &myConnReqWait);

	while (myConnReqWait.state == 0) {
		timeoutWait (&myConnReqWait.mutex, &myConnReqWait.cond, CONN_REQ_SLEEP_TIME);
	}

	deleteConnReqWaitMutex(&myConnReqWait);
}

int _getAndUseIFuseConn (iFuseConn_t **iFuseConn, rodsEnv *myRodsEnv) {
    int status;
    iFuseConn_t *tmpIFuseConn;

    *iFuseConn = NULL;

    while (*iFuseConn == NULL) {
        /* get a free IFuseConn */

        if (listSize(ConnectedConn) >= MAX_NUM_CONN && listSize(FreeConn) == 0) {
            /* have to wait */
        	_waitForConn();
			/* start from begining */
			continue;
        } else {
			tmpIFuseConn = (iFuseConn_t *) removeFirstElementOfConcurrentList(FreeConn);
			if(tmpIFuseConn == NULL) {
				if(listSize(ConnectedConn) < MAX_NUM_CONN) {
			        /* may cause num of conn > max num of conn */
			        /* get here when nothing free. make one */
			        tmpIFuseConn = newIFuseConn(&status);
			        if (status < 0) return status;

			        _useFreeIFuseConn (tmpIFuseConn);
			        addToConcurrentList(ConnectedConn, tmpIFuseConn);

			        *iFuseConn = tmpIFuseConn;
			        break;

				}
				_waitForConn();
				continue;
			} else {
				useIFuseConn(tmpIFuseConn);
				*iFuseConn = tmpIFuseConn;
				break;
			}
        }


    }	/* while *iFuseConn */

    if (++ConnManagerStarted == HIGH_NUM_CONN) {
	/* don't do it the first time */

#ifdef USE_BOOST
    	ConnManagerThr = new boost::thread( connManager );
#else
        status = pthread_create  (&ConnManagerThr, pthread_attr_default,(void *(*)(void *)) connManager, (void *) NULL);
#endif
        if (status < 0) {
        	ConnManagerStarted--;
            rodsLog (LOG_ERROR, "pthread_create failure, status = %d", status);
        }
    }
    return 0;
}

int
useIFuseConn (iFuseConn_t *iFuseConn)
{
    int status;
    if (iFuseConn == NULL || iFuseConn->conn == NULL)
        return USER__NULL_INPUT_ERR;
    LOCK_STRUCT (*iFuseConn);
    status = _useIFuseConn (iFuseConn);
    UNLOCK_STRUCT (*iFuseConn);

    return status;
}

int
_useIFuseConn (iFuseConn_t *iFuseConn)
{
    if (iFuseConn == NULL || iFuseConn->conn == NULL) 
	return USER__NULL_INPUT_ERR;

    iFuseConn->actTime = time (NULL);
    iFuseConn->pendingCnt++;
    UNLOCK_STRUCT (*iFuseConn);

    /* wait for iFuseConn to be unlocked */
    LOCK (iFuseConn->inuseLock);

    LOCK_STRUCT(*iFuseConn);
    iFuseConn->inuseCnt++;
    iFuseConn->pendingCnt--;

    /* move unlock to caller site */
    /* UNLOCK (ConnLock); */
    return 0;
}

int
_useFreeIFuseConn (iFuseConn_t *iFuseConn)
{
    if (iFuseConn == NULL) return USER__NULL_INPUT_ERR;
    iFuseConn->actTime = time (NULL);
    iFuseConn->inuseCnt++;
    LOCK(iFuseConn->inuseLock);
    return 0;
}

int
unuseIFuseConn (iFuseConn_t *iFuseConn)
{
    if (iFuseConn == NULL || iFuseConn->conn == NULL)
        return USER__NULL_INPUT_ERR;
	LOCK_STRUCT(*iFuseConn);
    iFuseConn->actTime = time (NULL);
    iFuseConn->inuseCnt--;
    if(iFuseConn->pendingCnt == 0) {
    	addToConcurrentList(FreeConn, iFuseConn);
    }
    UNLOCK_STRUCT(*iFuseConn);
    UNLOCK(iFuseConn->inuseLock);
    signalConnManager ();
    return 0;
}

int
ifuseConnect (iFuseConn_t *iFuseConn, rodsEnv *myRodsEnv)
{
	int status = 0;
	LOCK_STRUCT(*iFuseConn);
	if(iFuseConn->conn == NULL) {
		rErrMsg_t errMsg;
		iFuseConn->conn = rcConnect (myRodsEnv->rodsHost, myRodsEnv->rodsPort,
		  myRodsEnv->rodsUserName, myRodsEnv->rodsZone, NO_RECONN, &errMsg);

		if (iFuseConn->conn == NULL) {
		/* try one more */
			iFuseConn->conn = rcConnect (myRodsEnv->rodsHost, myRodsEnv->rodsPort,
			  myRodsEnv->rodsUserName, myRodsEnv->rodsZone, NO_RECONN, &errMsg);
		if (iFuseConn->conn == NULL) {
				rodsLogError (LOG_ERROR, errMsg.status,
				  "ifuseConnect: rcConnect failure %s", errMsg.msg);
				if (errMsg.status < 0) {
					return (errMsg.status);
				} else {
					return (-1);
				}
			}
		}

		status = clientLogin (iFuseConn->conn);
		if (status != 0) {
			rcDisconnect (iFuseConn->conn);
			iFuseConn->conn=NULL;
		}
	}
    UNLOCK_STRUCT(*iFuseConn);
    return (status);
}

void _ifuseDisconnect(iFuseConn_t *tmpIFuseConn) {
	if (tmpIFuseConn->conn != NULL) {
		rcDisconnect (tmpIFuseConn->conn);
		tmpIFuseConn->conn=NULL;
	}
}

void ifuseDisconnect(iFuseConn_t *tmpIFuseConn) {
	LOCK_STRUCT(*tmpIFuseConn);
	_ifuseDisconnect(tmpIFuseConn);
	UNLOCK_STRUCT(*tmpIFuseConn);
}


int
signalConnManager ()
{
	if (listSize(ConnectedConn) > HIGH_NUM_CONN) {
        notifyTimeoutWait(&ConnManagerLock, &ConnManagerCond);
    }
    return 0;
}

int
disconnectAll ()
{
    iFuseConn_t *tmpIFuseConn;
    while ((tmpIFuseConn = (iFuseConn_t *) removeFirstElementOfConcurrentList(ConnectedConn)) != NULL) {
    	ifuseDisconnect(tmpIFuseConn);
	}
    return 0;
}
/* have to do this after getIFuseConn - lock */
int
ifuseReconnect (iFuseConn_t *iFuseConn)
{
    int status = 0;

    if (iFuseConn == NULL || iFuseConn->conn == NULL)
    	return USER__NULL_INPUT_ERR;

    rodsLog (LOG_DEBUG, "ifuseReconnect: reconnecting");
    ifuseDisconnect (iFuseConn);
    status = ifuseConnect (iFuseConn, &MyRodsEnv);
    return status;
}

void
connManager ()
{
    time_t curTime;
    iFuseConn_t *tmpIFuseConn;
    ListNode *node;
    List *TimeOutList = newListNoRegion();

    while (1) {
        curTime = time (NULL);

		/* exceed high water mark for number of connection ? */
		if (listSize(ConnectedConn) > HIGH_NUM_CONN) {

			LOCK_STRUCT(*ConnectedConn);
			int disconnTarget = _listSize(ConnectedConn) - HIGH_NUM_CONN;
	        node = ConnectedConn->list->head;
	        while (node != NULL) {
	        	tmpIFuseConn = (iFuseConn_t *) node->value;
				if (tmpIFuseConn->inuseCnt == 0 && tmpIFuseConn->pendingCnt == 0 && curTime - tmpIFuseConn->actTime > IFUSE_CONN_TIMEOUT) {
					listAppendNoRegion(TimeOutList, tmpIFuseConn);
					node = node->next;
				}
			}
	        UNLOCK_STRUCT(*ConnectedConn);

			node = TimeOutList->head;
			int disconned = 0;
			while (node != NULL && disconned < disconnTarget) {
				tmpIFuseConn = (iFuseConn_t *) node->value;
				LOCK_STRUCT(*tmpIFuseConn);
				if (tmpIFuseConn->inuseCnt == 0 && tmpIFuseConn->pendingCnt == 0 && curTime - tmpIFuseConn->actTime > IFUSE_CONN_TIMEOUT) {
					removeFromConcurrentList2(FreeConn, tmpIFuseConn);
					removeFromConcurrentList2(ConnectedConn, tmpIFuseConn);
					if(tmpIFuseConn->status == 0) { /* no struct is referring to it, we can unlock it and free it */
						UNLOCK_STRUCT(*tmpIFuseConn);
						/* rodsLog(LOG_ERROR, "[FREE IFUSE CONN] %s:%d %p", __FILE__, __LINE__, tmpIFuseConn); */
						_freeIFuseConn(tmpIFuseConn);
					} else { /* set to timed out */
						_ifuseDisconnect(tmpIFuseConn);
						UNLOCK_STRUCT(*tmpIFuseConn);
					}
					disconned ++;
				} else {
					UNLOCK_STRUCT(*tmpIFuseConn);
				}
				node = node->next;
			}
	    	clearListNoRegion(TimeOutList);
		}

		while (listSize(ConnectedConn) <= MAX_NUM_CONN || listSize(FreeConn) != 0) {
			/* signal one in the wait queue */
			connReqWait_t *myConnReqWait = (connReqWait_t *) removeFirstElementOfConcurrentList(ConnReqWaitQue);
			/* if there is no conn req left, exit loop */
			if(myConnReqWait == NULL) {
				break;
			}
			myConnReqWait->state = 1;
			notifyTimeoutWait(&myConnReqWait->mutex, &myConnReqWait->cond);
		}
	#if 0
			rodsSleep (CONN_MANAGER_SLEEP_TIME, 0);
	#else
			timeoutWait(&ConnManagerLock, &ConnManagerCond, CONN_MANAGER_SLEEP_TIME);
	#endif
	

    }
    deleteListNoRegion(TimeOutList);
}



