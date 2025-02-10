
LIB_E 		= $(CCODES)/utilitarios-em-c/bin/static
LIB_C 		= $(CCODES)/utilitarios-em-c/bin/shared
HEADERS 		= $(CCODES)/utilitarios-em-c/include
DESTINO 		= lib/

codigos 		= progresso led monitor
objetos 		= auxiliar progresso led monitor
bibliotecas = libteste libtempo liblegivel libterminal libconversao
cabecalhos	= teste tempo legivel terminal conversao


# Compila tudo que existe(processo demorado).
all: importa-bibliotecas compila-objetos \
	test-progresso test-led test-monitor

limpa-testes:
	rm -fv bin/ut*
	rm -fv bin/tests/ut*

clean: limpa-testes
	rm -frv build/ bin/

cria-dirs-do-projeto:
	@echo "Template de projeto mais comun em projetos:"
	@echo "Talvez alguns sejam desnecessários neste."
	@mkdir -p lib/include
	@mkdir -p bin/tests
	@mkdir -p src
	@mkdir -p include
	@mkdir -p build
	@mkdir -p data

# Iteração que copia/ou compila, um processo mais automatizado.
$(bibliotecas):
	@cp -vu $(LIB_E)/$@.a $(DESTINO)

$(cabecalhos):
	@cp --verbose --update $(HEADERS)/$@.h  lib/include

$(objetos):
	@mkdir -vp build/
	@echo "Compilação do objeto de '$@' em 'build'..."
	clang++ -Ilib/include -std=gnu++17 -c -o build/$@.o src/$@.cpp

importa-bibliotecas-i: $(bibliotecas) $(cabecalhos)
	@echo "Importado bibliotecas estáticas e seus respectivos cabeçalhos."

compila-objetos: $(objetos)
	@echo "Todos objetos foram compilados em 'build'."

compila-testes-unitarios: compila-objetos test-progresso teste-led test-monitor
	@echo "\nCompilando todos testes unitários ...\n"

obj-test-progresso:
	clang++ -c -std=gnu++17 -Wall -I$(HEADERS) -D__UT_PROGRESSO__ \
		-o build/test-progresso.o src/progresso.cpp

obj-test-led:
	@clang++ -std=gnu++17 -I$(HEADERS) -D__UT_LED__ \
		-c -o build/test-led.o src/led.cpp
	@echo "Compilado 'test-led.o' em 'build'."

obj-test-monitor:
	clang++ -Wall -O0 -std=gnu++17 -I$(HEADERS) -D__unit_tests__ \
		-c -o build/test-monitor.o src/monitor.cpp



test-progresso: obj-test-progresso
	@mkdir -vp bin/tests
	@clang++ -O0 -std=gnu++17 -Wall -I$(HEADERS) \
		-o bin/tests/ut_progresso build/test-progresso.o build/led.o \
		build/monitor.o build/auxiliar.o \
		-lcurses -L$(LIB_C) -lteste -ltempo -llegivel -lterminal -lconversao

test-led: obj-test-led
	@mkdir -vp bin/tests
	@clang++ -ggdb -std=gnu++17 -Wall -I$(HEADERS) \
		-o bin/tests/ut_led build/test-led.o build/auxiliar.o \
		-lcurses -L$(LIB_C) -lteste -ltempo -llegivel \
		-lterminal -lconversao
	@echo "Compilado o teste 'ut_led' em 'bin/tests'."

test-monitor: obj-test-monitor obj-entrada
	@mkdir -vp bin/tests
	@clang++ -I$(HEADERS) -o bin/tests/ut_monitor build/test-monitor.o \
		build/led.o build/progresso.o build/auxiliar.o build/entrada.o \
		-lcurses -L$(LIB_C) -lteste -ltempo -llegivel -lterminal -lconversao

# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
obj-entrada:
	clang++ -I$(HEADERS) -O3 -std=gnu++17 -Wall \
		-c -o build/entrada.o src/entrada.cpp

obj-test-entrada:
	clang++ -Wall -O0 -std=gnu++17 -I$(HEADERS) -D__unit_tests__ \
		-c -o build/test-entrada.o src/entrada.cpp

test-entrada: obj-test-entrada
	@mkdir -vp bin/tests
	@clang++ -I$(HEADERS) -o bin/tests/ut_entrada build/test-entrada.o \
		build/led.o build/progresso.o build/auxiliar.o \
		-lcurses -L$(LIB_C) -lteste -ltempo -llegivel -lterminal -lconversao
	
run-monitor:
	@./bin/tests/ut_monitor

run-progresso:
	@./bin/tests/ut_progresso

run-led:
	@./bin/tests/ut_led


