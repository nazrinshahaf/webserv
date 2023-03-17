#!/usr/bin/python

import time
from datetime import datetime
import os
import sys

import select


print(f'DIS THE CGI PAGE BOI')
print(f"\t\t____STDIN____")
if select.select([sys.stdin, ], [], [], 0.0)[0]:
    for something in sys.stdin:
        print(something)
print(f"\t\t_____________")
print(f"PATH_INFO: {os.getenv('PATH_INFO')}")
print(f"QUERY_STRING: {os.getenv('QUERY_STRING')}")