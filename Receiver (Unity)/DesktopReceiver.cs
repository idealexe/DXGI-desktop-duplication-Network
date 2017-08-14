using System;
using System.Net;
using System.Net.Sockets;
using UnityEngine;


public class StateObject
{
    public NetworkStream stream;
    public byte[] buffer = new byte[1024 * 1024];
    public byte[] imgBuffer = new byte[1024 * 1024];
    public int writeOffset = 0;
}


public class DesktopReceiver : MonoBehaviour
{
    public static int port = 3389; // 一般的なリモートデスクトップのポートに合わせた
    private IPEndPoint endPoint;
    private UdpClient udpClient;
    private const int MAX_BUFFER_SIZE = 1024 * 1024; // 1MB
    private byte[] imgBuffer = new byte[MAX_BUFFER_SIZE]; // 受信した画像データを格納するバッファ
    private byte[] udpBuffer = new byte[1024];
    private static TcpListener tcpServer;
    private bool isReceiving = false;


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

        // テクスチャ更新
        UpdateTexture();
    }


    // 毎フレーム行う処理
    void Update()
    {
        // UDP受信
        while (udpClient.Available > 0)
        {
            udpBuffer = udpClient.Receive(ref endPoint);
        }

        // TCP
        if (!isReceiving)
        {
            tcpServer.BeginAcceptTcpClient(new AsyncCallback(DoAcceptTcpClientCallback), tcpServer); // 非同期で接続待機を開始
            isReceiving = true;
        }

        UpdateTexture();
    }


    // TCPクライアント接続時の処理
    public void DoAcceptTcpClientCallback(IAsyncResult ar)
    {
        Debug.Log("Start Accept Callback");

        TcpListener listener = (TcpListener)ar.AsyncState;
        TcpClient tcpClient = listener.EndAcceptTcpClient(ar);

        StateObject state = new StateObject();
        NetworkStream stream = tcpClient.GetStream();

        state.stream = stream;
        stream.BeginRead(state.buffer, 0, MAX_BUFFER_SIZE, new AsyncCallback(DoReceiveCallback), state);
    }


    // 受信処理
    public void DoReceiveCallback(IAsyncResult ar)
    {
        Debug.Log("Start Receive Callback");

        StateObject state = (StateObject)ar.AsyncState;
        NetworkStream stream = state.stream;
        int size = stream.EndRead(ar); // 読み込んだサイズを取得

        if (0 < size)
        {
            Debug.Log(size + " bytes");
            Array.Copy(state.buffer, 0, state.imgBuffer, state.writeOffset, size); // state.imgBufferに追加していく
            Debug.Log("writeOffset: " + state.writeOffset);
            state.writeOffset += size;
            // 読み込むデータが無くなるまで再帰的に読み込む
            stream.BeginRead(state.buffer, 0, MAX_BUFFER_SIZE, new AsyncCallback(DoReceiveCallback), state);
        }
        else
        {
            Array.Copy(state.imgBuffer, imgBuffer, state.imgBuffer.Length);
            Debug.Log("receive done!");
            stream.Close();
            isReceiving = false;
        }
    }


    // テクスチャ更新
    public void UpdateTexture()
    {
        Destroy(GetComponent<Renderer>().material.mainTexture); // 前のテクスチャを明示的に破棄してメモリ解放
        Texture2D tex = new Texture2D(2, 2); // サイズはロードした画像データで上書きされるので適当
        bool isReadable = tex.LoadImage(imgBuffer, true);
        if (isReadable)
        {
            GetComponent<Renderer>().material.mainTexture = tex;
        }
    }


    // バッファの両端を表示
    public void printBuffer(byte[] buffer, int size)
    {
        byte[] SOI = new byte[2]; // Start of Image
        byte[] EOI = new byte[2]; // End of Image

        Array.Copy(buffer, 0, SOI, 0, 2); // 一般的なJPEGの先頭はFF D8
        Array.Copy(buffer, size - 2, EOI, 0, 2); // 一般的なJPEGの終端はFF D9
        Debug.Log(BitConverter.ToString(SOI) + ":" + BitConverter.ToString(EOI));
    }


    // 終了時
    private void OnApplicationQuit()
    {
        udpClient.Close();
        tcpServer.Stop();
    }
}
