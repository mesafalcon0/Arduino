
#include <Ethernet.h>  // TCP 용
#include <SPI.h>

// TCP -------------------------------------------------
byte mac[] = { 0xBE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 92 };  // Arduino IP
byte server[] = { 192, 168, 0, 140 }; // Server IP
int tcp_port = 9090;
EthernetClient client;

// TCP 정보 설정
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;
boolean tcpStarted = false;
byte tcpOpcode = 0x00;
int tcpIndex = 0;

// TCP 서버로 보내는 패킷관련
#define STX 0x02
#define ETX 0x03
#define OPCODE_1 0x11
#define OPCODE_2 0x12
#define OPCODE_3 0x13

union Packet_t {
  byte data[3]; // STX(1) OPCODE(1) ETX(1)
  byte BytePacket[sizeof(data) / sizeof(byte)];
};

Packet_t packet;

// Setup -------------------------------------------------
void setup(void) {

  // 시리얼, 소프트웨어시리얼 시작
  Serial.begin(9600);
  Serial.println("Start");
  Serial.println("--");

  // TCP 설정 - 아두이노 IP, 서버 IP, 서버 Port
  setTcpInfo();

  //TCP 시작하기
  Ethernet.begin(mac, ip);
  delay(1000);
  Serial.println("Connecting...");
  clientConnect();
}

// Loop ----------------------------------------------------------------------------
void loop(void) {

  delay(500);

  // 수신대기 및 처리
  if (checkTcpReceived()) {
    sendToTcp(tcpOpcode);
  }

  //연결 확인
  if (!client.connected()) {
    Serial.println();
    Serial.println("reconnecting.");
    client.stop();
    clientConnect();
    delay(2000);
  }
}

void clientConnect(void)
{
  if (client.connect(server, tcp_port)) {  // 서버에 연결하기
    Serial.println("Connected to server");
    client.write(0xFF);  // 연결 확인용 한바이트 보내보기
  } else {
    Serial.println("connection failed");
  }
}

void setTcpInfo(void)
{
  Serial.print("Set Arduino IP (ex.192.168.0.92): ");
  recvWithEndMarker();
  showNewData();
  parseIPAddress();

  Serial.print("Set Server IP (ex.192.168.0.140): ");
  recvWithEndMarker();
  showNewData();
  parseServerAddress();

  Serial.print("Set Server Port (ex.9090): ");
  recvWithEndMarker();
  showNewData();
  parseServerPort();
}

void recvWithEndMarker() {
  byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (newData == false) {
    delay(100);

    while (Serial.available() > 0)  {
      rc = Serial.read();

      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        ndx = 0;
        newData = true;
      }
    }
  }
}

void showNewData() {
  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;
  }
}

void parseIPAddress()
{
  byte ndx = 0;
  char delim[] = ".";
  char *ptr;
  char *context = NULL;

  ptr = strtok_r(receivedChars, delim, &context);
  ip[ndx] = atoi(ptr);

  while (ptr = strtok_r(NULL, delim, &context)) {
    ndx++;
    ip[ndx] = atoi(ptr);
  }

  Serial.print("Arduino IP: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(ip[i]);
    if (i != 3) Serial.print(".");
    else Serial.println();
  }
}

void parseServerAddress()
{
  byte ndx = 0;
  char delim[] = ".";
  char *ptr;
  char *context = NULL;

  ptr = strtok_r(receivedChars, delim, &context);
  server[ndx] = atoi(ptr);

  while (ptr = strtok_r(NULL, delim, &context)) {
    ndx++;
    server[ndx] = atoi(ptr);
  }

  Serial.print("Server IP: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(server[i]);
    if (i != 3) Serial.print(".");
    else Serial.println();
  }
}

void parseServerPort()
{
  tcp_port = atoi(receivedChars);
  Serial.print("Server Port: ");
  Serial.println(tcp_port);
}

boolean checkTcpReceived(void)
{
  // 0x02, 0x01, 0x03 수신 확인부
  while (client.available() > 0)
  {
    // 수신내용 출력
    int8_t tmpData = client.read();
    Serial.print(tmpData);
    Serial.print(" ");

    if (tcpStarted == false &&  tmpData == STX) {
      Serial.println("STX");
      tcpStarted = true;
      tcpIndex = 0;
    }
    else if (tcpStarted == true && tmpData != ETX) {
      Serial.print("OPCODE: ");
      Serial.print(tmpData);
      Serial.print(" ");
      tcpOpcode = tmpData;
    }
    else if (tcpStarted == true && tmpData == ETX)
    {
      Serial.println("ETX");
      tcpStarted = false;
      if (tcpOpcode == OPCODE_1) {
        Serial.println("OPCODE_1");
        return true;
      } else if (tcpOpcode == OPCODE_2) {
        Serial.println("OPCODE_2");
        return true;
      } else if (tcpOpcode == OPCODE_3) {
        Serial.println("OPCODE_3");
        return true;
      } else {
        Serial.print("ETX OPCODE ERROR: ");
        Serial.println(tcpOpcode);
        return false;
      }
    }
    else {
      return false;
    }
  }
  return false;
}

void sendToTcp(byte data)
{

  packet.data[0] = STX;
  packet.data[1] = data;
  packet.data[2] = ETX;

  Serial.print("dataToServer: ");
  for (int i = 0; i < sizeof(packet.BytePacket); i++) {
    Serial.print(packet.BytePacket[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  client.write(packet.BytePacket, sizeof(packet.BytePacket));
}