
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
	@clang++ -Ilib/include -std=gnu++17 -c -o build/$@.o src/$@.cpp

importa-bibliotecas-i: $(bibliotecas) $(cabecalhos)
	@echo "Importado bibliotecas estáticas e seus respectivos cabeçalhos."

compila-objetos: $(objetos)
	@echo "Todos objetos foram compilados em 'build'."

compila-testes-unitarios: compila-objetos test-progresso teste-led test-monitor
	@echo "\nCompilando todos testes unitários ...\n"

	clang++ -Wall -O0 -std=gnu++17 -I$(HEADERS) -D__unit_tests__ \
		-c -o build/test-monitor.o src/monitor.cpp

# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
entrada-test:
	clang++ -D__unit_tests__ -g3 -O0 -Wall \
		-c -o build/entrada-test.o src/entrada.cpp
	clang++ -o bin/ut_entrada build/entrada-test.o -lcurses
# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
debug:
	clang++ -g3 -O0 -Wall -c -o build/entrada-debug.o src/entrada.cpp
	clang++ -D__unit_tests__ -g3 -O0 -Wall \
		-c -o build/painel-debug.o src/painel.cpp
	clang++ -g3 -O0 -Wall -D__unit_tests__ \
		-c -o build/main-debug.o src/main.cpp
	clang++ -o bin/debug		   \
		build/main-debug.o		\
		build/entrada-debug.o   \
		build/painel-debug.o	   \
			-lcurses
release:
	clang++ -O3 -Wall -c src/entrada.cpp src/painel.cpp src/main.cpp
	mv -v *.o build/
