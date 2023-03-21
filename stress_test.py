# import time
import requests
import datetime
import concurrent.futures
import threading

HOST = 'http://localhost:80'
API_PATH = '/'
ENDPOINT = HOST + API_PATH
MAX_THREADS = 25
CONCURRENT_THREADS = 10

count = 0
bad_count = 0
threadLock_count = threading.Lock()
threadLock_bad_count = threading.Lock()

def send_api_request():
    global count
    global bad_count

    threadLock_count.acquire()
    print('Sending API request: ', ENDPOINT)
    r = requests.get(ENDPOINT)
    try:
        count += 1
    finally:
        threadLock_count.release()
    print('Received: ', r.status_code)
    if (r.status_code != 200):
        threadLock_bad_count.acquire()
        try:
            bad_count += 1
        finally:
            threadLock_bad_count.release()
        print('Received Wrong Status Code')
        exit(1)

start_time = datetime.datetime.now()
print('Starting:', start_time)

# with concurrent.futures.ThreadPoolExecutor(MAX_THREADS) as executor:
#     futures = [ executor.submit(send_api_request) for x in range (10000) ]

for x in range(1):
    threads = [threading.Thread(target=send_api_request) for t in range(100)]

    for t in threads:
        t.start()

    for t in threads:
        t.join()

end_time = datetime.datetime.now()
print('Total Sent :', count)
print('Bad Sent :', bad_count)
print('Finished start time:', start_time, 'duration: ', end_time-start_time)
