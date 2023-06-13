# S3 Actions

The library includes high level actions such as *parallel download/upload to/from file* and 
some of the default S3 Actions listed below.

Future actions might be implemented as free functions accepting an `S3Api` 
context. It is already possible to implement all the available actions using the
current methods and functions; see `doc/docs/custom_requests.markdown`.

## Implemented (26)

* AbortMultipartUpload
* CompleteMultipartUpload
* CreateBucket
* CreateMultipartUpload
* DeleteBucket
* DeleteBucketTagging
* DeleteObject
* DeleteObjectTagging
* GetBucketAcl
* GetBucketTagging
* GetBucketVersioning
* GetObject
* GetObjectAcl
* GetObjectTagging
* HeadBucket
* HeadObject
* ListBuckets
* ListObjectVersions
* ListObjectsV2
* PutBucketAcl
* PutBucketTagging
* PutBucketVersiong
* PutObject
* PutObjectAcl
* PutObjectTagging
* UploadPart

## In progress (4)

* CopyObject
* DeleteObjects
* GetObjectAttributes
* SelectObjectContent

## All (133)

* AbortMultipartUpload
* CompleteMultipartUpload
* CopyObject
* CreateBucket
* CreateMultipartUpload
* DeleteBucket
* DeleteBucketAnalyticsConfiguration
* DeleteBucketCors
* DeleteBucketEncryption
* DeleteBucketIntelligentTieringConfiguration
* DeleteBucketInventoryConfiguration
* DeleteBucketLifecycle
* DeleteBucketMetricsConfiguration
* DeleteBucketOwnershipControls
* DeleteBucketPolicy
* DeleteBucketReplication
* DeleteBucketTagging
* DeleteBucketWebsite
* DeleteObject
* DeleteObjects
* DeleteObjectTagging
* DeletePublicAccessBlock
* GetBucketAccelerateConfiguration
* GetBucketAcl
* GetBucketAnalyticsConfiguration
* GetBucketCors
* GetBucketEncryption
* GetBucketIntelligentTieringConfiguration
* GetBucketInventoryConfiguration
* GetBucketLifecycle
* GetBucketLifecycleConfiguration
* GetBucketLocation
* GetBucketLogging
* GetBucketMetricsConfiguration
* GetBucketNotification
* GetBucketNotificationConfiguration
* GetBucketOwnershipControls
* GetBucketPolicy
* GetBucketPolicyStatus
* GetBucketReplication
* GetBucketRequestPayment
* GetBucketTagging
* GetBucketVersioning
* GetBucketWebsite
* GetObject
* GetObjectAcl
* GetObjectAttributes
* GetObjectLegalHold
* GetObjectLockConfiguration
* GetObjectRetention
* GetObjectTagging
* GetObjectTorrent
* GetPublicAccessBlock
* HeadBucket
* HeadObject
* ListBucketAnalyticsConfigurations
* ListBucketIntelligentTieringConfigurations
* ListBucketInventoryConfigurations
* ListBucketMetricsConfigurations
* ListBuckets
* ListMultipartUploads
* ListObjects
* ListObjectsV2
* ListObjectVersions
* ListParts
* PutBucketAccelerateConfiguration
* PutBucketAcl
* PutBucketAnalyticsConfiguration
* PutBucketCors
* PutBucketEncryption
* PutBucketIntelligentTieringConfiguration
* PutBucketInventoryConfiguration
* PutBucketLifecycle
* PutBucketLifecycleConfiguration
* PutBucketLogging
* PutBucketMetricsConfiguration
* PutBucketNotification
* PutBucketNotificationConfiguration
* PutBucketOwnershipControls
* PutBucketPolicy
* PutBucketReplication
* PutBucketRequestPayment
* PutBucketTagging
* PutBucketVersioning
* PutBucketWebsite
* PutObject
* PutObjectAcl
* PutObjectLegalHold
* PutObjectLockConfiguration
* PutObjectRetention
* PutObjectTagging
* PutPublicAccessBlock
* RestoreObject
* SelectObjectContent
* UploadPart
* UploadPartCopy
* WriteGetObjectResponse
