#!/bin/bash -l

uuidd
echo "Starting iRODS..."
irodsctl start
echo "iRODS started"
echo $PWD
sleep infinity