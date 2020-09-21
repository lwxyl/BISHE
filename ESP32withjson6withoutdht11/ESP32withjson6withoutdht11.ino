/*
IR Receiver (VS1838b)
VCC - 3.3V
GND - GND
OUT - IO26

IR Send LED (5MM 940mm Unknown Manufacturer Part Number)
+ - 100 ohms - 3.3V
- - 3(COLLECTOR 2N2222A) 

2N2222A
1(EMITTER) - GND
2(BASE) - 1k ohms - IO5
3(COLLECTOR) - -(IR Send LED)

RGBLED module
R - IO18
G - IO19
B - IO23
GND - GND

Battery
+ - VCC
- - GND
*/
#include <IRremote.h>    /* https://github.com/z3t0/Arduino-IRremote */
#include <ArduinoJson.h> /* https://arduinojson.org/ */
#include <WiFi.h>
#include <PubSubClient.h> /* https://github.com/knolleary/pubsubclient/ */

//#include <RGBLed.h>       /* https://github.com/wilmouths/RGBLed      analogWrite() don't work with esp32, use ledcWrite() https://github.com/espressif/arduino-esp32/issues/4 */

/* Update these with values suitable for your network */

const char *ssid = "AAA";
const char *password = "XXXXXXXXXX";
IPAddress mqtt_server(103, 127, 238, 122);

/*1
const char *ssid = "2333";
const char *password = "123456789";
IPAddress mqtt_server(192, 168, 137, 1);
*/
/*
const char *ssid = "PB";
const char *password = "cptbtptp";
IPAddress mqtt_server(192, 168, 101, 102);
*/
#define SENDTOPIC "IR/key"
#define COMMANDTOPIC "IR/command"
#define SERVICETOPIC "IR/service"

#define KEY_1 0xFF30CF     /* testIR() */
#define IR_RECEIVER_PIN 26 /* Tell IRremote which Arduino pin is connected to the IR Receiver (VS1838b) */
#define IR_LED_PIN 5       /* Tell IRremote which Arduino pin is connected to the IR Send LED (5MM 940mm) */
#define ledR 18
#define ledG 19
#define ledB 23
#define ledRGB 0 /* ledTurnOff() */


#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
uint8_t ledArray[3] = {1, 2, 3}; /* three led channels */
unsigned long premillis = 0;
IRrecv irrecv(IR_RECEIVER_PIN); /* Create instance of 'irrecv' */
IRsend irsend(IR_LED_PIN);      /* Create instance of 'irrecv' */
decode_results results;         /* Somewhere to store the decode results */
WiFiClient wifiClient;


/* Somewhere to store the recorded code */
int codeType = -1;             /* The type of code; like results.decode_type */
unsigned long codeValue;       /* The code value if not raw; like results.value */
unsigned int rawCodes[RAWBUF]; /* The durations if raw; like results.rawbuf */
int codeLen;                   /* The length of the code */
int toggle = 0;                /* The RC5/6 toggle state */

void callback(char *topic, byte *payload, unsigned int length)
{ /* Take actions when received an mqtt massage */
    String stopic;
    for (int i = 0; topic[i] > 0; i++)
    {
        stopic += (char)topic[i];
    }
    if (stopic == COMMANDTOPIC)
    {
        DynamicJsonDocument doc(200); /* https://arduinojson.org/assistant/ */
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            return;
        }
        printPayload(doc);
        unsigned long valu = doc["value"];
        uint8_t r = (valu & 0xFF0000) >> 16;
        uint8_t g = (valu & 0xFF00) >> 8;
        uint8_t b = valu & 0xFF;
        ledTurnOn(r, g, b);
        premillis = millis();
        sendCode(doc["type"], valu, doc["length"]);
        return;
    }
}


void printPayload(DynamicJsonDocument doc)
{
    Serial.print(F("Payload "));
    serializeJson(doc, Serial);
    //serializeJsonPretty(doc, Serial);
    Serial.println();
}

PubSubClient client(mqtt_server, 1883, callback, wifiClient);

void setup()
{
    Serial.begin(115200);
    connectWifi();
    connectMqttServer();
    ledcAttachPin(ledR, 1); /* assign RGB led pins to channels */
    ledcAttachPin(ledG, 2);
    ledcAttachPin(ledB, 3);
    /*
    Initialize channels
    ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
    channels 0-15,  freq limits depend on resolution  resolution 1-16 bits
    */
    ledcSetup(1, 12000, 8); /* 12 kHz PWM, 8-bit resolution */
    ledcSetup(2, 12000, 8);
    ledcSetup(3, 12000, 8);
    irrecv.enableIRIn(); /* Start the receiver */
    testRgbLed();
    testIR();
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    { /* Check WiFi connection status */
        if (client.connected())
        { /* Check MQTT Server connection status */
            if (irrecv.decode(&results))
            { /* Received an IR signal */
                unsigned long valu = results.value;
                uint8_t r = (valu & 0xFF0000) >> 16;
                uint8_t g = (valu & 0xFF00) >> 8;
                uint8_t b = valu & 0xFF;
                ledTurnOn(r, g, b);
                premillis = millis();
                storeCode(&results);
                irrecv.resume(); /* Receive the next value */
            }
            client.loop();
            delay(150);
        }
        else
        {
            Serial.println(F("Error in MQTT connection, retrying.."));
            connectMqttServer(); /* Reconnect MQTT Server */
        }
    }
    else
    {
        Serial.println(F("Error in WiFi connection, retrying.."));
        connectWifi();       /* Reconnect Wifi */
        connectMqttServer(); /* Reconnect MQTT Server */
    }
    if (millis() - premillis >= 200)
    {
        ledTurnOff(ledRGB);
    }
}

void testRgbLed()
{
    Serial.println(F("Send all LEDs a 255 and wait 3 seconds."));
    ledTurnOn(255, 255, 255);
    delay(3000);
    ledTurnOff(ledRGB);
}

void ledTurnOn(uint8_t r, uint8_t g, uint8_t b)
{
    ledcWrite(1, r);
    ledcWrite(2, g);
    ledcWrite(3, b);
}

void ledTurnOff(uint8_t ledX)
{
    switch (ledX)
    {
    case ledRGB:
    {
        ledcWrite(1, 0);
        ledcWrite(2, 0);
        ledcWrite(3, 0);
        return;
    }
    case ledR:
    {
        ledcWrite(1, 0);
        return;
    }
    case ledG:
    {
        ledcWrite(2, 0);
        return;
    }
    case ledB:
    {
        ledcWrite(3, 0);
        return;
    }
    default:
    {
        Serial.println(F(" Led TurnOff Error, Unknown Led "));
        return;
    }
    }
}

void connectWifi()
{
    Serial.println();
    Serial.print(F("Connecting to "));
    Serial.println(F(ssid));
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(F("."));
    }
    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
}

void testIR()
{
    Serial.println(F("IRtest..."));
    Serial.println(F("Please press key '1' "));
    while (1)
    {
        if (irrecv.decode(&results))
        { /* Received an IR signal */
            if (results.value == KEY_1)
            { /* 1 */
                Serial.println(F("Key '1' pressed"));
                Serial.println(F("IRreceive test passed"));
                Serial.println(F("Sending back '1' "));
                int type = NEC;
                unsigned long valu = KEY_1;
                int len = 32;
                sendCode(type, valu, len);
                Serial.println(F("IRtest done..."));
                irrecv.resume(); /* Receive the next value */
                return;
            }
            else
            {
                Serial.println(F("Error key pressed"));
                Serial.println(F("Please press key '1' "));
                irrecv.resume(); /* Receive the next value */
            }
        }
    }
}

void storeCode(decode_results *results)
{ /* Stores the code for later playback, most of this code is just logging */
    char buf[10];
    String cType = "";
    String IRcommand = "";
    DynamicJsonDocument doc(200); /* https://arduinojson.org/assistant/ */
    codeType = results->decode_type;
    if (codeType == UNKNOWN)
    {
        Serial.println(F("Received unknown code, saving as raw"));
        codeLen = results->rawlen - 1;
        /* 
        To store raw codes:
        Drop first value (gap)
         Convert from ticks to microseconds
        Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
        */
        for (int i = 1; i <= codeLen; i++)
        {
            if (i % 2)
            { /* Mark */
                rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
                Serial.print(F(" m"));
            }
            else
            { /* Space */
                rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
                Serial.print(F(" s"));
            }
            Serial.print(rawCodes[i - 1], DEC);
        }
        Serial.println();
    }
    else
    {
        if (codeType == NEC)
        {
            Serial.print(F("Received NEC: "));
            if (results->value == REPEAT /* 0xFFFFFFFF,  Long press */)
            { /* Don't record a NEC repeat value as that's useless */
                Serial.println(F("repeat; ignoring."));
                return;
            }
            else
                cType = "NEC";
        }
        else if (codeType == SONY)
        {
            Serial.print(F("Received SONY: "));
            cType = "SON";
        }
        else if (codeType == PANASONIC)
        {
            Serial.print(F("Received PANASONIC: "));
            cType = "PAN";
        }
        else if (codeType == JVC)
        {
            Serial.print(F("Received JVC: "));
            cType = "JVC";
        }
        else if (codeType == RC5)
        {
            Serial.print(F("Received RC5: "));
            cType = "RC5";
        }
        else if (codeType == RC6)
        {
            Serial.print(F("Received RC6: "));
            cType = "RC6";
        }
        else
        {
            Serial.print(F("Unexpected codeType "));
            Serial.print(codeType, DEC);
            Serial.println();
            cType = "UNKNOWN";
        }
        codeValue = results->value;
        codeLen = results->bits;
        Serial.println(codeValue, HEX);
        //Serial.println(codeLen);
        doc["type"] = codeType;
        doc["value"] = results->value;
        doc["length"] = results->bits;
        serializeJson(doc, IRcommand);
        printPayload(doc);
        publishMqtt(SENDTOPIC, IRcommand);
    }
}

void sendCode(int codeType, unsigned long codeValue, int codeLen)
{
    if (codeType == NEC)
    {
        irsend.sendNEC(codeValue, codeLen);
        Serial.print(F("Sent NEC: "));
        Serial.println(codeValue, HEX);
        //Serial.println(codeLen);
    }
    else if (codeType == SONY)
    {
        irsend.sendSony(codeValue, codeLen);
        Serial.print(F("Sent Sony: "));
        Serial.println(codeValue, HEX);
    }
    else if (codeType == PANASONIC)
    {
        irsend.sendPanasonic(codeValue, codeLen);
        Serial.print(F("Sent Panasonic: "));
        Serial.println(codeValue, HEX);
    }
    else if (codeType == JVC)
    {
        irsend.sendJVC(codeValue, codeLen, false);
        Serial.print(F("Sent JVC: "));
        Serial.println(codeValue, HEX);
    }
    else if (codeType == RC5 || codeType == RC6)
    {
        toggle = 1 - toggle; /* Put the toggle bit into the code to send */
        codeValue = codeValue & ~(1 << (codeLen - 1));
        codeValue = codeValue | (toggle << (codeLen - 1));
        if (codeType == RC5)
        {
            Serial.print(F("Sent RC5: "));
            Serial.println(codeValue, HEX);
            irsend.sendRC5(codeValue, codeLen);
        }
        else
        {
            irsend.sendRC6(codeValue, codeLen);
            Serial.print(F("Sent RC6: "));
            Serial.println(codeValue, HEX);
        }
    }
    else if (codeType == UNKNOWN /* i.e. raw */)
    {
        irsend.sendRaw(rawCodes, codeLen, 38); /* Assume 38 KHz */
        Serial.println(F("Sent raw: "));
    }
}

void publishMqtt(String topic, String message)
{
    if (!client.connected())
    {
        Serial.println(F("Error in MQTT connection, retrying.."));
        connectMqttServer();
    }
    client.publish(topic.c_str(), message.c_str());
}

void connectMqttServer()
{
    while (!client.connected())
    { /* Loop until we're reconnected */
        Serial.print(F("Attempting MQTT connection..."));
        if (client.connect("Mailbox"))
        {                                        /* Attempt to connect */
            uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
            uint16_t chip = (uint16_t)(chipid >> 32);
            char mcudevice[23]; /* https://arduino.stackexchange.com/questions/58677/get-esp32-chip-id-into-a-string-variable-arduino-c-newbie-here */
            snprintf(mcudevice, 23, "ESP32-%04X%08X", chip, (uint32_t)chipid);
            Serial.println(F("MQTT server connected"));
            Serial.println(mcudevice);
            publishMqtt(SERVICETOPIC, "MQTT server connected"); /* Once connected, publish an announcement and resubscribe */
            publishMqtt(SERVICETOPIC, mcudevice);
            client.subscribe(COMMANDTOPIC);
        }
        else
        {
            Serial.print(F("failed, rc = "));
            Serial.print(client.state());
            Serial.println(F(" try again in 5 seconds"));
            delay(5000); /* Wait 5 seconds before retrying */
        }
    }
}
