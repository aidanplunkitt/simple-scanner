hw1: hw1.cpp
	g++ hw1.cpp -o hw1

run: hw1
	./hw1 HW1-input.txt output.txt

.PHONY: edit clean

edit:
	vim hw1.cpp

clean:
	rm hw1 || true
