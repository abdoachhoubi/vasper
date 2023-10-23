#!/usr/bin/python3

import datetime
import cgi

print("HTTP/1.1 200 OK")
print("Content-type: text/html\r\n\r\n")
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Time</title>")
print("</head>")
print("<body>")
print(datetime.datetime.strftime(datetime.datetime.now(), "<h1>  %H:%M:%S </h1>"))
print("</body>")
print("</html>")