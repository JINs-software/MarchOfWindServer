#include "MOWServer.h"
#include "Configuration.h"
#include <conio.h>

int main() {
	MOWServer* mowServer = new MOWServer(
		MOW_SERVER_IP, MOW_SERVER_PORT, MOW_MAX_OF_CONNECTIONS,
		0, dfPACKET_CODE, dfPACKET_KEY, MOW_RECV_BUFFERING_MODE,
		MOW_MAX_OF_SESSIONS, MOW_IOCP_CONCURRENT_THREAD, MOW_NUM_OF_IOCP_WORKERS,
		MOW_TLS_MEM_POOL_UNIT_CNT, MOW_TLS_MEM_POOL_CAPACITY, MOW_TLS_MEM_POOL_REFERENCE_MODE, MOW_TLS_MEM_POOL_PLACEMENT_NEW_MODE,
		MOW_POOL_BUFFER_ALLOC_SIZE, MOW_SESSION_RECV_BUFF_SIZE
	);

	mowServer->Start();

	while (true) {
		if (_kbhit()) {
			char ctr = _getch();

			if (ctr == 'Q' || ctr == 'q') {
				break;
			}
		}
	}

	mowServer->Stop();

}