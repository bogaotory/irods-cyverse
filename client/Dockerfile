############################################################
# A simple iCommands-Enabled client machine
############################################################

FROM centos:centos7
MAINTAINER Bo Gao (bogao@dcs.warwick.ac.uk)
LABEL Description="A simple iCommands-Enabled machine"

RUN yum -y install \
				wget \
				fuse-libs \
				openssl

RUN wget ftp://ftp.renci.org/pub/irods/releases/4.1.7/centos7/irods-icommands-4.1.7-centos7-x86_64.rpm \
 && rpm -ivh irods-icommands-4.1.7-centos7-x86_64.rpm

RUN adduser foo -m \
 && echo "foo:foo" | chpasswd

ADD config/irods_environment.json /home/foo/.irods/irods_environment.json
RUN chown -R foo:foo /home/foo/

ADD bootstrap.sh /
RUN chown foo:foo /bootstrap.sh

USER foo
WORKDIR /home/foo/

RUN echo "try iput me" > hello.file

ENTRYPOINT ["/bootstrap.sh"]