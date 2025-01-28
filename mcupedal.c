#ifdef __AVR__
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

const uint8_t TUBE[] = {
    5,   5,   5,   5,   5,   5,   6,   6,   6,   6,   6,   6,   7,   7,   7,
    7,   8,   8,   8,   8,   8,   9,   9,   9,   10,  10,  10,  10,  11,  11,
    11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  15,  16,  16,  17,  17,
    18,  18,  19,  19,  20,  21,  21,  22,  22,  23,  24,  24,  25,  26,  27,
    27,  28,  29,  30,  31,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
    41,  42,  43,  44,  46,  47,  48,  49,  50,  52,  53,  54,  56,  57,  58,
    60,  61,  63,  64,  66,  67,  69,  70,  72,  74,  75,  77,  79,  80,  82,
    84,  86,  87,  89,  91,  93,  95,  97,  99,  100, 102, 104, 106, 108, 110,
    112, 114, 116, 118, 120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 140,
    142, 144, 146, 148, 150, 152, 154, 156, 157, 159, 161, 163, 165, 167, 169,
    170, 172, 174, 176, 177, 179, 181, 182, 184, 186, 187, 189, 190, 192, 193,
    195, 196, 198, 199, 200, 202, 203, 204, 206, 207, 208, 209, 210, 212, 213,
    214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 225, 226, 227,
    228, 229, 229, 230, 231, 232, 232, 233, 234, 234, 235, 235, 236, 237, 237,
    238, 238, 239, 239, 240, 240, 241, 241, 241, 242, 242, 243, 243, 243, 244,
    244, 245, 245, 245, 246, 246, 246, 246, 247, 247, 247, 248, 248, 248, 248,
    248, 249, 249, 249, 249, 250, 250, 250, 250, 250, 250, 251, 251, 251, 251,
    251};

struct dsp {};

uint8_t pedal(struct dsp *dsp, uint8_t adc, uint8_t x, uint8_t y) {
  (void)dsp;
  (void)x;
  (void)y;
  return TUBE[adc];
}

#ifdef __AVR__
ISR(ADC_vect) {
  static struct dsp dsp;
  static uint16_t i;
  static uint8_t x, y, xt, yt;
  OCR1A = pedal(&dsp, ADCH, x, y);
  const uint8_t t = (uint8_t)i;
  if (bit_is_set(i, 8)) {
    const uint8_t v = UINT8_MAX - t;
    if (!xt && bit_is_set(PORTB, PB0)) xt = v;
    if (!yt && bit_is_set(PORTB, PB4)) yt = v;
  } else if (!t) {
    DDRB |= _BV(DDB0);
    DDRB |= _BV(DDB4);
    PORTB &= ~_BV(PB0);
    PORTB &= ~_BV(PB4);
    x = xt;
    y = yt;
  } else if (t == 0xff) {
    DDRB &= ~_BV(DDB0);
    DDRB &= ~_BV(DDB4);
    xt = 0;
    yt = 0;
  }
  ++i;
}

int main() {
  PRR |= _BV(PRUSI);
  TCCR0A |= _BV(WGM01);
  TCCR0B |= _BV(CS01);
  TCCR0B |= _BV(CS00);
  OCR0A |= 1;
  ADMUX = _BV(MUX0) | _BV(ADLAR) | _BV(REFS1);
  ADCSRB |= _BV(ADTS1);
  ADCSRB |= _BV(ADTS0);
  DIDR0 |= _BV(ADC1D);
  ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS1) | _BV(ADPS0);
  PLLCSR |= _BV(PLLE);
  loop_until_bit_is_set(PLLCSR, PLOCK);
  PLLCSR |= _BV(PCKE);
  DDRB |= _BV(DDB1);
  TCCR1 = _BV(PWM1A) | _BV(COM1A1) | _BV(CS10);
  sei();
  sleep_enable();
  for (;;) sleep_cpu();
}
#else
int main(int argc, const char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s X Y\n", argv[0]);
    return EXIT_FAILURE;
  }
  const uint8_t x = atoi(argv[1]), y = atoi(argv[2]);
  struct dsp dsp = {};
  int n;
  uint8_t buf[BUFSIZ];
  while (n = read(0, buf, sizeof(buf)), n > 0) {
    for (int i = 0; i < n; ++i) buf[n] = pedal(&dsp, buf[n], x, y);
    write(1, buf, n);
  }
  return EXIT_SUCCESS;
}
#endif
