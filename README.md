### \[개요\]
JNetLibrary의 JNetGroupServer를 상속받은 게임 서버이다. 다양한 그룹 스레드를 만들기 위해 로비와 매치룸이 존재하는 게임을 선택하고자 하였고, 더미보다는 소수의 인원으로 비교적 더 많은 클라이언트 세션을 테스트 할 수 있는 RTS 장르를 선택하였다. 여러 유닛을 하나의 클라이언트로 제어할 수 있기에 개별 유닛에 네트워크 세션을 붙여 행위(공격, 이동, 동기화) 패킷을 독립적으로 서버에 송신하는 방식을 사용하였다.

### \[HubThread, MatchRoomThread\]

1. 클라이언트는 게임 접속 시 초기 화면(이하 Hub 창)에 게임 서버의 IP/Port 번호를 입력하고 닉네임을 전달함. 본 서버는 계정을 관리하지 않음. 일회성의 닉네임을 사용한다.
서버와의 연결이 수립되면 닉네임 정보를 담은 패킷을 전달한다. 서버는 연결 직후의 클라이언트 패킷을 Gateway 그룹 스레드에 전달한다.
Gateway 그룹 스레드는 메시지를 식별하고 세션 정보를 Hub 그룹 스레드로 이동시키고, 닉네임 패킷을 포워딩한다.

<p align="center">
  <img src="https://github.com/user-attachments/assets/089dc6c0-fb03-42b0-a484-c2ab4a1a29b2" alt="connect" width="45%">
  <img src="https://github.com/user-attachments/assets/6cb9eca4-d859-47e5-9027-c1b205062065" alt="접속" width="45%">
</p>


2. 클라이언트가 매치룸 생성 요청 패킷을 송신, 허브 그룹에선 매치룸 중복과 매치룸 생성 제한을 확인하여 생성 가능 시 매치룸 그룹을 생성하고 세션을 전달하며, 성공 응답 패킷을 반환(매치룸 그룹 스레드)한다.

<p align="center">
  <img src="https://github.com/user-attachments/assets/e9615fc6-fcf4-4402-95b1-7775f960e213" alt="createRoom" width="45%">
  <img src="https://github.com/user-attachments/assets/e15c604f-6273-493d-94ad-aa87aa06aed2" alt="방생성" width="45%">
</p>


3. 초기 화면에서 'Join a Match'를 클릭하면 로비로 이동하게 되고 기존에 생성된 매치룸 리스트를 확인할 수 있다. 접속하고자 하는 매치룸을 클릭하면 입장 패킷을 서버에 송신한다.
서버의 허브 그룹은 매칭룸 ID를 통한 유효성 확인을 하며, 매치룸 인원을 확인하여 입장이 가능하다면 세션 정보를 전달하면서 동시에 입장 성공 패킷으로 응답한다.

![join1](https://github.com/user-attachments/assets/8b4dced4-b1ca-4d8e-a1fa-5ef5af926cda)


4. 매치룸 내 호스트를 제외한 인원이 'READY' 상태이고, 호스트가 'Start' 버튼 클릭 시 게임 시작 요청 패킷을 서버에 전송한다.
   매치룸 그룹에서 게임 시작이 가능하다는 판단 하에 '배틀 필드' 그룹 스레드를 생성하고, 플레이어 세션 정보를 전달 및 응답 패킷을 전송한다.

<p align="center">
  <img src="https://github.com/user-attachments/assets/eeb76746-ec3b-492c-9fbb-9db52cb4ac68" alt="join2" width="45%">
  <img src="https://github.com/user-attachments/assets/5c76affc-1213-463a-a314-ce6ee1660fec" alt="방입장" width="45%">
</p>

### \[BattleThread\]

게임은 유닛들의 선택을 하고, 선택된 유닛들을 배틀 필드의 전장에서 싸워 적군의 구형 상징물을 파괴하는 것을 승리로 하는 게임이다.
건설 시스템은 없지만, RTS 류의 다중 유닛을 제어할 수 있도록 한 이유는 각 유닛 생성 시 서버와의 접속을 수행하여 세션을 만들고, 최대 4인용 게임이지만 그 배가 되는 세션을 만들어 부하를 조금이라도 더 주며 서버를 테스트하기 위함이 있다.
따라서 유닛 생성 시 서버와의 연결이 수립되며, 연결 후 유닛 생성 메시지를 송신한다. 유닛 생성 메시지는 배틀 필드 고유의 ID와 함께 전달되어 Gateway 그룹은 이를 통해 배틀 그룹을 식별하여 해당 그룹으로 메시지를 포워딩 할 수 있다.

### \[JPS PathFinding 라이브러리 적용\]

* FathFinding Library: https://github.com/JINs-software/PathFinderLibrary

길찾기 알고리즘은 유니티의 NavMeshAgent를 활용하고 있지만, 다른 Nav 오브젝트들에 막혀 목적지 또는 추적 대상 타겟에게 이동하지 못하고 제자리 걸음을 하는 경우, 일정 시간 또는 횟수가 지나면 서버에 JPS 길찾기 요청을 송신한다. 서버는 이 요청을 받아 JPS 알고리즘 연산을 수행하는 쓰레드(쓰레드 풀링으로 관리)를 깨워 요청의 작업을 수행하도록 한다. 작업의 결과는 우회 경로의 일부 좌표들이며, 클라이언트는 이 유닛을 바탕으로 제자리 걸음을 하던 유닛이 이동하도록 한다.

(디버그 모드로 더미 유닛들을 생성하여 타겟까지 이동 경로를 막도록 함. 기즈모를 통해 플레이어 유닛의 추적 범위와 공격 거리를 확인할 수 있다. 노란색 원이 추적 범위이고, 빨간색 원이 공격 거리이다.)
<p align="center">
  <img src="https://github.com/user-attachments/assets/fba07247-c5e6-40cc-8db5-4fadd13d04f8" alt="JPS1" width="45%">
  <img src="https://github.com/user-attachments/assets/4d23fafe-100c-4284-8f56-12ed88c08f10" alt="JPS2" width="45%">
</p>

