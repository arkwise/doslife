#ifndef DOSTCP_H
#define DOSTCP_H

#include <bios.h>
#include <dos.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "trace.h"
#include "utils.h"
#include "packet.h"
#include "arp.h"
#include "udp.h"
#include "dns.h"
#include "tcp.h"
#include "tcpsockm.h"

#define PORT_NUM 3333

void startConnecting( int argc, char *argv[] ) ;
void sendData(char* rawData);
void recvData(char* rawData);
void closeSocket();

#endif