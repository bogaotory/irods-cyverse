#include "setAVUMS.h"

#include <stddef.h>

#include "modAVUMetadata.h"
#include "reDefines.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"


#define VALIDATE_NOT_NULL(PARAM) \
  do { \
    if (!(PARAM)) { \
      rodsLog (LOG_ERROR, "msiSetAVU: input #PARAM is NULL"); \
      return SYS_INTERNAL_NULL_INPUT_ERR; \
    } \
  } while (0)


int
msiSetAVU(msParam_t * const itemTypeParam,
          msParam_t * const itemNameParam,
          msParam_t * const attrNameParam,
          msParam_t * const attrValParam,
          msParam_t * const attrUnitParam,
          ruleExecInfo_t * const rei)
{
  RE_TEST_MACRO("    Calling msiSetAVU")

  VALIDATE_NOT_NULL(itemTypeParam);
  VALIDATE_NOT_NULL(itemNameParam);
  VALIDATE_NOT_NULL(attrNameParam);
  VALIDATE_NOT_NULL(attrValParam);
  VALIDATE_NOT_NULL(attrUnitParam);
  VALIDATE_NOT_NULL(rei);

  if (!rei->rsComm) {
		rodsLog (LOG_ERROR, "msiSetAVU: input rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
  }

  modAVUMetadataInp_t avuOp;
  avuOp.arg0 = "set";
  avuOp.arg1 = parseMspForStr(itemTypeParam);
  avuOp.arg2 = parseMspForStr(itemNameParam);
  avuOp.arg3 = parseMspForStr(attrNameParam);
  avuOp.arg4 = parseMspForStr(attrValParam);
  avuOp.arg5 = parseMspForStr(attrUnitParam);
  avuOp.arg6 = "";
  avuOp.arg7 = "";
  avuOp.arg8 = "";
  avuOp.arg9 = "";

  rei->status = rsModAVUMetadata(rei->rsComm, &avuOp);
  return rei->status;
}


#undef VALIDATE_NOT_NULL
