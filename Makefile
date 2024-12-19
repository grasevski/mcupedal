.PHONY: flash build clean

CFLAGS = -O3 -Wall -Werror -Wextra

build: pedal pedal.hex

flash: pedal.hex
	avrdude -c usbasp -p t85 -U lfuse:w:0xE2:m -U flash:w:$<

clean:
	rm pedal pedal.hex pedal.elf

pedal.hex: pedal.elf
	avr-objcopy -O ihex $< $@

pedal.elf: pedal.c
	clang-format -Werror --dry-run --style Google $<
	avr-gcc -mmcu=attiny85 $(CFLAGS) $< -o $@

pedal: pedal.c
	gcc $(CFLAGS) $< -o $@
