#!/usr/bin/python3
import cgi, os
from os import environ
from datetime import datetime
import cgitb; cgitb.enable()
import sys

now = datetime.now()
now_str = now.strftime('%H:%M:%S')

# qqq = sys.stdin.read(5)
# print("Python DEBUG: 5 chars read from stdin: {0}".format(qqq), file=sys.stderr)

# ----------------------
# NEXT STEP, this line should work:
print("Python DEBUG line", file=sys.stderr)
form = cgi.FieldStorage()
print("form: {0}".format(form), file=sys.stderr)
# ----------------------

message = "tmp!!"
# fileitem = form['filename']

# # Test if the file was uploaded
# if fileitem.filename:
#     # strip leading path from file name to avoid
#     # directory traversal attacks
#     fn = os.path.basename(fileitem.filename)
#     # print("filename: {0}".format(fn), file=sys.stderr)
#     open('/tmp/' + fn, 'wb').write(fileitem.file.read())
#     message = 'The file "' + fn + '" was uploaded successfully'
# else:
#     message = 'No file was uploaded'


html_content = f'''<html><body>
                <h1>This is the message:</h1>
                <p>{message}</p>
                <h1>This is the form:</h1>
                <p>{form}</p>
                <p>Current time: {now_str}</p>
                <a href="/">go home</a>
                </body></html>'''

# todo: validate headers
# if environ.has_key('SERVER_NAME'):

print('Python debug line', file=sys.stderr)
for name, value in environ.items():
    print("{0}: {1}".format(name, value), file=sys.stderr)

print("HTTP/1.0 200 OK")  # start line
print(f"Server: {environ['SERVER_NAME']}")
print(f"Content-Length: {len(html_content)}")
print("") # blank line separating headers and html content
print(html_content)

# cgi.print_environ()
