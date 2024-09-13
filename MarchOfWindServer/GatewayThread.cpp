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
