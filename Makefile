all: myfs

myfs: main.c myfs.c
	gcc -Wall -Werror -Wextra -Wpedantic -o a.out main.c myfs.c -I.

clean:
	rm -f a.out v.disk
