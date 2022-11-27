#define LC_PERIOD 60000
#define LC_FREQ 1.0 / LC_PERIOD

struct Color {
	float r, g, b;
};

#define COLOR_RED {1, 0, 0}
#define COLOR_GREEN {0, 1, 0}
#define COLOR_BLUE {0, 0, 1}
#define COLOR_ORANGE {1, 0.3, 0}
#define COLOR_YELLOW {1, 1, 0}
#define COLOR_PURPLE {0.4, 0, 1}

Color lc_multiplyColor(Color c, float x) {
	return {c.r * x, c.g * x, c.b * x};
}

Color lc_interpolateColor(Color c1, Color c2, float x) {
	Color c;
	c.r = (1 - x) * c1.r + x * c2.r;
	c.g = (1 - x) * c1.g + x * c2.g;
	c.b = (1 - x) * c1.b + x * c2.b;
	return c;
}


typedef Color (*lc_timeFunction)(unsigned int);

// Simple sin function with the same period as the standard LC_PERIOD.
Color lc_tf_greenLight(unsigned int elapsed_ms) {
  double x = pow(sin(PI * LC_FREQ * 10 * elapsed_ms), 2);
  return {0, x, 0};
}

// Fast sin function.
Color lc_tf_SinFast(unsigned int elapsed_ms) {
  double x = (sin(2 * PI * LC_FREQ * 100 * elapsed_ms) + 1) / 2;
  return {0, x, 0};
}

// Helper function for color-pulsing time functions.
Color lc_colorPulsar(unsigned int elapsed_ms, Color* colors, unsigned int count) {
  const unsigned int colorPeriod = LC_PERIOD / count;
  double x = (sin(2 * PI * LC_FREQ * count * elapsed_ms - PI / 2) + 1) / 2;
  for (unsigned int i = 1; i <= count; i++) {
    if (elapsed_ms < colorPeriod * i) {
      return lc_multiplyColor(colors[i - 1], x);
    }
  }
  return lc_multiplyColor({1, 1, 1}, x);
}

// Red, Green, then Blue pulse.
Color lc_tf_rgbPulse(unsigned int elapsed_ms) {
  const Color colors[3];
  colors[0] = COLOR_RED;
  colors[1] = COLOR_GREEN;
  colors[2] = COLOR_BLUE;
  return lc_colorPulsar(elapsed_ms, colors, 3);
}

// Rainbow color function.
Color lc_tf_rainbowPulse(unsigned int elapsed_ms) {
  const Color colors[6];
  colors[0] = COLOR_RED;
  colors[1] = COLOR_ORANGE;
  colors[2] = COLOR_YELLOW;
  colors[3] = COLOR_GREEN;
  colors[4] = COLOR_BLUE;
  colors[5] = COLOR_PURPLE;
  return lc_colorPulsar(elapsed_ms, colors, 6);
}

Color lc_tf_yellowGreen(unsigned int elapsed_ms) {
  const Color colors[2];
  colors[0] = COLOR_YELLOW;
  colors[1] = COLOR_GREEN;
  return lc_colorPulsar(elapsed_ms, colors, 2);
}

// Flame
Color lc_tf_flame(unsigned int elapsed_ms) {
	float colorValue = (sin(2 * PI * LC_FREQ * 10 * elapsed_ms) + 1) / 2;
	float flameCoefficients[] = {
		0.1, 0.35, 0.04, 0.75, 0.92, 0.55, 0.4, 0.22, 0.64, 0.85
	};
	const unsigned int fc_count = 10;
	float flameIntensity = 0;
	for (int i = 0; i < fc_count; i++) {
		flameIntensity += (sin(2 * flameCoefficients[i] * elapsed_ms) + 1) / 2;
	}
	flameIntensity /= fc_count;
	Color flame = lc_interpolateColor({1.0, 0.025, 0}, {1.0, 0.1, 0}, colorValue);
	return lc_multiplyColor(flame, flameIntensity);
}

lc_timeFunction LC_TIME_FUNCTIONS[] = {
	&lc_tf_flame,
	&lc_tf_greenLight,
	&lc_tf_yellowGreen,
	&lc_tf_SinFast,
	&lc_tf_rgbPulse,
	&lc_tf_rainbowPulse
};

#define LC_TIME_FUNCTION_COUNT 6
