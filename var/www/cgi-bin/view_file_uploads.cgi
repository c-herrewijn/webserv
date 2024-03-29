#!/usr/bin/python3
import sys
from os import environ, listdir
from pathlib import Path

html_file_section = ""
fetch_script = "<script>function deleteRequest(path) {fetch(path, {method: 'DELETE'});}</script>"

p_var = ""
if ('SCRIPT_FILENAME' in environ.keys()):
    path_tmp = Path(environ['SCRIPT_FILENAME']).parent
    while (path_tmp.__str__() != path_tmp.root):
        path_tmp = path_tmp.parent
        if (path_tmp.name == "var"):
            p_var = path_tmp

    if (p_var != "" and (p_var / 'upload').is_dir()):
        upload_dir = (p_var / 'upload').__str__()
        file_list = listdir(path=upload_dir)
        if (len(file_list) == 0):
            html_file_section += "<p>There are no uploaded files on the server</p>"
        else:
            html_file_section += "\n<ul>"
            for file_name in listdir(path=upload_dir):
                delete_button = f'<input type="button" value="Delete" onclick="deleteRequest(\'/upload/{file_name}\');">'
                html_file_section += f"\n<li>{file_name} {delete_button}</li>"
            html_file_section += "\n</ul>"
    else:
        html_file_section += "<p>There are no uploaded files on the server</p>"

    html_content = f'''<html><body>
                    {fetch_script}
                    <h1>Uploaded files</h1>
                    <a href="/">go home</a>
                    {html_file_section}
                    </body></html>'''
    status = "200 OK"
else:
    html_content = f'''<html><body>
                <h1>500 - Internal Server Error</h1>
                <a href="/">go home</a>
                </body></html>'''
    status = "500 Internal Server Error"

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
#     print("{0}: {1}".format(name, value), file=sys.stderr)

print(f"Status: {status}", end='\r\n')
print("Content-type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
if ('SERVER_NAME' in environ.keys()):
    print(f"Server: {environ['SERVER_NAME']}", end='\r\n')
print("", end='\r\n')  # mandatory empty line
print(html_content)
