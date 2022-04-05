#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
using namespace std;
int main()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = 0;
    bind(lfd, (struct sockaddr *)&addr, sizeof(addr));

    listen(lfd, 128);

    int maxfd = lfd;
    fd_set rdset;
    fd_set rdtemp;
    FD_ZERO(&rdset);
    FD_SET(lfd, &rdset);

    while (1)
    {
        rdtemp = rdset;
        int num = select(maxfd + 1, &rdtemp, NULL, NULL, NULL);

        if (FD_ISSET(lfd, &rdtemp))
        {
            struct sockaddr_in cliaddr;
            socklen_t len = sizeof(cliaddr);
            int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);

            FD_SET(cfd, &rdset);
            maxfd = maxfd > cfd ? maxfd : cfd;
        }

        for (int i = 0; i <= maxfd; i++)
        {
            if (i != lfd && FD_ISSET(i, &rdtemp))
            {
                char buf[10] = {0};
                int len = read(i, buf, sizeof(buf));
                if (len == 0)
                {
                    cout << "client close" << endl;
                    FD_CLR(i, &rdset);
                    close(i);
                }
                else if (len > 0)
                {
                    cout << "client say:" << buf << endl;
                    write(i, buf, len);
                }
                else
                {
                    perror("read");
                }
            }
        }
    }

    return 0;
}