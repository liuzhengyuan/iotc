ALL:
	gcc iotc.c -laio -o iotc
	gcc pmerge_A.c -laio -o pmerge_A
	gcc pmerge_B.c -laio -o pmerge_B

clean:
	rm -rf iotc pmerge_A pmerge_B
