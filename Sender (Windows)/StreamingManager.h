
#ifndef _STREAMINGMANAGER_H_
#define _STREAMINGMANAGER_H_

#include "CommonTypes.h"
#include <DirectXTex.h>
#include <iostream>
#include <wincodec.h>
#include <boost/exception/diagnostic_information.hpp>

using boost::asio::ip::tcp;
using namespace DirectX;

//
// デスクトップ画像をTCP通信で送信する
//
class STREAMINGMANAGER
{
	public:
		STREAMINGMANAGER();
		~STREAMINGMANAGER();
		void SendImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Resource* resource);
		
	private:
		// メンバ
		std::string m_ClientAddr;
		int m_Port;
		tcp::endpoint m_EndPoint;
		boost::asio::io_service io_service;
		tcp::socket m_TcpSocket;
		float m_ImageQuality;
		float m_ResizeScale;
};

#endif
