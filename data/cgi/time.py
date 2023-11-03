#!/usr/bin/env python

import datetime
jwvd

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Time</title>")
print("</head>")
print("<body>")
print(datetime.datetime.strftime(datetime.datetime.now(), "<h1>  %H:%M:%S </h1>"))
print("</body>")
print("</html>")
