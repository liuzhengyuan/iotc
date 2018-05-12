ALL:
	gcc -laio iotc.c -o iotc
	gcc -laio pmerge_A.c -o pmerge_A
	gcc -laio pmerge_B.c -o pmerge_B

clean:
	rm -rf iotc pmerge_A pmerge_B
