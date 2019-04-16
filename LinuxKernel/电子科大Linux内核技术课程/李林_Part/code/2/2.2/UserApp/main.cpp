#include <iostream>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <string>

using namespace std;

int main()
{
	int fd = 0;
	int ret;

	string toSend = "hello driver";

	if((fd = open("/dev/msg_printer", O_RDWR)) < 0)
	{
		cerr << strerror(errno) << endl;	
		return -1;
	}

	ret = write(fd, toSend.c_str(), toSend.size());	
	if(ret < 0)
	{
		cerr << strerror(errno) << endl;
		close(fd);
		return -1;	
	}

	close(fd);

	return 0;
}
