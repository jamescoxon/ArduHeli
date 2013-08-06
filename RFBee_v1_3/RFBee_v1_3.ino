//  Firmware for rfBee 
//  see www.seeedstudio.com for details and ordering rfBee hardware.

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: July 14, 2010
//
//  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with this program; 
//  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA



#define FIRMWAREVERSION 13 // 1.1  , version number needs to fit in byte (0~255) to be able to store it into config
//#define FACTORY_SELFTEST
//#define DEBUG 

#include <avr/sleep.h>
#include <avr/power.h>
#include <NewPing.h>
#include <PID_v1.h>

#include "debug.h"
#include "globals.h"
#include "Config.h"
#include "CCx.h"
#include "rfBeeSerial.h"

#ifdef FACTORY_SELFTEST
#include "TestIO.h"  // factory selftest
#endif

#define GDO0 2 // used for polling the RF received data

volatile int f_timer= 6;
byte testData[50];
int count = 0, n, voltage, x_axis, y_axis, z_axis, z_gyro, x_gyro;
unsigned int   packet[22] = {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int outPin = 5, z = 0, trim = 0;
int distance = 0;

#define TRIGGER_PIN  7  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     8  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

//PID
double Setpoint, Input, Output;
double SetpointTrim, InputTrim, OutputTrim;
PID altPID(&Input, &Output, &Setpoint,1,2,1, DIRECT);
PID trimPID(&InputTrim, &OutputTrim, &SetpointTrim,1,2,1, DIRECT);

void setup(){
    //pinMode(outPin, INPUT);
    pinMode(outPin, OUTPUT);
    digitalWrite(outPin,LOW);
    
    Serial.begin(9600);
    
    setUartBaudRate();
    rfBeeInit();
    
    Input = distance;
    Setpoint = 40;
    
    altPID.SetOutputLimits(1, 16);
    //turn the PID on
    altPID.SetMode(AUTOMATIC);
    
    InputTrim = z_gyro;
    SetpointTrim = 383;
    
    trimPID.SetOutputLimits(1, 16);
    //turn the PID on
    trimPID.SetMode(AUTOMATIC);
    
    Serial.println("ok - boot complete");
    
    delay(10000);
}

void loop(){
    count++;
    
    //read voltage
    voltage = analogRead(0);
    x_axis = analogRead(1);
    y_axis = analogRead(2);
    z_axis = analogRead(3);
    x_gyro = analogRead(7);
    z_gyro = analogRead(4);
    
    unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
    distance = uS / US_ROUNDTRIP_CM;

    InputTrim = z_gyro;
    trimPID.Compute();
    trim = int(OutputTrim);
    
    Input = distance;
    altPID.Compute();
    
    z = int(Output);
    
    //After 15 seconds shut everything down
    if(count > 150) {
      z = 0;
    }
    
    throttle(z);
    calculateParity();
    header();
    mainPacket(packet);
    
    //Transmit
    n=sprintf((char*)testData, "A;%d;%d;%d;%d;%d,%d,%d,%d,%d,%d", voltage, x_axis, y_axis, z_axis,distance, z, x_gyro, z_gyro, trim, count);
    transmitData(testData,37,1,0);
    
    delay(100);
    
}


void rfBeeInit(){
    DEBUGPRINT()
    
    CCx.PowerOnStartUp();
    setCCxConfig();
   
    serialMode=SERIALDATAMODE;
    
    //attachInterrupt(0, ISRVreceiveData, RISING);  //GD00 is located on pin 2, which results in INT 0

    pinMode(GDO0,INPUT);// used for polling the RF received data

}


void header(){
    //header  600 -650 600 -1250 
  
      digitalWrite(outPin,LOW);
      delayMicroseconds(600);
      
      digitalWrite(outPin,HIGH);   
      delayMicroseconds(650);
      
      digitalWrite(outPin,LOW);
      delayMicroseconds(600);
      
      digitalWrite(outPin,HIGH); 
      delayMicroseconds(1250);
}

void mainPacket(unsigned int data[]){
    for (int i = 0; i < 22; i++) {
    if(data[i] == 1){
      digitalWrite(outPin,LOW);
      delayMicroseconds(1150);
      digitalWrite(outPin,HIGH);   
      delayMicroseconds(650);
      //Serial.print("1");
      
    }
    else{
      digitalWrite(outPin,LOW);
      delayMicroseconds(600);
      digitalWrite(outPin,HIGH);    
      delayMicroseconds(600);
      //Serial.print("0");
    }
  }
  digitalWrite(outPin,HIGH);
}

void throttle(int throttle) {
  switch (throttle){
    case 0:
      packet[2] = 0; packet[3] = 0; packet[4] = 0; packet[5] = 0; packet[6] = 0; packet[7] = 1; //000001
      break;
     case 1:
      packet[2] = 0; packet[3] = 0; packet[4] = 0; packet[5] = 1; packet[6] = 0; packet[7] = 1; //000101
      break;
     case 2:
     packet[2] = 0; packet[3] = 0; packet[4] = 0; packet[5] = 1; packet[6] = 1; packet[7] = 1; //000111
      break;
     case 3:
       if(count % 2 == 0){
        packet[2] = 0; packet[3] = 0; packet[4] = 1; packet[5] = 0; packet[6] = 0; packet[7] = 1; //001001
       }
       else {
         packet[2] = 0; packet[3] = 0; packet[4] = 1; packet[5] = 0; packet[6] = 0; packet[7] = 0; //001000
       }
      break;
     case 4:
      packet[2] = 0; packet[3] = 0; packet[4] = 1; packet[5] = 1; packet[6] = 0; packet[7] = 1; //001011 - incorrect
      break;
     case 5:
      packet[2] = 0; packet[3] = 0; packet[4] = 1; packet[5] = 1; packet[6] = 0; packet[7] = 1; //001101
      break;
     case 6:
      packet[2] = 0; packet[3] = 0; packet[4] = 1; packet[5] = 1; packet[6] = 1; packet[7] = 1; //001111
      break;
     case 7:
      packet[2] = 0; packet[3] = 1; packet[4] = 0; packet[5] = 0; packet[6] = 0; packet[7] = 1; //010001
      break;
     case 8:
       if(count % 2 == 0){
        packet[2] = 0; packet[3] = 1; packet[4] = 0; packet[5] = 0; packet[6] = 1; packet[7] = 1; //010011
       }
       else {
         packet[2] = 0; packet[3] = 1; packet[4] = 0; packet[5] = 0; packet[6] = 1; packet[7] = 0; //010010
       }
      break;
     case 9:
       if(count % 2 == 0){
        packet[2] = 0; packet[3] = 1; packet[4] = 0; packet[5] = 1; packet[6] = 1; packet[7] = 1; //010111
       }
       else {
         packet[2] = 0; packet[3] = 1; packet[4] = 0; packet[5] = 1; packet[6] = 1; packet[7] = 0; //010110
       }
      break;
     case 10:
      packet[2] = 0; packet[3] = 1; packet[4] = 1; packet[5] = 0; packet[6] = 1; packet[7] = 1; //011011
      break;
     case 11:
       if(count % 2 == 0){
        packet[2] = 0; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 0; packet[7] = 1; //011101
       }
       else {
         packet[2] = 0; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 0; packet[7] = 0; //011100
       }
      break;
     case 12:
       if(count % 2 == 0){
        packet[2] = 0; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 1; packet[7] = 0; //011110
       }
       else {
         packet[2] = 0; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 0; packet[7] = 0; //011100
       }
      break;
     case 13:
      packet[2] = 0; packet[3] = 1; packet[4] = 0; packet[5] = 0; packet[6] = 0; packet[7] = 1; //010001
      break;
     case 14:
       if(count % 2 == 0){
        packet[2] = 1; packet[3] = 1; packet[4] = 1; packet[5] = 0; packet[6] = 1; packet[7] = 1; //111011
       }
       else {
         packet[2] = 1; packet[3] = 1; packet[4] = 1; packet[5] = 0; packet[6] = 1; packet[7] = 0; //111010
       }
      break;
     case 15:
       if(count % 2 == 0){
        packet[2] = 1; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 0; packet[7] = 1; //111101
       }
       else {
         packet[2] = 1; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 0; packet[7] = 0; //111100
       }
      break;
     case 16:
       if(count % 2 == 0){
        packet[2] = 1; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 1; packet[7] = 1; //111111
       }
       else {
         packet[2] = 1; packet[3] = 1; packet[4] = 1; packet[5] = 1; packet[6] = 1; packet[7] = 0; //111110
       }
      break;
  }
}

void calculateParity(){
  int parity, num_ones = 0;
  
  for (int i=0; i < 19; i++){
    if (packet[i] == 1){
      num_ones++;
    }
  }
  parity = num_ones % 2;

  packet[20] = parity;
}



