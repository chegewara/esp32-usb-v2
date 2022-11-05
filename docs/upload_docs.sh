#!/bin/bash

cd docs
FOLDER='./documentation'

ftp_site=192.168.0.5
username=$1
passwd=$2
remote=/usb-lib/docs
cd ${FOLDER}
pwd

ncftp -u $username -p $passwd ftp://$ftp_site <<EOF
cd $remote
mput -r *
close
quit

