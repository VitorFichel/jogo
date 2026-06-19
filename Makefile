CXX = g++
LIBS = -lfreeglut -lopengl32 -lglu32 -lpthread -lm
SRCS = *.cpp 
OUT = labirinto

all:
	$(CXX) $(SRCS) -o $(OUT) $(LIBS)

run: all
	./$(OUT)

clean:
	rm -f $(OUT).exe