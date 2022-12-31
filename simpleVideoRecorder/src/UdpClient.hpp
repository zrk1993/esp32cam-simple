#include <IPAddress.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

class LightUDP {
   private:
    int udp_server;
    IPAddress remote_ip;
    uint32_t remote_ip_int;
    uint16_t server_port;
    uint16_t remote_port;

   public:
    LightUDP();
    void begin(IPAddress address, uint16_t port);
    void setServer(const char* host, uint16_t port);
    void send(uint8_t* buf, size_t len);
};