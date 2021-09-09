
#include <ESP8266WiFi.h> //와이파이와 http를 사용하기 위한 헤더
#include <ESP8266HTTPClient.h>

#define BLYNK_PRINT Serial //blynk와 그 위젯들을 사용하기 위한 헤더
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
// blynk를 사용하기 위한 토큰
char auth[] = "SMN3GS942Gdo6m_fb2gDKEe-9V7y9R_M";

// Your WiFi credentials.
// Set password to "" for open networks.
// 네트워크 이름과 비밀번호
char ssid[] = "dlink-550a-z";
char pass[] = "10240506";

// 기상청 RSS주소
const String endpoint = "http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=2671025000";

// 날씨 저장 변수
char pty0;
//현재 시각을 LCD에 출력하기위한 변수
String currentTime;
String currentDate;

int counter;  //날씨알람 보낼 시간인지 확인

int timeStart ; 

int pinV2;

char serialTest;
int testOn = 0;

float disToWin;

int numTones = 78; //부저 음악 출력_슈퍼마리오
int tones[] = {
  2637, 2637, 0, 2637,
  0, 2093, 2637, 0,
  3136, 0, 0,  0,
  1568, 0, 0, 0,

  2093, 0, 0, 1568,
  0, 0, 1319, 0,
  0, 1760, 0, 1976,
  0, 1865, 1760, 0,

  1568, 2637, 3136,
  3520, 0, 2794, 3136,
  0, 2637, 0, 2093,
  2349, 1976, 0, 0,

  2093, 0, 0, 1568,
  0, 0, 1319, 0,
  0, 1760, 0, 1976,
  0, 1865, 1760, 0,

  1568, 2637, 3136,
  3520, 0, 2794, 3136,
  0, 2637, 0, 2093,
  2349, 1976, 0, 0
};

String line = "";

WidgetTerminal terminal(V0);

BlynkTimer timer;

WidgetRTC rtc;




void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to the WiFi network");

  Blynk.begin(auth, ssid, pass);

  // terminal 화면을 clr
  terminal.clear();

  setSyncInterval(10 * 60); // 10 분마다 싱크 다시 맞춰줌

  // 1초마다 디스플레이에 표시_시간_초
  timer.setInterval(1000L, clockDisplay);

  counter = hour() - 1; //시작하자마자 일기예보를 한번 확인해보기 위해서 다음과 같이 입력

  pinMode (D0, OUTPUT);
  pinMode (D1, OUTPUT);
  pinMode (D2, OUTPUT);
  pinMode (D3, OUTPUT);
  pinMode (D4, INPUT);
  pinMode (D5, OUTPUT);

}

void loop() {
  Blynk.run();
  timer.run();

  rainChecker();

  Test();

}
// 부저에서 음악이 나오게 해줌
void buzzer() { 
  for (int i = 0; i < numTones; i++) {
    tone(D5, tones[i]);
    delay(200);
  }
  noTone(D5);
}
// 설계한 프로그램을 테스트하기 위해서 만듦
void Test() {
  serialTest = Serial.read();
  if (serialTest == 't') {
    Serial.println("TEST Start");
    testOn = 1;
    rainChecker();
    testOn = 0;
    Serial.println("TEST Fin");

  }
}
//이름대로 비 예보가 있는지 확인
void rainChecker() {
  int Ho;
  if (testOn == 1) {
    Ho = 10;
    Serial.println("Test rainChecker");
  }
  else {
    Ho = hour();
  }

  if (counter != Ho) {
    get_weather();
    if (testOn == 1) {
      pty0 = 1;
    }
    Serial.println("*******  Time Checked  *******");
    for (int i = 10; i < 23; i += 3)
    {
      if (i == Ho) {
        rainNotice();
        Serial.println("*******  Rain Noticed  *******");
      }
    }
    counter = Ho;
    Serial.println("*******  Rain Checked  *******");
  }

}
// RSS로 날씨를 받아옴
void get_weather() {
  if ((WiFi.status() == WL_CONNECTED)) { 
    Serial.println("Starting connection to server...");
    HTTPClient http;
    http.begin(endpoint); //기상청 RSS주소에 연결한다
    int httpCode = http.GET();
    if (httpCode > 0) { //요청 성공
      line = http.getString();
    }
    else {// 실패하면 에러가 발생했다고 출력
      Serial.println("Error on HTTP request"); 
    }
    parsing(); //이 함수를 이용해서 터미널에 일기예보를 출력
    http.end(); //Free the resources
  }
}

// LCD에 시간을 표시한다.
void clockDisplay()
{

  currentTime = String(hour()) + ":" + minute() + ":" + second();
  currentDate = String(year()) + " " + month() + " " + day();

  // 가상핀을 이용해 앱으로 시간을 보냄
  Blynk.virtualWrite(V3, currentTime);
  // 가상핀을 이용해 앱으로 날짜를 보냄
  Blynk.virtualWrite(V4, currentDate);
}

// 블라인드를 올리거나 내리는 것을 제어
void blindMove() {
  timeStart = millis();

  if (pinV2 == 1) {
    digitalWrite (D0, HIGH);
    digitalWrite (D1, LOW);

    Serial.println("A");
    digitalWrite (D2, HIGH);
  }

  else {
    digitalWrite (D0, LOW);
    digitalWrite (D1, HIGH);

    Serial.println("B");
    digitalWrite (D2, HIGH);
  }

  int timeStoE = 0;
  while (timeStoE < 5000) { //블라인드가 동작하는 시간을 정해주고 1초마다 정해진 시간동안 동작하였는지 확인
    timeStoE = millis() - timeStart;
    Serial.println("C");
    Serial.println(timeStoE);
    delay(1000);
  }
  digitalWrite (D2, LOW); //동작 후 정지

}


BLYNK_CONNECTED() {
  rtc.begin();
}

BLYNK_WRITE (V1) { //날씨에 따른 안내 가상핀 V1은 날짜정보 불러오기 버튼에 연결

  int pinValue = param.asInt ();
  if (pinValue == 1) {
    get_weather();
    terminal.flush();

    if (pty0 != '0') { //일기 예보에 따라 문구를 출력
      terminal.print(F("\n날씨가 흐리니 창문을 닫아주세요."));
    }
    else {
      terminal.print(F("\n날씨가 맑으니 환기하세요."));
    }

    int winCh = winCheck(); //창문이 열려 있는지 닫혀 있는지 확인가능
    if (winCh == 0) {
      terminal.print(F("\n창문이 닫혀 있습니다."));
    }
    else {
      terminal.print(F("\n창문이 열려 있습니다."));
    }

    terminal.flush();
  }
}

BLYNK_WRITE (V2) { //가상핀 2번은 블라인드 제어 버튼들에 연결
  pinV2 = param.asInt (); // V2에서 들어오는 값을 변수에 할당
  blindMove();
}

BLYNK_WRITE (V5) { //가상핀 5번은 창문거리 셋팅 버튼에 연결
  int pinV5 = param.asInt ();
  if (pinV5 == 1) {
    disToWin = dist();
  }
}

// 초음파 센서를 통해서 거리를 측정
float dist() {
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
  delayMicroseconds(2);
  digitalWrite(D3, HIGH);
  delayMicroseconds(10);
  digitalWrite(D3, LOW);

  // echoPin 이 HIGH를 유지한 시간을 저장한다.
  unsigned long duration = pulseIn(D4, HIGH);
  // HIGH 였을 때 시간(초음파가 보냈다가 다시 들어온 시간)을 가지고 거리를 계산 한다.
  float distance = ((float)(340 * duration) / 10000) / 2;

  Serial.print("거리 : ");
  Serial.print(distance);
  Serial.println("cm");

  return distance; //측정한 거리값을 출력
}

// 창문이 열려있는지 확인
int winCheck() {
  int d = dist();
  int distance_error = disToWin + 5;
  if (d > distance_error) {
    return 1;
  }
  else {
    return 0;
  }
}

// 만약 1시간 시점에 비 예보가 있으면 스마트폰 앱 알림을 통해서 알려준다.
void rainNotice() {
  if (pty0 != '0') {
    Blynk.notify("현재시각  " + String (hour()) + "시 " + minute() + "분 " + second() + "초 \n" + "1시간후 비예보가 있으니 창문을 닫아주세요!");
    int winCh = winCheck(); //1시간 시점에 비 예보가 있는데 창문이 열려있으면 부저를 통해서 음악을 출력
    if (winCh == 1) {
      buzzer();
    }
  }
}

//RSS로부터 데이터를 받아오고 터미널과 시리얼 모니터를 통해 출력
void parsing() {
  terminal.clear();
  String announce_time;
  int pubDate_start = line.indexOf(F("<pubDate>")); // "<pubDate>"문자가 시작되는 인덱스 값('<'의 인덱스)을 반환한다.
  int pubDate_end = line.indexOf(F("</pubDate>"));
  announce_time = line.substring(pubDate_start + 9, pubDate_end); // +9: "<pubDate>"스트링의 크기 9바이트, 9칸 이동
  Serial.print(F("announce_time: "));
  Serial.println(announce_time);

  terminal.print("기상예보 발표 시각 : ");
  terminal.println(announce_time);
  terminal.println("--------------------");

  String hour;
  int hour_start = line.indexOf(F("<hour>"));
  int hour_end = line.indexOf(F("</hour>"));
  hour = line.substring(hour_start + 6, hour_end);
  Serial.print(F("hour: "));
  Serial.println(hour);

  terminal.print(hour);
  terminal.println("시 기상예보 ");

  String temp;
  int temp_start = line.indexOf(F("<temp>"));
  int temp_end = line.indexOf(F("</temp>"));
  temp = line.substring(temp_start + 6, temp_end);
  Serial.print(F("temp: "));
  Serial.println(temp);

  terminal.print("기온 : ");
  terminal.print(temp);
  terminal.println("도");

  String wfKor;
  int wfKor_start = line.indexOf(F("<wfKor>"));
  int wfKor_end = line.indexOf(F("</wfKor>"));
  wfKor = line.substring(wfKor_start + 7, wfKor_end);
  Serial.print(F("weather: "));
  Serial.println(wfKor);

  terminal.print(F("날씨 : "));
  terminal.println(wfKor);
  terminal.println("--------------------");
  terminal.flush();

  String pty;
  int pty_start = line.indexOf(F("<pty>"));
  int pty_end = line.indexOf(F("</pty>"));
  pty = line.substring(pty_start + 5, pty_end);
  Serial.print(F("pty: "));
  Serial.println(pty);
  pty0 = pty.charAt(0);
  Serial.print(F("pty0: "));
  Serial.println(pty0);

  int del_index = line.indexOf(F("</data>"));
  line.remove(0, del_index + 7); // 시작 인덱스 부터 "</data>" 스트링 포함 삭제
  hour_start = line.indexOf(F("<hour>"));
  hour_end = line.indexOf(F("</hour>"));
  hour = line.substring(hour_start + 6, hour_end);
  Serial.print(F("hour: "));
  Serial.println(hour);

  terminal.print(hour);
  terminal.println("시 기상예보 ");

  temp_start = line.indexOf(F("<temp>"));
  temp_end = line.indexOf(F("</temp>"));
  temp = line.substring(temp_start + 6, temp_end);
  Serial.print(F("temp: "));
  Serial.println(temp);

  terminal.print("기온 : ");
  terminal.print(temp);
  terminal.println("도");

  wfKor_start = line.indexOf(F("<wfKor>"));
  wfKor_end = line.indexOf(F("</wfKor>"));
  wfKor = line.substring(wfKor_start + 7, wfKor_end);
  Serial.print(F("weather: "));
  Serial.println(wfKor);

  terminal.print(F("날씨 : "));
  terminal.println(wfKor);
  terminal.println("--------------------");
  terminal.flush();

  del_index = line.indexOf(F("</data>"));
  line.remove(0, del_index + 7);  // 시작 인덱스 부터 "</data>" 스트링 포함 삭제
  hour_start = line.indexOf(F("<hour>"));
  hour_end = line.indexOf(F("</hour>"));
  hour = line.substring(hour_start + 6, hour_end);
  Serial.print(F("hour: "));
  Serial.println(hour);

  terminal.print(hour);
  terminal.println("시 기상예보 ");

  temp_start = line.indexOf(F("<temp>"));
  temp_end = line.indexOf(F("</temp>"));
  temp = line.substring(temp_start + 6, temp_end);
  Serial.print(F("temp: "));
  Serial.println(temp);

  terminal.print("기온 : ");
  terminal.print(temp);
  terminal.println("도");

  wfKor_start = line.indexOf(F("<wfKor>"));
  wfKor_end = line.indexOf(F("</wfKor>"));
  wfKor = line.substring(wfKor_start + 7, wfKor_end);
  Serial.print(F("weather: "));
  Serial.println(wfKor);

  line = ""; // 스트링 변수 line 데이터 추출 완료 

  terminal.print(F("날씨 : "));
  terminal.println(wfKor);

  terminal.flush();
}
