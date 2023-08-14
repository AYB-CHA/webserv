import cgi
import os

form = cgi.FieldStorage()
if os.environ['REQUEST_METHOD'] == 'POST':
    name = form["name"].value
else:
    name = "World!"


string = """<html>
    <h1> Hello """ + name + """ </h1>
</html>
"""
print("Content-length:", len(string), end="\r\n")
print(end="\r\n")
print(string, end="\r\n")
