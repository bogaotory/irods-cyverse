############################################################
# A minimal iRODS Server for a Cyverse Discovery Environment
#
############################################################

FROM centos:centos7
MAINTAINER Bo Gao (bogao@dcs.warwick.ac.uk)
LABEL Description="Image containing a CyVerse-compatible iRODS service"


## Part I. Prepare dependencies and add source

ARG IES_CONTAINER_USERNAME
ARG IES_CONTAINER_USERPASS

ARG iRODS_RESOURCE_PATH
ARG iRODS_SRC_PATH

#### Create irods user
RUN adduser "$IES_CONTAINER_USERNAME" -m \
 && echo "$IES_CONTAINER_USERNAME:$IES_CONTAINER_USERPASS" | chpasswd
# USER irods
# RUN mkdir /home/irods/.ssh
# ADD ssh/id_rsa.pub /home/irods/.ssh/authorized_keys
# USER root

#### Install iRODS dependencies
RUN yum -y install wget \
 && wget http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-5.noarch.rpm \
 &&	rpm -ivh epel-release-7-5.noarch.rpm \
 && yum -y update

RUN yum clean all \
 && yum -y groupinstall "Development Tools" \
 && yum -y install \
			which \
			fuse-libs \
			openssl098e \
			authd \
			bind-utils \
			gcc \
			gcc-cpp \
			gcc-c++ \
			kernel-devel \
			kernel-headers \
			libtool-ltdl \
			qt \
			qt-devel

#### Install CyVerse specific dependencies
RUN yum -y install \
			python-pip \
			# python-pika
			uuidd

RUN pip install --upgrade pip \
 &&	pip install pika

RUN mkdir /var/uuidd
RUN chown -R $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME /var/uuidd

#### Create vault directory
RUN mkdir $iRODS_RESOURCE_PATH \
 && chown -R $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME $iRODS_RESOURCE_PATH
# VOLUME ["/cyverseVault"]

#### Add iRODS source
USER $IES_CONTAINER_USERNAME
# RUN echo "alias python2.6=/usr/bin/python" >> ~/.bashrc
RUN mkdir -p $iRODS_SRC_PATH
COPY iRODS $iRODS_SRC_PATH

#### Add configurations
USER root
WORKDIR $iRODS_SRC_PATH

ADD config/installPostgres.config ./config/installPostgres.config
ADD config/irods.config ./config/irods.config
ADD config/server.config ./server/config/server.config
ADD config/irodsHost ./server/config/irodsHost
ADD config/ipc-env.re ./server/config/reConfig/ipc-env.re
RUN chown -R $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME $iRODS_SRC_PATH

RUN chmod 664 ./config/installPostgres.config \
 && chmod 664 ./config/irods.config \
 && chmod 664 ./server/config/server.config \
 && chmod 664 ./server/config/irodsHost


## Part II. Build

ARG IES_HOSTNAME
ARG POSTGRES_HOME

ENV HOSTNAME=$IES_HOSTNAME

USER $IES_CONTAINER_USERNAME
WORKDIR $iRODS_SRC_PATH

RUN scripts/installPostgres --noask \
 && scripts/configure \
 && make \
 && scripts/finishSetup --noask --verbose
RUN printf '\n\nexport PATH='$iRODS_SRC_PATH'/clients/icommands/bin:$PATH' >> ~/.bashrc \
 && printf '\n\nexport PATH='$POSTGRES_HOME'/bin:$PATH' >> ~/.bashrc \
 && printf '\n\nexport PATH='$iRODS_SRC_PATH':$PATH' >> ~/.bashrc

# RUN su - irods -c "echo $HOSTNAME" 
# RUN su - irods -c "cd /home/irods/iRODS && scripts/installPostgres --noask" \
#  && su - irods -c "cd /home/irods/iRODS && scripts/configure" \
#  && su - irods -c "cd /home/irods/iRODS && make" \
#  && su - irods -c "cd /home/irods/iRODS && scripts/finishSetup --noask --verbose" \
#  && printf '\n\nexport PATH=/home/irods/iRODS/clients/icommands/bin:$PATH' >> /home/irods/.bashrc \
#  && printf '\n\nexport PATH=/home/irods/postgres/pgsql/bin:$PATH' >> /home/irods/.bashrc \
#  && printf '\n\nexport PATH=/home/irods/iRODS:$PATH' >> /home/irods/.bashrc


## Part III. Post Build

ARG iRODS_PORT
ARG POSTGRES_DB_PORT

USER root
WORKDIR /home/$IES_CONTAINER_USERNAME

ADD config/init-indices.sh .
ADD config/init-specific-queries.sh .
ADD bootstrap.sh /
RUN chown $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME ./*
RUN chown $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME /bootstrap.sh

# RUN su - irods -c "/home/irods/iRODS/irodsctl dbstart" \
#  && su - irods -c "psql -d ICAT -c \" UPDATE \"r_resc_main\" SET \"resc_net\" = 'ies' WHERE \"resc_name\" = 'demoResc' \" " \
#  && su - irods -c "/home/irods/iRODS/irodsctl dbstop"

USER $IES_CONTAINER_USERNAME

RUN . ~/.bashrc \
 && irodsctl start \
 && ~/init-indices.sh \
 && ~/init-specific-queries.sh \
 && irodsctl stop

# RUN su - irods -c "/home/irods/iRODS/irodsctl start" \
#  && su - irods -c "/home/irods/init-indices.sh" \
#  && su - irods -c "/home/irods/init-specific-queries.sh" \
#  && su - irods -c "/home/irods/iRODS/irodsctl stop"

# ADD de_config/irodsEnv /home/$IES_CONTAINER_USERNAME/.irods/.irodsEnv
# ADD de_config/odbc.ini /home/$IES_CONTAINER_USERNAME/.odbc.ini
# RUN chown -R $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME /home/$IES_CONTAINER_USERNAME/.irods/.irodsEnv
# RUN chown -R $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME /home/$IES_CONTAINER_USERNAME/.odbc.ini

EXPOSE $iRODS_PORT $POSTGRES_DB_PORT

# USER root
# RUN chown -R $IES_CONTAINER_USERNAME:$IES_CONTAINER_USERNAME /bootstrap.sh

# USER $IES_CONTAINER_USERNAME
WORKDIR $iRODS_SRC_PATH
ENTRYPOINT ["/bootstrap.sh"]