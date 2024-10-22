#pragma once
#include "JNetCore.h"
#include "Configuration.h"

using namespace jnet;
using namespace jgroup;

#if defined(TOKEN_CHECK)
const int TOKEN_LEN = 20;
namespace RedisCpp {
	class CRedisConn;
}
#endif

class GatewayThread : public JNetGroupThread
{
private:
	virtual void OnStart() override;
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override {}
	virtual void OnLeaveClient(SessionID64 sessionID) override {}
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;
	virtual void OnGroupMessage(GroupID groupID, JBuffer& msg) override {}

#if defined(TOKEN_CHECK)
	RedisCpp::CRedisConn* m_RedisConn;
	bool TokenCheck(JBuffer& tokenBuffer);
#endif
};

