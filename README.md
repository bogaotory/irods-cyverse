# cyverse-irods
This is an example project to demonstrate how to dockerise a CyVerse-compatible iRODS server. As well as an ICAT-Enabled Server (IES), this project also builds an AMQP Server and an iCommand-Enabled client to demonstrate how a minimum CyVerse-compatible iRODS server work. This project includes

1. An ICAT-Enabled Server (IES), hosts the PostgreSQL ICAT database, also is iCommands-Enabled
2. An AMQP Exchange Server
3. An iCommands-Enabled Client

## Requirement
The `docker-compose.yml` file is written using version 2 syntax, so in order to build the docker images used in this project, you will need Docker Engine version 1.10.0+ and Docker Compose version 1.6.0+

## Usage
Begin by fill in all the environment parameters in `env.properties`. This repository provides an example `env.properties` file that would get everything up and running correctly, but feel free to modify the settings to suit your requirement.

Then, a few configuration files would need to be written accordingly (to `env.properties`) before the docker images can be built. A script (`configure.sh`) is provided to do this. So conveniently execute
```
./configure.sh
```
The configuration files that are compiled by `configure.sh` can be found in these following folders:
- irods-legacy-3.3.1/config
 - irods.config           > ~/iRODS/config/
 - installPostgres.config > ~/iRODS/config/
 - server.config          > ~/iRODS/server/config/
 - ipc-env.re             > ~/iRODS/server/config/reConfigs/
 - irodsHost              > ~/iRODS/server/config/ (not sure how/if this is used)
 - (odbc.ini              > ~.odbc.ini) Not necessary if $HOSTNAME is set by ENV
 - (irodsEnv              > ~/.irods/.irodsEnv) Not necessary if $HOSTNAME is set by ENV
- client/config
 - irods_environment.json

Build the docker images with
```
./build.sh
```
Optionally, you can specify which docker image to build like `./build.sh ies`.

Boot up the server grid with
```
./up.sh
```
Optionally, you can specify which service to boot up like `./up.sh ies`.

Attach to the client machine with
```
$ . env.properties
$ docker exec -it irodscyverse_client_1 bash
```

## Acknowledgement
Big big thank you to [Tony Edgin](https://github.com/tedgin) whose [irods4-upgrade-env](https://github.com/iPlantCollaborativeOpenSource/irods4-upgrade-env) project provided the main point of reference for this project.
