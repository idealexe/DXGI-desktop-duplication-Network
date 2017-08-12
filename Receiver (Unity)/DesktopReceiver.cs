using System;
using System.Net;
using System.Net.Sockets;
using UnityEngine;


public class DesktopReceiver : MonoBehaviour
{
    public static int port = 3389; // 一般的なリモートデスクトップのポートに合わせた
    private IPEndPoint endPoint;
    private UdpClient udpClient;
    private const int MAX_BUFFER_SIZE = 1024 * 1024; // 1MB
    private byte[] recvBuffer = new byte[MAX_BUFFER_SIZE]; // 受信した一時的なデータを格納するバッファ
    private byte[] imgBuffer = new byte[MAX_BUFFER_SIZE]; // 受信した画像データを格納するバッファ
    private static TcpListener tcpServer;


    // フレームレート設定
    void Awake()
    {
        Application.targetFrameRate = 60;
    }

    // 初期化
    void Start()
    {
        // UDPクライアントの作成
        endPoint = new IPEndPoint(IPAddress.Any, port);
        udpClient = new UdpClient(endPoint);

        // TCP
        tcpServer = new TcpListener(endPoint);
        tcpServer.Start();
    }

    // 毎フレーム行う処理
    void Update()
    {
        // UDP受信
        while (udpClient.Available > 0)
        {
            imgBuffer = udpClient.Receive(ref endPoint);
        }

        // TCP
        tcpServer.BeginAcceptTcpClient(new AsyncCallback(DoAcceptTcpClientCallback), tcpServer);


        UpdateTexture();
    }

    // 終了時
    private void OnApplicationQuit()
    {
        tcpServer.Stop();
    }

    public void DoAcceptTcpClientCallback(IAsyncResult ar)
    {
        Debug.Log("start callback");

        // Get the listener that handles the client request.
        TcpListener listener = (TcpListener)ar.AsyncState;

        TcpClient tcpClient = listener.EndAcceptTcpClient(ar);

        NetworkStream stream = tcpClient.GetStream();
        int writeOffset = 0;
        int size = 0;
        while ((size = stream.Read(recvBuffer, writeOffset, recvBuffer.Length)) != 0)
        {
            byte[] header = new byte[2];
            byte[] end = new byte[2];
            Array.Copy(recvBuffer, 0, header, 0, 2); // 一般的なJPEGの先頭はFF D8
            Array.Copy(recvBuffer, size - 2, end, 0, 2); // 一般的なJPEGの終端はFF D9
            Debug.Log(size);
            Debug.Log(BitConverter.ToString(header) + ":" + BitConverter.ToString(end));
            writeOffset = size;
            if(BitConverter.ToString(end).Equals("FF-D9"))
            {
                Array.Copy(recvBuffer, 0, imgBuffer, 0, recvBuffer.Length);
            }
        }
    }

    public void UpdateTexture()
    {
        Destroy(GetComponent<Renderer>().material.mainTexture); // 前のテクスチャを明示的に破棄してメモリ解放
        Texture2D tex = new Texture2D(2, 2); // サイズはロードした画像データで上書きされる
        bool isReadable = tex.LoadImage(imgBuffer, true);
        if (isReadable)
        {
            GetComponent<Renderer>().material.mainTexture = tex;
        }
    }
}
