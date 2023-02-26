need to use epoll() 

need to implement
1. Requirements:
   1. status codes
   2. request methods: GET POST DELETE
   3. CGI (i.e executing python/php) (fork use only allowed for CGI)
   4. listen to multiple ports
   5. request should have 60 seconds timeout
   6. test cases
      - using ngninx as truth source
      - stress test
   7. must be able to serve a fully static website
   8. Implement FTP
   9. Non blocking file descriptors:
      - `read()` by default blocks a file descriptor before it can at least read 1 byte, this is called the 'blocking' mode
      - Alternatively, individual file descriptors can be switched to “non-blocking” mode, which means that a `read()` on a slow file will return immediately, even if no bytes are available.
      -  However, we are not allowed to do that as it would assume a lot of system resource as compared to using `poll()`
2. Allowed to use fcntl only as `fcntl(fd, F_SETFL, O_NONBLOCK);` , must use file descriptors in non-blocking mode in order to get a behaviour similar to the one of the other Unix OSes

3.  `poll()` and `select()`
    1.  Is it the case of, use one instead of the other? source https://www.ibm.com/docs/en/i/7.1?topic=designs-using-poll-instead-select
4.  `bind()`
    1.  Bind() function in socket programming is used to associate the socket with local address i.e. IP Address, port and address family.
    2.  If we have 3 servers A, B and C on the same machine. And a client wants to connect to server B. what is the identification of server B? So, to provide an identification to a server, `bind` associates sockets with IP address and port.  Now, if we bind socket with address e.g. (10.17.18.19, 6500) for server B, a client can connect to this particular server specifying the same IP address and port.