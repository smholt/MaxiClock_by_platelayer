// 10 leds in each 7-segment
//

void digitZero(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 30, CRGB(colour));
  fill_solid(&(leds[(40 + offset)]), 30, CRGB(colour));
}

void digitOne(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 10, CRGB(colour));
  fill_solid(&(leds[(40 + offset)]), 10, CRGB(colour));
}

void digitTwo(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 20, CRGB(colour));
  fill_solid(&(leds[(30 + offset)]), 10, CRGB(colour));
  fill_solid(&(leds[(50 + offset)]), 20, CRGB(colour));
}

void digitThree(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 20, CRGB(colour));
  fill_solid(&(leds[(30 + offset)]), 30, CRGB(colour));
}

void digitFour(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 10, CRGB(colour));
  fill_solid(&(leds[(20 + offset)]), 30, CRGB(colour));
}

void digitFive(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 50, CRGB(colour));
}

void digitSix(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 60, CRGB(colour));
}

void digitSeven(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 20, CRGB(colour));
  fill_solid(&(leds[(40 + offset)]), 10, CRGB(colour));
}

void digitEight(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 70, CRGB(colour));
}

void digitNine(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 60, CRGB(colour));
}

void digitCelcius(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 20, CRGB(colour)); 
  fill_solid(&(leds[(50 + offset)]), 20, CRGB(colour));
}

void digitFahrenheit(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 30, CRGB(colour)); 
  fill_solid(&(leds[(60 + offset)]), 10, CRGB(colour));
}

void digitGrads(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 40, CRGB(colour));
}