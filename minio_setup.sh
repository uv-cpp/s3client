#!/usr/bin/env bash
#Runs minio on localhost through podman and configures alias with
#secret and access keys
#usage: ./minio_setup.sh <alias name> <data directlry path>
#The data directory is created if it does not exist
#Usind default ports 9000 (APO) and 9090 (admin)
#Requires s3-gen-credentials to be built and 

#Access and secret keys are stored into environmnt variables

#check command line parameters
if (($# == 0))
then
  echo "Run minio with podman"
  echo "usage: $0 <minio alias name> <data path>"
  echo
  echo "Data path is created if it does not exist"
  echo "S3CLIENT_TEST_SECRET and S3CLIENT_TEST_ACCESS env variables contain generated access and secret"
  echo "An entry is added into $HOME/.mc/config.json"
  echo "Use 'podman ps' to view and 'podman kill' to kill running containers"
  echo "To remove entry from mc configuration run 'mc alias remove <alias name>'"
  echo "Access and secret are the username and password for the minio console"
  exit 0
fi
if (($# != 2))
then
  echo "Error - invalid number of arguments"
  echo "usage: $0 <minio alias name> <data path>"
  exit 2
fi
#create data directory
mkdir -p $2
if [ $? -ne 0 ]; then
  echo "Error creating directory"
  exit 2
fi
PORT=9000
ADMIN_PORT=9090
DATA_PATH=$2
ACCESS=`s3-gen-credentials access`
SECRET=`s3-gen-credentials secret`
#download and run minio server in container
podman run \
  -p $PORT:9000 \
  -p $ADMIN_PORT:9090 \
  -v "$DATA_PATH:/data" \
  -e "MINIO_ROOT_USER=$ACCESS" \
  -e "MINIO_ROOT_PASSWORD=$SECRET" \
  quay.io/minio/minio server /data --console-address ":$ADMIN_PORT" &
if [ $? -ne 0 ]; then
  echo "Error running podman"
  exit 2
fi
#wait for service to be available
while ! podman ps -f status=running | grep minio >> /dev/null;
do
  sleep 0.5s
done
#there is a delay between the time the service is up
#and the endpoint is accessible
sleep 5
URL="http://127.0.0.1:$PORT"
#create alias using default credentials set above
mc alias set $1 $URL $ACCESS $SECRET
if [ $? -ne 0 ]; then
  echo "Error running 'mc alias set'"
  exit 2
fi
echo export S3CLIENT_TEST_ACCESS=$ACCESS
echo export S3CLIENT_TEST_SECRET=$SECRET

