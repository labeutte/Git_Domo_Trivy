all: main.o sim.o serieLin.o accesBDD.o rcswitch.o monitFTP.o \
internet.o compress.o gestPipe.o alarme433.o Log.o
	g++ -o appli main.o sim.o serieLin.o monitFTP.o accesBDD.o internet.o \
	rcswitch.o compress.o gestPipe.o alarme433.o Log.o \
	-lwiringPi -lrt -lcrypt -lpthread -lmysqlclient -std=c++11 \
	-lopencv_highgui -lopencv_core -lopencv_objdetect -lopencv_imgcodecs \
	-lopencv_imgproc -lopencv_video -lopencv_features2d -lopencv_calib3d \
	-lmosquitto -lcurl -ltinyxml2 \
	-L/home/pi/Downloads/wiringPi/wiringPi -D_REENTRANT

Log.o: Log.cpp Log.h
	g++ -c Log.cpp -std=c++11 -W
		
serieLin.o: serieLin.cpp serieLin.h
	g++ -o serieLin.o -c serieLin.cpp  -std=c++11 -W

monitFTP.o: monitFTP.cpp monitFTP.h  structEch.h
	g++ -c monitFTP.cpp -std=c++11 -Wall

rcswitch.o: RCSwitch.cpp RCSwitch.h
	g++ -o rcswitch.o -c RCSwitch.cpp  -std=c++11

sim.o: sim.cpp sim.h
	g++ -c sim.cpp -std=c++11 -W

alarme433.o: alarme433.cpp alarme433.h
	g++ -c alarme433.cpp -std=c++11 -Wall

gestPipe.o: gestPipe.cpp gestPipe.h structEch.h
	g++ -c gestPipe.cpp -std=c++11 -W

accesBDD.o: accesBDD.cpp accesBDD.h
	g++ -c accesBDD.cpp -std=c++11 -W

internet.o: internet.cpp internet.h
	g++ -c internet.cpp -std=c++11 -W

compress.o: compress.cpp
	g++ -c compress.cpp  -std=c++11 
	
main.o: main.cpp  structEch.h
	g++ -c main.cpp  -std=c++11 -W
	
clean:
	rm *.o

