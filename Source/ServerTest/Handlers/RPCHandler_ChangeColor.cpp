#include "../RPCHandlerComponent.h"

bool URPCHandlerComponent::OnReq_ChangeColor_Validate(FRPCPacketWrapper inPacketWrapper)
{
	return true;
}

void URPCHandlerComponent::OnReq_ChangeColor(FRPCPacketWrapper inPacketWrapper)
{
	// ���ÿ� �Ҵ��Ѵ�.
	FRPCPacketReq_ChangeColor reqPacket;
	FMemoryReader reader(inPacketWrapper.Payload, true);
	reader.Seek(0);
	reqPacket.SerializePacket(reader); // Reader���� ������ �о� Packet ä���

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

	// �����Ѵ�.
	Client_ReceivePacket(resPacketWrapper);
}