#include <iostream>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include "../PrintingDriver/IoCtlSupport.h"

using namespace std;

int main()
{
	int fd = 0;
	int ret;
	unsigned short cs, ds, ss;

	string toSend = "hello driver";

	if((fd = open("/dev/msg_printer", O_RDWR)) < 0)
	{
		cerr << strerror(errno) << endl;	
		return -1;
	}

	asm volatile ("mov %%cs, %0 \n\t" : "=m"(cs));
	asm volatile ("mov %%ds, %0 \n\t" : "=m"(ds));
	asm volatile ("mov %%ss, %0 \n\t" : "=m"(ss));

	cout << "cs : " << hex << cs << endl;
	cout << "ds : " << hex << ds << endl;
	cout << "ss : " << hex << ss << endl;

	ret = write(fd, toSend.c_str(), toSend.size());	
	if(ret < 0)
	{
		cerr << strerror(errno) << endl;	
		return -1;	
	}
	
	close(ret);
	
	return 0;
}
