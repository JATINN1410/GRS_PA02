# MT25027
# Makefile

CC = gcc
CFLAGS = -Wall -pthread -O2

TARGETS = MT25027_Part_A1_Server MT25027_Part_A1_Client \
          MT25027_Part_A2_Server MT25027_Part_A2_Client \
          MT25027_Part_A3_Server MT25027_Part_A3_Client

all: $(TARGETS)

MT25027_Part_A1_Server: MT25027_Part_A1_Server.c
	$(CC) $(CFLAGS) -o $@ $<

MT25027_Part_A1_Client: MT25027_Part_A1_Client.c
	$(CC) $(CFLAGS) -o $@ $<

MT25027_Part_A2_Server: MT25027_Part_A2_Server.c
	$(CC) $(CFLAGS) -o $@ $<

MT25027_Part_A2_Client: MT25027_Part_A2_Client.c
	$(CC) $(CFLAGS) -o $@ $<

MT25027_Part_A3_Server: MT25027_Part_A3_Server.c
	$(CC) $(CFLAGS) -o $@ $<

MT25027_Part_A3_Client: MT25027_Part_A3_Client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.pdf *.png MT25027_PA02.zip server_out.log client_out.log
	rm -f ../MT25027_PA02.zip
