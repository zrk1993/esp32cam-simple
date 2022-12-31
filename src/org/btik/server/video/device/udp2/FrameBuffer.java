package org.btik.server.video.device.udp2;

public class FrameBuffer {
    // 2 + 4 + 2字节 2 字节的0 4字节ip 2字节端口
    long address;

    byte[] data;

    int size;

    public FrameBuffer(byte[] data) {
        this.data = data;
    }
}
