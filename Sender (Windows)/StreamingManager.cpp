
#include "StreamingManager.h"

//
// �R���X�g���N�^
//
STREAMINGMANAGER::STREAMINGMANAGER()
{
	m_ClientAddr = "192.168.1.12";
	m_Port = "3389";

	// DirectXTK �p�̏�����
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
// �f�X�g���N�^
//
STREAMINGMANAGER::~STREAMINGMANAGER()
{
}

//
// �摜�̑��M
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

		// ���T�C�Y
		ScratchImage destImage;
		hr = Resize(*img, 240, 150, TEX_FILTER_DEFAULT, destImage);
		img = destImage.GetImage(0, 0, 0);

		// blob�Ƀ�������ɍ쐬����jpg�̏����i�[
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

		// ���������jpg�̃o�C�i���f�[�^���擾���đ��M
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