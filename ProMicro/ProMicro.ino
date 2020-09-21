/*
IR Receiver (VS1838b)
VCC - 3.3V
GND - GND
OUT - IO5

RGBLED module
R - IO3
G - IO9
B - IO10
GND - GND

*/

#include <IRremote.h>    /* https://github.com/z3t0/Arduino-IRremote */
#include <HID-Project.h> /* https://github.com/NicoHood/HID */
#include <RGBLed.h>      /* https://github.com/wilmouths/RGBLed */

#define IR_RECEIVER_PIN 5 /* Tell IRremote which Arduino pin is connected to the IR Receiver (VS1838b) */
#define ledR 3
#define ledG 9
#define ledB 10
RGBLed led(ledR, ledG, ledB, COMMON_CATHODE);
unsigned long premillis = 0;

IRrecv irrecv(IR_RECEIVER_PIN);     /* Create instance of 'irrecv' */
decode_results results, preresults; /* Somewhere to store the results */

void setup()
{
    Serial.begin(115200);
    irrecv.enableIRIn();      /* Start the receiver */
    pinMode(2, INPUT_PULLUP); /* Make pin 2 an input and turn on the pull-up resistor so it goes high unless connected to ground */
    Keyboard.begin();         /* See HID Project documentation for more information. https://github.com/NicoHood/HID/wiki/Keyboard-API#improved-keyboard */
    Consumer.begin();         /* Sends a clean report to the host. This is important on any Arduino type. */
}

void loop()
{
    while (digitalRead(2) == HIGH)
    { /* Do nothing until pin 2 goes low */
        delay(500);
    }
    if (irrecv.decode(&results))
    { /* Received an IR signal */
        //Serial.println(results.value, HEX);
        unsigned long valu = results.value;
        uint8_t r = (valu & 0xFF0000) >> 16;
        uint8_t g = (valu & 0xFF00) >> 8;
        uint8_t b = valu & 0xFF;
        led.setColor(r, g, b);
        premillis = millis();
        translateIR(&results); /* Takes action based on IR code received */
        irrecv.resume();       /* Receive the next value */
    }
    if (millis() - premillis >= 200)
    {
        led.setColor(0, 0, 0);
    }
}

void translateIR(decode_results *results) /* Takes action based on IR code received */
{
    switch (results->value)
    {
    case 0xFFFFFFFF: /* Long press, repeat preresults */
    {
        translateIR(&preresults);
        return;
    }
    /* Line 1 */
    case 0xFFA25D: /* CH- */
    {
        //Serial.println(F(" CH - "));
        //Keyboard.press(KEY_PAGE_UP);
        //Keyboard.release(KEY_PAGE_UP);
        Keyboard.write(KEY_PAGE_UP);
        break;
    }
    case 0xFF629D: /* CH */
    {
        //Serial.println(F(" CH "));
        //Keyboard.press(KEY_ESC);
        //Keyboard.release(KEY_ESC);
        Keyboard.write(KEY_ESC);
        break;
    }
    case 0xFFE21D: /* CH+ */
    {
        //Serial.println(F(" CH + "));
        //Keyboard.press(KEY_PAGE_DOWN);
        //Keyboard.release(KEY_PAGE_DOWN);
        Keyboard.write(KEY_PAGE_DOWN);
        break;
    }
        /* Line 2 */

    case 0xFF22DD: /* |<< */
    {
        //Serial.println(F(" | << "));
        //Keyboard.press(KEY_LEFT_ARROW);
        //Keyboard.release(KEY_LEFT_ARROW);
        Keyboard.write(KEY_LEFT_ARROW);
        break;
    }
    case 0xFF02FD: /* >>| */
    {
        //Serial.println(F(" >> | "));
        //Keyboard.press(KEY_RIGHT_ARROW);
        //Keyboard.release(KEY_RIGHT_ARROW);
        Keyboard.write(KEY_RIGHT_ARROW);
        break;
    }
    case 0xFFC23D: /* >|| */
    {
        //Serial.println(F(" > || "));
        //Keyboard.press(' ');
        //Keyboard.release(' ');
        Keyboard.write(' ');
        break;
    }
        /* Line 3 */

    case 0xFFE01F: /* - */
    {
        //Serial.println(F(" - "));
        Consumer.write(MEDIA_VOL_DOWN); /* See HID Project documentation for more Consumer keys. https://github.com/NicoHood/HID/wiki/Consumer-API */
        break;
    }
    case 0xFFA857: /* + */
    {
        //Serial.println(F(" + "));
        Consumer.write(MEDIA_VOL_UP); /* See HID Project documentation for more Consumer keys. https://github.com/NicoHood/HID/wiki/Consumer-API */
        break;
    }
    case 0xFF906F: /* EQ */
    {
        //Serial.println(F(" EQ "));
        //Keyboard.press(KEY_RETURN);
        //Keyboard.release(KEY_RETURN);
        Keyboard.write(KEY_RETURN);
        break;
    }
    /* Line 4 */
    case 0xFF6897: /* 0 */
    {
        //Serial.println(F(" 0 "));
        //Keyboard.press('0');
        //Keyboard.release('0');
        Keyboard.write('0');
        break;
    }
    case 0xFF9867: /* 100+ */
    {
        //Serial.println(F(" 100 + "));
        //Keyboard.print("100+");
        //Keyboard.press('x');
        //Keyboard.release('x');
        Keyboard.write('x'); /* Reduce the video play speed in Potplayer */
        break;
    }
    case 0xFFB04F: /* 200+ */
    {
        //Serial.println(F(" 200 + "));
        //Keyboard.print("200+");
        //Keyboard.press('c');
        //Keyboard.release('c');
        Keyboard.write('c'); /* Increase the video play speed in Potplayer */
        break;
    }
    /* Line 5 */
    case 0xFF30CF: /* 1 */
    {
        //Serial.println(F(" 1 "));
        //Keyboard.press('1');
        //Keyboard.release('1');
        Keyboard.write('1');
        break;
    }
    case 0xFF18E7: /* 2 */
    {
        //Serial.println(F(" 2 "));
        //Keyboard.press('2');
        //Keyboard.release('2');
        Keyboard.write('2');
        break;
    }
    case 0xFF7A85: /* 3 */
    {
        //Serial.println(F(" 3 "));
        //Keyboard.press('3');
        //Keyboard.release('3');
        Keyboard.write('3');
        break;
    }
    /* Line 6 */
    case 0xFF10EF: /* 4 */
    {
        //Serial.println(F(" 4 "));
        //Keyboard.press('4');
        //Keyboard.release('4');
        Keyboard.write('4');
        break;
    }
    case 0xFF38C7: /* 5 */
    {
        //Serial.println(F(" 5 "));
        //Keyboard.press('5');
        //Keyboard.release('5');
        Keyboard.write('5');
        break;
    }
    case 0xFF5AA5: /* 6 */
    {
        //Serial.println(F(" 6 "));
        //Keyboard.press('6');
        //Keyboard.release('6');
        Keyboard.write('6');
        break;
    }
    /* Line 7 */
    case 0xFF42BD: /* 7 */
    {
        //Serial.println(F(" 7 "));
        //Keyboard.press('7');
        //Keyboard.release('7');
        Keyboard.write('7');
        break;
    }
    case 0xFF4AB5: /* 8 */
    {
        //Serial.println(F(" 8 "));
        //Keyboard.press('8');
        //Keyboard.release('8');
        Keyboard.write('8');
        break;
    }
    case 0xFF52AD: /* 9 */
    {
        //Serial.println(F(" 9 "));
        //Keyboard.press('9');
        //Keyboard.release('9');
        Keyboard.write('9');
        break;
    }
    /* Default */
    default:
    {
        Serial.println(F(" Unknown "));
        Serial.println(results->value, HEX);
        break;
    }
    }
    preresults = *results;
    delay(100); /* Do not get immediate repeat */
}
