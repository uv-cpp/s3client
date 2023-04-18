# Presign

All requests can be issued through standard web clients such as `curl` specifying a pre-signed url on
the command line.

## Exanple: Upload object

### Presign URL
Specify method, bucket name and key and expiration time of 10k seconds.

```
export PRESIGN=`build/s3-presign -a $S3CLIENT_TEST_ACCESS -s $S3CLIENT_TEST_SECRET -e $S3CLIENT_TEST_URL -m "PUT" -x 10000 -b buck -k xxx`
```
### Upload object
Invoke `curl` and specify custom metadata field.
```
curl $PRESIGN -X PUT -F "file=@CMakeCache.txt" -H "x-amz-meta-mymeta:123"
```
### Verify upload
Invoke *HEAD* operation on uploaded object and read metadata.

```
build/s3-client -a $S3CLIENT_TEST_ACCESS -s $S3CLIENT_TEST_SECRET -e $S3CLIENT_TEST_URL -b buck -k xxx -m HEAD
```

Output
```
HTTP/1.1 200 OK
Accept-Ranges: bytes
Content-Length: 16648
Content-Security-Policy: block-all-mixed-content
Content-Type: multipart/form-data; boundary=------------------------15d8c3bbfb7a3227
ETag: "fb0e1d6d755201cd9e2102b8b1733406"
Last-Modified: Mon, 17 Apr 2023 12:29:34 GMT
Server: MinIO
Strict-Transport-Security: max-age=31536000; includeSubDomains
Vary: Origin
Vary: Accept-Encoding
X-Amz-Id-2: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
X-Amz-Request-Id: 1756B8D2388779A5
X-Content-Type-Options: nosniff
X-Xss-Protection: 1; mode=block

!!!!
x-amz-meta-mymeta: 123 <===
!!!!

Date: Mon, 17 Apr 2023 12:32:03 GMT
```

