/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/*
 Simple program to test the Rcat Low Level routines to Postgres or Oracle.
 */

#include "rods.h"
#include "icatLowLevel.h"

/*int ProcessType=CLIENT_PT; */

int
main(int argc, char **argv) {
   int i;

   i = cllTest(argv[1], argv[2]);
   return(i);
}

/* This is a dummy version of icatApplyRule for this test program so
   the rule-engine is not needed in this ICAT test. */
int
icatApplyRule(rsComm_t *rsComm, char *ruleName, char *arg1) {
  return(0);
}
