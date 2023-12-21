#!/usr/bin/python3
from os import environ

with open('static_page.html', 'r') as file:
    html_content = file.read()

# todo: validate headers
# if environ.has_key('SERVER_NAME'):

print("HTTP/1.0 200 OK")  # start line
print(f"Server: {environ['SERVER_NAME']}")
print(f"Content-Length: {len(html_content)}")
print("") # blank line separating headers and html content
print(html_content)
