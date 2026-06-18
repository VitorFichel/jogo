CXX = g++
LIBS = -lfreeglut -lopengl32 -lglu32 
SRCS = *.cpp 
OUT = labirinto

all:
	$(CXX) $(SRCS) -o $(OUT) $(LIBS)

run: all
	./$(OUT)

clean:
	rm -f $(OUT).exe