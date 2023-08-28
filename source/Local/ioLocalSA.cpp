#include "../stdafx.h"
#include "./ioLocalSA.h"
#include "../MainProcess.h"
#include "../NodeInfo/ServerNode.h"
#include "../DataBase/LogDBClient.h"
#include "../Channeling/ioChannelingNodeParent.h"
#include "../ThreadPool/ioThreadPool.h"
#include "../NodeInfo/ServerNodeManager.h"
#include "../EtcHelpFunc.h"

extern CLog LOG;

ioLocalSA::ioLocalSA(void)
{
}


ioLocalSA::~ioLocalSA(void)
{
}

ioLocalManager::LocalType ioLocalSA::GetType()
{
	return ioLocalManager::LCT_SA;
}

void ioLocalSA::Init()
{
	//ip, port, servicevalue -> ���� �� 
	ioINILoader kLoader( "ls_config_billingsvr.ini" );
	kLoader.SetTitle( "Local" );

	char szSoftnyxLoginURL[MAX_PATH]="";
	kLoader.LoadString( "SoftnyxLoginURL", "", szSoftnyxLoginURL, sizeof( szSoftnyxLoginURL ) );
	m_sLoginURL = szSoftnyxLoginURL;

	char szSoftnyxBillingGetURL[MAX_PATH]="";
	kLoader.LoadString( "SoftnyxBillingURL", "", szSoftnyxBillingGetURL, sizeof( szSoftnyxBillingGetURL ) );
	m_sBillingGetURL = szSoftnyxBillingGetURL;

	char szSoftnyxBillingOutPutURL[MAX_PATH]="";
	kLoader.LoadString( "SoftnyxBillingURL", "", szSoftnyxBillingOutPutURL, sizeof( szSoftnyxBillingOutPutURL ) );
	m_sBillingOutPutURL = szSoftnyxBillingOutPutURL;

	if( m_sLoginURL.IsEmpty() || m_sBillingGetURL.IsEmpty() || m_sBillingOutPutURL.IsEmpty() )
		LOG.PrintTimeAndLog(0,"Error Softnyx URL");
}
void ioLocalSA::OnLoginData( ServerNode *pServerNode, SP2Packet &rkPacket )
{
	ioHashString szBillingGUID;
	ioHashString szPrivateID;
	ioHashString sEncodePW;
	ioHashString szPublicIP;
	DWORD        dwReturnMsgType = 0;
	ioHashString szMacAddress;
	ioHashString szKey;
	DWORD		 dwIndex = 0;	//accountIDX
	__int64		 dwVCode = 0;
	int			 intID   = 0;
	int			 reqNum = 0;

	rkPacket >> szBillingGUID >> szPrivateID >> dwIndex >> szPublicIP >> intID >> dwVCode; // ����
	
	if( !pServerNode )
	{
		LOG.PrintTimeAndLog(0, "%s pServerNode == NULL:%s:%s:%s:%s", __FUNCTION__, szBillingGUID.c_str(), szPrivateID.c_str(), sEncodePW.c_str(), szPublicIP.c_str() );
		return;
	}
	reqNum++;	//������
	
	ioData kData;
	kData.SetBillingGUID( szBillingGUID );
	kData.SetPrivateID( szPrivateID );
	kData.SetUserIndex( dwIndex );
	kData.SetUserIP( szPublicIP );
	kData.SetUserVCode( dwVCode );
	kData.SetUserIntID( intID );
	kData.SetUserReqNum( reqNum );

	if( pServerNode )
	{
		kData.SetServerIP( pServerNode->GetIP() );
		kData.SetServerPort( pServerNode->GetClientPort() );
	}
	kData.SetEmpty( false );
	kData.SetProcessType( ioData::PT_LOGIN );

	g_ThreadPool.SetData( kData );
}

void ioLocalSA::_OnGetCash( ServerNode *pServerNode, SP2Packet &rkPacket )
{
	int          iChannelingType   = 0;
	ioHashString szBillingGUID;
	DWORD        dwUserIndex       = 0;
	ioHashString szPublicID;
	ioHashString szPrivateID;
	bool         bSetUserMouse     = false;
	ioHashString szUserIP;	
	int			 intID   = 0;

	rkPacket >> iChannelingType >> szBillingGUID >> dwUserIndex >> szPrivateID >> szPublicID >> intID >> szUserIP >> bSetUserMouse; 
	
	if( !pServerNode )
	{
		LOG.PrintTimeAndLog(0, "%s pServerNode == NULL:%s:%d:%s", __FUNCTION__, szBillingGUID.c_str(), dwUserIndex, szPublicID.c_str() );
		return;
	}
	
	ioData kData;
	kData.SetBillingGUID( szBillingGUID );
	kData.SetPrivateID( szPrivateID );
	kData.SetPublicID( szPublicID );
	kData.SetUserIndex( dwUserIndex );
	kData.SetUserIP( szUserIP );
	kData.SetSetUserMouse( bSetUserMouse );
	kData.SetUserIntID( intID );

	if( pServerNode )
	{
		kData.SetServerIP( pServerNode->GetIP() );
		kData.SetServerPort( pServerNode->GetClientPort() );
	}
	kData.SetEmpty( false );
	kData.SetProcessType( ioData::PT_GET_CASH );

	g_ThreadPool.SetData( kData );
}

void ioLocalSA::_OnOutputCash( ServerNode *pServerNode, SP2Packet &rkPacket, DWORD dwGoodsNo, const ioHashString &rszGoodsName )
{
	int          iChannelingType = 0;
	ioHashString szBillingGUID;
	DWORD        dwUserIndex = 0;
	ioHashString szPublicID;
	ioHashString szPrivateID;
	ioHashString szUserIP;
	int          iPayAmt  = 0;
	int          iType    = 0;
	ioHashString szBillingUserKey;
	int          iUserGradeLevel = 0;
	int			 intID   = 0;
#if(_TEST)
	//�׽�Ʈ �ϰ�� ���Ƿ� ������ ��Ŷ�� �����Ƿ� �Ʒ��� ���� �޴´�. 
	int temp1, temp2, temp3, temp4, temp5, temp6 ;
	rkPacket >> iChannelingType >> szBillingGUID >> dwUserIndex >> szPublicID >> szPrivateID >> szUserIP >> iPayAmt >> iType >> temp1 >> temp2 >> temp3 >> temp4 >> temp5 >> temp6 >> intID ; 
#else if
	rkPacket >> iChannelingType >> szBillingGUID >> dwUserIndex >> szPublicID >> szPrivateID >> szUserIP >> iPayAmt >> iType >> intID ; 
#endif
	int iItemValueList[MAX_ITEM_VALUE];
	for (int i = 0; i < MAX_ITEM_VALUE ; i++)
		iItemValueList[i] = 0;
	ioChannelingNodeParent::GetItemValueList( rkPacket, iType, iItemValueList );

	ioData kData;
	kData.SetBillingGUID( szBillingGUID );
	kData.SetUserIndex( dwUserIndex );
	kData.SetPublicID( szPublicID );
	kData.SetPrivateID( szPrivateID );
	kData.SetUserIP( szUserIP );
	kData.SetItemPayAmt( iPayAmt );
	kData.SetItemType( iType );
	kData.SetItemValueList( iItemValueList );
	kData.SetGoodsNo( dwGoodsNo );
	kData.SetGoodsName( rszGoodsName );
	kData.SetUserIntID( intID );
	
	if( pServerNode )
	{
		kData.SetServerIP( pServerNode->GetIP() );
		kData.SetServerPort( pServerNode->GetClientPort() );
	}
	kData.SetEmpty( false );
	kData.SetProcessType( ioData::PT_OUTPUT_CASH );

	g_ThreadPool.SetData( kData );
}
//VCODE ���� �ް� �� �� �ʱ�ȭ ���ִ� �κ� �ʿ���
void ioLocalSA::ThreadLogin( const ioData &rData, LoginManager &rLoginMgr )
{
	if( rData.IsEmpty() )
	{
		LOG.PrintTimeAndLog( 0, "%s Data is Empty.", __FUNCTION__ );
		return;
	}
	if( g_App.IsReserveLogOut() )
	{
		LOG.PrintTimeAndLog(0, "%s LogOut:%s:%s:%s", __FUNCTION__, rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		SP2Packet kPacket( BSTPK_LOGIN_RESULT );
		kPacket << rData.GetPrivateID();
		kPacket << rData.GetBillingGUID();
		kPacket << BILLING_LOGIN_RESULT_FAIL;
		kPacket << false;
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadLogin", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		return;
	}

	if( g_App.IsTestMode() )
	{
		char szReturnValue[WEB_BUFF_SIZE]="";
		
		ioHTTP Winhttp; 
		
		char szFullURL[MAX_PATH*2] = "http://112.175.228.42:2015/webCertify.aspx?REQNUM=799&GAMETYPE=O&LOCAL=1&INTID=1200&IP=211.55.81.64&VCODE=1000";
		
		
		if( !Winhttp.GetResultData( szFullURL, "GET", szReturnValue, WEB_BUFF_SIZE ) )
		{
			LOG.PrintTimeAndLog( 0, "%s Http Error: %s:%s:%d", "ioLocalSA::ThreadLogin", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), GetLastError() );
			SP2Packet kPacket( BSTPK_LOGIN_RESULT );
			kPacket << rData.GetPrivateID();
			kPacket << rData.GetBillingGUID();
			kPacket << BILLING_LOGIN_RESULT_FAIL;
			kPacket << false;
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadLogin", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			return;
		}
		//VCODE����
		char szInitURL[MAX_PATH*2]="";
		char szTestReturnValue[WEB_BUFF_SIZE]="";
		StringCbPrintf( szInitURL, sizeof( szInitURL ), "%s?INTID=%d", "http://112.175.228.42:1999/ResetVCode.aspx",rData.GetUserIntID() );
		if( !Winhttp.GetResultData( szInitURL, "GET", szTestReturnValue, WEB_BUFF_SIZE ) )
		{
			LOG.PrintTimeAndLog( 0, "%s Http Error: %s:%s:%d", "ioLocalSA::ThreadLogin", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), GetLastError() );
			SP2Packet kPacket( BSTPK_LOGIN_RESULT );
			kPacket << rData.GetPrivateID();
			kPacket << rData.GetBillingGUID();
			kPacket << BILLING_LOGIN_RESULT_FAIL;
			kPacket << false;
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadLogin", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			return;
		}
	}
	else
	{
		//����������(������ȣ 105, ���ѷ��� 10, ����, ī����30, ����20��)
		//1234567890;01378;aabbddc;1;105;;10;;;;;0;30;20
		//reqNum;intid;userid;returnValue(1:����);������ȣ;������;�Ϲ�����;ī����;����
		char szReturnValue[WEB_BUFF_SIZE]="";
		
		ioHTTP Winhttp; 
		
		char szFullURL[MAX_PATH*2]="";
		StringCbPrintf( szFullURL, sizeof( szFullURL ), "%s?REQNUM=%d&GAMETYPE=%c&LOCAL=%d&INTID=%d&IP=%s&VCODE=%d" , m_sLoginURL.c_str(), rData.GetUserReqNum(), SA_GAMETYPE, SA_LOCAL, rData.GetUserIntID(), rData.GetUserIP().c_str(), rData.GetUserVCode() );
		
		if( !Winhttp.GetResultData( szFullURL, "GET", szReturnValue, WEB_BUFF_SIZE ) )
		{
			LOG.PrintTimeAndLog( 0, "%s Http Error: %s:%s:%d", "ioLocalSA::ThreadLogin", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), GetLastError() );
			SP2Packet kPacket( BSTPK_LOGIN_RESULT );
			kPacket << rData.GetPrivateID();
			kPacket << rData.GetBillingGUID();
			kPacket << BILLING_LOGIN_RESULT_FAIL;
			kPacket << false;
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadLogin", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			return;
		}
		
#if(_TEST)
		//TEST �� ��� VCODE �ʱ�ȭ		
		//http://112.175.228.42:1999/ResetVCode.aspx?INTID=1200
		char szInitURL[MAX_PATH*2]="";
		char szTestReturnValue[WEB_BUFF_SIZE]="";
		StringCbPrintf( szInitURL, sizeof( szInitURL ), "%s?INTID=%d", "http://112.175.228.42:1999/ResetVCode.aspx",rData.GetUserIntID() );
		if( !Winhttp.GetResultData( szInitURL, "GET", szTestReturnValue, WEB_BUFF_SIZE ) )
		{
			LOG.PrintTimeAndLog( 0, "%s Http Error: %s:%s:%d", "ioLocalSA::ThreadLogin", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), GetLastError() );
			SP2Packet kPacket( BSTPK_LOGIN_RESULT );
			kPacket << rData.GetPrivateID();
			kPacket << rData.GetBillingGUID();
			kPacket << BILLING_LOGIN_RESULT_FAIL;
			kPacket << false;
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadLogin", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			return;
		}
		
#endif
		ioHashStringVec vParseStr;
		Help::GetParseData( szReturnValue, vParseStr, SA_TOKEN );
		if( vParseStr.size() < MAX_LOGIN_ARRAY )
		{
			LOG.PrintTimeAndLog( 0, "%s Size Error. %s", "ioLocalSA::ThreadLogin", szReturnValue );
			return;
		}
		/*
		Return 
		-1	User ���̺� �����;���
		1	Ok (���� ó��)
		2	
		3	
		4	
		5	Database Connect Error
		6	Reader Close Error
		7	DataFunction Class Error
		8	DataUtil Class Error
		9	System Error
		10	�̵�� ID
		11	VCODEƲ��
		12	Ban
		13	�����ڵ�˻��� �����ڵ� ����
		14	Gender ����
		15	Authority ����
		16	Email ����
		17	INTID�� ȣ���������
		UserID�� ã�� �� ����
		18	���������
		40	�̵�� ���� �̰ų� ĳ������ �Ұ� ����
		41	��� �Ұ� ����
		42	���� ���� ����
		52	��û�� ���� �߿� ���� ���ų� ������ ���� ����
		*/
		if( vParseStr[3] != "1" )
		{
			SP2Packet kPacket( BSTPK_LOGIN_RESULT );
			kPacket << rData.GetPrivateID();
			kPacket << rData.GetBillingGUID();
			kPacket << BILLING_LOGIN_RESULT_FAIL;
			kPacket << false;
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadLogin", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			LOG.PrintTimeAndLog( 0, "%s Login Error: reqNum:%d Error:%d %s:%s:%d:%d:%d", "ioLocalSA::ThreadLogin", rData.GetUserReqNum(), vParseStr[3].c_str(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetUserIndex(), rData.GetUserIntID(), rData.GetUserVCode() );
			return;
		}
		
	}
	LOG.PrintTimeAndLog( 0, "%s Login Success: reqNum:%d %s:%s:%d:%d:%d", "ioLocalSA::ThreadLogin", rData.GetUserReqNum(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetUserIndex(), rData.GetUserIntID(), rData.GetUserVCode() );
}

void ioLocalSA::ThreadGetCash( const ioData &rData )
{
	if( rData.IsEmpty() )
	{
		LOG.PrintTimeAndLog( 0, "%s Data is Empty.", __FUNCTION__ );
		return;
	}
	if( g_App.IsReserveLogOut() )
	{
		LOG.PrintTimeAndLog(0, "%s LogOut:%s:%s:%s", __FUNCTION__, rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		return;
	}
	char szReturnValue[WEB_BUFF_SIZE]="";
	ioHTTP Winhttp; 
	/*
	//CashConsumptionAPI.aspx?USERID=&INTID=�����������������̵�&PROCESSTYPE=2&RETURNTYPE=1&SERVICEVALUE=Service Value*&GAMETYPE=Game Type*
	USERID			Varchar(20)	User.id ���̰�  INTID �Ķ���� ���� ������ ����
	INTID			INT(11) user.IdSerial (������ ���� ���̵�)
	PROCESSTYPE		Char(1)	2 (Fixed)
	RETURNTYPE		Char(1)	1 (Fixed)
	SERVICEVALUE	Char(1)	1 Billing test IP �� �׽�Ʈ �� ���
	GAMETYPE		Char(3)	O Lostsaga
	*/
	char szFullURL[MAX_PATH*2]="";
	int reqNum = 1;
		
	StringCbPrintf( szFullURL, sizeof( szFullURL ), "%s?USERID=%s&INTID=%d&PROCESSTYPE=2&RETURNTYPE=1&SERVICEVALUE=%d&GAMETYPE=%c" , m_sBillingGetURL.c_str(), rData.GetPublicID().c_str(), rData.GetUserIntID(), SA_SERVICEVALUE, SA_GAMETYPE );
		
	if( !Winhttp.GetResultData( szFullURL, "GET", szReturnValue, WEB_BUFF_SIZE ) )
	{
		LOG.PrintTimeAndLog( 0, "%s Http Error: %s:%s:%d", "ioLocalSA::ThreadGetCash", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), GetLastError() );
			
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;
			
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		return;
	}
		
	ioHashStringVec vParseStr;
	Help::GetParseData( szReturnValue, vParseStr, SA_TOKEN );
	if( vParseStr.size() < MAX_GETCASH_ARRAY )
	{

		LOG.PrintTimeAndLog( 0, "%s Size Error. %s", "ioLocalSA::ThreadGetCash", szReturnValue );
		return;
	}
	/*
	1��° ����	
	-1	Error
	0	Cash is 0
	1	Normal
	10	Not found ID
	30	Not enough Cash (Affected üũ �� ���������� ������ Cash ����)
	52	Not found request parameters value

	2��° ����	Cash
	*/
	if( vParseStr[0].c_str() == "-1" )
	{
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;
			
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}

		LOG.PrintTimeAndLog( 0, "%s GetCash Error: %s:%s:%d:%d:%s", "ioLocalSA::ThreadGetCash", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetUserIndex(), rData.GetUserIntID(), vParseStr[0].c_str() );
		return;
	}
	LOG.PrintTimeAndLog( 0, "%s GetCash Success: %s:%s:%d:%d:%s", "ioLocalSA::ThreadGetCash", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetUserIndex(), rData.GetUserIntID(), vParseStr[1].c_str() );


}

void ioLocalSA::ThreadOutputCash( const ioData &rData )
{
	if( rData.IsEmpty() )
	{
		LOG.PrintTimeAndLog( 0, "%s Data is Empty.", __FUNCTION__ );
		return;
	}
	if( g_App.IsReserveLogOut() )
	{
		LOG.PrintTimeAndLog(0, "%s LogOut:%s:%s:%s", __FUNCTION__, rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		
		SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT  );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << CASH_RESULT_EXCEPT;
		kPacket << rData.GetItemType();
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		return;
	}
	if( g_App.IsTestMode() )
	{
		char szReturnValue[WEB_BUFF_SIZE]="";
		
		ioHTTP Winhttp; 
		
		/*
		//CashConsumptionAPI.aspx?USERID=&INTID=�����������������̵�&PROCESSTYPE=3&ITEMCODE=111&ITEMPRICE=100&RETURNTYPE=1&SERVICEVALUE= Service Value*&GAMETYPE= Game Type*&CC=222

		USERID			Varchar(20)	User.id ���̰�  INTID �Ķ���� ���� ������ ����
		INTID			INT(11) user.IdSerial (������ ���� ���̵�)
		PROCESSTYPE		Char(1)	2 (Fixed)
		RETURNTYPE		Char(1)	1 (Fixed)
		SERVICEVALUE	Char(1)	1 Billing test IP �� �׽�Ʈ �� ���
		GAMETYPE		Char(3)	O Lostsaga
		*/
		char szFullURL[MAX_PATH*2] = "http://112.175.228.40:8000/CashConsumptionAPI.aspx?USERID=testUser&INTID=1200&PROCESSTYPE=3&ITEMCODE=100030055&ITEMPRICE=9800&RETURNTYPE=1&SERVICEVALUE=1&GAMETYPE=O";
		
		if( !Winhttp.GetResultData( szFullURL, "GET", szReturnValue, WEB_BUFF_SIZE ) )
		{
			LOG.PrintTimeAndLog( 0, "%s Http Error: %s:%s:%d", "ioLocalSA::ThreadOutputCash", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), GetLastError() );
			SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT  );
			kPacket << rData.GetUserIndex();
			kPacket << rData.GetBillingGUID();
			kPacket << CASH_RESULT_EXCEPT;
			kPacket << rData.GetItemType();
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			return;
		}
		
		ioHashStringVec vParseStr;
		Help::GetParseData( szReturnValue, vParseStr, SA_TOKEN );
		if( vParseStr.size() < MAX_GETCASH_ARRAY )
		{
			LOG.PrintTimeAndLog( 0, "%s Size Error. %s", "ioLocalSA::ThreadOutputCash", szReturnValue );
			return;
		}
	}
	else
	{
		char szReturnValue[WEB_BUFF_SIZE]="";
		
		ioHTTP Winhttp; 
		
		/*
		//CashConsumptionAPI.aspx?USERID=&INTID=�����������������̵�&PROCESSTYPE=3&ITEMCODE=111&ITEMPRICE=100&RETURNTYPE=1&SERVICEVALUE= Service Value*&GAMETYPE= Game Type*&CC=222

		USERID			Varchar(20)	User.id ���̰�  INTID �Ķ���� ���� ������ ����
		INTID			INT(11) user.IdSerial (������ ���� ���̵�)
		PROCESSTYPE		Char(1)	2 (Fixed)
		RETURNTYPE		Char(1)	1 (Fixed)
		SERVICEVALUE	Char(1)	1 Billing test IP �� �׽�Ʈ �� ���
		GAMETYPE		Char(3)	O Lostsaga
		*/
		char szFullURL[MAX_PATH*2]="";
		StringCbPrintf( szFullURL, sizeof( szFullURL ), "%s?USERID=%s&INTID=%d&PROCESSTYPE=3&ITEMCODE=%d&ITEMPRICE=%d&RETURNTYPE=1&SERVICEVALUE=%d&GAMETYPE=%c" , m_sBillingGetURL.c_str(), rData.GetPublicID().c_str(), rData.GetUserIntID(), rData.GetGoodsNo(), rData.GetItemPayAmt(), SA_SERVICEVALUE, SA_GAMETYPE );
		
		if( !Winhttp.GetResultData( szFullURL, "GET", szReturnValue, WEB_BUFF_SIZE ) )
		{
			SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT  );
			kPacket << rData.GetUserIndex();
			kPacket << rData.GetBillingGUID();
			kPacket << CASH_RESULT_EXCEPT;
			kPacket << rData.GetItemType();
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			return;
		}
		
		ioHashStringVec vParseStr;
		Help::GetParseData( szReturnValue, vParseStr, SA_TOKEN );
		if( vParseStr.size() < MAX_GETCASH_ARRAY )
		{
			LOG.PrintTimeAndLog( 0, "%s Size Error. %s", "ioLocalSA::ThreadOutputCash", szReturnValue );
			return;
		}
		/*
		-1	Error
		0	Cash is 0
		1	Normal
		10	Not found ID
		30	Not enough Cash (Affected üũ �� ���������� ������ Cash ����)
		52	Not found request parameters value
		*/
		if( vParseStr[0].c_str() == "-1" )
		{
			SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT  );
			kPacket << rData.GetUserIndex();
			kPacket << rData.GetBillingGUID();
			kPacket << CASH_RESULT_EXCEPT;
			kPacket << rData.GetItemType();
			if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
			{
				LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioLocalSA::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
			}
			
			LOG.PrintTimeAndLog( 0, "%s OutputCash Error: %s:%s:%d:%d:%s", "ioLocalSA::ThreadOutputCash", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetUserIndex(), rData.GetUserIntID(), vParseStr[0].c_str() );
			return;
		}
		LOG.PrintTimeAndLog( 0, "%s OutputCash Success: %s:%s:%d:%d:%s", "ioLocalSA::ThreadOutputCash", rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetUserIndex(), rData.GetUserIntID(), vParseStr[1].c_str() );
	}
}
