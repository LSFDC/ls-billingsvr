#ifndef __NexonNISMSServer_h_
#define __NexonNISMSServer_h_

#include "../NexonEUDefine.h"
class CConnectNode;
class NexonNISMSServer : public CConnectNode 
{
protected:
	enum 
	{ 
		UPDATE_TIME = 60000, // 2�� connect() �Լ� ������ ����, �ּ� 3�п� 1ȸ �̻� ��û�� �ִ°ɷ�...
		ALIVE_TIME = 10000,
	};

	
protected:
	static NexonNISMSServer *sg_Instance;
	EU_HEADER m_packetParse;
	DWORD  m_dwCurrentTime;
	bool   m_bSendAlive;

protected:
	char m_szServerIP[MAX_PATH];
	int m_iPort;
	int m_serverNo;

public:
	static NexonNISMSServer &GetInstance();
	static void ReleaseInstance();

public:
	virtual void SessionClose( BOOL safely=TRUE );
	virtual void DispatchReceive(CPacket& packet, DWORD bytesTransferred);
	virtual bool SendMessage( CPacket &rkPacket );
	virtual void ReceivePacket( CPacket &packet );
	virtual void PacketParsing( CPacket &packet );
	virtual bool AfterCreate();

public:
	virtual void OnClose( SP2Packet &rkPacket );
	virtual void OnCreate();       //�ʱ�ȭ
	virtual void OnDestroy();
	virtual bool CheckNS( CPacket &rkPacket );	
	virtual int  GetConnectType();

public:
	bool ConnectTo( bool bStart );
	void InitData();
	bool SendInitPacket( bool );

public:
	void ProcessTime();

public:
	void OnInitialize( SP2Packet &rkPacket );
	void OnAlive( SP2Packet &rkPacket );
	void OnCheckBalance( SP2Packet &rkPacket );
	void OnPurchaseAmount( SP2Packet &rkPacket );
	void OnPurchaseItem( SP2Packet &rkPacket );
	void OnPurchaseGift( SP2Packet &rkPacket );
	
public:
	bool m_bFirst;
	typedef std::list< DWORD > AliveList;

protected:
	NexonNISMSServer( SOCKET s, DWORD dwSendBufSize, DWORD dwRecvBufSize );
	~NexonNISMSServer(void);
};

#define g_NexonNISMSServer NexonNISMSServer::GetInstance()
#endif // __NexonNISMSServer_h_