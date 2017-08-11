
#include "StreamingManager.h"

//
// コンストラクタ
//
STREAMINGMANAGER::STREAMINGMANAGER() :
	m_UdpSocket(io_service),
	m_TcpSocket(io_service)
{
	m_ClientAddr = "192.168.1.5"; // 受信する端末のアドレス
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
		int scale = 4;
		hr = Resize(*img, 160*scale, 90*scale, TEX_FILTER_DEFAULT, destImage);
		img = destImage.GetImage(0, 0, 0);

		// blobにメモリ上に作成したjpgの情報をblobに格納
		hr = SaveToWICMemory(*img, WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, blob, &GUID_WICPixelFormat24bppBGR);

		// メモリ上のjpgのバイナリデータを取得
		auto p = (byte*)blob.GetBufferPointer();
		size_t size = blob.GetBufferSize();
		std::vector<byte> jpgData(p, p + size);


		// UDP送信
		if (m_UdpSocket.available() > 0)
		{
			//m_UdpSocket.send_to(boost::asio::buffer(jpgData), m_Endpoint);
		}

		// TCP送信
		tcp::socket m_TcpSocket(io_service);

		m_TcpSocket.connect(tcp::endpoint(boost::asio::ip::address::from_string(m_ClientAddr), 3389), error);

		boost::asio::write(m_TcpSocket, boost::asio::buffer(jpgData), error);
		if (error) {
			std::cout << "send failed: " << error.message() << std::endl;
			std::string s = "send failed: " + error.message() + "\n";
			OutputDebugStringA(s.c_str());
		}
		else {
			std::cout << "send correct!" << std::endl;
		}
	}
}