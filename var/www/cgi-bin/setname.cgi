#!/usr/bin/python3
from os import environ
import urllib.parse
import cgitb; cgitb.enable()

first_name: str = ""
last_name: str = ""

# get name from query string
query_string_dict = urllib.parse.parse_qs(environ['QUERY_STRING'])
if ('first_name' in query_string_dict):
    first_name = query_string_dict['first_name'][0]
if ('last_name' in query_string_dict):
    last_name = query_string_dict['last_name'][0]

# update cookies
if (first_name):
    cookie_header = f"Set-Cookie: first_name={first_name}; Max-Age={3600*24*365}; SameSite=Lax\r\n"
else:
    cookie_header = f"Set-Cookie: first_name=; Max-Age=0; SameSite=Lax\r\n"
if (last_name):
    cookie_header += f"Set-Cookie: last_name={last_name}; Max-Age={3600*24*365}; SameSite=Lax\r\n"
else:
    cookie_header += f"Set-Cookie: last_name=; Max-Age=0; SameSite=Lax\r\n"

html_content = f'''<html><body>
                <h1>Hello {f"{first_name} {last_name}".strip() if first_name or last_name else "anonymous user"}!</h1>
                <a href="/">go home</a>
                </body></html>'''

print("Status: 200 OK", end='\r\n')
print("Content-type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
print(f"Server: {environ['SERVER_NAME']}", end='\r\n')
print(cookie_header, end='')
print("", end='\r\n')  # mandatory empty line
print(html_content)
