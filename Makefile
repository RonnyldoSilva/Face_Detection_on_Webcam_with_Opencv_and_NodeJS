PYTHON_VERSION = 2.7
PYTHON_INCLUDE = /usr/include/python$(PYTHON_VERSION)

# location of the Boost Python include files and library

BOOST_INC = /usr/include 
BOOST_LIB = /usr/lib

# your filename here
TARGET = faceHunter/faceHunter

$(TARGET).so: $(TARGET).o
	g++ -shared -Wl,--export-dynamic $(TARGET).o -L$(BOOST_LIB) -lboost_python -L/usr/lib/python$(PYTHON_VERSION)/config -lpython$(PYTHON_VERSION) -o faceHunter.so `pkg-config --libs opencv` `pkg-config --cflags opencv`

$(TARGET).o: $(TARGET).cpp
	g++ -I$(PYTHON_INCLUDE) -I$(BOOST_INC) -fPIC -c $(TARGET).cpp
	mv faceHunter.o faceHunter

clean:
	rm faceHunter/*.o
