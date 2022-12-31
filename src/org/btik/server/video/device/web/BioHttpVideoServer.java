package org.btik.server.video.device.web;


import org.btik.server.video.device.iface.DevChannel;
import org.btik.server.VideoServer;
import org.btik.server.util.ByteUtil;
import org.btik.server.video.device.task.AsyncTaskExecutor;
import org.btik.server.video.device.iface.HttpConstant;
import org.btik.server.video.device.iface.VideoChannel;

import java.io.IOException;
import java.io.InputStream;
import java.net.*;
import java.util.Arrays;
import java.util.Date;
import java.util.Enumeration;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.nio.charset.StandardCharsets;

/***
 * 以http mjpeg 合成视频流对VideoServer的实现
 *
 * */
public class BioHttpVideoServer extends Thread implements HttpConstant, VideoServer {
    private boolean runFlag = true;

    private final ExecutorService executorService = Executors.newFixedThreadPool(3, r -> new Thread(r, "client" + System.currentTimeMillis()));

    private AsyncTaskExecutor asyncTaskExecutor;

    private DevChannel devChannel;

    private int httpPort;

    private final ConcurrentHashMap<String, MJPEGVideoChannel> videoChannelMap = new ConcurrentHashMap<>();

    public void setAsyncTaskExecutor(AsyncTaskExecutor asyncTaskExecutor) {
        this.asyncTaskExecutor = asyncTaskExecutor;
    }

    public void setHttpPort(int httpPort) {
        this.httpPort = httpPort;
    }

    public void setDevChannel(DevChannel devChannel) {
        this.devChannel = devChannel;
    }

    @Override
    public synchronized void start() {
        super.start();
        System.out.println("bio video server started");
    }

    @Override
    public void run() {
        try (ServerSocket serverSocket = new ServerSocket(httpPort)) {
            byte[] uri = new byte[URI_LEN];
            //channel 是 /{sn} 的形式 目前为 12位字符
            byte[] channel = new byte[devChannel.channelIdLen() + 1];
            while (runFlag) {
                Socket client = serverSocket.accept();
                InputStream inputStream = client.getInputStream();
                client.setSoTimeout(3000);

                try {
                    if (inputStream.read(uri) < URI_LEN) {
                        asyncTaskExecutor.execute(() -> printChannel(client));
                        continue;
                    }
                    // 判断uri
                    if (!Arrays.equals(uri, HttpConstant.uri)) {
                        asyncTaskExecutor.execute(() -> printChannel(client));
                        continue;
                    }
                    if (inputStream.read(channel) < channel.length) {
                        asyncTaskExecutor.execute(() -> do404(client));
                        continue;
                    }
                    String channelStr = new String(channel);
                    System.out.println("pre open" + new Date());
                    executorService.submit(() -> doStreamOpen(client, channelStr));
                } catch (IOException e) {
                    disConnect(client, e);
                }

            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void doStreamOpen(Socket client, String channel) {
        try {
            MJPEGVideoChannel videoChannel = videoChannelMap.get(channel);
            if (null == videoChannel) {
                // 频道不存在，主播还未开启直播间
                System.err.println("channel not exists");
                do404(client);
                return;
            }
            videoChannel.joinChannel(client);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void printChannel(Socket client) {
        try {
            String html = "<!DOCTYPE html><html lang='en'>" +
            "<head>" +
                "<meta charset='UTF-8'>" +
                "<title>直播间</title>" +
                "<style>" +
                    ".videoContainer{" +
                        "display: inline-block;" +
                    "}" +
                "</style>" +
            "</head>" +
            "<body>" +
            "<div>";
            for (String s : videoChannelMap.keySet()) {
                html += "<div class='videoContainer'>" +
                    "<img src='/video"+ s.toString() +"'/>" +
                "</div>";
            }
            html += "</div>" +
            "</body>" +
            "</html>";
            String s = "HTTP/1.1 200 \r\nContent-Type: text/html\r\nContent-Length: " + html.length() + "\r\n\r\n" + html;
            System.out.println("==========");
            System.out.println(s);
            System.out.println("==========");
            client.getOutputStream().write(s.getBytes(StandardCharsets.UTF_8));
        } catch (IOException e) {
            System.err.println(e.getMessage());
        } finally {
            disConnect(client, new Exception("404"));
        }
    }

    private void do404(Socket client) {
        try {
            client.getOutputStream().write(NOT_FOUND);
        } catch (IOException e) {
            System.err.println(e.getMessage());
        } finally {
            disConnect(client, new Exception("404"));
        }

    }

    void disConnect(Socket socket, Exception e) {
        asyncTaskExecutor.execute(() -> {
            System.err.println(e.getMessage());
            try {
                System.err.println("close:" + socket.getRemoteSocketAddress());
                socket.close();
            } catch (IOException e0) {
                System.err.println(e.getMessage());
            }
        });

    }


    public void shutDown(String msg) {
        System.err.println("exit: " + msg);
        runFlag = false;
    }


    @Override
    public VideoChannel createChannel(byte[] channelId) {
        channelId[0] = HTTP_PATH_SEPARATOR;
        String channelIdPath = new String(channelId);
        System.out.println("new channel:");
        printHttpAddress(channelIdPath);
        return videoChannelMap.computeIfAbsent(channelIdPath,
                channelIdStr -> new MJPEGVideoChannel(channelIdStr, asyncTaskExecutor));
    }

    private void printHttpAddress(String channelIdPath) {
        try {

            String channelHttpAddress = "http://%s:" + httpPort + "/video" + channelIdPath + "\r\n";
            Enumeration<NetworkInterface> networkInterfaces = NetworkInterface.getNetworkInterfaces();
            while (networkInterfaces.hasMoreElements()) {
                NetworkInterface networkInterface = networkInterfaces.nextElement();
                Enumeration<InetAddress> inetAddresses = networkInterface.getInetAddresses();
                while (inetAddresses.hasMoreElements()) {
                    InetAddress obj = inetAddresses.nextElement();
                    if (!(obj instanceof Inet4Address)) {
                        continue;
                    }
                    System.out.printf(channelHttpAddress, ByteUtil.ipv42Str(obj.getAddress()));
                }
            }

        } catch (SocketException e) {
            e.printStackTrace();
        }
    }
}
