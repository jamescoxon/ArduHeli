  
int outPin = 5;
int count = 0;
int z = 0;
unsigned int fullthro[22] = {1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int middthro[22] = {1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0};
unsigned int lowthrot[22] = {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int shutDown[22] = {1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0};
unsigned int   packet[22] = {1,1s,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//lowest 101
// last bit is stop
// second to last bit is parity

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(outPin, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(outPin,LOW);
  digitalWrite(13,LOW);   
  delay(3000);
}

void loop() {
  
  if(count % 5 == 0){
    if(z < 16){
      z++;
    }
    else {
      z = 0;
    }
    Serial.print("Throttle: ");
    Serial.print(z);
    Serial.print(" ");
    throttle(z);
  }
  
  count++;
  Serial.print("Throttle: ");
  Serial.print(z);
  Serial.print(" ");
  throttle(z);
  calculateParity();
  Serial.println();
  header();
  mainPacket(packet);
  delay(500);
 

  /*
  
  //Power up for 3 seconds
  Serial.println("Full Throttle");
  calculateParity();
  header();
  mainPacket(packet);
  delay(2000);

  //Power up for 3 seconds
  Serial.println("Middle Throttle");
  header();
  mainPacket(middthro);
  delay(2000);
  
  //Power up for 3 seconds
  Serial.println("Low Throttle");
  calculateParity();
  for (int i=0; i < 22; i++){
      Serial.print(packet[i]);
    }
    Serial.println();
  header();
  mainPacket(packet);
  delay(2000);
  
  
  //Shutdown
  Serial.println("Shutdown");
  header();
  mainPacket(shutDown);
  delay(20000);
 */
}

void header(){
    //header  600 -650 600 -1250 
  
      digitalWrite(outPin,LOW);
      digitalWrite(13,LOW);
      delayMicroseconds(600);
      
      digitalWrite(outPin,HIGH);
      digitalWrite(13,HIGH);    
      delayMicroseconds(650);
      
      digitalWrite(outPin,LOW);
      digitalWrite(13,LOW);
      delayMicroseconds(600);
      
      digitalWrite(outPin,HIGH);
      digitalWrite(13,HIGH);    
      delayMicroseconds(1250);
}

void mainPacket(unsigned int data[]){
    for (int i = 0; i < 22; i++) {
    if(data[i] == 1){
      digitalWrite(outPin,LOW);
      digitalWrite(13,LOW);
      delayMicroseconds(1150);
      digitalWrite(outPin,HIGH);
      digitalWrite(13,HIGH);    
      delayMicroseconds(650);
      //Serial.print("1");
      
    }
    else{
      digitalWrite(outPin,LOW);
      digitalWrite(13,LOW);
      delayMicroseconds(600);
      digitalWrite(outPin,HIGH);
      digitalWrite(13,HIGH);    
      delayMicroseconds(600);
      //Serial.print("0");
    }
  }
  digitalWrite(outPin,HIGH);
  digitalWrite(13,HIGH); 
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
      packet[2] = 0; packet[3] = 0; packet[4] = 1; packet[5] = 0; packet[6] = 1; packet[7] = 1; //001011
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
  Serial.print(",");
  Serial.print(num_ones);
  Serial.print(",");
  Serial.println(parity);
  packet[20] = parity;
  
   for (int i=0; i < 22; i++){
      Serial.print(packet[i]);
    }
}
