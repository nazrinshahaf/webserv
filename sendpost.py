import requests

json = {"name":"terikmaki", "age":3}
r = requests.post("http://localhost:80/cgi", data=json)
print(r.text)