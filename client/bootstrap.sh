#!/bin/bash -l

until $(imiscsvrinfo &> /dev/null)
do
	sleep 5
done
printf "iRODS server is online. \nTry execute \"docker exec -it ${PROJECT_NAME}_${CLIENT_HOSTNAME}_1 bash\" to attach to the client server. \n"

sleep infinity