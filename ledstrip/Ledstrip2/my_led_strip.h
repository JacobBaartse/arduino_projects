//PREVIOUSLY USED https://github.com/FastLED/FastLED/blob/master/examples/ColorPalette/ColorPalette.ino

#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif


#define LED_PIN_1     5
#define NUM_LEDS_1    120
#define BRIGHTNESS  25
#define LED_TYPE_1    WS2812B
#define COLOR_ORDER_1 GRB
CRGB leds_1[NUM_LEDS_1];


#define LED_PIN_2     A0
#define NUM_LEDS_2    120
#define LED_TYPE_2    WS2812B
#define COLOR_ORDER_2 GRB
CRGB leds_2[NUM_LEDS_2];


#define SCROLING_RAINBOW     1
#define RAINBOW         2
#define SPARKLING       3
#define RUN_AROUND      4
#define FADE_IN_OUT     5
#define CANNON_SHOT_LEDS 6
#define LIGHT_GOES_UP   7
#define MAX_EFFECT      8

long prev_millis_leds=0;
void pattern_on_ledstrip(int pattern, long start_millis, long duration){
    long now = millis();
    uint8_t position;
    int brightness;
    int num_leds;
    if (now > start_millis + duration)
        pattern = RAINBOW;  // The default pattern if nothing else is active.
    switch (pattern){
        case SCROLING_RAINBOW:
            for (uint8_t i=0; i<NUM_LEDS_2; i++)
                leds_2[(i+millis()/5)%NUM_LEDS_2] = CHSV(4*i, 200, 255);  // hue , saturation, value
            break;
        case RAINBOW:
             for (uint8_t i=0; i<NUM_LEDS_2; i++)
                leds_2[i] = CHSV(2*i, 255, 255);
            break;    
        case SPARKLING:
            if (now > prev_millis_leds + 20){
                prev_millis_leds = now;
                leds_2[random8(0, NUM_LEDS_2 - 1)] = CRGB(255,255,255);
                fadeToBlackBy(leds_2, NUM_LEDS_2, 40);
            }
            break; 
        case RUN_AROUND:
            position = (uint8_t) (now /10 %NUM_LEDS_2);
            for (uint8_t i=0; i<NUM_LEDS_2; i++)
            {
                uint8_t index = (position+i)%NUM_LEDS_2;

                if (i > 60)
                    leds_2[index] = CRGB(255,255,255);
                else
                    leds_2[index] = CRGB(0,0,0);
            }
            break;
        case FADE_IN_OUT:
            brightness = ((now - start_millis)) % 512;
            if (brightness > 255)
                brightness = 511 - brightness;

            for (uint8_t i=0; i<NUM_LEDS_2; i++)
                leds_2[i] = CHSV(2*i, 255, (uint8_t) brightness);
            break;
        case CANNON_SHOT_LEDS:
            num_leds = (now - start_millis)/ 20;
            if (num_leds> 20) num_leds = 20;
            for (int i =0; i< num_leds; i++){
                leds_2[43+i] = CRGB(255,255,255);
                leds_2[43-i] = CRGB(255,255,255);
            }
            break;
        case LIGHT_GOES_UP:
            num_leds = ((now - start_millis)/ 10 ) % 40;
            for (int i =0; i< num_leds; i++){
                leds_2[i] = CRGB(255,255,255);
                leds_2[NUM_LEDS_2-i-1] = CRGB(255,255,255);
            }
            for (int i=num_leds; i<NUM_LEDS_2 - num_leds; i++) leds_2[i] = CRGB(0,0,0);
            break;

    }
}

bool left1hit = false;
bool left2hit = false;
bool left3hit = false;
long left1blink_until = 0;
long left2blink_until = 0;
long left3blink_until = 0;
long leftblinkall_until = 0;

bool right1hit = false;
bool right2hit = false;
bool right3hit = false;
bool right4hit = false;
long right1blink_until = 0;
long right2blink_until = 0;
long right3blink_until = 0;
long right4blink_until = 0;
long rightblinkall_until = 0;

void leds_on(CRGB leds[], int led1, int led2, CRGB color){
    leds[led1] = color;  
    leds[led2] = color;
}

void blink(CRGB leds[], int led1, int led2, CRGB color){
    if (millis() % 100 > 50) 
        color = CRGB::Black;
    leds[led1] = color;  
    leds[led2] = color;
}

void reset_left_hit(){
    left1hit = false;
    left2hit = false;
    left3hit = false;
}

void reset_right_hit(){
    right1hit = false;
    right2hit = false;
    right3hit = false;
    right4hit = false;
}

void score_onleds(){
    debugln("Show leds score");
    long now = millis();
    if ((left3hit & left3blink_until>now)| leftblinkall_until>now) blink(leds_1, 8, 9,  CRGB::Green);
    else if (left3hit) leds_on(leds_1, 8, 9,  CRGB::Green);
         else leds_on(leds_1, 8, 9,  CRGB::Black);

    if ((left2hit & left2blink_until>now)| leftblinkall_until>now) blink(leds_1, 10, 11,  CRGB::Red);
    else if (left2hit) leds_on(leds_1, 10, 11,  CRGB::Red);
             else leds_on(leds_1, 10, 11,  CRGB::Black);
             
    if ((left1hit & left1blink_until>now)| leftblinkall_until>now) blink(leds_1, 12, 13,  CRGB::Blue);
    else if (left1hit) leds_on(leds_1, 12, 13,  CRGB::Blue);
             else leds_on(leds_1, 12, 13,  CRGB::Black);

    if (left1hit & left2hit & left3hit)
        if (left1blink_until<now & left2blink_until<now & left3blink_until<now)
        {
            leftblinkall_until = now + 2000;
            reset_left_hit();
        };

    if ((right1hit & right1blink_until>now)| rightblinkall_until>now) blink(leds_2, 0, 1,  CRGB::Green);
    else if (right1hit) leds_on(leds_2, 0, 1,  CRGB::Green);
         else leds_on(leds_2, 0, 1,  CRGB::Black);

    if ((right2hit & right2blink_until>now)| rightblinkall_until>now) blink(leds_2, 2, 3,  CRGB::Red);
    else if (right2hit) leds_on(leds_2, 2, 3,  CRGB::Red);
         else leds_on(leds_2, 2, 3,  CRGB::Black);

    if ((right3hit & right3blink_until>now)| rightblinkall_until>now) blink(leds_2, 4, 5,  CRGB::Blue);
    else if (right3hit) leds_on(leds_2, 4, 5,  CRGB::Blue);
         else leds_on(leds_2, 4, 5,  CRGB::Black);

    if ((right4hit & right4blink_until>now)| rightblinkall_until>now) blink(leds_2, 6, 7,  CRGB::Yellow);
    else if (right4hit) leds_on(leds_2, 6, 7,  CRGB::Yellow);
         else leds_on(leds_2, 6, 7,  CRGB::Black);

    if (right1hit & right2hit & right3hit & right4hit)
        if (right1blink_until<now & right2blink_until<now & right3blink_until<now & right4blink_until<now)
        {
            rightblinkall_until = now + 2000;
            reset_right_hit();
        };
}

void ledstrip_setup() {
    FastLED.addLeds<LED_TYPE_1, LED_PIN_1, COLOR_ORDER_1>(leds_1, NUM_LEDS_1).setCorrection( TypicalLEDStrip );
    FastLED.addLeds<LED_TYPE_2, LED_PIN_2, COLOR_ORDER_2>(leds_2, NUM_LEDS_2); //.setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);  // 5v  0,5A
    FastLED.clear();
    FastLED.show();



}


void blink_all_leds(int duration_ms){
    while (duration_ms>0){
        fill_solid(leds_1, NUM_LEDS_1, CRGB::Gray);
        FastLED.show();
        delay(50);
        fill_solid(leds_1, NUM_LEDS_1, CRGB::Black);
        FastLED.show();
        delay(50);
        duration_ms -=100;
    }
}


void blink_all_leds_blue_red(int duration_ms){
    while (duration_ms>0){
        fill_solid(leds_1, NUM_LEDS_1, CRGB::Blue);
        FastLED.show();
        delay(50);
        fill_solid(leds_1, NUM_LEDS_1, CRGB::Red);
        FastLED.show();
        delay(50);
        duration_ms -=100;
    }
}


#undef debug
#undef debugln
