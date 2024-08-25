compile: driver.o scanner.o ast.o parser.o ast-print.o helper.o codegen.o mipsgen.o
	gcc -o compile driver.o scanner.o ast.o parser.o ast-print.o helper.o codegen.o mipsgen.o

driver.o: driver.c scanner.h
	gcc -c driver.c

scanner.o: scanner.c scanner.h
	gcc -c scanner.c

parser.o: parser.c scanner.h
	gcc -c parser.c

ast.o: ast.c ast.h
	gcc -c ast.c

ast-print.o: ast-print.c ast.h
	gcc -c ast-print.c

helper.o: helper.c helper.h parser.h
	gcc -c helper.c

codegen.o: codegen.c codegen.h helper.h ast.h
	gcc -c codegen.c

mipsgen.o: mipsgen.c mipsgen.h helper.h ast.h
	gcc -c mipsgen.c

clean:
	rm -f *.o compile
