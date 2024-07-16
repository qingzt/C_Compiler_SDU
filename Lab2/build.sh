g++ -c lexer.cpp -o lexer.o -flto -fno-exceptions -fno-rtti
g++ -c parser.cpp -o parser.o -flto -fno-exceptions -fno-rtti
g++ -c grammar.cpp -o grammar.o -flto -fno-exceptions -fno-rtti
g++ -c main.cpp -o main.o -flto -fno-exceptions -fno-rtti
g++ lexer.o parser.o grammar.o main.o -o Main -lm