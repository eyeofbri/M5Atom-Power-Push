#include <Arduino.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Adafruit_NeoPixel.h>
#include "ui.h"
WebServer server(80); Preferences prefs; Servo servo; WiFiManager wm; Adafruit_NeoPixel pixel(1,27,NEO_GRB+NEO_KHZ800);
uint32_t restartSeconds=0; int pwmMin=1000,pwmMax=2000; bool portal=false; uint32_t idleSince=0; uint32_t cycle=0;
String host="powerpush", label="Switch", target=""; int rest=1500, press=1500, hold=500; bool configured=false, active=false, preview=false; uint32_t started=0, duration=0, lastProbe=0; int reach=-1; bool mdns=false; bool reboot=false; uint32_t rebootAt=0;
void reply(int status,const char* msg){JsonDocument d; d["message"]=msg; String s; serializeJson(d,s); server.send(status,"application/json",s);}
bool validHost(String s){if(!s.length()||s.length()>63||s[0]=='-'||s[s.length()-1]=='-')return false; for(char c:s)if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'))return false;return true;}
void release(){servo.writeMicroseconds(rest);if(active){idleSince=millis();cycle++;}active=false;preview=false;}
void moveFor(int angle,int ms,bool test){servo.writeMicroseconds(angle);started=millis();duration=ms;active=true;preview=test;}
void state(){JsonDocument d; d["host"]=host;d["label"]=label;d["target"]=target;d["restUs"]=rest;d["pressUs"]=press;d["calibrationSchema"]=2;d["hold"]=hold;d["configured"]=configured;d["active"]=active;d["remaining"]=active?max(0,(int)(duration-(millis()-started))):0;d["duration"]=duration;d["rssi"]=WiFi.RSSI();d["ip"]=WiFi.localIP().toString();d["ssid"]=WiFi.SSID();d["reach"]=reach;d["uptime"]=esp_timer_get_time()/1000000LL;d["restartSeconds"]=restartSeconds;d["pwmMin"]=pwmMin;d["pwmMax"]=pwmMax;d["portal"]=portal;d["minHeap"]=ESP.getMinFreeHeap();d["resetReason"]=(int)esp_reset_reason();d["cycle"]=cycle;d["heap"]=ESP.getFreeHeap();d["mdns"]=mdns;String s;serializeJson(d,s);server.send(200,"application/json",s);}
bool body(JsonDocument &d){if(!server.hasArg("plain")||deserializeJson(d,server.arg("plain"))){reply(400,"Invalid JSON");return false;}return true;}
void startMdns(){if(mdns) MDNS.end(); mdns=MDNS.begin(host.c_str()); if(mdns) MDNS.addService("http","tcp",80); Serial.printf("[MDNS] %s.local: %s\n",host.c_str(),mdns?"OK":"FAILED");}
// Store the entire calibration atomically under a new key. Legacy keys remain untouched.
struct CalibrationV2 { uint32_t magic; int lo, hi, restUs, pressUs, holdMs; uint32_t ready; };
constexpr uint32_t CAL_MAGIC=0x50505732;
bool validRange(int lo,int hi){return lo>=500&&lo<=1500&&hi>=1500&&hi<=2400&&lo<hi;}
bool validPulse(int us,int lo,int hi){return us>=lo&&us<=hi;}
bool validCalibration(const CalibrationV2& c){return c.magic==CAL_MAGIC&&validRange(c.lo,c.hi)&&validPulse(c.restUs,c.lo,c.hi)&&validPulse(c.pressUs,c.lo,c.hi)&&c.holdMs>=100&&c.holdMs<=2000&&c.ready<=1&&(!c.ready||c.restUs!=c.pressUs);}
void loadCalibration(){
  CalibrationV2 c={CAL_MAGIC,1000,2000,1500,1500,500,0};
  if(prefs.isKey("calV2")){
    CalibrationV2 saved{};
    if(prefs.getBytesLength("calV2")==sizeof(saved)&&prefs.getBytes("calV2",&saved,sizeof(saved))==sizeof(saved)&&validCalibration(saved))c=saved;
    else Serial.println("[CAL] Invalid saved calibration; defaults, actuation disabled");
  }else{
    int lo=prefs.getInt("pwmMin",1000),hi=prefs.getInt("pwmMax",2000),r=prefs.getInt("rest",90),p=prefs.getInt("press",90),h=prefs.getInt("hold",500);
    if(validRange(lo,hi)&&r>=0&&r<=180&&p>=0&&p<=180&&h>=100&&h<=2000){
      // Arduino map() used by the old angle API truncates the integer result.
      c={CAL_MAGIC,lo,hi,lo+(hi-lo)*r/180,lo+(hi-lo)*p/180,h,(uint32_t)(prefs.getBool("ready",false)&&r!=p)};
    }
    Serial.println("[CAL] Legacy positions converted to pulse widths");
  }
  pwmMin=c.lo;pwmMax=c.hi;rest=c.restUs;press=c.pressUs;hold=c.holdMs;configured=c.ready;
}
bool storeCalibration(const CalibrationV2& c){return validCalibration(c)&&prefs.putBytes("calV2",&c,sizeof(c))==sizeof(c);}
void setup(){Serial.begin(115200);Serial.printf("\n[BOOT] Power Push v0.1.5 reset=%d\n",(int)esp_reset_reason());pinMode(39,INPUT);pixel.begin();pixel.setBrightness(20);prefs.begin("powerpush",false);host=prefs.isKey("host")?prefs.getString("host","powerpush"):String("powerpush");label=prefs.isKey("label")?prefs.getString("label","Switch"):String("Switch");target=prefs.isKey("target")?prefs.getString("target",""):String("");restartSeconds=prefs.getUInt("restart",0);loadCalibration();servo.setPeriodHertz(50);servo.attach(26,pwmMin,pwmMax);servo.writeMicroseconds(rest);WiFi.mode(WIFI_STA);WiFi.setSleep(false);WiFi.setAutoReconnect(true);WiFi.setHostname(host.c_str());wm.setConfigPortalTimeout(0);wm.setEnableConfigPortal(false);if(wm.getWiFiIsSaved()){WiFi.begin();}else{portal=true;wm.setEnableConfigPortal(true);while(!wm.startConfigPortal("PowerPush-Setup")){delay(100);}portal=false;}WiFi.mode(WIFI_STA);WiFi.setSleep(false);WiFi.setAutoReconnect(true);Serial.printf("[WIFI] IP=%s RSSI=%d dBm\n",WiFi.localIP().toString().c_str(),WiFi.RSSI());if(WiFi.status()==WL_CONNECTED)startMdns();
server.on("/",HTTP_GET,[]{server.sendHeader("Cache-Control","no-store");server.send_P(200,"text/html",UI);});server.on("/api/state",HTTP_GET,state);
server.on("/api/press",HTTP_POST,[]{if(reboot){reply(409,"Restart pending");return;}if(!configured){reply(409,"Calibrate first");return;}if(active){reply(409,"Actuator busy");return;}moveFor(press,hold,false);reply(200,"Press started");});
server.on("/api/release",HTTP_POST,[]{release();reply(200,"Released");});
server.on("/api/test",HTTP_POST,[]{JsonDocument d;if(!body(d))return;if(active||reboot){reply(409,"Actuator busy");return;}if(!d["pulseUs"].is<int>()||!validPulse(d["pulseUs"].as<int>(),pwmMin,pwmMax)){reply(400,"Pulse must be within saved PWM limits");return;}moveFor(d["pulseUs"].as<int>(),500,true);reply(200,"Testing position");});
server.on("/api/settings",HTTP_POST,[]{if(active||reboot){reply(409,"Actuator busy");return;}JsonDocument d;if(!body(d))return;String h=d["host"]|"", l=d["label"]|"",t=d["target"]|"";h.toLowerCase();l.trim();t.trim();IPAddress ip;uint32_t rs=d["restartSeconds"]|0U;if(!validHost(h)||!l.length()||l.length()>40||(t.length()&&!ip.fromString(t))||!d["restartSeconds"].is<uint32_t>()||(rs&&rs<300)||rs>2592000){reply(400,"Invalid device settings (restart: 5 minutes to 30 days)");return;}bool changed=h!=host;host=h;label=l;target=t;restartSeconds=rs;reach=-1;prefs.putString("host",host);prefs.putString("label",label);prefs.putString("target",target);prefs.putUInt("restart",rs);reply(200,changed?"Saved; reconnect using the new hostname":"Device settings saved");if(changed){reboot=true;rebootAt=millis();}});
server.on("/api/calibration",HTTP_POST,[]{if(active||reboot){reply(409,"Actuator busy");return;}JsonDocument d;if(!body(d))return;for(auto k:{"restUs","pressUs","hold","pwmMin","pwmMax"})if(!d[k].is<int>()){reply(400,"Whole pulse widths and duration required");return;}
int r=d["restUs"],p=d["pressUs"],m=d["hold"],lo=d["pwmMin"],hi=d["pwmMax"];
if(!validRange(lo,hi)||m<100||m>2000){reply(400,"Invalid PWM range or duration");return;}
bool changed=lo!=pwmMin||hi!=pwmMax;
if(changed&&!(d["confirmRange"]|false)){reply(400,"Confirm disconnected linkage first");return;}
if(!changed&&(!validPulse(r,lo,hi)||!validPulse(p,lo,hi)||r==p)){reply(400,"Choose distinct positions within saved PWM limits");return;}
CalibrationV2 c={CAL_MAGIC,lo,hi,changed?1500:r,changed?1500:p,m,changed?0U:1U};
if(!storeCalibration(c)){reply(500,"Calibration not saved; storage error");return;}
configured=c.ready;rest=c.restUs;press=c.pressUs;hold=c.holdMs;
if(changed){pwmMin=lo;pwmMax=hi;servo.detach();servo.attach(26,lo,hi);}
release();reply(200,changed?"PWM range changed. Test and save new positions before use.":"Calibration saved");});
server.on("/api/restart",HTTP_POST,[]{if(active){reply(409,"Actuator busy");return;}reboot=true;rebootAt=millis();reply(200,"Restarting device");});
server.begin();Serial.printf("[HTTP] port 80 ready; heap=%u\n",(unsigned)ESP.getFreeHeap());}
void loop(){if(active&&millis()-started>=duration)release();server.handleClient();static bool wasConnected=false;static IPAddress previousIP=WiFi.localIP();static uint32_t lastMdnsTry=0;bool connected=WiFi.status()==WL_CONNECTED;if(!active){if(!connected&&wasConnected){if(mdns)MDNS.end();mdns=false;Serial.println("[WIFI] Disconnected");}if(connected&&(!wasConnected||WiFi.localIP()!=previousIP||(!mdns&&millis()-lastMdnsTry>10000))){WiFi.setSleep(false);previousIP=WiFi.localIP();Serial.printf("[WIFI] IP=%s RSSI=%d dBm\n",previousIP.toString().c_str(),WiFi.RSSI());startMdns();lastMdnsTry=millis();}wasConnected=connected;}if(active&&millis()-started>=duration)release();if(restartSeconds&&esp_timer_get_time()/1000000LL>=restartSeconds&&!active&&!reboot&&millis()-idleSince>1000&&digitalRead(39)){reboot=true;rebootAt=millis();}if(reboot&&!active&&millis()-rebootAt>800){release();delay(350);ESP.restart();}static uint32_t retry=0;if(!connected&&!active&&millis()-retry>15000){retry=millis();WiFi.reconnect();}static bool previous=true;static uint32_t down=0;bool b=digitalRead(39);if(previous&&!b)down=millis();if(!previous&&b){if(millis()-down>5000&&!active){wm.resetSettings();ESP.restart();}else if(millis()-down<1000&&configured&&!active&&!reboot)moveFor(press,hold,false);}previous=b; if(!active&&!reboot&&connected&&target.length()&&millis()-lastProbe>10000){lastProbe=millis();IPAddress ip;ip.fromString(target);WiFiClient client;reach=client.connect(ip,445,150)?1:0;client.stop();}static uint32_t led=0;if(millis()-led>150){led=millis();pixel.setPixelColor(0,active?pixel.Color(255,120,0):WiFi.status()==WL_CONNECTED?pixel.Color(0,150,80):pixel.Color(180,0,0));pixel.show();}delay(2);}
