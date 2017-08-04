
#include "StreamingManager.h"

//
// �R���X�g���N�^
//
STREAMINGMANAGER::STREAMINGMANAGER() :
	m_UdpSocket(io_service)
{
	m_ClientAddr = "192.168.1.12"; // ��M����[���̃A�h���X
	m_Port = "3389";

	// UDP�\�P�b�g���쐬
	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v4(), m_ClientAddr, m_Port);
	m_Endpoint = *resolver.resolve(query);
	m_UdpSocket.open(udp::v4());

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
	m_UdpSocket.close();
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
		hr = Resize(*img, 192, 108, TEX_FILTER_DEFAULT, destImage);
		img = destImage.GetImage(0, 0, 0);

		// blob�Ƀ�������ɍ쐬����jpg�̏���blob�Ɋi�[
		hr = SaveToWICMemory(*img, WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, blob, &GUID_WICPixelFormat24bppBGR);

		try
		{
			// ���������jpg�̃o�C�i���f�[�^���擾���đ��M
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