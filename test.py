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
print(f"_____________")
print(f"_____________")

# print('PATH_INFO=', end='')
print(os.getenv('PATH_INFO'))
print(f"_____________")
print(f"_______123456")
# print('QUERY_STRING=', end='')
# print(os.environ['QUERY_STRING'])