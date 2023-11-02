#!/usr/bin/env python

import cgi
import cgitb

# Enable CGI script error handling
cgitb.enable()

# Get the uploaded file
form = cgi.FieldStorage()

# print the form to stderr


file_item = form['file']

if file_item:
	# Set the destination path for storing the uploaded file
	upload_path = '/home/astro/Desktop/vasper/data/cgi/uploads/'  # Replace with your desired path

	# Save the uploaded file
	with open(upload_path + file_item.filename, 'wb') as f:
		f.write(file_item.file.read())

	print("Content-Type: text/html\n")
	print("File uploaded successfully.")
else:
	print("Content-Type: text/html\n")
	print("No file uploaded.")

