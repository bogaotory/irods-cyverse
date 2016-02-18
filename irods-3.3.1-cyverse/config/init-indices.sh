#! /bin/bash
#
# This script creates the custom indices for iPlant Collaborative's iRODS deployment.
#

# if [ $# -lt 4 ]
# then
#     echo "Usage: $0 <icat-db-server-host> <port> <user> <password>"
#     exit 1
# fi

# export PGHOST=$1
# export PGPORT=$2
# export PGDATABASE=ICAT
# export PGUSER="$3"
# export PGPASSWORD="$4"

declare -A index_defs
index_defs[idx_coll_main_coll_type]='r_coll_main (coll_type)'
index_defs[idx_coll_main_parent_coll_name]='r_coll_main (parent_coll_name)'
index_defs[idx_objt_access_access_type_id]='r_objt_access (access_type_id)'
index_defs[idx_objt_access_user_id]='r_objt_access (user_id)'
index_defs[idx_tokn_main_token_namespace]='r_tokn_main (token_namespace)'
index_defs[idx_user_main_user_type_name]='r_user_main (user_type_name)'
index_defs[idx_user_password_user_id]='r_user_password (user_id)'

function query-db () {
    command=$1

    psql --no-align --tuples-only -d ICAT --command="$command"
}

function index-exists() {
    index=$1

    query-db "SELECT COUNT(*) > 0 FROM pg_indexes WHERE indexname='$index'"
}

for idx in "${!index_defs[@]}"
do
    if [ "$(index-exists $idx)" = "f" ]
    then
        echo "Creating index $idx"
        query-db "CREATE INDEX $idx ON ${index_defs[$idx]}" > /dev/null
    fi
done
