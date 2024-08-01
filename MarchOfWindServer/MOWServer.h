#pragma once

#include "JNetCore.h"

#include "Group.h"
#include "GatewayThread.h"
#include "LobbyThread.h"

using namespace jnet;
using namespace jgroup;

class MOWServer : public JNetGroupServer
{
public:
	MOWServer(
		const char* serverIP, uint16 serverPort, uint16 maximumOfConnections,
		BYTE packetCode_LAN, BYTE packetCode, BYTE packetSymmetricKey,
		bool recvBufferingMode,
		uint16 maximumOfSessions,
		uint32 numOfIocpConcurrentThrd, uint16 numOfIocpWorkerThrd,
		size_t tlsMemPoolUnitCnt, size_t tlsMemPoolUnitCapacity,
		bool tlsMemPoolMultiReferenceMode, bool tlsMemPoolPlacementNewMode,
		uint32 memPoolBuffAllocSize,
		uint32 sessionRecvBuffSize
	)
		: JNetGroupServer(
			serverIP, serverPort, maximumOfConnections,
			packetCode_LAN, packetCode, packetSymmetricKey,
			recvBufferingMode,
			maximumOfSessions,
			numOfIocpConcurrentThrd, numOfIocpWorkerThrd,
			tlsMemPoolUnitCnt, tlsMemPoolUnitCapacity,
			tlsMemPoolMultiReferenceMode, tlsMemPoolPlacementNewMode,
			memPoolBuffAllocSize,
			sessionRecvBuffSize
		)
	{}
	~MOWServer() {}

	bool Start() {
		if (!JNetGroupServer::Start()) {
			return false;
		}

		CreateGroup(GATEWAY_GROUP, new GatewayThread());
		CreateGroup(LOBBY_GROUP, new LobbyThread());
	}

private:
	virtual void OnPrintLogOnConsole() {}
	virtual void OnClientJoin(SessionID64 sessionID, const SOCKADDR_IN& clientSockAddr) override;
	virtual void OnClientLeave(SessionID64 sessionID) override;
};

