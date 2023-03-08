all:		c_pcb c_pcb_dsn c_pcb_view

c_pcb:		c_pcb.cpp mymath.cpp mymath.h router.cpp router.h layer.cpp layer.h io.h
			g++ -g --std=c++14 c_pcb.cpp router.cpp layer.cpp mymath.cpp -o c_pcb

c_pcb_dsn:	c_pcb_dsn.cpp router.h mymath.h dsnparser.h dsnparser.cpp
			g++ -g --std=c++14 c_pcb_dsn.cpp  dsnparser.cpp  -pthread -o c_pcb_dsn 

#c_pcb_view:	c_pcb_view.cpp mymath.cpp mymath.h io.h
#			g++ -O2 --std=c++14 `pkg-config --cflags glfw3` c_pcb_view.cpp mymath.cpp -oc_pcb_view `pkg-config --static --libs glfw3` -framework OpenGL -D GL_SILENCE_DEPRECATION

c_pcb_view:	c_pcb_view.cpp mymath.cpp mymath.h io.h
			g++ -g c_pcb_view.cpp mymath.cpp --std=c++14 -lglut -lGL -lglfw3 -lpthread -lm -ldl -lGLEW   -o c_pcb_view

clean:
			rm c_pcb c_pcb_dsn c_pcb_view
