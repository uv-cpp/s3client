#Todo

## Implement versionId support

Actions to update:

### PutBucketVersioning
Enable versioning

### ListObjectVersions
Returns metadata about all versions of the objects in a bucket. You can also use request parameters as selection criteria to return metadata about a subset of all the object...

### DeleteObjectTagging
Removes the entire tag set from the specified object. For more information about managing object tags, see Object Tagging .

### DeleteObject
Removes the null version (if there is one) of an object and inserts a delete marker, which becomes the latest version of the object. If there isn't a null version, Amazon S3 does...

### GetObjectTagging
Returns the tag-set of an object. You send the GET request against the tagging subresource associated with the object.

### PutObjectTagging
Sets the supplied tag-set to an object that already exists in a bucket.

### UploadPartCopy
Uploads a part by copying data from an existing object as data source. You specify the data source by adding the request header x-amz-copy-source in your request and a byte range...

### ObjectIdentifier
Object Identifier is unique value to identify objects.

### GetObjectAttributes
Retrieves all the metadata from an object without returning the object itself. This action is useful if you're interested only in an object's metadata. To use GetObjectAttributes...

### PutObjectRetention
Places an Object Retention configuration on an object. For more information, see Locking Objects . Users or accounts require the s3:PutObjectRetention permission in order to place...

### GetObject
Retrieves objects from Amazon S3. To use GET , you must have READ access to the object. If you grant READ access to the anonymous user, you can return the object without using an...



