#!/usr/bin/python

import time
from datetime import datetime
import os
import sys

import select

print('<!DOCTYPE html>')
print(f'<h1>DIS THE OTHER CGI PAGE BOI</h1>')
print(f"<p>____STDIN____</p>")
flag = 0
if select.select([sys.stdin, ], [], [], 0.0)[0]:
    for something in sys.stdin:
        if (str(something).startswith('Content-Disposition: form-data; name="filename"')):
            flag = 1
        elif (flag == 1 and str(something).startswith('------WebKitFormBoundary')):
            flag = 0
        elif (flag):
            print(f"<p>{something} CGI2</p>")
print(f"<p>_____________</p>")
print(f"<p>PATH_INFO: {os.getenv('PATH_INFO')}</p>")
print(f"<p>QUERY_STRING: {os.getenv('QUERY_STRING')}</p>")
print('<ul>')
print('<li><a href="/cgi2">Make a CGI request</a></li>')
print('<li><a href="/cgi2/users">Make a CGI request with a path</a></li>')
print('<li><a href="/cgi2/users?campus=15&is_active=true">Make a CGI request with a path and query string</a></li>')
print('<form method="post" enctype="multipart/form-data" action="/cgi2">')
print('<input type="text" id="sometest" name="filename">')
print('<input type="submit" value="Upload" name="submit">')
print('</form>')
print('</ul>')
