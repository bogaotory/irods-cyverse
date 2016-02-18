# VERSION 2
#
# These are rules for constructing serialized JSON documents. This version only supports string and
# object field types.

encodeObject(*SerialFields) {
  *res = '{';
  if (size(*SerialFields) > 0) {
	*res = *res ++ hd(*SerialFields);
    foreach(*field in tl(*SerialFields)) {
      *res = *res ++ ',' ++ *field;
    }
  }
  *res ++ '}';
}

encodeString(*Str) {
  *escStr = '';
  *len = strlen(*Str);
  *pos = 0;
  while (*len > *pos) {
    *c = substr(*Str, *pos, *pos + 1);
    *escStr = *escStr ++ (match *c with
                            | '"' => '\\"'
                            | '\\' => '\\\\'
                            | *_ => *c);
    *pos = *pos + 1;
  }
  '"' ++ *escStr ++ '"';
}

mkField(*Label, *SerialVal) = '"' ++ *Label ++ '":' ++ *SerialVal

# construct a serialized JSON number field. This version performs no error checking.
#
# Parameters:
#   *Label - the name of the field
#   *Val - the value of the field
#
ipc_jsonNumber(*Label, *Val) = mkField(*Label, '*Val')

# construct a serialized JSON string field
#
# Parameters:
#   *Label - the name of the field
#   *Val - the value of the field
#
ipc_jsonString(*Label, *Val) = mkField(*Label, encodeString(*Val))

# construct a serialized JSON array field from its serialized elements
#
# Parameters:
#   *Label - the name of the field
#   *Val - the serialized array values
#
ipc_jsonStringArray(*Label, *Values) {
  *serialArray = '[';
  if (size(*Values) > 0) {
    *serialArray = *serialArray ++ encodeString(hd(*Values));
    foreach (*val in tl(*Values)) {
      *serialArray = *serialArray ++ ',' ++ encodeString(*val);
    }
  }
  *serialArray = *serialArray ++ ']';
  mkField(*Label, *serialArray);
}

# construct a serialized JSON object field from its serialized fields
#
# Parameters:
#   *Label - the name of the object field
#   *SerialFields - the list of pre-serialized fields ot include in the object
#
ipc_jsonObject(*Label, *SerialFields) = mkField(*Label, encodeObject(*SerialFields))

# construct a serialized JSON document from its serialized fields
#
# Parameters:
#   *SerialFields - A list of pre-serialized fields to include in the document
#
ipc_jsonDocument(*SerialFields) = encodeObject(*SerialFields)

# construct a serialized JSON object field from a Boolean value
#
# Parameters:
#   *Label - the name of the object field
#   *flag - the Boolean value of the field.
#
ipc_jsonBoolean(*Label, *flag) {
  if (*flag) {
    mkField(*Label, "true");
  } else {
    mkField(*Label, "false");
  }
}