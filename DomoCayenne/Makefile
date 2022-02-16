
CC := g++
DOMODIR := ../Domotik/
COMMON_INCLUDE_DIRS := -ICayenne
CFLAGS := -Wall -Wstrict-prototypes -O2 -MMD -MP $(COMMON_INCLUDE_DIRS)
CXXFLAGS := -Wall -O2 -MMD -MP $(COMMON_INCLUDE_DIRS)

#Paths containing source files
vpath %cpp Cayenne

#COMMON_SOURCES := $(notdir Cayenne/CayenneUtils.c $(wildcard Cayenne/*.c))
#COMMON_OBJS := $(COMMON_SOURCES:.c=.o)
#NETWORK_OBJS := Network.o

#Ojbects and dependency files for examples
#SIMPLE_PUBLISH_OBJS := $(addprefix ./, $(COMMON_OBJS) simplepub.o)

#Ojbects and dependency files for tests
#TEST_CLIENT_OBJS := $(addprefix $(TEST_BUILD_DIR)/, $(COMMON_OBJS) TestClient.o)

#.PHONY: all

all: domoCayenne

domoCayenne: domoCayenne.o Cayenne/*.o accesBDD.o gestPipe.o Log.o
	$(CC) $(CXXFLAGS) domoCayenne.o Cayenne/CayenneUtils.o \
	Cayenne/MQTTSubscribeClient.o Cayenne/MQTTSerializePublish.o \
	Cayenne/MQTTPacket.o Cayenne/MQTTDeserializePublish.o \
	Cayenne/MQTTConnectClient.o Log.o \
	accesBDD.o gestPipe.o -lmysqlclient -std=c++11 \
	-lpthread -ltinyxml2 \
	-o domoCayenne
    

domoCayenne.o: domoCayenne.cpp  ../Domotik/structEch.h
	$(CC) $(CXXFLAGS) -c -o domoCayenne.o domoCayenne.cpp -std=c++11 -W
	
accesBDD.o: $(DOMODIR)accesBDD.cpp $(DOMODIR)accesBDD.h
	g++ -c $(DOMODIR)accesBDD.cpp -std=c++11 -W
	
gestPipe.o: $(DOMODIR)gestPipe.cpp $(DOMODIR)gestPipe.h $(DOMODIR)structEch.h
	g++ -c $(DOMODIR)gestPipe.cpp -std=c++11 -W

Log.o: $(DOMODIR)Log.cpp $(DOMODIR)Log.h
	g++ -c $(DOMODIR)Log.cpp -std=c++11 -W

clean:
	rm *.o
