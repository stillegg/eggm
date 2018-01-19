CC = gcc
#CFLAGS = -W
CFLAGS = -g -W -Wall -lrt -pthread 
TARGET = eggm
OBJECTS =  eggm_io.o eggm_time.o eggm_common.o eggm_pw.o eggm_music.o eggm_pad.o eggm_schedule.o  eggm.o
#OBJECTS = eggm_common.o eggm_time.o eggm_socket.o eggm_client.o eggm_serv.o eggm.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	      $(CC) $(CFLAGS) -o $@ $^

#$(TARGET) : socket.o eggm.o
#	$(CC) $(CFLAGS) -o $(TARGET) socket.o eggm.o

#socket.o : socket.c
#	$(CC) $(CFLAGS) -c -o socket.o socket.c

#eggm.o : eggm.c
#	$(CC) $(CFLAGS) -c -o eggm.o eggm.c

clean :
	rm -rf *.o $(TARGET) 
