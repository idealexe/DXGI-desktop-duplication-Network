
#include "StreamingManager.h"

//
// �R���X�g���N�^
//
STREAMINGMANAGER::STREAMINGMANAGER() :
	m_TcpSocket(io_service)
{
	m_ClientAddr = "192.168.43.2"; // �f�X�N�g�b�v�摜�̑��M��[���̃A�h���X
	m_Port = 3389;
	m_EndPoint = tcp::endpoint(boost::asio::ip::address::from_string(m_ClientAddr), m_Port);
	m_ImageQuality = 0.2f;  // jpeg���k�̉掿
	m_ResizeScale = 0.2f;  // ���T�C�Y�{���i���{��1920x1080�j


	//
	// DirectXTK �p�̏�����
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
// �f�X�g���N�^
//
STREAMINGMANAGER::~STREAMINGMANAGER()
{
	m_TcpSocket.close();
}

//
// �摜�̑��M
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

		// ���T�C�Y
		ScratchImage destImage;
		hr = Resize(*img, 1920 * m_ResizeScale, 1080 * m_ResizeScale, TEX_FILTER_DEFAULT, destImage);
		img = destImage.GetImage(0, 0, 0);

		// blob�Ƀ�������ɍ쐬����jpg�̏����i�[
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

		// ���������jpg�̃o�C�i���f�[�^���擾
		auto p = (byte*)blob.GetBufferPointer();
		size_t size = blob.GetBufferSize();
		std::vector<byte> jpgData(p, p + size);
		OutputDebugStringA((std::to_string(size) + "bytes\n").c_str());

		//
		// TCP���M
		// �Q�l�Fhttps://boostjp.github.io/tips/network/tcp.html
		//
		tcp::socket m_TcpSocket(io_service);
		m_TcpSocket.connect(m_EndPoint, error);
		if (error) 
		{
			// �ڑ����s
			std::string str = "connect failed: " + error.message() + "\n";
			OutputDebugStringA(str.c_str()); // boost�̃G���[���b�Z�[�W�����{��ŏo��̂�StringA���g�p
		}
		else
		{
			// �ڑ�����
			boost::asio::write(m_TcpSocket, boost::asio::buffer(jpgData), error);
			if (error)
			{
				// ���M���s
				std::string str = "send failed: " + error.message() + "\n";
				OutputDebugStringA(str.c_str());
			}
			else
			{
				// ���M����
				OutputDebugStringA("send correct!\n");
			}
		}
	}
}