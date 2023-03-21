import time
import requests
import datetime
import concurrent.futures

HOST = 'http://localhost:80'
API_PATH = '/'
ENDPOINT = HOST + API_PATH
MAX_THREADS = 1000
CONCURRENT_THREADS = 1000

def send_api_request():
    print ('Sending API request: ', ENDPOINT)
    r = requests.get(ENDPOINT)
    print ('Received: ', r.status_code)

start_time = datetime.datetime.now()
print ('Starting:', start_time)

with concurrent.futures.ThreadPoolExecutor(MAX_THREADS) as executor:
    futures = [ executor.submit(send_api_request) for x in range (10000) ]
end_time = datetime.datetime.now()
print ('Finished start time:', start_time, 'duration: ', end_time-start_time)