#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE !FALSE

typedef unsigned char BOOL;

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    raw.c_oflag &= ~OPOST;
    raw.c_cflag |= (CS8);
    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() 
{
	BOOL running = TRUE;
    // Disable C library buffering for stdin and stdout
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    // Enable raw terminal mode
    enableRawMode();

    char c;

    running = TRUE;
    while (running) {
		printf(">");
		read(STDIN_FILENO, &c, 1);
        // Process character immediately
        printf("%c", c);
        if (c == 'q') {
			printf("\n\n\nThe quick brown dog jumps over the lazy fox!\n");
			running = FALSE;
		}
    }

    // disableRawMode() is called automatically via atexit()

    return 0;
}
