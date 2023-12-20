#!/bin/bash

# hardcoded example CGI, with minimal http header

# serve static page (Hardcoded!)
static_page=static_page.html
html_content=$(< $static_page)
content_length=${#html_content}

# header (Hardcoded!)
echo \
"HTTP/1.0 200 OK
Server: MyServer
Content-Length: $content_length"

# line that separates header from body
echo ""

# body with html content
echo $html_content

# debug:
# >&2 echo "debug:" $SERVER_NAME
