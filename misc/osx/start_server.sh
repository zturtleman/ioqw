#!/bin/sh
echo "Edit this script to change the path to Quake Wars' dedicated server executable.\nSet the sv_dlURL setting to a url like http://yoursite.com/quakewars_path for clients to download extra data.\n"
/Applications/Quake Wars/quakewars.app/Contents/MacOS/quakewars-server +set dedicated 2 +set sv_allowDownload 1 +set sv_dlURL "" "$@"
