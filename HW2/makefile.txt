#
#
#  Makefile for CS7680 HW2
#
#
#

# Edit the following line to reflect the current project number
PROJNUM = 2

HDR = fakenews.h
SRC = fakenews.c

OBJ = fakenews.o
GRD = makefile ${SRC} ${HDR}

CC=gcc
CFLAGS = -g0



# Create all files

all:		fakenews.o

fakenews.o:	fakenews.c fakenews.h
	$(CC)	$(CFLAGS) -c fakenews.c


#===================================	
# Create the den executable:
TARGET = den
SRCS = den.c fakenews.c
OBJS = $(SRCS:.c=.o)
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC)	$(CFLAGS) $(OBJS) -o $(TARGET)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
#===================================	



#
# Clean up script
#
clean:
	/bin/rm -f *.o 
	rm -f $(OBJS) $(TARGET)
