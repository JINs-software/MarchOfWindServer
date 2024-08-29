#pragma once
#include "JNetCore.h"

using namespace jnet;
using namespace jgroup;

class GatewayThread : public JNetGroupThread
{
private:
	virtual void OnStart() override {}
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override {}
	virtual void OnLeaveClient(SessionID64 sessionID) override {}
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;
	virtual void OnGroupMessage(GroupID groupID, JBuffer& msg) override {}
};

