#include "music.h"
#include "songs.h"
#include "light.h"

#define BUZZER 11
#define LED_RED 6
#define LED_GREEN 9
#define LED_BLUE 10
#define BUTTON_IN 12

// Standard delay of 30 milliseconds for slightly above 30fps.
#define STD_DELAY 30

// MUSIC CONTROL

struct PlayingSong mc_playing_song;

// Begins playing a new song.
void mc_play_song() {
  long choice = random(0, SONG_COUNT);
  playSong(&mc_playing_song, SONGS[choice]);
}

// Updates the music controller, playing a note if needed.
void mc_update(unsigned int elapsed_ms) {
  struct Note* note_to_play = updateSong(&mc_playing_song, elapsed_ms);
  if (note_to_play != NULL) {
    tone(BUZZER, note_to_play->frequency, getMillisecondsPerNote(mc_playing_song.song) / note_to_play->duration);
  }
}

// LIGHT CONTROL

unsigned int lc_elapsed_ms;
unsigned char lc_chosen_time_function = 0;

void lc_cycle_function() {
  lc_chosen_time_function++;
  if (lc_chosen_time_function >= LC_TIME_FUNCTION_COUNT) {
    lc_chosen_time_function = 0;
  }
}

void lc_set_led(float r, float g, float b) {
  analogWrite(LED_RED, r * 255);
  analogWrite(LED_GREEN, g * 255);
  analogWrite(LED_BLUE, b * 255);
}

void lc_set_color(Color c) {
  lc_set_led(c.r, c.g, c.b);
}

void lc_update(unsigned int elapsed_ms) {
  lc_elapsed_ms += elapsed_ms;
  if (lc_elapsed_ms > LC_PERIOD) lc_elapsed_ms -= LC_PERIOD;
  if (mc_playing_song.playing) {
    float i = getInterpolatedRelativeIntensity(&mc_playing_song);
    //Serial.println(i);
    lc_set_color({i, i, i});
  } else {
    lc_set_color(LC_TIME_FUNCTIONS[lc_chosen_time_function](lc_elapsed_ms));
  }
}

// INPUT MANAGEMENT

#define IM_CLICK_DELAY 500

bool im_button_pressed = false;
unsigned long im_ms_since_press = 10000;
unsigned char im_press_count = 0;

void im_update(unsigned int elapsed_ms) {
  im_ms_since_press += elapsed_ms;
  if (im_ms_since_press > 1000000000) im_ms_since_press = 10000; // Reset counter to avoid overflow.
  int state = digitalRead(BUTTON_IN);
  
  if (!im_button_pressed && state == HIGH) {
    im_button_pressed = true;
  }

  if (im_button_pressed && state == LOW) {
    im_button_pressed = false;
    im_press_count++;
    im_ms_since_press = 0;
  }

  if (im_ms_since_press > IM_CLICK_DELAY) {
    if (im_press_count == 1) {
      mc_play_song();
    } else if (im_press_count == 2) {
      lc_cycle_function();
    } else if (im_press_count == 3) {
      mc_play_song();
      lc_cycle_function();
    }
    im_press_count = 0;
  }
}

void setup() {
  randomSeed(analogRead(0)); // Read analog noise to inialize random numbers.
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_IN, INPUT);
  //playSong(&mc_playing_song, SONG_TEST);
  pinMode(13, OUTPUT);
  digitalWrite(12, LOW);

  //Serial.begin(9600);
}

void loop() {
  delay(STD_DELAY);
  mc_update(STD_DELAY);
  lc_update(STD_DELAY);
  im_update(STD_DELAY);
}
