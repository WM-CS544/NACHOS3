#include "syscall.h"

int
main()
{
	SpaceId newProc;
	OpenFileId input = ConsoleInput;
	OpenFileId output = ConsoleOutput;
	char prompt[2], ch, buffer[60], redirBuffer[60], *args[5];
	char arg1[60], arg2[60], arg3[60], arg4[60], arg5[60];
	int i, x, redir, redirOut, redirIn, argsNow, argsIndex;
	int comment, read, exec=1;

	prompt[0] = '-';
	prompt[1] = '-';

	while( 1 ) {
		if (exec) {
			Write(prompt, 2, output);
		}
		/*initialize variables*/
		args[0] = buffer;
		args[1] = arg2;
		args[2] = arg3;
		args[3] = arg4;
		args[4] = arg5;
		i = 0;
		redir= 0;
		redirOut = 0;
		redirIn  = 0;
		argsNow = 0;
		argsIndex = 0;
		comment = 0;
		exec = 0;
		read = Read(&ch, 1, input);
		while (i < 60 && ch != '\n' && redir < 60) {
			/*stop if done reading*/
			if (read == 0) {
				Halt();
			}
			if (!comment) {
				if (ch == '#') {
					comment = 1;
				}
				if (ch == '>') {
					redirOut = 1;
				} else if (ch == '<') {
					redirIn = 1;
				} else {
					/*have not seen any redirection yet*/
					if (!redirIn && !redirOut) {
						if (argsNow > 0) {
							if (ch != ' ') { 
								args[argsNow][argsIndex] = ch;
								argsIndex++;
							} else {
								args[argsNow][argsIndex] = '\0';
								argsNow++;
								argsIndex=0;
							}
						} else {
							if (ch != ' ') {
								buffer[i] = ch;
								i++;
							} else { /*now looking at arguments*/
								argsNow++;
							}
						}
					/*we have seen redirection*/
					} else {
						if (ch != ' ') {
							redirBuffer[redir] = ch;
							redir++;
						}
					}
				}
			}
			/*read next character*/
			read = Read(&ch, 1, input);
		}
		/*add null byte to strings*/
		if (argsIndex != 0) {
			args[argsNow][argsIndex] = '\0';
		}
		buffer[i] = '\0';
		redirBuffer[redir] = '\0';

		/*last argument in array is null*/
		if (!redirOut && !redirIn) {
			args[argsNow+1] = (char *)0;
		} else {
			args[argsNow] = (char *)0;
		}

		if( i > 0 && buffer[0] != '#') {
			exec = 1;
			newProc = Fork();
			/*child*/
			if (newProc == 0) {
				int fd;
				/*if redirection*/
				if (redir > 0) {
					if (redirIn) {
						fd = Open(redirBuffer);
						if (fd >= 0) {
							Close(0);
							Dup(fd);
							Close(fd);
						}
					} else if (redirOut) {
						Create(redirBuffer);
						fd = Open(redirBuffer);
						if (fd >= 0) {
							Close(1);
							Dup(fd);
							Close(fd);
						}
					}
				}
				/*start command*/
				Exec(buffer, args);
				/*if returned exit*/
				Exit(-1);
			/*parent*/
			} else {
				Join(newProc);
			}
		}
	}
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
