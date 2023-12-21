#!/usr/bin/python3
from os import environ
from datetime import datetime

# static page
# with open('static_page.html', 'r') as file:
#     html_content = file.read()

now = datetime.now()
now_str = now.strftime('%H:%M:%S')
html_content = f"<html><body><h1>Current time: {now_str}</h1></body></html>"

# todo: validate headers
# if environ.has_key('SERVER_NAME'):

print("HTTP/1.0 200 OK")  # start line
print(f"Server: {environ['SERVER_NAME']}")
print(f"Content-Length: {len(html_content)}")
print("") # blank line separating headers and html content
print(html_content)
