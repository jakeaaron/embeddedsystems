CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =


all:	blink_led read_switch gpio_extra

blink_led:	blink_led.o
	$(CC) $(LFLAGS) -o blink_led blink_led.o

blink_led.o:	blink_led.c
	$(CC) $(CFLAGS) -c blink_led.c


read_switch:	read_switch.o
	$(CC) $(LFLAGS) -o read_switch read_switch.o

read_switch.o:	read_switch.c
	$(CC) $(CFLAGS) -c read_switch.c


gpio_extra:	gpio_extra.o
	$(CC) $(LFLAGS) -o gpio_extra gpio_extra.o

gpio_extra.o:	gpio_extra.c
	$(CC) $(CFLAGS) -c gpio_extra.c


submit:	blink_led.c read_switch.c gpio_extra.c Makefile README
	tar -czvf hw4_submit.tar.gz blink_led.c read_switch.c gpio_extra.c Makefile README

clean:	
	rm -f *~ *.o blink_led read_switch gpio_extra
