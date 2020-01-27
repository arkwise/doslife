#include "DOSTCP.H"

// Function prototypes

static void parseArgs( int argc, char *argv[] );
static void shutdown( int rc );


// Global vars and flags

char     ServerAddrName[80];     // Server name
uint16_t ServerPort;             // Only used when we are a client.
uint16_t LclPort = 2048;         // Local port to use for our socket (0 means not set)

int8_t   Listening = -1;         // Are we the server (1) or the client (0)?


#define RECV_BUFFER_SIZE (1024)
uint8_t recvBuffer[ RECV_BUFFER_SIZE ];





// Ctrl-Break and Ctrl-C handlers.

// Check this flag once in a while to see if the user wants out.
volatile uint8_t CtrlBreakDetected = 0;

void __interrupt __far ctrlBreakHandler( ) {
  CtrlBreakDetected = 1;
}

void __interrupt __far ctrlCHandler( ) {
  // Do Nothing - Ctrl-C is a legal character
}

TcpSocket *mySocket;

void startConnecting( int argc, char *argv[] ) {

  // fprintf( stderr, "mTCP Sample program by M Brutman (mbbrutman@gmail.com) (C)opyright 2012-2020\n\n" );

  // Read command line arguments
  parseArgs( argc, argv );


  // Setup mTCP environment
  if ( Utils::parseEnv( ) != 0 ) {
    exit(-1);
  }

  // Initialize TCP/IP stack
  if ( Utils::initStack( 2, TCP_SOCKET_RING_SIZE, ctrlBreakHandler, ctrlCHandler ) ) {
    fprintf( stderr, "\nFailed to initialize TCP/IP - exiting\n" );
    exit(-1);
  }


  // From this point forward you have to call the shutdown( ) routine to
  // exit because we have the timer interrupt hooked.

  int8_t rc;
  if ( Listening == 0 ) {
    // TCP Client
    fprintf( stderr, "Resolving server address - press Ctrl-Break to abort\n\n" );

    IpAddr_t serverAddr;

    // Resolve the name and definitely send the request
    int8_t rc2 = Dns::resolve( ServerAddrName, serverAddr, 1 );
    if ( rc2 < 0 ) {
      fprintf( stderr, "Error resolving server\n" );
      shutdown( -1 );
    }

    uint8_t done = 0;

    while ( !done ) {

      if ( CtrlBreakDetected ) break;
      if ( !Dns::isQueryPending( ) ) break;

      PACKET_PROCESS_SINGLE;
      Arp::driveArp( );
      Dns::drivePendingQuery( );

    }

    // Query is no longer pending or we bailed out of the loop.
    rc2 = Dns::resolve( ServerAddrName, serverAddr, 0 );

    if ( rc2 != 0 ) {
      fprintf( stderr, "Error resolving server\n" );
      shutdown( -1 );
    }

    mySocket = TcpSocketMgr::getSocket( );

    mySocket->setRecvBuffer( RECV_BUFFER_SIZE );

    fprintf( stderr, "Server resolved to %d.%d.%d.%d - connecting\n\n",
             serverAddr[0], serverAddr[1], serverAddr[2], serverAddr[3] );

    // Non-blocking connect.  Wait 10 seconds before giving up.
    rc = mySocket->connect( LclPort, serverAddr, ServerPort, 10000 );

  }
  else {
    // TCP Server 
    fprintf( stderr, "Waiting for a connection on port %u. Press [ESC] to abort.\n\n", LclPort );

    TcpSocket *listeningSocket = TcpSocketMgr::getSocket( );
    listeningSocket->listen( LclPort, RECV_BUFFER_SIZE );

    // Listen is non-blocking.  Need to wait
    while ( 1 ) {

      if ( CtrlBreakDetected ) {
        rc = -1;
        break;
      }

      PACKET_PROCESS_SINGLE;
      Arp::driveArp( );
      Tcp::drivePackets( );

      mySocket = TcpSocketMgr::accept( );
      if ( mySocket != NULL ) {
        listeningSocket->close( );
        TcpSocketMgr::freeSocket( listeningSocket );
        rc = 0;
        break;
      }

      if ( _bios_keybrd(1) != 0 ) {

        char c = _bios_keybrd(0);

        if ( (c == 27) || (c == 3) ) {
          rc = -1;
          break;
        }
      }


    }


  }

  if ( rc != 0 ) {
    fprintf( stderr, "Socket open failed\n" );
    shutdown( -1 );
  }

  if ( Listening == 0 ) {
    fprintf( stderr, "Connected!\n\n" );
  }
  else {
    fprintf( stderr, "Connection received from %d.%d.%d.%d:%u\n\n",
             mySocket->dstHost[0], mySocket->dstHost[1],
             mySocket->dstHost[2], mySocket->dstHost[3],
             mySocket->dstPort );
  }
}

void sendData(char* rawData)
{
  uint8_t done = 0;

  // Service the connection
  PACKET_PROCESS_SINGLE;
  Arp::driveArp( );
  Tcp::drivePackets( );

  if ( mySocket->isRemoteClosed( ) ) {
    done = 1;
  }

  int8_t sendRc = mySocket->send( (uint8_t *)rawData, strlen(rawData) );
  if(sendRc == 0)
    done = 1;

}


void recvData(char* rawData)
{
  uint8_t done = 0;

  while(!done) {

    // Service the connection
    PACKET_PROCESS_SINGLE;
    Arp::driveArp( );
    Tcp::drivePackets( );

    if ( mySocket->isRemoteClosed( ) ) {
      done = 1;
    }

    // Process incoming packets first.

    int16_t recvRc = mySocket->recv( (unsigned char *)rawData, RECV_BUFFER_SIZE );

    if ( recvRc > 0 ) {
      write( 1, rawData, recvRc );
      break;
    }
    else if ( recvRc < 0 ) {
      fprintf( stderr, "\nError reading from socket\n" );
      done = 1;
    }

    if ( CtrlBreakDetected ) {
      fprintf( stderr, "\nCtrl-Break detected\n" );
      closeSocket();
      done = 1;
    }
  }
}

void closeSocket()
{
  mySocket->close( );
  TcpSocketMgr::freeSocket( mySocket );
  shutdown(0);
}

char *HelpText[] = {
  "\nUsage: For Client : PG_NAME -C <ipaddr>\n",
  "   or: For Server : PG_NAME \n\n",
  "<ipaddr> is numerial IP address\n",
  NULL
};


void usage( void ) {
  uint8_t i=0;
  while ( HelpText[i] != NULL ) {
    fprintf( stderr, HelpText[i] );
    i++;
  }
  exit( 1 );
}


void errorMsg( char *msg ) {
  fprintf( stderr, msg );
  usage( );
}



static void parseArgs( int argc, char *argv[] ) {

  if ( argc == 1 ) {
    LclPort = PORT_NUM;
    Listening = 1;
    return;
  }

  int i=1;
  for ( ; i<argc; i++ ) {

    if ( stricmp( argv[i], "-help" ) == 0 ) {
      usage( );
    }
    else if ( stricmp( argv[i], "-C" ) == 0 || // Only connect to started peer
      stricmp( argv[i], "-CS" ) == 0) { // Start

      if ( Listening != -1 ) {
        errorMsg( "Specify -C\n" );
      }

      i++;
      if ( i == argc ) {
        errorMsg( "Need to provide a target server\n" );
      }

      strcpy( ServerAddrName, argv[i] );

      ServerPort = PORT_NUM;

      Listening = 0;
    }
    
    else {
      fprintf( stderr, "Unknown option %s\n", argv[i] );
      usage( );
    }

  }

  if ( Listening == -1 ) {
    errorMsg( "Must specify -C\n" );
  }

}



static void shutdown( int rc ) {
  Utils::endStack( );
  Utils::dumpStats( stderr );
  exit( rc );
}


