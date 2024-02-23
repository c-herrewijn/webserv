#!/usr/bin/python3
import cgi, os
from os import environ
from datetime import datetime
import cgitb; cgitb.enable()
import sys

now = datetime.now()
now_str = now.strftime('%H:%M:%S')

form = cgi.FieldStorage()

message = ""
fileitem = form['filename']

# Test if the file was uploaded
if fileitem.filename:
    # strip leading path from file name to avoid directory traversal attacks
    file_name = os.path.basename(fileitem.filename)
    upload_dir = os.path.join(os.getcwd(), "uploads")
    file_path = os.path.join(upload_dir, file_name)
    if os.path.exists(file_path):
        message = 'Not uploaded: file with same name already present on server!'
    else:
        if not os.path.exists(upload_dir):
            os.makedirs(upload_dir)
        open(file_path, 'wb').write(fileitem.file.read())
        message = 'The file "' + file_name + '" was uploaded successfully'
else:
    message = 'No file found in request!'

html_content = f'''<html><body>
                <h1>File upload</h1>
                <p>{message}:</p>
                <p>Current time: {now_str}</p>
                <a href="/">go home</a>
                </body></html>'''

# todo: validate headers
# if environ.has_key('SERVER_NAME'):

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
# #     print("{0}: {1}".format(name, value), file=sys.stderr)

# print("HTTP/1.0 200 OK")  # start line
# print(f"Server: {environ['SERVER_NAME']}")
print("Status: 201 Created", end='\r\n')
print("Content-type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
print(f"Server: {environ['SERVER_NAME']}", end='\r\n\r\n')
print(html_content)

# cgi.print_environ()
