#!/bin/bash -x

# For iRODS 3.3.1, the patches (updates) for the ICAT are all new
# specific queries, so instead of updates at the DBMS level this is
# done via the following iadmin commands.  To make the patch, start up
# irods 3.3.1, authenticate as the admin (iinit), and then run this
# script (./patch3.3to3.3.1.sh) to add these new specific queries.
#
# Some of these were defined in a previous patch, patch3.2to3.3.sh,
# but to make sure your instance is using the current definitions, the
# existing ones, if any, are first removed.

# These are used by Jargon in cases where a general-query either will
# not work or performs poorly.

iadmin rsq listQueryByAliasLike
iadmin rsq findQueryByAlias
iadmin rsq ilsLACollections
iadmin rsq ilsLADataObjects
iadmin rsq listUserACLForDataObjViaGroup
iadmin rsq listUserACLForCollectionViaGroup

iadmin asq "select alias,sqlStr from R_SPECIFIC_QUERY where alias like ?" listQueryByAliasLike

iadmin asq "select alias,sqlStr from R_SPECIFIC_QUERY where alias = ?" findQueryByAlias

iadmin asq "SELECT c.parent_coll_name, c.coll_name, c.create_ts, c.modify_ts, c.coll_id, c.coll_owner_name, c.coll_owner_zone,c.coll_type, u.user_name, u.zone_name, a.access_type_id, u.user_id FROM r_coll_main c JOIN r_objt_access a ON c.coll_id = a.object_id JOIN r_user_main u ON a.user_id = u.user_id WHERE c.parent_coll_name = ? ORDER BY r_coll_main.coll_name, r_user_main.user_name,r_objt_access.access_type_id DESC LIMIT ? OFFSET ?" ilsLACollections

iadmin asq "SELECT s.coll_name, s.data_name, s.create_ts, s.modify_ts, s.data_id, s.data_size, s.data_repl_num, s.data_owner_name, s.data_owner_zone, u.user_name, u.user_id, a.access_type_id,  u.user_type_name, u.zone_name FROM ( SELECT c.coll_name, d.data_name, d.create_ts, d.modify_ts, d.data_id, d.data_repl_num, d.data_size, d.data_owner_name, d.data_owner_zone FROM r_coll_main c JOIN r_data_main d ON c.coll_id = d.coll_id  WHERE c.coll_name = ?  ORDER BY d.data_name) s JOIN r_objt_access a ON s.data_id = a.object_id JOIN r_user_main u ON a.user_id = u.user_id ORDER BY r_coll_main.coll_name, r_data_main.data_name, r_user_main.user_name, r_objt_access.access_type_id DESC LIMIT ? OFFSET ?" ilsLADataObjects

iadmin asq "SELECT  r_user_main.user_name, r_user_main.user_id, r_objt_access.access_type_id, r_user_main.user_type_name, r_user_main.zone_name,  r_coll_main.coll_name, r_data_main.data_name, USER_GROUP_MAIN.user_name, r_data_main.data_name,r_coll_main.coll_name FROM r_user_main AS USER_GROUP_MAIN JOIN r_user_group JOIN r_user_main ON r_user_group.user_id = r_user_main.user_id ON USER_GROUP_MAIN.user_id = r_user_group.group_user_id JOIN r_objt_access ON r_user_group.group_user_id = r_objt_access.user_id JOIN r_data_main JOIN r_coll_main ON r_data_main.coll_id = r_coll_main.coll_id ON r_objt_access.object_id = r_data_main.data_id WHERE  r_coll_main.coll_name = ? AND r_data_main.data_name = ? AND   r_user_main.user_name = ? ORDER BY  r_coll_main.coll_name, r_data_main.data_name,r_user_main.user_name, r_objt_access.access_type_id DESC" listUserACLForDataObjViaGroup

iadmin asq "SELECT  r_user_main.user_name, r_user_main.user_id, r_objt_access.access_type_id, r_user_main.user_type_name, r_user_main.zone_name,  r_coll_main.coll_name, USER_GROUP_MAIN.user_name,r_coll_main.coll_name FROM r_user_main AS USER_GROUP_MAIN JOIN r_user_group  JOIN r_user_main ON r_user_group.user_id = r_user_main.user_id ON USER_GROUP_MAIN.user_id = r_user_group.group_user_id JOIN r_objt_access ON r_user_group.group_user_id = r_objt_access.user_id JOIN r_coll_main ON r_objt_access.object_id = r_coll_main.coll_id WHERE  r_coll_main.coll_name = ?  AND   r_user_main.user_name = ? ORDER BY r_coll_main.coll_name, r_user_main.user_name,r_objt_access.access_type_id DESC" listUserACLForCollectionViaGroup
