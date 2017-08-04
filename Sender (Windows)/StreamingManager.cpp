
#include "StreamingManager.h"

//
// コンストラクタ
//
STREAMINGMANAGER::STREAMINGMANAGER() :
	m_UdpSocket(io_service)
{
	m_ClientAddr = "192.168.1.12"; // 受信する端末のアドレス
	m_Port = "3389";

	// UDPソケットを作成
	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v4(), m_ClientAddr, m_Port);
	m_Endpoint = *resolver.resolve(query);
	m_UdpSocket.open(udp::v4());

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
	m_UdpSocket.close();
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
		hr = Resize(*img, 192, 108, TEX_FILTER_DEFAULT, destImage);
		img = destImage.GetImage(0, 0, 0);

		// blobにメモリ上に作成したjpgの情報をblobに格納
		hr = SaveToWICMemory(*img, WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, blob, &GUID_WICPixelFormat24bppBGR);

		try
		{
			// メモリ上のjpgのバイナリデータを取得して送信
			auto p = (byte*)blob.GetBufferPointer();
			auto size = blob.GetBufferSize();
			std::vector<byte> jpgData(p, p + size);
			m_UdpSocket.send_to(boost::asio::buffer(jpgData), m_Endpoint);
		}
		catch (std::exception e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
}