#pragma once


#include "../BaseClass/handler.h"

class LSLogicHandler :
	public Handler
{
public:
	LSLogicHandler(MPSCRecvQueue* queue) : Handler(queue) //kyg ����
	{
		Init();
	};
	virtual ~LSLogicHandler(void);

public://fuction
	virtual void Init();

};

