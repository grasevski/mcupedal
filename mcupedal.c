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

const uint8_t DISTORTION[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,
    3,   4,   4,   4,   4,   5,   5,   5,   6,   6,   7,   7,   7,   8,   8,
    9,   10,  10,  11,  12,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
    23,  24,  25,  27,  28,  30,  32,  34,  35,  37,  39,  42,  44,  46,  49,
    51,  54,  57,  59,  62,  65,  68,  72,  75,  78,  82,  85,  89,  92,  96,
    100, 104, 108, 112, 116, 120, 124, 127, 130, 134, 138, 142, 146, 150, 154,
    158, 162, 165, 169, 172, 176, 179, 182, 186, 189, 192, 195, 197, 200, 203,
    205, 208, 210, 212, 215, 217, 219, 220, 222, 224, 226, 227, 229, 230, 231,
    233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 242, 243, 244, 244, 245,
    246, 246, 247, 247, 247, 248, 248, 249, 249, 249, 250, 250, 250, 250, 251,
    251, 251, 251, 252, 252, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253,
    253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254,
    254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
    254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
    254};

#ifdef __AVR__
ISR(ADC_vect) { OCR1A = DISTORTION[ADCH]; }

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
int main() {
  int n;
  uint8_t buf[BUFSIZ];
  while (n = read(0, buf, sizeof(buf)), n > 0) {
    for (int i = 0; i < n; ++i) buf[n] = DISTORTION[buf[n]];
    write(1, buf, n);
  }
  return EXIT_SUCCESS;
}
#endif
