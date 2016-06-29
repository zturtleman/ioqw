#!/bin/sh
echo "Edit this script to change the path to Quake War's dedicated server executable and which binary if you aren't on x86_64.\n Set the sv_dlURL setting to a url like http://yoursite.com/quakewars_path for clients to download extra data"
~/quakewars/quakewars-server_x86_64 +set sv_public 1 +set sv_allowDownload 1 +set sv_dlURL "" "$@"
