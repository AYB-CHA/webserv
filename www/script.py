import cgi

form = cgi.FieldStorage()

name = str(form.getfirst("name"))

string = """<html>
    <h1> Hello """ + name + """ </h1>
</html>
"""
print("Content-length:", len(string), end="\r\n")
print(end="\r\n")
print(string, end="\r\n")
