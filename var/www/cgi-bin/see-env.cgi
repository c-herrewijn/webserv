#!/usr/bin/python3
from os import environ
import sys

env_list_str = "".join(["<li>{0}: {1}</li>".format(name, value) for name, value in environ.items() if value])
html_content = f'''<html><body>
                <h1>meta properties of your request and server: </h1>
                <ul>{env_list_str}</ul>
                <a href="/">go home</a>
                </body></html>'''

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
#     print("{0}: {1}".format(name, value), file=sys.stderr)

print("Status: 200 OK")
print("Content-type: text/html")
print(f"Content-Length: {len(html_content)}")
print(f"Server: {environ['SERVER_NAME']}")
print("") # blank line separating headers and html content
print(html_content)
