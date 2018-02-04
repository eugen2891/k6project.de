#!/usr/bin/env python

import os
import sys

def html_safe(str):
    res = str.replace("&", "&amp;")
    res = res.replace(">", "&gt;")
    res = res.replace("<", "&lt;")
    res = res.decode("utf-8")
    res = res.replace(unichr(252), "&uuml;")
    return res

def make_heading(tline):
    pos = 0
    for c in tline:
        if c == '!':
            pos += 1
    level = 5 if pos > 5 else pos
    return "<h%d>%s</h%d>" % (level, html_safe(tline[pos:]), level)

def make_paragraph(tline):
    return "<p>%s</p>" % html_safe(tline)

def process_template_line(line):
    if (line.startswith("!")):
        return make_heading
    return line

def process_template(tpl, src, dst, header, footer):
    body = []
    in_code = False
    code_fragment = None
    src_file = os.path.join(src, tpl + ".htpl")
    dst_file = os.path.join(dst, tpl + ".html")
    for line in open(os.path.join(src_file), "r"):
        tline = line.rstrip()
        if not in_code:
            if tline.startswith("!"):
                body.append(make_heading(tline))
            elif tline.startswith("<<<"):
                code_fragment = []
                in_code = True
            else:
                body.append(make_paragraph(tline))
        else:
            if tline.startswith(">>>"):
                in_code = False
                body.append("<pre>\n%s</pre>" % "\n".join(code_fragment))
            else:
                code_fragment.append(html_safe(tline))
    if not os.path.exists(dst):
        os.makedirs(dst)
    with open(dst_file, "w") as out:
        lines = [header] + body + [footer]
        out.write("\n".join(lines))

header = ""
footer = ""
src_path = os.path.join(os.getcwd(), "src")
www_path = os.path.join(os.getcwd(), "www")
if not os.path.exists(www_path):
    os.makedirs(www_path)
with open("header.htpl", "r") as data:
    header = data.read()
with open("footer.htpl", "r") as data:
    footer = data.read()
if len(sys.argv[1:]):
    dirname = os.path.dirname(sys.argv[1])
    fname = os.path.basename(sys.argv[1])
    base, ext = os.path.splitext(fname)
    if ext == ".htpl":
        rel_path = os.path.relpath(dirname, src_path)
        src_path = os.path.join(src_path, rel_path)
        dst_path = os.path.join(www_path, rel_path)
        process_template(base, src_path, dst_path, header, footer)
else:
    for root, dirs, files in os.walk(src_path):
        for fname in files:
            base, ext = os.path.splitext(fname)
            if ext == ".htpl":
                rel_path = os.path.relpath(root, src_path)
                dst_path = os.path.join(www_path, rel_path)
                process_template(base, root, dst_path, header, footer)
