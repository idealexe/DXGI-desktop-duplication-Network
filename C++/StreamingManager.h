
#ifndef _STREAMINGMANAGER_H_
#define _STREAMINGMANAGER_H_

#include "CommonTypes.h"
#include <DirectXTex.h>
#include <iostream>
#include <wincodec.h>

using boost::asio::ip::udp;
using namespace DirectX;

//
// �f�X�N�g�b�v�摜��UDP�ʐM�ő��M����
//
class STREAMINGMANAGER
{
public:
	STREAMINGMANAGER();
	~STREAMINGMANAGER();
	void SendImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Resource* resource);


private:
	// ���\�b�h

	// �����o
	std::string m_ClientAddr;
	std::string m_Port;
};

#endif
