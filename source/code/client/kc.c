

int main()
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in saddr = {AF_INET, htons(8888), {0}};
    inet_aton("192.168.124.42", &saddr.sin_addr);

    if(connect(cfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
        printf("error\n");
        return 0;
    }

    printf("connect succeed\n");

    for (int i = 0; i < 5; i++) {
        char sendbuf[20] = "abcdef";
        printf("len: %lu\n", strlen(sendbuf));

        int  writesz = write(cfd, sendbuf, strlen(sendbuf));
        if (writesz < 0) {
            printf("write error\n");
        }
    }

    return 0;
}