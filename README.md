# RPCHandler

언리얼 엔진 5의 RPC 시스템 위에 클라이언트 측 비동기 요청/응답 처리 모듈을 구축하는 R&D를 진행했습니다.

주요 목표는 명확한 역할 분리였습니다. 네트워크 전송 로직은 URPCHandlerComponent(UActorComponent)가 전담하고, 클라이언트의 요청 큐 및 응답 콜백 처리는 URPCRequestManager(ULocalPlayerSubsystem)가 담당하도록 분리했습니다.

통신 방식은 커스텀 구조체와 FArchive 기반의 직렬화/역직렬화를 통해 표준화하였고, 이를 통해 협업 및 유지보수 측면의 이점을 확보하고자 합니다.
