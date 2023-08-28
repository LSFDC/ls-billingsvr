#include "../stdafx.h"
#include "../MainProcess.h"
#include "../Network/GameServer.h"
#include "../Network/iocpHandler.h"
#include "../Network/ioPacketQueue.h"
#include "./ThailandLoginServer.h"
#include "../Util/Ringbuffer.h"
#include "../channeling/iochannelingnodemanager.h"
#include "../database/logdbclient.h"
#include "../Local/ioLocalManager.h"
#include "../Local/ioLocalThailand.h"


extern CLog LOG;

ThailandLoginServer *ThailandLoginServer::sg_Instance = NULL;
ThailandLoginServer::ThailandLoginServer( SOCKET s, DWORD dwSendBufSize, DWORD dwRecvBufSize ) : CConnectNode( s, dwSendBufSize, dwRecvBufSize )
{
	InitData();
}

ThailandLoginServer::~ThailandLoginServer()
{	
}

ThailandLoginServer &ThailandLoginServer::GetInstance()
{
	if(sg_Instance == NULL)
	{
		ioINILoader kLoader( "ls_config_billingsvr.ini" );
		kLoader.SetTitle( "Thailand Server Session" );
		int iSendBufferSize = kLoader.LoadInt( "SendBufferSize", 16384 );
		int iRecvBufferSize = kLoader.LoadInt( "RecvBufferSize", 16384 );
		sg_Instance = new ThailandLoginServer( INVALID_SOCKET, iSendBufferSize, iRecvBufferSize );
	}
	return *sg_Instance;
}

void ThailandLoginServer::ReleaseInstance()
{		
	SAFEDELETE( sg_Instance );
}

bool ThailandLoginServer::ConnectTo( bool bStart )
{
	ioINILoader kLoader( "ls_config_billingsvr.ini" );
	kLoader.SetTitle( "NETWORK" );

	char szServerIP[MAX_PATH];
	kLoader.LoadString( "ThailandLoginServerIP", "", szServerIP, MAX_PATH );

	int iSSPort = kLoader.LoadInt( "ThailandLoginServerPORT", 9000 );


	SOCKET socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( socket == INVALID_SOCKET )
	{
		LOG.PrintTimeAndLog( 0, "%s fail socket %d[%s:%d]", __FUNCTION__, GetLastError(), szServerIP, iSSPort );
		return false;
	}
	sockaddr_in serv_addr;
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr( szServerIP );
	serv_addr.sin_port			= htons( iSSPort );

	// non-block
	unsigned long arg = 1;
	ioctlsocket( socket, FIONBIO, &arg );

	int retval = ::connect( socket, (sockaddr*)&serv_addr, sizeof(serv_addr) );
	if( retval != 0 ) 
	{
		DWORD dwError = GetLastError();
		if( dwError != WSAEWOULDBLOCK )
		{
			LOG.PrintTimeAndLog( 0, "%s fail connect %x:%d[%s:%d]", __FUNCTION__, this, dwError, szServerIP, iSSPort );
			return false;
		}
	}

	// block
	arg = 0;
	ioctlsocket( socket, FIONBIO, &arg );

	if( retval != 0 )
	{
		// timeout
		struct timeval tv;
		if( bStart )
			tv.tv_sec  = CONNECT_WAIT_SECONDS*2;
		else
			tv.tv_sec  = CONNECT_WAIT_SECONDS;
		tv.tv_usec = 0;

		fd_set writedfds;    
		FD_ZERO(&writedfds);
		FD_SET(socket, &writedfds);

		retval = select(socket+1, NULL, &writedfds, NULL, &tv);

		if(retval == 0)
		{
			LOG.PrintTimeAndLog( 0, "%s Error 1 %d",  __FUNCTION__, GetLastError() );
			return false;
		}
		else if(retval == SOCKET_ERROR)
		{
			LOG.PrintTimeAndLog( 0, "%s Error 2 %d",  __FUNCTION__, GetLastError() );
			return false;
		}

		if(!FD_ISSET(socket, &writedfds))
		{
			LOG.PrintTimeAndLog( 0, "%s Error 3 %d",  __FUNCTION__, GetLastError() );
			return false;
		}
	}

	g_iocp.AddHandleToIOCP( (HANDLE)socket, (DWORD)this );
	CConnectNode::SetSocket( socket );

	OnCreate();	
	AfterCreate();
	LOG.PrintTimeAndLog( 0, "%s OnConnect (IP:%s PORT:%d RESULT:%d)", __FUNCTION__, szServerIP, iSSPort, 0 );
	return true;
}

void ThailandLoginServer::InitData()
{
	m_dwCurrentTime = 0;
	SetActive(false);
}

void ThailandLoginServer::OnCreate()
{
	InitData();

	CConnectNode::OnCreate();
	
	m_dwCurrentTime = TIMEGETTIME();
}

void ThailandLoginServer::OnDestroy()
{
	CConnectNode::OnDestroy();

	LOG.PrintTimeAndLog( 0, "%s", __FUNCTION__ );
}

bool ThailandLoginServer::SendMessage( CPacket &rkPacket )
{
	if( m_socket == INVALID_SOCKET ) return false;

	CPacket SendPacket( rkPacket.GetBuffer(), rkPacket.GetBufferSize() );
	return CConnectNode::SendMessage( SendPacket.GetBuffer() + SendPacket.GetCurPos(), SendPacket.GetBufferSize() - SendPacket.GetCurPos(), TRUE );
}

bool ThailandLoginServer::CheckNS( CPacket &rkPacket )
{
	return true;             //��Ʈ�� ���� �ʿ����.
}

int ThailandLoginServer::GetConnectType()
{
	return CONNECT_TYPE_THAILAND_LOGIN_SERVER;
}

void ThailandLoginServer::ProcessTime()
{	
	if( g_App.IsWantExit() ) return;

	if( TIMEGETTIME() - m_dwCurrentTime > UPDATE_TIME )       
	{
		static int iCnt = 0;
		if( !IsActive() )
		{
			ConnectTo( false );
		}
		else
		{	
			ioLocalThailand *pLocal = static_cast<ioLocalThailand*> ( g_LocalMgr.GetLocal( ioLocalManager::LCT_THAILAND ) );
			if( pLocal )
			{
				ioHashString sPID;
				pLocal->SendLogin( "lsasb025", "12345678", "THAS", "127.0.0.1", true, sPID );
			}
		}
		
		m_dwCurrentTime = TIMEGETTIME();
	}
}


void ThailandLoginServer::DispatchReceive(CPacket& packet, DWORD bytesTransferred)
{
	m_recvIO.AddBytesTransferred( bytesTransferred );

	while( m_recvIO.GetBytesTransferred() > 0 )
	{
		// ����� ���� ��Ŷ�� ����� ���Ѵ�. CConnectNode::DispatchReceive() �̺κи� �ٸ�.
		const char* buffer = m_recvIO.GetBuffer();
		DWORD dwPacketSize = 0;
		for (int i = 0; i < (int)m_recvIO.GetBytesTransferred() ; i++)
		{
			if( buffer[i] == ';' )
			{
				dwPacketSize = i+1;
				break;
			}
		}

		if( dwPacketSize == 0 )
			break;

		SP2Packet RecvPacket( BTTPK_LOGIN_RESULT );
		RecvPacket.SetDataAdd( (char*)m_recvIO.GetBuffer(), min( dwPacketSize, m_recvIO.GetBytesTransferred() ), true );
		if( RecvPacket.IsValidPacket() == true && m_recvIO.GetBytesTransferred() >= dwPacketSize )
		{
			// TEST
			//char szLog[2000]="";
			//for (int i = 0; i < (int)m_RecvIO.m_dwBytesTransferred; i++)
			//{
			//	char szOneLog[MAX_PATH]="";
			//	StringCbPrintf( szOneLog, sizeof( szOneLog ), "%d|", m_RecvIO.m_aBuffer[i] );
			//	StringCbCat( szLog, sizeof( szLog ), szOneLog );
			//}
			//LOG.PrintTimeAndLog( 0, "%s [%d][%s]", __FUNCTION__, m_RecvIO.m_dwBytesTransferred, szLog );
			//
			if( !CheckNS( RecvPacket ) ) return;

			ReceivePacket( RecvPacket );

			m_recvIO.AfterReceive( dwPacketSize );
		}
		else 
			break;
	}

	WaitForPacketReceive();
}

void ThailandLoginServer::ReceivePacket( CPacket &packet )
{
	g_RecvQueue.InsertQueue( (DWORD)this, packet, PK_QUEUE_SESSION );
}

void ThailandLoginServer::PacketParsing( CPacket &packet )
{
	SP2Packet &kPacket = (SP2Packet&)packet;

	switch( kPacket.GetPacketID() )
	{
	case ITPK_CLOSE_SESSION:
		{
			OnClose(kPacket);
		}
		break;

	case BTTPK_LOGIN_RESULT:
		OnLogin( kPacket );
		break;
	default:
		LOG.PrintTimeAndLog( 0, "%s �˼����� ��Ŷ : 0x%x", __FUNCTION__, kPacket.GetPacketID() );
		break;
	}
}

void ThailandLoginServer::OnLogin( SP2Packet &rkPacket )
{
	ioHashString sResult;
	rkPacket >> sResult;
	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal )
		pLocal->OnRecieveLoginData( sResult );
}

void ThailandLoginServer::SessionClose( BOOL safely/*=TRUE */ )
{
	CPacket packet(ITPK_CLOSE_SESSION);
	ReceivePacket( packet );
}

void ThailandLoginServer::OnClose( SP2Packet &rkPacket )
{ 
	if(IsActive())
		OnDestroy();
}