#ifndef ___PROTOCOL_H__
#define ___PROTOCOL_H__


#define SSTPK_CLOSE								0x3097
// BillingRelayServer to Server TCP PACKET 0x10001 ~ 0x10500
#define BSTPK_GET_CASH									0x10001
#define BSTPK_GET_CASH_RESULT							0x10002
#define BSTPK_OUTPUT_CASH								0x10003
#define BSTPK_OUTPUT_CASH_RESULT						0x10004
#define BSTPK_SERVER_IPPORT								0x10005
#define BSTPK_LOGIN										0x10006
#define BSTPK_LOGIN_RESULT								0x10007
#define BSTPK_REFUND_CASH								0x10008
#define BSTPK_REFUND_CASH_RESULT						0x10009
#define BSTPK_USER_INFO									0x10010
#define BSTPK_USER_INFO_RESULT							0x10011
#define BSTPK_AUTOUPGRADE_LOGIN							0x10012
#define BSTPK_AUTOUPGRADE_LOGIN_RESULT					0x10013
#define BSTPK_PCROOM									0x10014
#define BSTPK_PCROOM_RESULT								0x10015
#define BSTPK_OTP										0x10016
#define BSTPK_OTP_RESULT								0x10017
#define BSTPK_GET_MILEAGE								0x10018
#define BSTPK_GET_MILEAGE_RESULT						0x10019
#define BSTPK_ADD_MILEAGE								0x10020
#define BSTPK_ADD_MILEAGE_RESULT						0x10021
#define BSTPK_IPBONUS									0x10022
#define BSTPK_IPBONUS_RESULT							0x10023
#define BSTPK_CANCEL_CASH								0x10024
#define BSTPK_IPBONUS_OUT								0x10025
#define BSTPK_ADD_CASH									0x10026
#define BSTPK_ADD_CASH_RESULT							0x10027
#define BSTPK_FILL_CASH_URL								0x10028
#define BSTPK_FILL_CASH_URL_RESULT						0x10029
#define BSTPK_SUBSCRIPTION_RETRACT_CASH_CHECK			0x10030
#define BSTPK_SUBSCRIPTION_RETRACT_CASH_CHECK_RESULT	0x10031
#define BSTPK_SUBSCRIPTION_RETRACT_CASH					0x10032
#define BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT			0x10033
#define BSTPK_SUBSCRIPTION_RETRACT_CASH_FAIL			0x10034
#define BSTPK_SUBSCRIPTION_RETRACT_CASH_FAIL_REULST		0x10035
#define BSTPK_SESSION_CONTROL							0x10036
#define BSTPK_SESSION_CONTROL_RESULT					0x10037
#define BSTPK_CLOSE										0x10038
#define BSTPK_REQUEST_USERINFO							0x10039
#define BSTPK_TIMEOUT_BILLINGGUID						0x10040
#define BSTPK_FIRST_LOGIN								0x10041
#define BSTPK_LOGOUT									0x10042
#define BSTPK_CCU_COUNT									0x10043
#define BSTPK_DAUM_SHUTDOWN_CHECK						0x10044
#define BSTPK_REQUEST_TIME_CASH							0x10045
#define BSTPK_REQUEST_TIME_CASH_RESULT					0x10046

enum SessionControlTypes
{
	NexonTerminate = 3, //과금시간 끝 종료 
	NexonSessionMessage,
	NexonSessionLogin,
	NexonSessionLogout,
	NexonReconnect,
	NexonIDTrial,//넌 체험 유저
	NexonOnPCRoom,
	NexonIDMaxError,
	//ShutDownStart
	NexonWelcomeMessage,//시간이 0이라면 종료(int값으로 남은시간이 감)
	NexonIPDuplicate,//무조건 종료
	NexonIPMaxError,//무조건 종료
	NexonPCRoomBlocked,//넥슨 쪽에서 블록한 피시방임 
	NexonMessageShutDown,// 셧다운메시지 
	NexonMessageTerminate,//과금 관련 메시지 
	NexonAccountNotAllowed,//허용된 ID 가 아닙니다
	NexonAccountMaxConnected,//개인 최대접속 ID 수를 넘었습니다
	NexonPrepaidExhausted,//PC방 정량제 시간이 모두 소진되었습니다 //무조건 종료 
	NexonAccountMachineIDBlocked,//캐시도용 사용자 입니다.
	NexonSelctShutdown,
	NexonShutDown, // 셧다운  종료 
	NexonForbidden,
	NexonNotNexonUser,
	NexonUnknownShutDown,
	NexonAgeError,
	NexonSessionServerError,
	NexonAddressNotAllowed,
};

/************************************************************************/
/* Internal Packet                                                                     */
/************************************************************************/
#define ITPK_ACCEPT_SESSION					0x5555
#define ITPK_CLOSE_SESSION					0x5556
#define ITPK_TEST_TIMER						0x5567
#define ITPK_OPERATION						0x5568
#define ITPK_NEXONALIVE						0x5569
#define ITPK_NEXON_SESSION_CHECK			0x5570
#define ITPK_WRITEPROCESSLOG				0x5571
#define ITPK_TIMEOUT_BILLINFO				0x5572

// BillingRelayServer to mgame billing server TCP PACKET 0x10501 ~ 0x10600
#define BMTPK_CASH_ACTION_REQUEST 0x10501 
#define BMTPK_CASH_ACTION_RESULT  0x10502

// BillingRelayServer to buddy billing server TCP PACKET 0x10601 ~ 0x10700
#define BBTPK_CASH_ACTION_REQUEST     0x10601
#define BBTPK_FIND_CASH_RESULT        0x10602
#define BBTPK_BUY_CASH_RESULT         0x10603

// BillingRelayServer to Wemade billing server TCP PACKET 0x10701 ~ 0x10800
#define BWTPK_CLOSE					  0x10700
#define BWTPK_HEALTH_CHECK_REQUEST    0x10701
#define BWTPK_HEALTH_CHECK_RESULT     0x10702
#define BWTPK_CHECKPREMIUM2_REQUEST   0x10703
#define BWTPK_CHECKPREMIUM2_RESULT    0x10704
#define BWTPK_GS_CONNECT_REQUEST      0x10705
#define BWTPK_GS_CONNECT_RESULT       0x10706

// BillingRelayServer to thailand login server TCP PACKET 0x10801 ~ 0x10802
#define BTTPK_LOGIN_REQUEST           0x10801
#define BTTPK_LOGIN_RESULT            0x10802

// BillingRelayServer to thailand OTP server TCP PACKET 0x10803 ~ 0x10804
#define BTTPK_OTP_REQUEST             0x10803
#define BTTPK_OTP_RESULT              0x10804

// BillingRelayServer to thailand IPBonus server TCP PACKET 0x10805 ~ 0x10806
#define BTTPK_IPBONUS_REQUEST         0x10805
#define BTTPK_IPBONUS_RESULT          0x10806

// BillingRelayServer to thailand Billing server TCP PACKET 0x10807 ~ 0x10810
#define BTTPK_BILLING_GET_REQUEST     0x10807
#define BTTPK_BILLING_GET_RESULT      0x10808
#define BTTPK_BILLING_SET_REQUEST     0x10809
#define BTTPK_BILLING_SET_RESULT      0x10810

// BillingRelayServer to tooniland billing server TCP PACKET 0x10811 ~ 0x10816
#define BTNTPK_FIND_CASH_REQUEST      0x10811
#define BTNTPK_FIND_CASH_RESULT       0x10812
#define BTNTPK_BUY_CASH_REQUEST       0x10813
#define BTNTPK_BUY_CASH_RESULT        0x10814
#define BTNTPK_SUBSCRIPTION_RETRACT_CASH_REQUEST    0x10815
#define BTNTPK_SUBSCRIPTION_RETRACT_CASH_RESULT     0x10816

// BillingRelayServer to thailand IPBonus OUT server TCP PACKET 0x10817 ~ 0x10818
#define BTTPK_IPBONUS_OUT_REQUEST     0x10817
#define BTTPK_IPBONUS_OUT_RESULT      0x10818

// BillingRelayServer to Wemade Buy server TCP PACKET 0x10819 ~ 0x10828 & 0x11838 ~ 
#define BTWBTPK_ALIVE_REQUEST				0x10819
#define BTWBTPK_ALIVE_RESULT				0x10820
#define BTWBTPK_BALANCE_REQUEST				0x10821
#define BTWBTPK_BALANCE_RESULT				0x10822
#define BTWBTPK_BUY_REQUEST					0x10823
#define BTWBTPK_BUY_RESULT					0x10824
#define BTWBTPK_PRESENT_REQUEST				0x10825
#define BTWBTPK_PRESENT_RESULT				0x10826
#define BTWBTPK_CANCEL_REQUEST				0x10827
#define BTWBTPK_CANCEL_RESULT				0x10828
#define BTWBTPK_SUBSCRIPTION_RETRACT		0x11838
#define BTWBTPK_SUBSCRIPTION_RETRACT_CHECK	0x11839
#define BTWBTPK_SUBSCRIPTION_RETRACT_CANCEL	0x11840

// BillingRelayServer to US Billing Server TCP PACKET 0x10829 ~ 0x10836
#define BTUBTPK_ALIVE_REQUEST		  0x10829
#define BTUBTPK_ALIVE_RESULT		  0x10830
#define BTUBTPK_BALANCE_REQUEST       0x10831
#define BTUBTPK_BALANCE_RESULT        0x10832
#define BTUBTPK_BUY_REQUEST           0x10833
#define BTUBTPK_BUY_RESULT            0x10834
#define BTUBTPK_CANCEL_REQUEST        0x10835
#define BTUBTPK_CANCEL_RESULT         0x10836
#define BTUBTPK_CLOSE                 0x10837


// BMTPK_CASH_ACTION_REQUEST
#define CASH_ACTION_REQUEST_GET_CASH  0x90
#define CASH_ACTION_REQUEST_BUY       0x91
#define CASH_ACTION_REQUEST_CHECK     0x95

// Monitoring Server <-> Server TCP PACKET 0x10651 ~ 0x10700
#define MNSTPK_STATUS_REQUEST	0x10651
#define MNSTPK_STATUS_RESULT	0x10652
#define MNSTPK_CHANGE_REQUEST	0x10653
#define MNSTPK_CHANGE_RESULT	0x10654
#define MNSTPK_CLOSE			0x10605

// BMTPK_CASH_ACTION_RESULT ( 로스트사가 빌링 (mgame).doc 참고 )
#define CASH_ACTION_RESULT_SUCCESS            1  
#define CASH_ACTION_RESULT_ALIVE_SUCCESS      6
#define CASH_ACTION_RESULT_FAIL_CHANNEL       2
#define CASH_ACTION_RESULT_FAIL_CALL          3
#define CASH_ACTION_RESULT_FAIL_PAYMENT       4
#define CASH_ACTION_RESULT_FAIL_NONE_ACCOUNT  5
#define CASH_ACTION_RESULT_FAIL_SYSTEM_CHECK  99
#define CASH_ACTION_RESULT_FAIL_ARQUMENT      -1

// BSTPK_GET_CASH_RESULT / BSTPK_OUTPUT_CASH_RESULT
#define CASH_RESULT_SUCCESS           1 // 1이외의 값은 모두 에러 BillFlex IPG readme 참고
#define CASH_RESULT_ERROR_EXCESS_BUY  -12
#define CASH_RESULT_EXCEPT            -1000

// BBTPK_FIND_CASH_RESULT / BBTPK_BUY_CASH_RESULT ( 030_-_버디게임캐쉬연동가이드(게임).ppt 참고 )
#define BUDDY_CASH_ACTION_RESULT_SUCCESS            0x00
#define BUDDY_CASH_ACTION_RESULT_FAIL_CONNECT       0x01
#define BUDDY_CASH_ACTION_RESULT_FAIL_EXCEPTION     0x02
#define BUDDY_CASH_ACTION_RESULT_FAIL_RETURN_VALUE  0x03
#define BUDDY_CASH_ACTION_RESULT_FAIL_REQUEST_VALUE 0x04
#define BUDDY_CASH_ACTION_RESULT_FAIL_UNKNOWN       0xFF


// BSTPK_OUTPUT_CASH
#define OUTPUT_CASH_SOLDIER         1
#define OUTPUT_CASH_DECO            2
#define OUTPUT_CASH_SOLDIER_EXTEND  3
#define OUTPUT_CASH_ETC             4
#define OUTPUT_CASH_SOLDIER_CHANGE_PERIOD  5
#define OUTPUT_CASH_EXTRA                  6
#define OUTPUT_CASH_PRESENT                7
#define OUTPUT_CASH_SUBSCRIPTION   8
#define OUTPUT_CASH_POPUP			10

// BSTPK_LOGIN_RESULT
#define BILLING_LOGIN_RESULT_SUCCESS   1
#define BILLING_LOGIN_RESULT_FAIL      2

// BSTPK_REFUND_CASH_RESULT
#define BILLING_REFUND_CASH_RESULT_SUCCESS   1
#define BILLING_REFUND_CASH_RESULT_FAIL      2

// BSTPK_SUBSCRIPTION_RETRACT_CASH_RESULT
#define BILLING_SUBSCRIPTION_RETRACT_RESULT_SUCCESS   1
#define BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL      2

// BSTPK_OTP_RESULT
#define BILLING_OTP_RESULT_SUCCESS   1
#define BILLING_OTP_RESULT_FAIL      2

// BSTPK_GET_MILEAGE_RESULT
#define BILLING_GET_MILEAGE_RESULT_SUCCESS   1
#define BILLING_GET_MILEAGE_RESULT_FAIL      2

// BSTPK_ADD_MILEAGE_RESULT
#define BILLING_ADD_MILEAGE_RESULT_SUCCESS   1
#define BILLING_ADD_MILEAGE_RESULT_FAIL      2

// BSTPK_IPBONUS_RESULT
#define BILLING_IPBONUS_RESULT_SUCCESS   1
#define BILLING_IPBONUS_RESULT_FAIL      2


//BSTPK_REQUEST_TIME_CASH_RESULT
#define BILLING_TIME_CASH_RESULT_SUCCESS	1
#define BILLING_TIME_CASH_RESULT_FAIL		2

//DB TCP PACKET
#define DTPK_QUERY                0x0001
#define DTPK_QUERY_PING           0x0002 // DBAgent에서 리턴값 없음.

// DBLOG TYPE
#define BILLING_ERROR_LOG_NO_ANSWER								-1001
#define BILLING_ERROR_LOG_INVALID_HEADER						-1002
#define BILLING_ERROR_LOG_DISCONNECT_GAMESERVER					-1003
#define BILLING_ERROR_LOG_FAIL_SEND_MGAME_BILLINGSERVER			-1004
#define BILLING_ERROR_LOG_FAIL_ADD_BUYINFO						-1005
#define BILLING_ERROR_LOG_EXCEPTION								-1006
#define BILLING_ERROR_LOG_FAIL_SEND_BUDDY_BILLINGSERVER			-1007
#define BILLING_ERROR_LOG_FAIL_SEND_WEMADE_BILLINGSERVER		-1008
#define BILLING_ERROR_LOG_FAIL_SEND_TOONILAND_BILLINGSERVER		-1009
#define BILLING_ERROR_LOG_FAIL_SEND_NEXON_SESSIONSERVER			-1010

#define BILLING_ERROR_LOG_US_EXCEPTION                  -2001


#define CTPKM_REQUEST_NUM_CONNECT						0x8001
#define CTPKM_MONITOR_IP								0x8005
#define CTPKM_SERVER_INFO_REQUEST						0x8007
#define CTPKM_BILLINGSERVER_USER_REQUEST				0x8043
#define CTPKM_BILLINGSERVER_FIRST_CONNECT				0x8044
#define CTPKM_BILLINGSERVER_RELOAD_INI					0x8045
#define CTPKM_BILLINGSERVER_NEXONPCROOM_LOGOUT			0x8046

//SERVER SEND TCP MONITORING PACKET
#define STPKM_ANSWER_NUM_CONNECT						0x8501
#define STPKM_ALL_SERVER_INFO							0x8505
#define STPKM_SERVER_INFO_ACK							0x8506
#define STPKM_BILLING_USER								0x8509//
#define STPKM_ANSWER_BILLING_CONNECT					0x8510//
#define STPKM_BILLINGSERVER_RELOAD_INI					0x8511
		
/************************************************************************/
/* OperationID Define                                                                     */
/************************************************************************/
enum
{
	OPID_TESTOPERATION,
	OPID_ONCONNECT,
	OPID_WRITEPROCESSLOG,
	OPID_TESTCLOSEOPERATION,
};


#endif
