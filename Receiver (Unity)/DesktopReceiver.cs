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
        // Get the listener that handles the client request.
        TcpListener listener = (TcpListener)ar.AsyncState;

        TcpClient tcpClient = listener.EndAcceptTcpClient(ar);

        NetworkStream stream = tcpClient.GetStream();
        int len;
        byte[] buffer = new byte[MAX_BUFFER_SIZE];
        while ((len = stream.Read(buffer, 0, buffer.Length)) != 0)
        {
            imgBuffer = buffer;
        }
    }

    public void UpdateTexture()
    {
        Texture2D tex = new Texture2D(2, 2); // サイズはロードした画像データで上書きされる
        bool isReadable = tex.LoadImage(imgBuffer, true);
        if (isReadable)
        {
            GetComponent<Renderer>().material.mainTexture = tex;
        }
    }
}
