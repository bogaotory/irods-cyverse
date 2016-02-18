# VERSION 2
#
# These are the rules related to UUIDs

ipc_uuidGenerate {
  *status = errorcode(msiExecCmd("generateuuid.sh", "", ipc_RE_HOST, "null", "null", *out));
  if (*status == 0) {
    msiGetStdoutInExecCmdOut(*out, *uuid);
    trimr(*uuid, "\n");
  } else {
    writeLine("serverLog", "failed to generate UUID");
    fail;
  }
}
