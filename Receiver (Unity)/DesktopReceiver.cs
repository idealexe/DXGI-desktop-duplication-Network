using System.Net;
using System.Net.Sockets;
using UnityEngine;

public class DesktopReceiver : MonoBehaviour {
    public static int port = 3389; // 一般的なリモートデスクトップのポートに合わせた
    private UdpClient udpClient;
    private IPEndPoint endPoint;
    private const int MAX_BUFFER_SIZE = 1024 * 1024 * 8; // 8MB
    private byte[] imgBuffer = new byte[MAX_BUFFER_SIZE]; // UDPで受信した画像データを格納する

    // 初期化
    void Start()
    {
        // UDPクライアントの作成
        endPoint = new IPEndPoint(IPAddress.Any, port);
        udpClient = new UdpClient(endPoint);
    }

    // 毎フレーム行う処理
    void Update () {

        while (udpClient.Available > 0)
        {
            imgBuffer = udpClient.Receive(ref endPoint);
            Texture2D tex = new Texture2D(1920, 1200);
            tex.LoadImage(imgBuffer, true);
            GetComponent<Renderer>().material.mainTexture = tex;
        }
    }

}
