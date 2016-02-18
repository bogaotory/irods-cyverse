## irods-3.3.1-cyverse

This folder contains the necessary files that builds an iCAT Enabled Server (IES)
- ./config/: Configuration files and Post-Installation initialisation scripts
- ./iRODS/: This is a CyVerse-compatible iRODS 3.3.1 release

#### Customise iRODS 3.3.1 for CyVerse
Start with legacy [3.3.1 code](https://github.com/irods/irods-legacy/releases/tag/3.3.1), and then
1. Replace `3.3.1/iRODS/server/core/collection.c` with [collection.c](https://github.com/iPlantCollaborativeOpenSource/irods4-upgrade-env/blob/master/base-3.3.1/collection.c), for a bug fix.
2. Add module [setAVU](https://github.com/cyverse/irods-setavu-mod) to `3.3.1/iRODS/modules/setavu`. Do a `chmod -R 775` on the `setavu` folder.
3. Add [command scripts](https://github.com/cyverse/irods-cmd-scripts) to `3.3.1/iRODS/server/bin/cmd`. These scripts require Python version 2.6 installed as python2.6 along with the pika 0.9.13 package. They also require the uuidd service to be running locally.
(**Important**: Depending on the base OS used for the image, the #! in `amqptopicsend.py` may need to be modified to look for python2 instead of python2.6)
4. Add iPlant custom rules to `3.3.1/iRODS/server/config/reConfigs`. This includes five files:
			* `ipc-amqp.re`
			* `ipc-custom.re`
			* **`ipc-env.re`**
			* `ipc-json.re`
			* `ipc-logic.re`
			* `ipc-uuid.re`