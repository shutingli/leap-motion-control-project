#ifndef SEND_H
#define SEND_H

void sendUdpMessage(const std::string msg);
void sendTcpMessage(const std::string msg);
void sendStopMessage();
void getKey();
#endif

