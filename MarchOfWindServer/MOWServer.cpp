#include "MOWServer.h"

void MOWServer::OnClientJoin(SessionID64 sessionID, const SOCKADDR_IN& clientSockAddr)
{
	cout << "OnClientJoin, sessionID: " << sessionID << endl;
	EnterSessionGroup(sessionID, GATEWAY_GROUP);
}

void MOWServer::OnClientLeave(SessionID64 sessionID)
{
	cout << "OnClientLeave, sessionID: " << sessionID << endl;
	LeaveSessionGroup(sessionID);
}
