
#include "StreamingManager.h"

//
// コンストラクタ
//
STREAMINGMANAGER::STREAMINGMANAGER() :
	m_TcpSocket(io_service)
{
	m_ClientAddr = "192.168.43.2"; // デスクトップ画像の送信先端末のアドレス
	m_Port = 3389;
	m_EndPoint = tcp::endpoint(boost::asio::ip::address::from_string(m_ClientAddr), m_Port);
	m_ImageQuality = 0.2f;  // jpeg圧縮の画質
	m_ResizeScale = 0.2f;  // リサイズ倍率（等倍で1920x1080）


	//
	// DirectXTK 用の初期化
	// https://github.com/Microsoft/DirectXTex/wiki/WIC-I-O-Functions
	//
#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
	Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
	if (FAILED(initialize))
	{

	}
#else
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
	{

	}
#endif
}

//
// デストラクタ
//
STREAMINGMANAGER::~STREAMINGMANAGER()
{
	m_TcpSocket.close();
}

//
// 画像の送信
//
void STREAMINGMANAGER::SendImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Resource* resource)
{
	Blob blob;
	ScratchImage image;
	boost::system::error_code error;
	HRESULT hr = CaptureTexture(device, context, resource, image);

	if (SUCCEEDED(hr))
	{
		const Image* img = image.GetImage(0, 0, 0);
		assert(img);

		// リサイズ
		ScratchImage destImage;
		hr = Resize(*img, 1920 * m_ResizeScale, 1080 * m_ResizeScale, TEX_FILTER_DEFAULT, destImage);
		img = destImage.GetImage(0, 0, 0);

		// blobにメモリ上に作成したjpgの情報を格納
		hr = SaveToWICMemory(*img, WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, blob, &GUID_WICPixelFormat24bppBGR,
			[&](IPropertyBag2* props)
			{
				PROPBAG2 options[1] = { 0 };
				options[0].pstrName = L"ImageQuality";

				VARIANT varValues[1];
				varValues[0].vt = VT_R4;
				varValues[0].fltVal = m_ImageQuality;

				(void)props->Write(1, options, varValues);
			});

		// メモリ上のjpgのバイナリデータを取得
		auto p = (byte*)blob.GetBufferPointer();
		size_t size = blob.GetBufferSize();
		std::vector<byte> jpgData(p, p + size);
		OutputDebugStringA((std::to_string(size) + "bytes\n").c_str());

		//
		// TCP送信
		// 参考：https://boostjp.github.io/tips/network/tcp.html
		//
		tcp::socket m_TcpSocket(io_service);
		m_TcpSocket.connect(m_EndPoint, error);
		if (error) 
		{
			// 接続失敗
			std::string str = "connect failed: " + error.message() + "\n";
			OutputDebugStringA(str.c_str()); // boostのエラーメッセージが日本語で出るのでStringAを使用
		}
		else
		{
			// 接続成功
			boost::asio::write(m_TcpSocket, boost::asio::buffer(jpgData), error);
			if (error)
			{
				// 送信失敗
				std::string str = "send failed: " + error.message() + "\n";
				OutputDebugStringA(str.c_str());
			}
			else
			{
				// 送信成功
				OutputDebugStringA("send correct!\n");
			}
		}
	}
}