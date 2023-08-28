// QueryData.cpp: implementation of the CQueryData class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "QueryData.h"

extern CLog LOG;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQueryData::CQueryData()
{
	memset( m_szReturnBuf, 0, sizeof( m_szReturnBuf ) );
	m_iReturnCount = 0;

	m_pBuffer = NULL;
	CQuery::Clear();
}

CQueryData::~CQueryData()
{
	CQuery::Clear();
}

char *CQueryData::NewResultBuffer(int &nSize)
{
	//������ ��������� ������ ������.
	nSize = GetBufferSize();
	nSize -= ((sizeof(ValueType) * GetValueCnt()) + GetQuerySize());
	//�����͸� �̾Ƽ� �������� ������.
	for(int i = 0;i < GetValueCnt();i++)
		nSize += GetValueType(i).size;
	
	return new char[nSize];
}

char *CQueryData::GetReturnResultData(int &nSize)
{
	char *pBuffer = GetBuffer();
	pBuffer += GetQuerySize();      //���� �����κ� �ѱ�.
	pBuffer += (sizeof(ValueType) * GetValueCnt());  //������ Ÿ�� �κ� �ѱ�.
	nSize = GetBufferSize();
	nSize -= ((sizeof(ValueType) * GetValueCnt()) + GetQuerySize());
	return pBuffer;
}

char *CQueryData::GetQuery(int &nLen)
{
	if(m_pBuffer == NULL)
	{
		LOG.PrintTimeAndLog(0,"Query Buffer NULL POINTER!!");
		return NULL;
	}
	nLen = m_queryHeader.nQuerySize;
	
	return &m_pBuffer[0];
}
ValueType CQueryData::GetValueType(int nPos)
{
	ValueType vt;
	if(m_pBuffer == NULL)
	{
		LOG.PrintTimeAndLog(0,"Query Buffer NULL POINTER!!");
		return vt;
	}
	int nCnt = (sizeof(ValueType)*nPos)+m_queryHeader.nQuerySize;
	memcpy(&vt,&m_pBuffer[nCnt],sizeof(ValueType));
	
	return vt;
}

void CQueryData::SetReturnData( const void *pData, int iSize )
{
	memcpy( &m_szReturnBuf[m_iReturnCount], pData, iSize );
	m_iReturnCount += iSize;

	if( m_iReturnCount >= MAX_BUFFER )
		LOG.PrintTimeAndLog( 0, "CQueryData::SetReturnData Return Size : %d", m_iReturnCount );
}

void CQueryData::SetBuffer(const char *buffer)
{
	memcpy(&m_queryHeader,buffer,sizeof(QueryHeader));
	buffer+= sizeof(QueryHeader);
	m_pBuffer = new char[m_queryHeader.nQueryBufferSize];
	memcpy(m_pBuffer,buffer,m_queryHeader.nQueryBufferSize);		
}

void CQueryData::SetData(unsigned int socket,int nResult_Type,int nMsgType, int nQueryType,
						 char *szQuery, int nQuerySize,
						 ValueType* pTypes, int nTypeCount,int nSetValue)
{
	CQuery::Clear();
	//���
	m_queryHeader.csocket           = socket;
	m_queryHeader.nResult_Type      = nResult_Type;
	m_queryHeader.nMsgType			= nMsgType;
	m_queryHeader.nQueryType		= nQueryType;
	m_queryHeader.nQuerySize		= nQuerySize;
	m_queryHeader.nValueTypeCnt		= nTypeCount;
	m_queryHeader.nReturnValueSize	= m_iReturnCount;
	m_queryHeader.nSetValue         = nSetValue;
	m_queryHeader.nQueryBufferSize  = nQuerySize + (sizeof(ValueType) * nTypeCount) + m_iReturnCount;	
	//���� �޸� �Ҵ�.
	if(m_queryHeader.nQueryBufferSize != 0)
	{
		m_pBuffer = new char[m_queryHeader.nQueryBufferSize];
		//������.
		int nSize = 0;                     //������ ��ġ
		int nlen = 0;
		if(szQuery != NULL)
		{
			nlen = strlen(szQuery);        //ī���� ������ ������
			memcpy(&m_pBuffer[nSize],szQuery,nlen);					 //���� ����
		}
		if(pTypes != NULL)
		{
			nSize += nlen;                      
			nlen = sizeof(ValueType)*nTypeCount;
			memcpy(&m_pBuffer[nSize],pTypes,nlen);      //��� Ÿ��.
		}
		if(m_iReturnCount > 0)
		{
			nSize += nlen;
			memcpy(&m_pBuffer[nSize],m_szReturnBuf,m_iReturnCount);   //�������� ������.
		}
	}
}


///////////////////////////////////////////////////////////////
