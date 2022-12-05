# ADIOS 2 - S3 integration

Sample code showing how to talk to an S3 endpoint from ADIOS 2 by implementing
plugins derived from the `PluginEngineInterface` class.

The first attempt is simply to modify the provided `example/engine` code,
rewriting the following methods and functions:

* `ExampleWritePlugin::WriteVarsFromIO`
* `ExampleWritePlugin::WriteArray`
* `ExampleReadPlugin::Init`
* `ReadVariable` 

to send requests to an S3 service instead of interacting with local files.

The long term plan is do develop a plugin which:

* maps variables to objects: one variable (key) per object (value)
* stores additional per-variable information such as data types in the associated per-object meta-data
