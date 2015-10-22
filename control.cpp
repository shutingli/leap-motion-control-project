/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include "Leap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include "send.h"
#include "conio.h"

using namespace std;
using namespace Leap;

#define BUFFSIZE 102
#define PORTNUM 4578

#define ROTSENS 2

class SampleListener : public Listener {
  public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);

  private:
};

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};
bool singleFlag;
int machineFlag;
int sCounter;
double scale = 1.0;

void SampleListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
  controller.enableGesture(Gesture::TYPE_CIRCLE);
  controller.enableGesture(Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}


void sendTcpMessage(const string msg){

  int TCPsock;
 
  
  std::string x = msg;
  char buffer[x.size()];
  std::strcpy(buffer, x.c_str());

  struct sockaddr_in TCPserver;
  //init socket
        
    if((TCPsock=socket(AF_INET, SOCK_STREAM, 0))<0) 
        {
            cout<<"Failed to create TCP Socket"<<endl;
           
        } 

  TCPserver.sin_family= AF_INET;
  if (machineFlag == 1)
  {
    TCPserver.sin_addr.s_addr= inet_addr("141.142.212.5");
  }
  else if (machineFlag == 2)
  {
    TCPserver.sin_addr.s_addr= inet_addr("141.142.21.63");
  }
  else
  {
    TCPserver.sin_addr.s_addr= inet_addr("141.142.21.64");
  }
  TCPserver.sin_port = htons(4578); 


  //connecting socket server
  if(connect(TCPsock,(struct sockaddr*) &TCPserver, sizeof(TCPserver))!=0){
  std::cout<<"Failed to connect UDP Socket"<<endl;
  }
  send(TCPsock, buffer, x.size(),0);
  
  close(TCPsock);
  
  

}
void sendUdpMessage(const string msg){

  int UDPsock;
 
  
  std::string x = msg;
  char buffer[x.size()];
  std::strcpy(buffer, x.c_str());


  struct sockaddr_in UDPserver;


  //init socket
    if((UDPsock=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0) 
    {
        cerr<<"Failed to create UDP Socket"<<endl;
       
    }

  // set up the server addr
  UDPserver.sin_family= AF_INET;
  UDPserver.sin_addr.s_addr= inet_addr("141.142.21.63");
  UDPserver.sin_port = htons(4578); 
  

  //connecting socket server
  if (connect(UDPsock,(struct sockaddr*) &UDPserver, sizeof(UDPserver))!=0){
     std::cout<<"Failed to connect UDP Socket"<<endl;
  }
  
  send(UDPsock, buffer, x.size() ,0);

  close(UDPsock);
  

}

void sendStopMessage(){
  sendUdpMessage("NAVIGATION : 0 0 0 0 81 0");
  sendTcpMessage("VIRDIR COMMAND : stop");
}

void getKey(char a, bool singleFlag){
  if(singleFlag){
    //change char to string 
    stringstream ss;
    string s;
    ss << a;
    ss >> s;
    //read the file
    ifstream file;
    file.open ("commands.txt");
    string line,left,right;
    while (getline(file,line))
    {   
        istringstream iss(line);
        iss >> left;
        if (left == s)
        {
          right = line.substr(2);
          size_t found = right.find("scale");
          if (found!=std::string::npos)
          {
            found = right.find("Down");
            right = right.substr(9);
            if (found!=std::string::npos)
            {
              scale = scale/std::stod(right); 
              right = "scale " + std::to_string(scale);
            }
            else
           {   
              scale = scale*std::stod(right);
              right = "scale " + std::to_string(scale); 
            }           
          }                   
          string command = "VIRDIR COMMAND : ";
          sendTcpMessage(command+ right);
          std::cout << " "<< right << std::endl;
          break;         
        }
    }

    file.close();
  
  }

}

void keepListen(){
  while(1){
  // get the single key stroke
  if (singleFlag){
  char singleKey = getche();
  if (singleKey != 'q')
     getKey(singleKey, singleFlag);
  else
    singleFlag = false;
  }
  else{
  // get the command after enter
  string myKey;
  string command = "VIRDIR COMMAND : ";
  std::cin >> myKey;
    if (myKey != "q")
    {
      //std::cout << "Sending Message: "<<command+ myKey << std::endl; 
      sendTcpMessage(command+ myKey);
    }
    else
    {
      singleFlag = true;
    }
  }
  
 
  }// while loop ends
}

void SampleListener::onFrame(const Controller& controller) {
  // Get the most recent frame and report some basic information
  const Frame frame = controller.frame();

  HandList hands = frame.hands();

  
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    // Get the first hand
    const Hand hand = *hl;
    
    std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
    int count =0;
    if(count == 3){
      cout<<'\b'; 
      cout<<'\b';
      cout<<'\b';
      count = 0;
    }
    else{
      cout<<"."; 
      count++;
    }
    
    const Vector normal = hand.palmNormal();
    const Vector direction = hand.direction();
    const Vector palmPosition = hand.palmPosition();
    std::stringstream ss;
    ss << "NAVIGATION : "<<palmPosition[0]<<" "
    << (palmPosition[1]-280)<<" "<<(1.1*palmPosition[2])<<" "
    << (direction[1]*180/3.1415 * ROTSENS)<<" "<< (81 + ROTSENS*normal.roll() * RAD_TO_DEG)
    <<" "<<(-direction.yaw() * RAD_TO_DEG * ROTSENS );
     //   ss << "NAVIGATION : "<<" 0 0 0 0 81 "<<direction.yaw() * RAD_TO_DEG;

    std::string msg = ss.str();
    
    sendUdpMessage(msg);
    //Print flashing message to indicate detecting status
    if (sCounter == 0) {
    std::cout << "."; 
    std::cout << '\b' << std::flush;}
    else{
      std::cout << "."; 
      std::cout << '\b';
      std::cout << '\b' << std::flush;
    }
    if (sCounter % 3 == 0){
 
      std::cout << "....";
      std::cout << '\b';
      std::cout << '\b';
      std::cout << '\b';
      std::cout << '\b';
      std::cout << '\b'<< std::flush; 
        }
   
    if (sCounter % 6 == 0){
 
      std::cout << "      ";
      std::cout << '\b';
      std::cout << '\b';
      std::cout << '\b';
      std::cout << '\b';
      std::cout << '\b';
      std::cout << '\b'<< std::flush; 
        }
     sCounter++;

    // Get fingers
    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Finger finger = *fl;
      

      // Get finger bones
      for (int b = 0; b < 4; ++b) {
        Bone::Type boneType = static_cast<Bone::Type>(b);
        Bone bone = finger.bone(boneType);
        // std::cout << std::string(6, ' ') <<  boneNames[boneType]
        //           << " bone, start: " << bone.prevJoint()
        //           << ", end: " << bone.nextJoint()
        //           << ", direction: " << bone.direction() << std::endl;
      }
    }
  }

  // Get tools
  const ToolList tools = frame.tools();
  for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
    const Tool tool = *tl;
    // std::cout << std::string(2, ' ') <<  "Tool, id: " << tool.id()
    //           << ", position: " << tool.tipPosition()
    //           << ", direction: " << tool.direction() << std::endl;
  }

  // Get gestures
  const GestureList gestures = frame.gestures();
  for (int g = 0; g < gestures.count(); ++g) {
    Gesture gesture = gestures[g];

    switch (gesture.type()) {
      case Gesture::TYPE_CIRCLE:
      {
        CircleGesture circle = gesture;
        std::string clockwiseness;

        if (circle.pointable().direction().angleTo(circle.normal()) <= PI/2) {
          clockwiseness = "clockwise";
        } else {
          clockwiseness = "counterclockwise";
        }

        // Calculate angle swept since last frame
        float sweptAngle = 0;
        if (circle.state() != Gesture::STATE_START) {
          CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
          sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
        }

        break;
      }
      case Gesture::TYPE_SWIPE:
      {
        SwipeGesture swipe = gesture;
        // std::cout << std::string(2, ' ')
        //   << "Swipe id: " << gesture.id()
        //   << ", state: " << stateNames[gesture.state()]
        //   << ", direction: " << swipe.direction()
        //   << ", speed: " << swipe.speed() << std::endl;
        break;
      }
      case Gesture::TYPE_KEY_TAP:
      {
        KeyTapGesture tap = gesture;
/*        std::cout << std::string(2, ' ')
          << "Key Tap id: " << gesture.id()
          << ", state: " << stateNames[gesture.state()]
          << ", position: " << tap.position()
          << ", direction: " << tap.direction()<< std::endl;*/
        break;
      }
      case Gesture::TYPE_SCREEN_TAP:
      {
        ScreenTapGesture screentap = gesture;
        // std::cout << std::string(2, ' ')
        //   << "Screen Tap id: " << gesture.id()
        //   << ", state: " << stateNames[gesture.state()]
        //   << ", position: " << screentap.position()
        //   << ", direction: " << screentap.direction()<< std::endl;
        break;
      }
      default:
       // std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
        break;
    }
  }

  if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
   // std::cout << std::endl;
  }

}

void SampleListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
   // std::cout << "id: " << devices[i].toString() << std::endl;
    //std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void SampleListener::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}

int main(int argc, char** argv) {

  char input;
  std::cout << "Type corresponding number and press enter to choose a machine."<<std::endl;
  std::cout <<" 1.tone 2.trapeze (room 1005) 3.mangala (room 2019)"<< std::endl;
  input = getche();
  switch (input){
    case '1':
          machineFlag=0;
          break;
    case '2':
          machineFlag=1;
          break;
    case '3':
          machineFlag=2;
          break;
  }

  // Create a sample listener and controller
  SampleListener listener;
  Controller controller;
  sendStopMessage();
  
  // Have the sample listener receive events from the controller
  controller.addListener(listener);
  singleFlag = true;
  // Use a thread to listen to the key stroke
  thread mThread(keepListen);

  if (argc > 1 && strcmp(argv[1], "--bg") == 0)
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
  mThread.join();
  // Keep this process running until Enter is pressed
  std::cout << "Press Enter to quit..." << std::endl;
  //std::cin.get();

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
