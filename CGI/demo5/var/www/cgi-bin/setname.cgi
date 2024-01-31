#!/usr/bin/python3
from os import environ
import sys
import urllib.parse

query_string_dict = urllib.parse.parse_qs(environ['QUERY_STRING'])
if 'first_name' in query_string_dict and 'last_name' in query_string_dict:
    name = query_string_dict['first_name'][0] + " " + query_string_dict['last_name'][0]
elif 'first_name' in query_string_dict and 'last_name' not in query_string_dict:
    name = query_string_dict['first_name'][0]
elif 'first_name' not in query_string_dict and 'last_name' in query_string_dict:
    name = query_string_dict['last_name'][0]
else:
    name = "anonymous user"

html_content = f'''<html><body>
                <h1>Hello {name}!</h1>
                <a href="/">go home</a>
                </body></html>'''

# todo: validate headers
# if environ.has_key('SERVER_NAME'):

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
#     print("{0}: {1}".format(name, value), file=sys.stderr)

print("HTTP/1.0 200 OK")  # start line
print(f"Server: {environ['SERVER_NAME']}")
print(f"Content-Length: {len(html_content)}")
print("") # blank line separating headers and html content
print(html_content)
