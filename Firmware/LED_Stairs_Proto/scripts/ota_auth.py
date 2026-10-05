"""
Hands OTA_PASSWORD from include/secrets.h to the espota uploader, so the
password only lives in one gitignored place and never in platformio.ini.
"""

import os
import re

Import("env")

SECRETS_PATH = os.path.join(env.subst("$PROJECT_DIR"), "include", "secrets.h")

password = None
if os.path.exists(SECRETS_PATH):
    with open(SECRETS_PATH, encoding="utf-8") as secrets:
        match = re.search(r'OTA_PASSWORD\[\]\s*=\s*"([^"]*)"', secrets.read())
        if match:
            password = match.group(1)

if password is None:
    print("ota_auth.py: OTA_PASSWORD not found in include/secrets.h")
    env.Exit(1)

# Runs after the platform has built the espota command line, so the flag goes
# straight onto the uploader arguments it actually uses.
env.Append(UPLOADERFLAGS=["--auth=" + password])
