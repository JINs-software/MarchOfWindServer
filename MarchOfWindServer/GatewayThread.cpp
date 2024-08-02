#include "GatewayThread.h"
#include "Group.h"
#include "LobbyThread.h"
#include "BattleThread.h"

void GatewayThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type) {
		case enPacketType::REQ_SET_PLAYER_NAME:
		{
			ForwardSessionToGroup(sessionID, LOBBY_GROUP);
			JBuffer* connMsg = AllocSerialBuff();
			MSG_REQ_SET_PLAYER_NAME* body = connMsg->DirectReserve<MSG_REQ_SET_PLAYER_NAME>();
			recvData.Dequeue(reinterpret_cast<BYTE*>(body), sizeof(MSG_REQ_SET_PLAYER_NAME));
			SendMessageGroupToGroup(sessionID, connMsg);
		}
		break;
		case enPacketType::UNIT_S_CONN_BATTLE_FIELD:
		{
			MSG_UNIT_S_CONN_BATTLE_FIELD msg;
			recvData >> msg;
			ForwardSessionToGroup(sessionID, msg.fieldID);
		}
		break;
		case enPacketType::UNIT_S_CREATE_UNIT:
		{
			// 임시, 테스트 유닛이 곧바로 생성 메시지 전송
			JBuffer* fwdCrtMsg = AllocSerialBuff();
			recvData.Dequeue(fwdCrtMsg->GetEnqueueBufferPtr(), sizeof(MSG_UNIT_S_CREATE_UNIT));
			fwdCrtMsg->DirectMoveEnqueueOffset(sizeof(MSG_UNIT_S_CREATE_UNIT));
			SendMessageGroupToGroup(sessionID, fwdCrtMsg);
		}
		break;
		default:
		{
			DebugBreak();
		}
		break;
		}

	}
}
