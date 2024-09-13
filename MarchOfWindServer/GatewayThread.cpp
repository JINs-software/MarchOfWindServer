#include "GatewayThread.h"
#include "Group.h"
#include "HubThread.h"
#include "BattleThread.h"

#include "Protocol.h"

void GatewayThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type) {
		case MOW_HUB::C2S_CONNECTION:
		{
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
			// => 세션 포워딩을 하더라도 다른 그룹에서 처리될 메시지가 이미 해당 스레드 그룹의
			// 메시지 큐에 들어올 경우 이를 포워딩 해주어야 함.
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
