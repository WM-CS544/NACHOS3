#include "syscall.h"

/* cp.c
 *
 * Copies contents of args[1] to args[2]
 * 
*/

int
main(int argc, char *args[])
{
    OpenFileId input, output;
    char c;
    int numbytes;

		/*not enough args*/
		if (argc < 3) {
			Exit(-1);
		}

		input = Open(args[1]);
		Create(args[2]);
		output = Open(args[2]);
		
		if (input == -1 || output == -1) {
			Exit(-1);
		}

    numbytes = 0;
    while (Read(&c, 1, input) == 1){
      numbytes++;
      Write(&c, 1, output);
    }

    Close(input);
		Close(output);
		Exit(1);
}

/* Print a null-terminated string "s" on open file descriptor "file". */

prints(s,file)
char *s;
OpenFileId file;

{
  int count = 0;
  char *p;

  p = s;
  while (*p++ != '\0') count++;
  Write(s, count, file);  

}


/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i, pos=0, divisor=1000000000, d, zflag=1;
  char c;
  char buffer[11];
  
  if (n < 0) {
    buffer[pos++] = '-';
    n = -n;
  }
  
  if (n == 0) {
    Write("0",1,file);
    return;
  }

  for (i=0; i<10; i++) {
    d = n / divisor; n = n % divisor;
    if (d == 0) {
      if (!zflag) buffer[pos++] =  (char) (d % 10) + '0';
    } else {
      zflag = 0;
      buffer[pos++] =  (char) (d % 10) + '0';
    }
    divisor = divisor/10;
  }
  Write(buffer,pos,file);
}
