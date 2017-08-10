
#ifndef _STREAMINGMANAGER_H_
#define _STREAMINGMANAGER_H_

#include "CommonTypes.h"
#include <DirectXTex.h>
#include <iostream>
#include <wincodec.h>
#include <boost/exception/diagnostic_information.hpp>

using boost::asio::ip::udp;
using boost::asio::ip::tcp;
using namespace DirectX;

//
// デスクトップ画像をUDP通信で送信する
//
class STREAMINGMANAGER
{

public:
	STREAMINGMANAGER();
	~STREAMINGMANAGER();
	void SendImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Resource* resource);


private:
	// メソッド

	// メンバ
	std::string m_ClientAddr;
	std::string m_Port;
	boost::asio::io_service io_service;
	udp::endpoint m_Endpoint;
	udp::socket m_UdpSocket;
	tcp::socket m_TcpSocket;
};

#endif
