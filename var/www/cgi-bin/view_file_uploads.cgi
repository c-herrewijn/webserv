#!/usr/bin/python3
import sys
from os import environ, listdir
from pathlib import Path


script_path = Path(environ['SCRIPT_FILENAME']).parent
p_var = ""

html_file_section = ""

while (script_path.__str__() != script_path.root):
    script_path = script_path.parent
    if (script_path.name == "var"):
        p_var = script_path

if (p_var != "" and (p_var.parent / 'uploads').is_dir()):
    upload_dir = (p_var.parent / 'uploads').__str__()
    file_list = listdir(path=upload_dir)
    if (len(file_list) == 0):
        html_file_section += "<p>There are no uploaded files on the server</p>"
    else:
        html_file_section += "<ul>"
        for file_name in listdir(path=upload_dir):
            html_file_section += f"<li>{file_name}</li>"
        html_file_section += "</ul>"
else:
    html_file_section += "<p>There are no uploaded files on the server</p>"


html_content = f'''<html><body>
                <h1>Uploaded files</h1>
                <a href="/">go home</a>
                {html_file_section}
                </body></html>'''

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
#     print("{0}: {1}".format(name, value), file=sys.stderr)

print("Status: 200 OK", end='\r\n')
print("Content-type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
print(f"Server: {environ['SERVER_NAME']}", end='\r\n\r\n')
print(html_content)
