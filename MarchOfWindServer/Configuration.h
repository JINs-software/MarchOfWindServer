#pragma once

//MOWServer(
//	const char* serverIP, uint16 serverPort, uint16 maximumOfConnections,
//	BYTE packetCode_LAN, BYTE packetCode, BYTE packetSymmetricKey,
//	bool recvBufferingMode,
//	uint16 maximumOfSessions,
//	uint32 numOfIocpConcurrentThrd, uint16 numOfIocpWorkerThrd,
//	size_t tlsMemPoolUnitCnt, size_t tlsMemPoolUnitCapacity,
//	bool tlsMemPoolMultiReferenceMode, bool tlsMemPoolPlacementNewMode,
//	uint32 memPoolBuffAllocSize,
//	uint32 sessionRecvBuffSize
//);

#define MOW_SERVER_IP						NULL
#define MOW_SERVER_PORT						7777
#define	MOW_MAX_OF_CONNECTIONS				1000

#define dfPACKET_CODE		119
#define dfPACKET_KEY		50

#define MOW_RECV_BUFFERING_MODE				false

#define	MOW_MAX_OF_SESSIONS					1000

#define	MOW_IOCP_CONCURRENT_THREAD			0
#define MOW_NUM_OF_IOCP_WORKERS				4

#define MOW_TLS_MEM_POOL_UNIT_CNT			1000
#define MOW_TLS_MEM_POOL_CAPACITY			5000

#define	MOW_TLS_MEM_POOL_REFERENCE_MODE		true
#define MOW_TLS_MEM_POOL_PLACEMENT_NEW_MODE	false

#define	MOW_POOL_BUFFER_ALLOC_SIZE			1000

#define MOW_SESSION_RECV_BUFF_SIZE			10000
