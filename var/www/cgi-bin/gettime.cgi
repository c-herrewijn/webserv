#!/usr/bin/python3
from os import environ
from datetime import datetime
import sys

# get user name from cookie
first_name: str = ""
last_name: str = ""
if ('HTTP_COOKIE' in environ.keys() and environ['HTTP_COOKIE'] != ""):
    for cookie in environ['HTTP_COOKIE'].split(";"):
        key, value = map(str.strip, cookie.split('='))
        if (key == "first_name"):
            first_name = value
        if (key == "last_name"):
            last_name = value

# get time
now = datetime.now()
now_str = now.strftime('%H:%M:%S')

html_content = f'''<html><body>
                <h1>Hello {f"{first_name} {last_name}".strip() if first_name or last_name else "anonymous user"}!</h1>
                <h2>Current time: {now_str}</h2>
                <a href="/">go home</a>
                </body></html>'''

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
#     print("{0}: {1}".format(name, value), file=sys.stderr)

print("Status: 200 OK", end='\r\n')
print("Content-type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
print(f"Server: {environ['SERVER_NAME']}", end='\r\n\r\n')
print(html_content)
