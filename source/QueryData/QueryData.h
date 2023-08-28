// QueryData.h: interface for the CQueryData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUERYDATA_H__BC410DEE_2BBD_424A_A266_499332FBEE59__INCLUDED_)
#define AFX_QUERYDATA_H__BC410DEE_2BBD_424A_A266_499332FBEE59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace std;

// ������ Ÿ�԰� ������
enum VariableType 
{ 
	vChar = 100,
	vWChar,
	vTimeStamp,
	vLONG,
	vINT64,
	vSHORT,
	vWrong
};

typedef struct tagValueType
{
	VariableType type;			// � Ÿ���� �������ΰ�
	int size;					// �������?
	tagValueType()
	{
		type = vWrong;
		size = 0;
	}
}ValueType;
typedef vector<ValueType> vVALUETYPE;

// CQuery�� ���
typedef struct tagQueryHeader
{
	int nMsgType;				// �޼��� Ÿ��,	DB�������� ����ϱ⺸�� ���߿� ���Ӽ����� ����� �޾Ƽ� ó���Ҷ� �ʿ�
	int nQueryType;				// ������ Ÿ�� (insert = 0, delete = 1, select = 2, update = 3)
	int nQuerySize;				// ���� �κ� ������
	int nValueTypeCnt;			// ������� ����
	int nReturnValueSize;		// �ٽ� �ǵ��� �޾ƾ� �ϴ� �������� ������
	int nSetValue;				// MoveNext : �����̺� ��� ����?
	int nQueryBufferSize;       // ������ ������ (���� ������)
	unsigned int csocket;       // ����.
	int nResult_Type;           // ��� ������ �ൿ.
	tagQueryHeader()
	{
		nMsgType		= -1;
		nQueryType		= -1;
		nQuerySize		= -1;
		nValueTypeCnt	= -1;
		nReturnValueSize= -1;
		nSetValue       = -1;
		nQueryBufferSize= -1;
		csocket         = 0;
		nResult_Type    = 0;
	}
}QueryHeader;

class CQuery
{
protected:
	QueryHeader	m_queryHeader;					// �������
	char		*m_pBuffer;		// ��������, ���������Ÿ�Ե�, �ٽ� �����޾ƾ��� ������
public:
	void Clear()
	{
		memset(&m_queryHeader,0,sizeof(QueryHeader));
		if(m_pBuffer != NULL)
		{
			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}
	}
};

class CQueryData : public CQuery  
{
public://GET 
	//HEADER
	int GetMsgType(){ return m_queryHeader.nMsgType; }
	int GetQueryType(){ return m_queryHeader.nQueryType; }
	int GetQuerySize(){ return m_queryHeader.nQuerySize; }
	int GetValueCnt(){ return m_queryHeader.nValueTypeCnt; }
	int GetReturnValueCnt() {return m_queryHeader.nReturnValueSize; }
	int GetSetValue() { return m_queryHeader.nSetValue; }
	int GetBufferSize(){ return m_queryHeader.nQueryBufferSize; }
	unsigned int GetSocket(){ return m_queryHeader.csocket; }
	int GetResultType(){ return m_queryHeader.nResult_Type; }
	//DATA
	ValueType GetValueType(int nPos);
	char *GetQuery(int &nLen);
	QueryHeader *GetHeader(){ return &m_queryHeader; }
	char *GetBuffer(){ return m_pBuffer; }
	
public:
	//SET
	void SetData(unsigned int socket,int nResult_Type,int nMsgType, int nQueryType,
				 char *szQuery, int nQuerySize, ValueType* pTypes, int nTypeCount, int nSetValue = 1);
	void SetBuffer(const char *buffer);
	
public:
	//���Ǹ� ���� ���� �Լ�.
	//ResultMessage�� ���鶧 ������ ������ �� NEW�� �Ҵ��ϱ� ���� �Լ�.
	char *NewResultBuffer(int &nSize);
	//�������� �������K���� �������� �����Ϳ� ������ ��ȯ.
	//���� �����Ͱ� �ƴϴ� ����.
	char *GetReturnResultData(int &nSize);

protected:    //�����ޱ� ���� �����ϴ� ������
	char m_szReturnBuf[MAX_BUFFER];
	int  m_iReturnCount;
public:
	void SetReturnData( const void *pData, int iSize );

	
public:
	CQueryData();
	virtual ~CQueryData();
};


#endif // !defined(AFX_QUERYDATA_H__BC410DEE_2BBD_424A_A266_499332FBEE59__INCLUDED_)
