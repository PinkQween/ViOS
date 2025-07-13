#ifndef TERMIOS_H
#define TERMIOS_H

#include <sys/types.h>

// Type definitions (must come before struct definition)
typedef unsigned int tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int speed_t;

#define NCCS 32

// Terminal input flags
#define BRKINT 0000002
#define ICRNL 0000100
#define IGNBRK 0000001
#define IGNCR 0000200
#define IGNPAR 0000004
#define INLCR 0000400
#define INPCK 0000020
#define ISTRIP 0000040
#define IXANY 0004000
#define IXOFF 0010000
#define IXON 0002000
#define PARMRK 0000010

// Terminal output flags
#define OPOST 0000001
#define ONLCR 0000004
#define OCRNL 0000010
#define ONOCR 0000020
#define ONLRET 0000040
#define OFILL 0000100
#define OFDEL 0000200
#define NLDLY 0000400
#define CRDLY 0003000
#define TABDLY 0014000
#define BSDLY 0020000
#define VTDLY 0040000
#define FFDLY 0100000

// Terminal control flags
#define CBAUD 0010017
#define B0 0000000
#define B50 0000001
#define B75 0000002
#define B110 0000003
#define B134 0000004
#define B150 0000005
#define B200 0000006
#define B300 0000007
#define B600 0000010
#define B1200 0000011
#define B1800 0000012
#define B2400 0000013
#define B4800 0000014
#define B9600 0000015
#define B19200 0000016
#define B38400 0000017
#define EXTA B19200
#define EXTB B38400
#define CSIZE 0000060
#define CS5 0000000
#define CS6 0000020
#define CS7 0000040
#define CS8 0000060
#define CSTOPB 0000100
#define CREAD 0000200
#define PARENB 0000400
#define PARODD 0001000
#define HUPCL 0002000
#define CLOCAL 0004000

// Terminal local flags
#define ISIG 0000001
#define ICANON 0000002
#define XCASE 0000004
#define ECHO 0000010
#define ECHOE 0000020
#define ECHOK 0000040
#define ECHONL 0000100
#define NOFLSH 0000200
#define TOSTOP 0000400
#define ECHOCTL 0001000
#define ECHOPRT 0002000
#define ECHOKE 0004000
#define FLUSHO 0010000
#define PENDIN 0040000
#define IEXTEN 0100000

// Special characters
#define VEOF 0
#define VEOL 1
#define VEOL2 2
#define VERASE 3
#define VWERASE 4
#define VKILL 5
#define VREPRINT 6
#define VINTR 8
#define VQUIT 9
#define VSUSP 10
#define VSTART 12
#define VSTOP 13
#define VLNEXT 14
#define VDISCARD 15
#define VMIN 16
#define VTIME 17

// Terminal control operations
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2

// Standard file descriptors
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Terminal control structure
struct termios
{
    tcflag_t c_iflag; // input flags
    tcflag_t c_oflag; // output flags
    tcflag_t c_cflag; // control flags
    tcflag_t c_lflag; // local flags
    cc_t c_cc[NCCS];  // control characters
    speed_t c_ispeed; // input speed
    speed_t c_ospeed; // output speed
};



// Function declarations
int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int tcsendbreak(int fd, int duration);
int tcdrain(int fd);
int tcflush(int fd, int queue_selector);
int tcflow(int fd, int action);
void cfmakeraw(struct termios *termios_p);
speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);
int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);

#endif // TERMIOS_H