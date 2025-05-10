#include "../RPCHandlerComponent.h"

bool URPCHandlerComponent::OnReq_ChangeColor_Validate(FRPCPacketWrapper inPacketWrapper)
{
	return true;
}

void URPCHandlerComponent::OnReq_ChangeColor(FRPCPacketWrapper inPacketWrapper)
{
	// 스택에 할당한다.
	FRPCPacketReq_ChangeColor reqPacket;
	FMemoryReader reader(inPacketWrapper.Payload, true);
	reader.Seek(0);
	reqPacket.SerializePacket(reader); // Reader에서 데이터 읽어 Packet 채우기

	int32 playerNumber = reqPacket.PlayerNumber;
	FGuid serialNumber = reqPacket.SerialNumber;
	uint64 timeStamp = reqPacket.TimeStamp;

	FRPCPacketRes_ChangeColor resPacket;
	resPacket.ResponseCode = 0;
	resPacket.SerialNumber = serialNumber;
	resPacket.TimeStamp = timeStamp;

	FRPCPacketWrapper resPacketWrapper;
	resPacketWrapper.PacketType = EPacketType::ChangeColor_Response;
	resPacketWrapper.SerializePacket(resPacket);

	// 응답한다.
	Client_ReceivePacket(resPacketWrapper);
}