#pragma once

#include "JNetCore.h"
#include "Protocol.h"

using namespace jnet;

class BatthFieldServer : public JNetServer
{

private:
	virtual void OnClientJoin(SessionID64 sessionID, const SOCKADDR_IN& clientSockAddr);
	virtual void OnClientLeave(SessionID64 sessionID);
	virtual void OnRecv(SessionID64 sessionID, JBuffer& recvBuff);
};

