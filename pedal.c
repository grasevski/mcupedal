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
  static uint16_t adc, i;
  static uint8_t x, y;
  adc += ADCH;
  if (i & 1) {
    OCR1A = pedal(&dsp, (uint8_t)(adc >> 1), x, y);
    adc = 0;
  } else if (!i) {
    DDRB |= _BV(DDB0) | _BV(DDB4);
    PORTB &= ~(_BV(PB0) | _BV(PB4));
  } else if (i & 0x200) {
    if (i == 0x200) {
      x = 0;
      y = 0;
      DDRB &= ~(_BV(DDB0) | _BV(DDB4));
    }
    const uint8_t portb = PORTB, v = UINT8_MAX - (uint8_t)(i >> 1);
    if (!x && bit_is_set(portb, PB0)) x = v;
    if (!y && bit_is_set(portb, PB4)) y = v;
  }
  i = (i + 1) & 0x3ff;
}

int main() {
  PRR |= _BV(PRUSI);
  TCCR0A |= _BV(WGM01);
  TCCR0B |= 3 << CS00;
  OCR0A |= 2;
  ADMUX = _BV(MUX0) | _BV(ADLAR) | _BV(REFS1);
  ADCSRB |= 3 << ADTS0;
  DIDR0 |= _BV(ADC1D);
  ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | (3 << ADPS0) | _BV(ADSC);
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
