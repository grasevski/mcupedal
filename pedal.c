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

struct dsp {};

uint8_t pedal(struct dsp *dsp, uint8_t adc, uint8_t x, uint8_t y) {
  (void)dsp;
  (void)x;
  (void)y;
  return adc;
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
  ADCSRA =
      _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADSC);
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
