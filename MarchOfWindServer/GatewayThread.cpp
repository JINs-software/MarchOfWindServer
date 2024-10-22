#include "GatewayThread.h"
#include "Group.h"
#include "HubThread.h"
#include "BattleThread.h"
#include "Protocol.h"

#if defined(TOKEN_CHECK)
#include "CRedisConn.h"
#endif

void GatewayThread::OnStart()
{
#if defined(TOKEN_CHECK)
	m_RedisConn = new RedisCpp::CRedisConn();
	if (m_RedisConn == NULL) {
		DebugBreak();
	}
#endif
}

void GatewayThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type) {
		case MOW_HUB::C2S_CONNECTION:
		{
#if defined(TOKEN_CHECK) 
			recvData.DirectMoveDequeueOffset(sizeof(type));
			if (!TokenCheck(recvData)) {
				continue;
			}
#endif
			ForwardSessionToGroup(sessionID, LOBBY_GROUP);

			JBuffer* connMsg = AllocSerialBuff();
			MOW_HUB::MSG_C2S_CONNECTION* body = connMsg->DirectReserve<MOW_HUB::MSG_C2S_CONNECTION>();
			recvData.Dequeue(reinterpret_cast<BYTE*>(body), sizeof(MOW_HUB::MSG_C2S_CONNECTION));
			ForwardSessionMessage(sessionID, connMsg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_CONN_TO_BATTLE_FIELD:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD msg;
			recvData >> msg;
			ForwardSessionToGroup(sessionID, msg.BATTLE_FIELD_ID);
		}
		break;
		default:
		{
			//DebugBreak();
			// => ���� �������� �ϴ��� �ٸ� �׷쿡�� ó���� �޽����� �̹� �ش� ������ �׷���
			// �޽��� ť�� ���� ��� �̸� ������ ���־�� ��.
			JBuffer* msg = AllocSerialBuff();
			//recvData.Dequeue(msg->GetBeginBufferPtr(), recvData.GetUseSize());
			msg->Enqueue(recvData.GetDequeueBufferPtr(), recvData.GetUseSize());
			recvData.DirectMoveDequeueOffset(recvData.GetUseSize());
			ForwardSessionMessage(sessionID, msg);
		}
		break;
		}
	}
}

#if defined(TOKEN_CHECK)
bool GatewayThread::TokenCheck(JBuffer& tokenBuffer)
{
	uint16 accountNo;
	int len;
	WCHAR Tokens[TOKEN_LEN] = { NULL, };
	
	tokenBuffer >> accountNo;
	tokenBuffer >> len;
	tokenBuffer.Dequeue((BYTE*)Tokens, len * sizeof(WCHAR));
	wstring wtoken(Tokens);
	string token;
	token.assign(wtoken.begin(), wtoken.end());

	return m_RedisConn->get(to_string(accountNo), token);
}
#endif