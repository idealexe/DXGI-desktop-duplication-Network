
#include "StreamingManager.h"

//
// コンストラクタ
//
STREAMINGMANAGER::STREAMINGMANAGER()
{
	m_ClientAddr = "192.168.1.12";
	m_Port = "3389";

	// DirectXTK 用の初期化
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
}

//
// 画像の送信
//
void STREAMINGMANAGER::SendImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Resource* resource)
{
	Blob blob;
	ScratchImage image;
	HRESULT hr = CaptureTexture(device, context, resource, image);


	if (SUCCEEDED(hr))
	{
		const Image* img = image.GetImage(0, 0, 0);
		assert(img);

		// リサイズ
		ScratchImage destImage;
		hr = Resize(*img, 240, 150, TEX_FILTER_DEFAULT, destImage);
		img = destImage.GetImage(0, 0, 0);

		// blobにメモリ上に作成したjpgの情報を格納
		hr = SaveToWICMemory(*img, WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, blob, &GUID_WICPixelFormat24bppBGR);
	}

	try
	{
		boost::asio::io_service io_service;
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), m_ClientAddr, m_Port);
		udp::endpoint m_Receiver_endpoint = *resolver.resolve(query);
		udp::socket m_Socket(io_service);
		m_Socket.open(udp::v4());

		// メモリ上のjpgのバイナリデータを取得して送信
		auto p = (byte*)blob.GetBufferPointer();
		auto size = blob.GetBufferSize();
		std::vector<byte> jpgData(p, p+size);
		m_Socket.send_to(boost::asio::buffer(jpgData), m_Receiver_endpoint);
	}
	catch (std::exception e)
	{
		std::cerr << e.what() << std::endl;
	}
}