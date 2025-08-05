//PREVIOUSLY USED https://github.com/FastLED/FastLED/blob/master/examples/ColorPalette/ColorPalette.ino

#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif


#define LED_PIN     5
#define NUM_LEDS    120
#define BRIGHTNESS  25
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define SCROLING_RAINBOW     1
#define RAINBOW         2
#define SPARKLING       3
#define RUN_AROUND      4
#define FADE_IN_OUT     5
#define CANNON_SHOT_LEDS 6

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
            for (uint8_t i=0; i<NUM_LEDS; i++)
                leds[(i+millis()/5)%NUM_LEDS] = CHSV(4*i, 200, 255);  // hue , saturation, value
            break;
        case RAINBOW:
             for (uint8_t i=0; i<NUM_LEDS; i++)
                leds[i] = CHSV(2*i, 255, 255);
            break;    
        case SPARKLING:
            if (now > prev_millis_leds + 20){
                prev_millis_leds = now;
                leds[random8(0, NUM_LEDS - 1)] = CRGB(255,255,255);
                fadeToBlackBy(leds, NUM_LEDS, 40);
            }
            break; 
        case RUN_AROUND:
            position = (uint8_t) (now /10 %NUM_LEDS);
            for (uint8_t i=0; i<NUM_LEDS; i++)
            {
                uint8_t index = (position+i)%NUM_LEDS;

                if (i > 60)
                    leds[index] = CRGB(255,255,255);
                else
                    leds[index] = CRGB(0,0,0);
            }
            break;
        case FADE_IN_OUT:
            brightness = ((now - start_millis)) % 512;
            if (brightness > 255)
                brightness = 511 - brightness;

            for (uint8_t i=0; i<NUM_LEDS; i++)
                leds[i] = CHSV(2*i, 255, (uint8_t) brightness);
            break;
        case CANNON_SHOT_LEDS:
            num_leds = (now - start_millis)/ 20;
            if (num_leds> 20) num_leds = 20;
            for (int i =0; i< num_leds; i++){
                leds[43+i] = CRGB(255,255,255);
                leds[43-i] = CRGB(255,255,255);
            }
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

void leds_on(int led1, int led2, CRGB color){
    leds[led1] = color;  
    leds[led2] = color;
}

void blink(int led1, int led2, CRGB color){
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

void score_onleds(){
    debugln("Show leds score");
    long now = millis();
    if ((left3hit & left3blink_until>now)| leftblinkall_until>now) blink(8, 9,  CRGB::Green);
    else if (left3hit) leds_on(8, 9,  CRGB::Green);
         else leds_on(8, 9,  CRGB::Black);

    if ((left2hit & left2blink_until>now)| leftblinkall_until>now) blink(10, 11,  CRGB::Red);
    else if (left2hit) leds_on(10, 11,  CRGB::Red);
             else leds_on(10, 11,  CRGB::Black);
             
    if ((left1hit & left1blink_until>now)| leftblinkall_until>now) blink(12, 13,  CRGB::Blue);
    else if (left1hit) leds_on(12, 13,  CRGB::Blue);
             else leds_on(12, 13,  CRGB::Black);

    if (left1hit & left2hit & left3hit)
        if (left1blink_until<now & left2blink_until<now & left3blink_until<now)
        {
            leftblinkall_until = now + 2000;
            reset_left_hit();
        }
}

void ledstrip_setup() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    FastLED.clear();
    FastLED.show();
}


void blink_all_leds(int duration_ms){
    while (duration_ms>0){
        fill_solid(leds, NUM_LEDS, CRGB::Gray);
        FastLED.show();
        delay(50);
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        delay(50);
        duration_ms -=100;
    }
}


void blink_all_leds_blue_red(int duration_ms){
    while (duration_ms>0){
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
        FastLED.show();
        delay(50);
        fill_solid(leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        delay(50);
        duration_ms -=100;
    }
}


#undef debug
#undef debugln
