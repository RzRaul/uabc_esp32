def init_html_doc(title):
    boilerplate = """<!DOCTYPE html>
    <meta charset="UTF-8">
    <title>{}</title>
    <link rel="stylesheet" href="style.css">
    <script src="index.js"></script>
    <head>
    <body>

    """.format(title)
    return boilerplate


if __name__ == '__main__':
    f = open('test.html', 'w')
    f.write(init_html_doc('test'))
    f.write('<h1>Test</h1>')
    f.write('</body>')
    f.write('</html>')
    f.close()
