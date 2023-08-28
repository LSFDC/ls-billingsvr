#include "../stdafx.h"
#include "./iochannelingnodedaum.h"
#include "../NodeInfo/ServerNode.h"
#include "../xml/ioxmldocument.h"
#include "../xml/ioxmlelement.h"
#include "../Util/ioHashString.h"
#include "../ThreadPool/ioThreadPool.h"
#include "../NodeInfo/ServerNodeManager.h"
#include "../MainProcess.h"
#include "../database/logdbclient.h"
#include "../Local/ioLocalParent.h"

extern CLog LOG;

ioChannelingNodeDaum::ioChannelingNodeDaum(void)
{
	char szTemp[MAX_PATH]="";
	ioINILoader kLoader( "ls_config_billingsvr.ini" );
	kLoader.SetTitle( "Channeling" );

	kLoader.LoadString( "DaumGetURL", "", szTemp, sizeof( szTemp ) );
	m_sGetURL = szTemp;

	kLoader.LoadString( "DaumOutputURL", "", szTemp, sizeof( szTemp ) );
	m_sOutputURL = szTemp;

	kLoader.LoadString( "DaumCancelURL", "", szTemp, sizeof( szTemp ) );
	m_sSubscriptionRetractURL = szTemp;

	kLoader.LoadString( "DaumApikey", "", szTemp, sizeof( szTemp ) );
	m_sApikey = szTemp;
}

ioChannelingNodeDaum::~ioChannelingNodeDaum(void)
{
}

ChannelingType ioChannelingNodeDaum::GetType()
{
	return CNT_DAUM;
}

void ioChannelingNodeDaum::_OnGetCash( ServerNode *pServerNode, SP2Packet &rkPacket )
{
	int          iChannelingType   = 0;
	ioHashString szBillingGUID;
	DWORD        dwUserIndex       = 0;
	ioHashString szPrivateID;
	ioHashString szPublicID;
	bool         bSetUserMouse     = false;
	ioHashString szDaumNo;

	rkPacket >> iChannelingType >> szBillingGUID >> dwUserIndex >> szPrivateID >> szPublicID >> bSetUserMouse; // �������
	rkPacket >> szDaumNo; // daum��

	ioData kData;
	kData.SetChannelingType( iChannelingType );
	kData.SetBillingGUID( szBillingGUID );
	kData.SetUserIndex( dwUserIndex );
	kData.SetPrivateID( szPrivateID );
	kData.SetPublicID( szPublicID );
	kData.SetSetUserMouse( bSetUserMouse );
	kData.SetUserNo( szDaumNo );
	if( pServerNode )
	{
		kData.SetServerIP( pServerNode->GetIP() );
		kData.SetServerPort( pServerNode->GetClientPort() );
	}
	kData.SetEmpty( false );
	kData.SetProcessType( ioData::PT_GET_CASH );

	g_ThreadPool.SetData( kData );
}

void ioChannelingNodeDaum::_OnOutputCash( ServerNode *pServerNode, SP2Packet &rkPacket, DWORD dwGoodsNo, const ioHashString &rszGoodsName )
{
	int          iChannelingType = 0;
	ioHashString szBillingGUID;
	int          iReturnItemPrice = 0;
	DWORD        dwUserIndex = 0;
	ioHashString szPublicID;
	ioHashString szPrivateID;
	ioHashString szUserIP;
	int          iPayAmt  = 0;
	int          iType    = 0;
	int			 iBonusCashSize	= 0;

	ioData kData;

	rkPacket >> iChannelingType >> szBillingGUID >> iReturnItemPrice >> dwUserIndex >> szPublicID >> szPrivateID >> szUserIP >> iPayAmt >> iBonusCashSize;
	
	for( int i = 0; i < iBonusCashSize; i++ )
	{
		int iIndex		= 0;
		int iBonusCash	= 0;

		rkPacket >> iIndex >> iBonusCash;

		kData.AddBonusCashInfoForConsume(iIndex, iBonusCash);
	}

	rkPacket >> iType; // �������
	
	int iItemValueList[MAX_ITEM_VALUE];
	for (int i = 0; i < MAX_ITEM_VALUE ; i++)
		iItemValueList[i] = 0;
	GetItemValueList( rkPacket, iType, iItemValueList );

	ioHashString szDaumNo;
	rkPacket >> szDaumNo; // daum �� 

	
	kData.SetChannelingType( iChannelingType );
	kData.SetBillingGUID( szBillingGUID );
	kData.SetExp( iReturnItemPrice );
	kData.SetUserIndex( dwUserIndex );
	kData.SetPublicID( szPublicID );
	kData.SetPrivateID( szPrivateID );
	kData.SetUserIP( szUserIP );
	kData.SetItemPayAmt( iPayAmt );
	kData.SetItemType( iType );
	kData.SetItemValueList( iItemValueList );
	kData.SetUserNo( szDaumNo );
	kData.SetGoodsNo( dwGoodsNo );
	kData.SetGoodsName( rszGoodsName );
	if( pServerNode )
	{
		kData.SetServerIP( pServerNode->GetIP() );
		kData.SetServerPort( pServerNode->GetClientPort() );
	}
	kData.SetEmpty( false );
	kData.SetProcessType( ioData::PT_OUTPUT_CASH );

	g_ThreadPool.SetData( kData );
}

void ioChannelingNodeDaum::ThreadGetCash( const ioData &rData )
{
	if( rData.IsEmpty() )
	{
		LOG.PrintTimeAndLog( 0, "[error][daum]%s Data is Empty.", __FUNCTION__ );
		return;
	}

	if( g_App.IsReserveLogOut() )
	{
		LOG.PrintTimeAndLog( 0, "%s LogOut: %d:%s:%s:%s:%s",__FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str() );
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s", "ioChannelingNodeDaum::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		}
		return;
	}

	long iReturnCash    = 0;
	long iPurchasedCash = 0; // ������ �ְ� ������ ĳ��
	char szReturnValue[WEB_BUFF_SIZE]="";
	ZeroMemory( szReturnValue, WEB_BUFF_SIZE );

	char szFullURL[MAX_PATH*2]="";
	StringCbPrintf( szFullURL, sizeof( szFullURL ), "%s/%s?apiKey=%s" , m_sGetURL.c_str(), rData.GetUserNo().c_str(), m_sApikey.c_str() );

	ioHTTP Winhttp; //kyg Ȯ�� �ʿ� 
	if( !Winhttp.GetResultData( szFullURL, "GET", szReturnValue, WEB_BUFF_SIZE ) )
	{
		LOG.PrintTimeAndLog( 0, "%s Http Error: %d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;
		
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s", "ioChannelingNodeDaum::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		}
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"Billing exception daum." );
		return;
	}

	ioXMLDocument xmlDoc;
	//strcpy(szReturnValue,_T("<result>  <header>    <code>0000</code>    <message>����ó�� �Ǿ����ϴ�.</message>    <resultCnt>1</resultCnt>    <comment>ĳ���ܾ� ��ȸ (����)</comment>    <requestTime>2013-06-10T19:49:52.520</requestTime>    <responseTime>2013-06-10T19:49:52.530</responseTime>    <isNextData></isNextData>    <nextDataUrl></nextDataUrl>  </header>  <request>    <requestUrl>http://test.bill.daum.net/Alice/REST/lostsaga/Inquiry/CashBalance20100812/9lflB?apiKey=6b7d0746b3829ae8260b1aa4af095002ff5c20a2bdf8ada0fba17f16b7baf2c4856c8359bd33484b66c20865e7b19a01d86dc4ba922fcf0f175413e48cb2325a120b65ff8510b704b50ddf18</requestUrl>    <clientIp>112.218.160.51</clientIp>    <methodType>GET</methodType>    <requestParameters></requestParameters>  </request>  <response>    <itemCnt>1</itemCnt>    <items>      <item>        <cashAmt>0</cashAmt>        <contentsCashAmt>0</contentsCashAmt>        <actualCashAmt>0</actualCashAmt>      </item>    </items>  </response></result> "));

	if( !xmlDoc.LoadFromMemory( szReturnValue ) )
	{
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;
		
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s", "ioChannelingNodeDaum::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		}
		LOG.PrintTimeAndLog(0, "%s Fail ioXMLDocument::LoadFromMemory :%d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"Billing exception daum." );
		return;
	}

	ioXMLElement xRootElement = xmlDoc.GetRootElement();
	if( xRootElement.IsEmpty() )	
	{
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;
		
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s", "ioChannelingNodeDaum::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		}
		LOG.PrintTimeAndLog(0, "[error][daum]%s ioXMLElement is Empty. :%d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		return;
	}

	ioHashString sReturnCode;
	ioXMLElement xErrorChildElement = xRootElement.FirstChild( "header" );
	ioXMLElement xErrorGrandChildElement;
	if( !xErrorChildElement.IsEmpty() )
		xErrorGrandChildElement = xErrorChildElement.FirstChild( "code" );
	if( !xErrorGrandChildElement.IsEmpty() && xErrorGrandChildElement.GetText() != NULL )
		sReturnCode = xErrorGrandChildElement.GetText();

	if( sReturnCode != "0000" )
	{
		SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << rData.GetSetUserMouse();
		kPacket << CASH_RESULT_EXCEPT;

		ioHashString sError;
		xErrorGrandChildElement = xErrorChildElement.FirstChild( "message" );
		if( !xErrorGrandChildElement.IsEmpty() && xErrorGrandChildElement.GetText() != NULL )
			sError = xErrorGrandChildElement.GetText();

		char szAnsiError[MAX_PATH*2]="";
		int iReturnAnsiSize = 0;
		UTF8ToAnsi( sError.c_str(), szAnsiError, iReturnAnsiSize, sizeof( szAnsiError ) );

		if( !sError.IsEmpty() )
		{
			kPacket << true;
			kPacket << szAnsiError;
		}

		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s", "ioChannelingNodeDaum::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		}
		LOG.PrintTimeAndLog(0, "%s xml error return. :%d:%s:%s:%s:%s:%d:%s", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError(), szAnsiError );
		LOG.PrintTimeAndLog(0, "%s Fail SendValue :%s", __FUNCTION__, szFullURL );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		return;
	}

/*
<result>
	<header> 
		<code> �����ڵ� </code> 
		<message> ����޼��� </message> 
		<resultCnt> ��ü ������ �Ǽ� </resultCnt> 
		<comment> �ش� API �ڸ�Ʈ </comment> 
		<requestTime> ó����û�ð� </requestTime> 
		<responseTime> ó������ð� </responseTime> 
		<isNextData> ���� ������ ���� ���� </isNextData> 
		<nextDataUrl> ���� ������ URL </nextDataUrl>
	</header> 
	<request> 
		<requestUrl> ��û�� URL </requestUrl> 
		<clientIp> ��û�� Client Ip �ּ� </clientIp> 
		<methodType> ��û�� �޼ҵ� ��� </methodType> 
		<requestParameters> ��û�� �Ķ���� (POST ����� ��� ��µ�) </requestParameters> 
	</request> 
	<response> 
		<itemCnt> ��ȸ�� Item �Ǽ� </itemCnt> 
		<items> ��ȸ�� �����۵� 
		<item> 
			<cashAmt> ĳ�� �ݾ� </cashAmt>
			<contentsCashAmt> ������ĳ�� �ݾ� </contentsCashAmt> 
			<actualCashAmt> �ǹ�ĳ�� �ݾ� </actualCashAmt>
		</item> 
		</items> 
	</response> 
</result>

	0000	 ����ó�� �Ǿ����ϴ�.	
	A002	 do not allow method	 �ش� API�� ����ϱ� ���ؼ��� �ùٸ� Method(GET, POST, PUT, DELETE)�� ����Ͽ� ȣ���ϼž��մϴ�.
	A003	 do not support url	 �ּҰ� �߸��Ǿ����ϴ�.
	A004	 invalid format	 �Ķ������ ������ �߸��� ���
	A005	 *** is null	 �Ķ���Ͱ��� null�� ���
	A006	 �ش� �Ǵ� ���񽺸� �̿��Ͻ� �� �����ϴ�.	 �ش� API�� ����ϱ� ���ؼ��� API��� ��û�� �ϼž� �մϴ�.
	A007	 �ش� �����Ͱ� �����ϴ�.	 ���ϴ� ���� DB�� ���� ���
	A008	 ������ �����Ͽ����ϴ�.	 Sign���� ���� �ʴ� ���
	A009	 DB ����Դϴ�.	 ���� DB���� ����
	A100	 ��Ұ� �Ұ����մϴ�.	 ��� API ���� �̹���ҵǾ��ų� ��Ÿ����� ������ ��Ұ� �Ұ����� ���
	A101	 �ߺ��� �ŷ��Դϴ�.	 ClientSeq������ �ߺ��� ���� ���� ���
	A102	 Confirm ó���Ϸ� ����ŷ� �Դϴ�.	confirm �Ϸ�� ����ŷ��� ���� ConfirmSettlement �� �ߺ� ��û�� ���
	A103	 ���� �Ϸ���� ���� ����ŷ� �Դϴ�.	BI ������ �Ϸ���� ���� ����ŷ��� ���� ConfirmSettlement �� ��û�� ���
	A104	 ���հ������� �ƴϹǷ� Confirm ó���� �� �����ϴ�.	���հ��� ���� �ƴ� ����ŷ��� ���� ConfirmComplexSettlement �� ��û�� ���
	A200	 �������� �ʴ� ��ǰ �Դϴ�.	 itemId �� �߸��� ���
	A9**	 System error	 ���� �ý��� ������ �ش� ���� �߻��� ���� �ٶ��ϴ�.
	I010	 Daumĳ�� ��ü�ѵ��ʰ��Դϴ�.	
	I012	 expired item	 �ش� �������� ��ȿ�Ⱓ�� ����� ���
	I013	 �ش� CP Item �ѵ��ʰ��Դϴ�.	
	I014	 �ش� CP �� Item �� �ƴմϴ�.	
	I015	 �ݾ��� �߸��Ǿ����ϴ�.	
	I017	 ���� ĳ���� ��������� ȸ���Դϴ�.	
	I018	 ���� ȯ�ݾ� ���� ȯ�� ��û �ݾ��� Ŭ�� �����ϴ�.	
	I019	 �߸��� payType �Դϴ�.	
	I020	 �߸��� PERIOD(�����ֱ�) �Դϴ�.	
	I021	 �߸��� ���� �Դϴ�.	
	I100	 Cancel Error	 ��Ұ��� �����Դϴ�.
	I101	 Cancel Error : PG ��Ұ� �Ұ����մϴ�.	
	I110	 �������Ա��� ���� ���Աݻ����Դϴ�.	
	I111	 �ݾ��� �����մϴ�.	
	I112	 �̹� ȯ�������� ���Դϴ�.	
	I113	 ������ PG�Դϴ�.	
	I114	 �ܾ� �����Դϴ�.	
	I115	 ĳ��Ÿ���� �߸��Ǿ����ϴ�.	 ĳ�� ������, �ش� ���� ĳ���� Ÿ�� ����
	I400	 �Ҽȼ��� CP �� ��ϰ����մϴ�.	 ������ ��ǰ ����� �Ҽȼ��� CP (shoppinghow, shoppinghow2) �� ����
	I401	 ��Ŀ�� ����� ���� �������� �ʽ��ϴ�.	 ��ǰ ��Ͻ� �ش� ��ǰ�� �����ְ� �������� ��� ��� �Ұ�
	I402	 �̹� �����ϴ� ��ǰID �Դϴ�	
	B003	 NO DATA FOUND	 ���� �����͸� ��ȸ�� ���
	B004	 �̹� ��ϵ� ������ �Դϴ�	
	B005	 �߸��� �����Դϴ�.	
	B006	 0���� ó���Ǿ����ϴ�. ����� 1�Ǹ� ó���Ǿ�� �մϴ�.	 insert �� ����
	B007	 0���� ó���Ǿ����ϴ�. ������ 1�Ǹ� ó���Ǿ�� �մϴ�.	 update �� ����
	B008	 0���� �����Ǿ����ϴ�. ������ 1�Ǹ� ó���Ǿ�� �մϴ�.	 delete �� ����
	B010	 �㰡���� ���� PGCATE �ڵ尪�Դϴ�	
	B011	 �ܾ׺����Դϴ�.	
	B012	 �ܾ������� �������� ���߽��ϴ�.	
	B015	 Daumĳ�� ����� �����Ǿ����ϴ�.	
	B017	 ���� ���� ���� CASH TYPE �Դϴ�	
	B018	 ���� ��� IP �� �ƴմϴ�	
	B019	 ��ϿϷ�� API �� �ƴմϴ�	
	B020	 �ݾ��� �����մϴ�	
	B101	 apiKey�� 2�������� �߱ް����մϴ�.	
	C101	 �������� �ý��� �������� ���Ͽ� ���񽺸� �̿��� �� �����ϴ�.	
	C102	 �����翡�� NULL�� �����Ͽ����ϴ�.	
	C121	 ��������� ��� �����Դϴ�.	
	C122	 PG���� ���̺귯�� �����Դϴ�.	
	C125	 ���� �����Ͱ� �߸��Ǿ����ϴ�.	
	C201	 �������� ������ �����մϴ�.	
	C9**	 System error	 ���� �ý��� ������ �ش� ���� �߻��� ���� �ٶ��ϴ�.

*/

	// xml parsing
	ioXMLElement xChildElement = xRootElement.FirstChild( "response" );
	ioXMLElement xGrandChildElement;
	if( !xChildElement.IsEmpty() )
		xGrandChildElement = xChildElement.FirstChild( "items" );
	ioXMLElement xGreatGrandChildElement;
	if( !xGrandChildElement.IsEmpty() )
		xGreatGrandChildElement = xGrandChildElement.FirstChild( "item" );

	enum { MAX_LOOP = 1, };
	ioXMLElement xCurrentElement;
	if( !xGreatGrandChildElement.IsEmpty() )
	{
		char szCashTitles[MAX_LOOP][MAX_PATH]={ "cashAmt" };
		for (int i = 0; i < MAX_LOOP ; i++)
		{
			xCurrentElement = xGreatGrandChildElement.FirstChild( szCashTitles[i] );
			if( !xCurrentElement.IsEmpty() && xCurrentElement.GetText() != NULL )
			{
				iReturnCash += atoi( xCurrentElement.GetText() );	
			}
		}
	}

	// ĳ���� ���� ������ ĳ���� �����ϴ�.
	iPurchasedCash = iReturnCash;

	SP2Packet kPacket( BSTPK_GET_CASH_RESULT );
	kPacket << rData.GetUserIndex();
	kPacket << rData.GetBillingGUID();
	kPacket << rData.GetSetUserMouse();
	kPacket << CASH_RESULT_SUCCESS;
	kPacket << iReturnCash;
	kPacket << iPurchasedCash;
	if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
	{
		LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s", "ioChannelingNodeDaum::ThreadGetCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str() );
		return;		
	}
	//[%d:%d]
	LOG.PrintTimeAndLog( 0, "%s Success: %d:%s:PrivateID %s:%s:%s[%d:%d]", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), iReturnCash, iPurchasedCash  );
	//LOG.PrintTimeAndLog(0, "%s Success ReturnValue :%s", __FUNCTION__, szReturnValue );
}

void ioChannelingNodeDaum::ThreadOutputCash( const ioData &rData )
{
	if( rData.IsEmpty() )
	{
		LOG.PrintTimeAndLog( 0, "[error][daum]%s Data is Empty.", __FUNCTION__ );
		return;
	}

	if( g_App.IsReserveLogOut() )
	{
		LOG.PrintTimeAndLog( 0, "%s LogOut: %d:%s:%s:%s",__FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str() );
		SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT  );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << CASH_RESULT_EXCEPT;
		kPacket << rData.GetExp();
		kPacket << rData.GetItemType();
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioChannelingNodeDaum::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		return;
	}

	char szReturnValue[WEB_BUFF_SIZE]="";
	ZeroMemory( szReturnValue, WEB_BUFF_SIZE );

	char szAgencyNo[ioLocalParent::MAX_AGENCY_NO_PLUS_ONE]="";
	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal )
	{
		pLocal->GetAgencyNo( szAgencyNo, sizeof( szAgencyNo ), true );
	}

	char ts[16]="";
	char nonce[41]="";
	char *sign = NULL;

	char szParameters[MAX_PATH*4]="";
	StringCbPrintf( szParameters, sizeof( szParameters ), "SettlementPayCashNormallostsaga%s%s10000", szAgencyNo, rData.GetUserNo().c_str() );

	try 
	{
		getNewTimeStamp(ts); //kyg 
		getNewNonce(nonce);
		sign = getSig( (char*)m_sApikey.c_str(), szParameters, ts, nonce);
	}
	catch ( ... )
	{
		LOG.PrintTimeAndLog( 0, "%s Daum Error: %d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << CASH_RESULT_EXCEPT;
		kPacket << rData.GetExp();
		kPacket << rData.GetItemType();
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioChannelingNodeDaum::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"Billing exception daum." );
		return;
	}

	char szPostData[MAX_PATH*2]="";
	StringCbPrintf( szPostData, sizeof( szPostData ), "userId=%s&itemId=10000&amt=%d&memo=%d&sign=%s&ts=%s&nonce=%s&clientSeq=%s", rData.GetUserNo().c_str(), rData.GetItemPayAmt(), rData.GetGoodsNo(), sign, ts, nonce, szAgencyNo );
	free(sign); ////////////////////// �ݵ�� free �ʿ� DAUM �ҽ�

	ioHTTP Winhttp; //kyg Ȯ�� �ʿ�  Get��� �Ǵ��� Ȯ�� �ؾ��� 
	if( !Winhttp.GetResultData( m_sOutputURL.c_str(), szPostData, szReturnValue, WEB_BUFF_SIZE ) )
	{
		LOG.PrintTimeAndLog( 0, "%s Http Error: %d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << CASH_RESULT_EXCEPT;
		kPacket << rData.GetExp();
		kPacket << rData.GetItemType();
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioChannelingNodeDaum::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"Billing exception daum." );
		return;
	}

	ioXMLDocument xmlDoc;
	if( !xmlDoc.LoadFromMemory( szReturnValue ) )
	{
		SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << CASH_RESULT_EXCEPT;
		kPacket << rData.GetExp();
		kPacket << rData.GetItemType();
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioChannelingNodeDaum::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		LOG.PrintTimeAndLog(0, "%s Fail ioXMLDocument::LoadFromMemory :%d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"Billing exception daum." );
		return;
	}

	ioXMLElement xRootElement = xmlDoc.GetRootElement();
	if( xRootElement.IsEmpty() )	
	{
		SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << CASH_RESULT_EXCEPT;
		kPacket << rData.GetExp();
		kPacket << rData.GetItemType();
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioChannelingNodeDaum::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		LOG.PrintTimeAndLog(0, "[error][daum]%s ioXMLElement is Empty. :%d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		return;
	}

	ioHashString sReturnCode;
	ioXMLElement xErrorChildElement = xRootElement.FirstChild( "header" );
	ioXMLElement xErrorGrandChildElement;
	if( !xErrorChildElement.IsEmpty() )
		xErrorGrandChildElement = xErrorChildElement.FirstChild( "code" );
	if( !xErrorGrandChildElement.IsEmpty() && xErrorGrandChildElement.GetText() != NULL )
		sReturnCode = xErrorGrandChildElement.GetText();

	if( sReturnCode != "0000" )
	{
		SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << CASH_RESULT_EXCEPT;
		kPacket << rData.GetExp();
		kPacket << rData.GetItemType();

		ioHashString sError;
		xErrorGrandChildElement = xErrorChildElement.FirstChild( "message" );
		if( !xErrorGrandChildElement.IsEmpty() && xErrorGrandChildElement.GetText() != NULL )
			sError = xErrorGrandChildElement.GetText();

		char szAnsiError[MAX_PATH*2]="";
		int iReturnAnsiSize = 0;
		UTF8ToAnsi( sError.c_str(), szAnsiError, iReturnAnsiSize, sizeof( szAnsiError ) );

		if( !sError.IsEmpty() )
		{
			kPacket << true;
			kPacket << szAnsiError;
		}

		
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioChannelingNodeDaum::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		}
		LOG.PrintTimeAndLog(0, "%s xml error return. :%d:%s:%s:%s:%s:%d[%d:%s:%d:%s]", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt(), szAnsiError );
		LOG.PrintTimeAndLog(0, "%s Fail SendValue :%s?%s", __FUNCTION__, m_sOutputURL.c_str(), szPostData );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		return;
	}


/*
���� �ڵ�� get cash�� ����
<result>
	<header> 
		<code> �����ڵ� </code> 
		<message> ����޼��� </message> 
		<resultCnt> ��ü ������ �Ǽ� </resultCnt> 
		<comment> �ش� API �ڸ�Ʈ </comment> 
		<requestTime> ó����û�ð� </requestTime> 
		<responseTime> ó������ð� </responseTime> 
		<isNextData> ���� ������ ���� ���� </isNextData> 
		<nextDataUrl> ���� ������ URL </nextDataUrl>
	</header> 
	<request> 
		<requestUrl> ��û�� URL </requestUrl> 
		<clientIp> ��û�� Client Ip �ּ� </clientIp> 
		<methodType> ��û�� �޼ҵ� ��� </methodType> 
		<requestParameters> ��û�� �Ķ���� (POST ����� ��� ��µ�) </requestParameters> 
	</request> 
	<response>
		<itemCnt> ������ �Ǽ� </itemCnt>
		<items>
			<item>
				<logSeq> ������ �ŷ��� �ش� �ŷ���ȣ </logSeq>
				<usedAmt> ���� �ݾ� </usedAmt>
				<cashAmt> ������ ���� ĳ�� </cashAmt>
			</item>
		</items>
	</response>
</result>
*/

	// xml parsing
	int  iReturnCash = 0; 
	ioHashString szReturnBuyNO;

	ioXMLElement xChildElement = xRootElement.FirstChild( "response" );
	ioXMLElement xGrandChildElement;
	if( !xChildElement.IsEmpty() )
		xGrandChildElement = xChildElement.FirstChild( "items" );
	ioXMLElement xGreatGrandChildElement;
	if( !xGrandChildElement.IsEmpty() )
		xGreatGrandChildElement = xGrandChildElement.FirstChild( "item" );

	enum { MAX_LOOP = 1, };
	ioXMLElement xCurrentElement;
	if( !xGreatGrandChildElement.IsEmpty() )
	{
		char szCashTitles[MAX_LOOP][MAX_PATH]={ "cashAmt" };
		for (int i = 0; i < MAX_LOOP ; i++)
		{
			xCurrentElement = xGreatGrandChildElement.FirstChild( szCashTitles[i] );
			if( !xCurrentElement.IsEmpty() && xCurrentElement.GetText() != NULL )
				iReturnCash += atoi( xCurrentElement.GetText() );	
		}
	}

	// for log
	xCurrentElement = xGreatGrandChildElement.FirstChild( "logSeq" );
	if( !xCurrentElement.IsEmpty() && xCurrentElement.GetText() != NULL )
		szReturnBuyNO = xCurrentElement.GetText();	

	// ĳ���� ���� ������ ĳ���� �����ϴ�.
	int iPurchasedCash = iReturnCash;

	SP2Packet kPacket( BSTPK_OUTPUT_CASH_RESULT );
	kPacket << rData.GetUserIndex();
	kPacket << rData.GetBillingGUID();
	kPacket << CASH_RESULT_SUCCESS;
	kPacket << rData.GetExp();
	kPacket << rData.GetItemType();
	kPacket << rData.GetItemPayAmt();
	kPacket << 0; // TransactionID ( FOR US )
	kPacket << szReturnBuyNO; // same szChargeNo
	
	int iItemValueList[MAX_ITEM_VALUE];
	for (int i = 0; i <MAX_ITEM_VALUE; i++)
		iItemValueList[i] = rData.GetItemValue( i );
	SetItemValueList( kPacket, rData.GetItemType(), iItemValueList );
	kPacket << rData.GetChannelingType(); // ����
	kPacket << iReturnCash;
	kPacket << iPurchasedCash;
    //kyg �ϴ�.. �־����� ���� szReturnBuyNO�� 
	if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
	{
		LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d:%s:%d", "ioChannelingNodeDaum::ThreadOutputCash", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt());
		return;	
	}
	//LOG.PrintTimeAndLog(0, "%s Success ReturnValue :%s", __FUNCTION__, szReturnValue );
	LOG.PrintTimeAndLog(0, "%s Success. :%d:%s:PrivateID %s:%s:%s[%d:%s:%d:%d:%d:%s]", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),
		rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), //PrivateID %s:%s:%s
		rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt(), iReturnCash, iPurchasedCash, szReturnBuyNO.c_str() );

#if(0) //kyg Test����
	ioData testData;
	testData.SetBillingGUID(rData.GetBillingGUID());
	testData.SetUserIndex(rData.GetUserIndex());
	testData.SetChargeNo(szReturnBuyNO);
	testData.SetEmpty(false);
	testData.SetUserNo(rData.GetUserNo());
	ThreadSubscriptionRetract(testData);
#endif
}

void ioChannelingNodeDaum::ThreadSubscriptionRetract( const ioData& rData )
{
	ioHashString sError = "UnKnownError";

	if( rData.IsEmpty() )
	{
		LOG.PrintTimeAndLog( 0, "[error][daum]%s Data is Empty.", __FUNCTION__ );
		return;
	}

	if( g_App.IsReserveLogOut() )
	{
		LOG.PrintTimeAndLog( 0, "%s LogOut: %d:%s:%s:%s",__FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str() );

		SP2Packet kPacket( BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT );

		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << (DWORD) 0;
		kPacket << rData.GetChargeNo();
		kPacket << BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL;
		kPacket << sError;		
		
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d", "ioChannelingNodeDaum::ThreadSubscriptionRetract", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetChargeNo());
		}
		return;
	}

	char szReturnValue[WEB_BUFF_SIZE]="";
	ZeroMemory( szReturnValue, WEB_BUFF_SIZE );

	char szAgencyNo[ioLocalParent::MAX_AGENCY_NO_PLUS_ONE]="";
	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal )
	{
		pLocal->GetAgencyNo( szAgencyNo, sizeof( szAgencyNo ), true );
	}

	char ts[16]="";
	char nonce[41]="";
	char *sign = NULL;

	char szParameters[MAX_PATH*4]="";

	
	StringCbPrintf( szParameters, sizeof( szParameters ), "CancelCancellationlostsaga%s%s", szAgencyNo, rData.GetChargeNo().c_str() );

	try 
	{
		getNewTimeStamp(ts); //kyg 
		getNewNonce(nonce);
		sign = getSig( (char*)m_sApikey.c_str(), szParameters, ts, nonce);
	}
	catch ( ... )
	{
		LOG.PrintTimeAndLog( 0, "%s Daum Error: %d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );

		SP2Packet kPacket( BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT );

		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << (DWORD) 0;
		kPacket << rData.GetChargeNo();
		kPacket << BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL;
		kPacket << sError;

		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d", "ioChannelingNodeDaum::ThreadSubscriptionRetract", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetChargeNo());
		}
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"ThreadSubscriptionRetract::Billing exception daum." );

		return;
	}

	char szPostData[MAX_PATH*4]="";
	StringCbPrintf( szPostData, sizeof( szPostData ), "logSeq=%s&sign=%s&ts=%s&nonce=%s&clientSeq=%s",rData.GetChargeNo().c_str(), sign, ts, nonce, szAgencyNo );
	free(sign); ////////////////////// �ݵ�� free �ʿ� DAUM �ҽ�

	ioHTTP Winhttp; //kyg Ȯ�� �ʿ�  Get��� �Ǵ��� Ȯ�� �ؾ��� 

	if( !Winhttp.GetResultData( m_sSubscriptionRetractURL.c_str(), szPostData, szReturnValue, WEB_BUFF_SIZE ) )
	{
		LOG.PrintTimeAndLog( 0, "%s http fail %d:%s:%s:%s",__FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetPublicID().c_str() );
		SP2Packet kPacket( BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << (DWORD) 0;
		kPacket << rData.GetChargeNo();
		kPacket << BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL;
		kPacket << sError;

		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d", "ioChannelingNodeDaum::ThreadSubscriptionRetract", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetChargeNo());
		}
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"ThreadSubscriptionRetract::Billing exception daum." );
		return;
	}

	ioXMLDocument xmlDoc;
	if( !xmlDoc.LoadFromMemory( szReturnValue ) )
	{
		SP2Packet kPacket( BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << (DWORD) 0;
		kPacket << rData.GetChargeNo();
		kPacket << BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL;
		kPacket << sError;
		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d", "ioChannelingNodeDaum::ThreadSubscriptionRetract", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetChargeNo());
		}
		LOG.PrintTimeAndLog(0, "%s Fail ioXMLDocument::LoadFromMemory :%d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		g_LogDBClient.OnInsertBillingServerError( CNT_DAUM, BILLING_ERROR_LOG_EXCEPTION,"Billing exception daum." );
		return;
	}

	ioXMLElement xRootElement = xmlDoc.GetRootElement();
	if( xRootElement.IsEmpty() )	
	{
		SP2Packet kPacket( BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT );
		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << (DWORD) 0;
		kPacket << rData.GetChargeNo();
		kPacket << BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL;
		kPacket << sError;

		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d", "ioChannelingNodeDaum::ThreadSubscriptionRetract", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetChargeNo());
		}
		LOG.PrintTimeAndLog(0, "[error][daum]%s ioXMLElement is Empty. :%d:%s:%s:%s:%s:%d", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError() );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		return;
	}

	ioHashString sReturnCode;
	ioXMLElement xErrorChildElement = xRootElement.FirstChild( "header" );
	ioXMLElement xErrorGrandChildElement;
	if( !xErrorChildElement.IsEmpty() )
		xErrorGrandChildElement = xErrorChildElement.FirstChild( "code" );
	if( !xErrorGrandChildElement.IsEmpty() && xErrorGrandChildElement.GetText() != NULL )
		sReturnCode = xErrorGrandChildElement.GetText();

	if( sReturnCode != "0000" )
	{
		SP2Packet kPacket( BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT );

		kPacket << rData.GetUserIndex();
		kPacket << rData.GetBillingGUID();
		kPacket << (DWORD) 0;
		kPacket << rData.GetChargeNo();
		kPacket << BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL;
		kPacket << sError;
				
		xErrorGrandChildElement = xErrorChildElement.FirstChild( "message" );
		if( !xErrorGrandChildElement.IsEmpty() && xErrorGrandChildElement.GetText() != NULL )
			sError = xErrorGrandChildElement.GetText();

		char szAnsiError[MAX_PATH*2]="";
		int iReturnAnsiSize = 0;
		UTF8ToAnsi( sError.c_str(), szAnsiError, iReturnAnsiSize, sizeof( szAnsiError ) );

		if( !sError.IsEmpty() )
		{
			kPacket << true;
			kPacket << szAnsiError;
		}

		if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
		{
			LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d", "ioChannelingNodeDaum::ThreadSubscriptionRetract", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetChargeNo());
		}
		LOG.PrintTimeAndLog(0, "%s xml error return. :%d:%s:%s:%s:%s:%d[%d:%s:%d:%s]", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(), GetLastError(), rData.GetGoodsNo(), rData.GetGoodsName().c_str(), rData.GetItemPayAmt(), szAnsiError );
		LOG.PrintTimeAndLog(0, "%s Fail SendValue :%s?%s", __FUNCTION__, m_sOutputURL.c_str(), szPostData );
		LOG.PrintTimeAndLog(0, "%s Fail ReturnValue :%s", __FUNCTION__, szReturnValue );
		return;
	}
/*
���� �ڵ�� get cash�� ����
<response>
<itemCnt>1</itemCnt>
<items>
<item>
<cancelTypeCode>5</cancelTypeCode>
<logSeq>123</logSeq>
<cancelAmt>123</cancelAmt>
<remainAmt>0</remainAmt>
</item>
</items>
</response>
*/

	// xml parsing
	int cancelType = 0;
	ioHashString szLogSeq;
	int cancelAmt = 0;
	int remainAmt = 0;

	ioXMLElement xChildElement = xRootElement.FirstChild( "response" );
	ioXMLElement xGrandChildElement;
	if( !xChildElement.IsEmpty() )
		xGrandChildElement = xChildElement.FirstChild( "items" );
	ioXMLElement xGreatGrandChildElement;
	if( !xGrandChildElement.IsEmpty() )
		xGreatGrandChildElement = xGrandChildElement.FirstChild( "item" );

	enum { MAX_LOOP = 1, };
	ioXMLElement xCurrentElement;

	xCurrentElement = xGreatGrandChildElement.FirstChild("cancelTypeCode");
	if( !xCurrentElement.IsEmpty() && xCurrentElement.GetText() != NULL )
		cancelType = atoi( xCurrentElement.GetText() );

	// for log
	xCurrentElement = xGreatGrandChildElement.FirstChild( "logSeq" );
	if( !xCurrentElement.IsEmpty() && xCurrentElement.GetText() != NULL )
		szLogSeq = xCurrentElement.GetText();	

	xCurrentElement = xGreatGrandChildElement.FirstChild( "cancelAmt" );
	if( !xCurrentElement.IsEmpty() && xCurrentElement.GetText() != NULL )
		cancelAmt = atoi( xCurrentElement.GetText() ); //��ҵ� �ݾ� 

	xCurrentElement = xGreatGrandChildElement.FirstChild( "remainAmt" );
	if( !xCurrentElement.IsEmpty() && xCurrentElement.GetText() != NULL )
		remainAmt = atoi( xCurrentElement.GetText() ); //���ŷ� �ݾ� - ��ҵ� �ݾ� 

	// ĳ���� ���� ������ ĳ���� �����ϴ�.
	SP2Packet kPacket( BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT );
	
	kPacket << rData.GetUserIndex();
	kPacket << rData.GetBillingGUID();
	kPacket << rData.GetIndex();
	kPacket << rData.GetChargeNo();
	kPacket << BILLING_SUBSCRIPTION_RETRACT_RESULT_SUCCESS;
	kPacket << cancelAmt;
	kPacket << remainAmt; 
	 
	//kyg �ϴ�.. �־����� ���� szReturnBuyNO�� 
	if(!g_ServerNodeManager.SendMessageIP( (ioHashString) rData.GetServerIP(),  rData.GetServerPort(), kPacket ))
	{
		LOG.PrintTimeAndLog( 0, "%s Send Fail: %d:%s:%s Ret %d", "ioChannelingNodeDaum::ThreadSubscriptionRetract", rData.GetUserIndex(), rData.GetBillingGUID().c_str(), rData.GetPrivateID().c_str(), rData.GetChargeNo());
		return;
	}
	LOG.PrintTimeAndLog(0, "%s Success. :%d:%s:PrivateID %s:%s:%s[%s]", __FUNCTION__, rData.GetUserIndex(), rData.GetBillingGUID().c_str(),
		rData.GetPrivateID().c_str(), rData.GetPublicID().c_str(), rData.GetUserNo().c_str(),rData.GetChargeNo().c_str() );
}

void ioChannelingNodeDaum::UTF8ToAnsi( IN const char *szUTF8, OUT char *szAnsi, OUT int &riReturnAnsiSize, IN int iAnsiBufferSize )
{
	if( szUTF8 == NULL )
		return;

	WCHAR szUni[MAX_PATH*2];
	ZeroMemory( szUni, sizeof(szUni) );

	int iUnisize = MultiByteToWideChar(CP_UTF8, 0, szUTF8, -1 , szUni, MAX_PATH*2 );
	riReturnAnsiSize = WideCharToMultiByte(CP_ACP, 0 , szUni, iUnisize, szAnsi, iAnsiBufferSize ,NULL ,NULL );
}

//------------------------------------------------------------------------------------------------DAUM
#include "openssl/sha.h"
#include "openssl/hmac.h"

#pragma warning (disable : 4996)
#pragma warning (disable : 4554)
#pragma warning (disable : 4018)

void ioChannelingNodeDaum::decode(unsigned char* s, unsigned char* ret)
{
	int i=0;
	for(; i < strlen((const char*)s); i += 2)
	{
		unsigned char c = s[i];
		unsigned char c1 = s[i + 1];
		int j = i / 2;
		if(c < 'a')
			ret[j] = (unsigned char)(c - 48 << 4);
		else
			ret[j] = (unsigned char)((c - 97) + 10 << 4);
		if(c1 < 'a')
			ret[j] += (unsigned char)(c1 - 48);
		else
			ret[j] += (unsigned char)((c1 - 97) + 10);
	}
	ret[strlen((const char*)s)/2] = '\0';
}

char *ioChannelingNodeDaum::pt(unsigned char *md, char* buf)
{
	int i;

	for (i=0; i<SHA_DIGEST_LENGTH; i++)
		sprintf(&(buf[i*2]),"%02x",md[i]);
	return(buf);
}

char *ioChannelingNodeDaum::getSig(char *apiKey, char* data, char* ts, char* nonce) 
{
	int orgLen = strlen(data) + strlen(ts) + strlen(nonce) + 1;
	char *orgStr = (char*)malloc(orgLen);
	char *digestStr = (char*)malloc(SHA_DIGEST_LENGTH*2 + 1);
	char decodeApiKey[80];
	char *md;
	char tmp[80];
	char c[80];

	strncpy_s(orgStr,orgLen, data, strlen(data)+1);
	strncat_s(orgStr,orgLen, ts, strlen(ts)+1);
	strncat_s(orgStr,orgLen, nonce, strlen(nonce)+1);	

	decode((unsigned char*)apiKey, (unsigned char*)decodeApiKey);

	md =(char*) HMAC(EVP_sha1(),
		decodeApiKey, strlen(decodeApiKey),
		(const unsigned char*)orgStr, strlen(orgStr),
		(unsigned char*)c, NULL);

	strncpy_s(digestStr,SHA_DIGEST_LENGTH*2 + 1, pt((unsigned char *)md, tmp), SHA_DIGEST_LENGTH*2);
	digestStr[SHA_DIGEST_LENGTH*2] = '\0';

	free(orgStr);

	return digestStr;

}

void ioChannelingNodeDaum::getNewNonce(char *nonce) 
{
	int i, tmp;

	memset(nonce, '0', SHA_DIGEST_LENGTH*2);
	srand(time(NULL));
	for(i=0; i<40; i+=6) {
		tmp = rand();
		sprintf(&(nonce[i]),"%02x", tmp);
	}
	nonce[40] = '\0';
}

void ioChannelingNodeDaum::getNewTimeStamp(char *ts) 
{
	time_t          cal;
	struct tm       *lctm;

	if ((cal = time(NULL)) < 0)  {
		return;
	}

	lctm = localtime(&cal);

	strftime(ts, 15, "%Y%m%d%H%M%S", lctm);
	ts[15] = '\0'; //kyg ������ 15¥���� 15�� �ΰ�����.. 
}

void ioChannelingNodeDaum::_OnSubscriptionRetract( ServerNode* pServerNode, SP2Packet& rkPacket ) //kyg �̷�ƾ Ÿ�°ɷε� �׽�Ʈ �غ����� 
{
	int          iChannelingType = 0;
	ioHashString szBillingGUID;
	DWORD        dwUserIndex = 0;
	ioHashString szPublicID;
	ioHashString szPrivateID;
	DWORD		 dwIndex = 0;
	ioHashString szChargeNo;
	ioHashString szDaumNo;

	rkPacket >> iChannelingType;
	rkPacket >> szBillingGUID;
	rkPacket >> dwUserIndex;
	rkPacket >> szPublicID;
	rkPacket >> szPrivateID;
	rkPacket >> dwIndex; 
	//�������� �������� 
	rkPacket >> szChargeNo;
	rkPacket >> szDaumNo;
 


	ioData kData;

	kData.SetChannelingType( iChannelingType );
	kData.SetBillingGUID( szBillingGUID );
	kData.SetUserIndex( dwUserIndex );
	kData.SetPublicID( szPublicID );
	kData.SetPrivateID( szPrivateID );
	kData.SetUserNo( szDaumNo );
	kData.SetChargeNo( szChargeNo );
	kData.SetIndex( dwIndex );
 
	if( pServerNode )
	{
		kData.SetServerIP( pServerNode->GetIP() );
		kData.SetServerPort( pServerNode->GetClientPort() );
	}
	kData.SetEmpty( false );
	kData.SetProcessType( ioData::PT_SUBSCRIPTION_RETRACT );

	g_ThreadPool.SetData( kData );
}
