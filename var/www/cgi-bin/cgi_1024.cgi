#!/usr/bin/python3
from os import environ
from datetime import datetime
import sys

now = datetime.now()
now_str = now.strftime('%H:%M:%S')

longSection =  ""
for _ in range(39):
    longSection += "<p>Twenty chars!</p>"

longSection += "<p>extra.....</p>"
html_content = f'''<html><body>
                <a href="/">go home</a>
                {longSection}
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
