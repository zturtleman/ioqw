#!/bin/sh
echo "Edit this script to change the path to Quake Wars' dedicated server executable and which binary if you aren't on x86_64.\nSet the sv_dlURL setting to a url like http://yoursite.com/quakewars_path for clients to download extra data.\n"
~/Quake Wars/quakewars-server.x86_64 +set dedicated 2 +set sv_allowDownload 1 +set sv_dlURL "" "$@"
