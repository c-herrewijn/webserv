#!/usr/bin/python3
from os import environ
from datetime import datetime
import sys

now = datetime.now()
now_str = now.strftime('%H:%M:%S')

html_content = f'''<html><body>
                <h1>Current time: {now_str}</h1>
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
