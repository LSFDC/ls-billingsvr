#ifndef _MgrToolNode_h_
#define _MgrToolNode_h_

#define BILLING_ANSWER 3

class CConnectNode;
class SP2Packet;
class GoodsManager;

class MgrToolNode : public CConnectNode
{
	friend class MgrToolNodeManager;

	DWORD m_dwCurrentTime;
	ioHashString m_szGUID;			// ���ӽ� ���� ���Ӱ� �ο��Ǵ� ������ id, node �ĺ���.
	ioHashString m_szMgrToolIP;
	ioHashString m_szID;			// ������� ������ ���̵�
	int m_billingIndex;
	
	


public:
	static bool m_bUseSecurity;
	static int  m_iSecurityOneSecRecv;

public:
	virtual void SessionClose( BOOL safely=TRUE );
	virtual bool SendMessage( CPacket &rkPacket );
	virtual void ReceivePacket( CPacket &packet );
	virtual void PacketParsing( CPacket &packet );

public:
	virtual void OnCreate();       //�ʱ�ȭ
	virtual void OnDestroy();
	virtual bool CheckNS( CPacket &rkPacket );	
	virtual int  GetConnectType();

public:
	static void LoadHackCheckValue();
	

public:
	bool IsGhostSocket();

protected:
	void InitData();
	void CreateGUID(OUT char *szGUID, IN int iSize);

public:
	void OnClose( SP2Packet &packet );
	void OnRequestConnect( SP2Packet &rkPacket );
	void OnRequestServerInfo( SP2Packet &rkPacket );
	void OnRequestServerUserCount( SP2Packet &rkPacket );
	void OnServerReloadINI( SP2Packet &rkPacket );
	void OnRequestNexonPCRoomLogout( SP2Packet &rkPacket );

	inline const ioHashString& GetGUID() const	{ return m_szGUID; }
	inline const ioHashString& GetID() const	{ return m_szID; }

protected:


public:
	MgrToolNode( SOCKET s=INVALID_SOCKET, DWORD dwSendBufSize=0, DWORD dwRecvBufSize=0 );
	virtual ~MgrToolNode();
};

#endif