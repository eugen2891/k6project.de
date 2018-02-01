#!/usr/bin/env python

import os
import sys
import json
from ftplib import FTP

config = {}

if not os.path.exists("deploy.conf"):
    print("Missing deploy.conf file")
    sys.exit(-1)
with open("deploy.conf", "r") as conf:
    config = json.load(conf)

try:
    ftp_host = config["ftp_host"]
    ftp_user = config["ftp_user"]
except:
    print("Broken config file")
    sys.exit(-1)

try:
    print("Trying to connect to ftp://%s" % ftp_host)
    ftp = FTP(ftp_host)
    print("Connected, logging in as: %s" % ftp_user)
    ftp.login(ftp_user, "private13")
    ftp.retrlines("LIST")
    ftp.quit()
except:
    print("FTP error")
