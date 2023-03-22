#!/usr/bin/python

import time
from datetime import datetime
import os
import sys

import select
import time
print('<!DOCTYPE html>')
print(f'<h1>DIS THE CGI PAGE BOI</h1>')
print(f"<p>____STDIN____</p>")
save = []
if select.select([sys.stdin, ], [], [], 0.0)[0]:
    for something in sys.stdin:
        save.append(something)

if (any("WebKitFormBoundary" in string for string in save)):
    flag = 0
    for line in save:
        if (str(line).startswith('Content-Disposition: form-data; name="filename"')):
            flag = 1
        elif (flag == 1 and str(line).startswith('------WebKitFormBoundary')):
            flag = 0
        elif (flag):
            print(f"<p>{line.upper()}</p>")
else:
    for line in save:
        print(line)

print(f"<p>_____________</p>")
print(f"<p>PATH_INFO: {os.getenv('PATH_INFO')}</p>")
print(f"<p>QUERY_STRING: {os.getenv('QUERY_STRING')}</p>")
print('<ul>')
print('<li><a href="/cgi">Make a CGI request</a></li>')
print('<li><a href="/cgi/users">Make a CGI request with a path</a></li>')
print('<li><a href="/cgi/users?campus=15&is_active=true">Make a CGI request with a path and query string</a></li>')
print('<form method="post" enctype="multipart/form-data" action="/cgi">')
print('<input type="text" id="sometest" name="filename">')
print('<input type="submit" value="Upload" name="submit">')
print('</form>')
print('</ul>')
