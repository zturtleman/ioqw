#!/bin/sh
echo "Edit this script to change the path to Quake War's dedicated server executable.\n Set the sv_dlURL setting to a url like http://yoursite.com/quakewars_path for clients to download extra data"
/Applications/Quake Wars/quakewars.app/Contents/MacOS/quakewars-server +set sv_public 1 +set sv_allowDownload 1 +set sv_dlURL "" "$@"
