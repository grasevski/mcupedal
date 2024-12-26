.PHONY: flash build clean

CFLAGS = -O3 -Wall -Werror -Wextra

build: mcupedal mcupedal.hex

flash: mcupedal.hex
	avrdude -c usbasp -p t85 -U lfuse:w:0xE2:m -U flash:w:$<

clean:
	rm mcupedal mcupedal.hex mcupedal.elf

mcupedal.hex: mcupedal.elf
	avr-objcopy -O ihex $< $@

mcupedal.elf: mcupedal.c
	clang-format -Werror --dry-run --style Google $<
	avr-gcc -mmcu=attiny85 $(CFLAGS) $< -o $@

mcupedal: mcupedal.c
	gcc $(CFLAGS) $< -o $@
