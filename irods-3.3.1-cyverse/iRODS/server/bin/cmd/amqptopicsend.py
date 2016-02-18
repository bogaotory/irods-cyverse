#!/usr/bin/env python2
#
# This always exits with 0 status, as a work around for iRODS msiExecCmd microservice. To detect
# errors, monitor stderr.

import sys
import logging
import pika

try:
    host = sys.argv[1]
    port = int(sys.argv[2])
    user = sys.argv[3]
    password = sys.argv[4]

    # Indicates if the exchange is only temporary (for testing)
    ephemeral = sys.argv[5].lower() == 'true'  

    key = sys.argv[6]
    body = sys.argv[7]

    exchange='irods'

    logging.basicConfig(stream=sys.stdout)

    credentials = pika.PlainCredentials(user, password)

    connection = pika.BlockingConnection(
	pika.ConnectionParameters(host=host,
			          port=port,
			          credentials=credentials
	)
    )

    channel = connection.channel()

    channel.exchange_declare(exchange=exchange,
                             type='topic',
                             durable=(not ephemeral),
                             auto_delete=ephemeral
    )

    channel.basic_publish(exchange=exchange,
                          routing_key=key,
                          body=body,
                          properties=pika.BasicProperties(delivery_mode=2)
    )

    connection.close()
except BaseException as e:
    sys.stderr.write('ERROR: %s\n' % e) 
