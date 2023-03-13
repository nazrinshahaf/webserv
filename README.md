Need to use epoll() but MacOS only had poll()

need to implement
1. Requirements:
   1. status codes TODO: dynamically respond by checking the request object, look at request directory, incase its in a location block of the webserv config.
   2. delete
   3. HTTP redirections
   4.  
   5. CGI (i.e executing python/php) (fork use only allowed for CGI)
      1. As soon as you hit the cgi route, ignore checking the existence of the following routes, example: work/cgi.py/users?campus=42kl = access("work/cgi.py") to check if that cgi exists, then /users is PATH_INFO, and QUERRY_STRING = ?campus=42kl
      2. use chdir to change child directory to cgi directory for relative file access
      3.  QUERRY_STRING, PATH_INFO, 
      4. IF REQUEST METHOD IS POST: write to child pipe the request body
   6. request should have 60 seconds timeout
   7. test cases
      - using ngninx as truth source
      - stress test
